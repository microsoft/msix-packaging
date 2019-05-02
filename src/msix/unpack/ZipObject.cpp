//
//  Copyright (C) 2017 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
// 
#include "Exceptions.hpp"
#include "StreamBase.hpp"
#include "ObjectBase.hpp"
#include "ComHelper.hpp"
#include "ZipObject.hpp"
#include "ZipFileStream.hpp"
#include "InflateStream.hpp"
#include "VectorStream.hpp"

#include <memory>
#include <string>
#include <limits>
#include <functional>
#include <algorithm>
namespace MSIX {
/* Zip File Structure
[LocalFileHeader 1]
[encryption header 1]
[file data 1]
[data descriptor 1]
.
.
[LocalFileHeader n]
[encryption header n]
[file data n]
[data descriptor n]
[archive decryption header]
[archive extra data record]
[CentralFileHeader 1]
.
.
[CentralFileHeader n]
[Zip64EndOfCentralDirectoryRecord]
[Zip64EndOfCentralDirectoryLocator]
[EndCentralDirectoryRecord]
*/
enum class ZipVersions : std::uint16_t
{
    Zip32DefaultVersion = 20,
    Zip64FormatExtension = 45,
};

// from AppNote.txt, section 4.5.2:
enum class HeaderIDs : std::uint16_t
{
    Zip64ExtendedInfo = 0x0001, // Zip64 extended information extra field
    AV                = 0x0007, // AV Info
    RESERVED_1        = 0x0008, // Reserved for extended language encoding data (PFS) (see APPENDIX D)
    OS2               = 0x0009, // OS/2
    NTFS              = 0x000a, // NTFS 
    OpenVMS           = 0x000c, // OpenVMS
    UNIX              = 0x000d, // UNIX
    RESERVED_2        = 0x000e, // Reserved for file stream and fork descriptors
    PatchDescriptor   = 0x000f, // Patch Descriptor
    UNSUPPORTED_1     = 0x0014, // PKCS#7 Store for X.509 Certificates
    UNSUPPORTED_2     = 0x0015, // X.509 Certificate ID and Signature for individual file
    UNSUPPORTED_3     = 0x0016, // X.509 Certificate ID for Central Directory
    UNSUPPORTED_4     = 0x0017, // Strong Encryption Header
    RecordManagement  = 0x0018, // Record Management Controls
    UNSUPPORTED_5     = 0x0019, // PKCS#7 Encryption Recipient Certificate List
    IBMS390           = 0x0065, // IBM S/390 (Z390), AS/400 (I400) attributes - uncompressed
    IBM_Reserved      = 0x0066, // Reserved for IBM S/390 (Z390), AS/400 (I400) attributes - compressed
    RESERVED_3        = 0x4690, // POSZIP 4690 (reserved) 
};

// from ZIP file format specification detailed in AppNote.txt
enum class Signatures : std::uint32_t
{
    LocalFileHeader         = 0x04034b50,
    DataDescriptor          = 0x08074b50,
    CentralFileHeader       = 0x02014b50,
    Zip64EndOfCD            = 0x06064b50,
    Zip64EndOfCDLocator     = 0x07064b50,
    EndOfCentralDirectory   = 0x06054b50,
};

enum class CompressionType : std::uint16_t
{
    Store = 0,
    Deflate = 8,
};

// Hat tip to the people at Facebook.  Timestamp for files in ZIP archive 
// format held constant to make pack/unpack deterministic
enum class MagicNumbers : std::uint16_t
{
    FileTime = 0x6B60,  // kudos to those know this
    FileDate = 0xA2B1,  // :)
};

enum class GeneralPurposeBitFlags : std::uint16_t
{
    UNSUPPORTED_0 = 0x0001,         // Bit 0: If set, indicates that the file is encrypted.

    Deflate_MaxCompress = 0x0002,   // Maximum compression (-exx/-ex), otherwise, normal compression (-en)
    Deflate_FastCompress = 0x0004,  // Fast (-ef), if Max+Fast then SuperFast (-es) compression

    GeneralPurposeBit = 0x0008,     // the field's crc-32 compressed and uncompressed sizes = 0 in the local header
                                    // the correct values are put in the data descriptor immediately following the
                                    // compressed data.
    EnhancedDeflate = 0x0010,
    CompressedPatchedData = 0x0020,
    UNSUPPORTED_6 = 0x0040,         // Strong encryption.
    UnUsed_7 = 0x0080,              // currently unused
    UnUsed_8 = 0x0100,              // currently unused
    UnUsed_9 = 0x0200,              // currently unused
    UnUsed_10 = 0x0400,             // currently unused

    EncodingMustUseUTF8 = 0x0800,   // Language encoding flag (EFS).  File name and comments fields MUST be encoded UTF-8

    UNSUPPORTED_12 = 0x1000,        // Reserved by PKWARE for enhanced compression
    UNSUPPORTED_13 = 0x2000,        // Set when encrypting the Central Directory
    UNSUPPORTED_14 = 0x4000,        // Reserved by PKWARE
    UNSUPPORTED_15 = 0x8000,        // Reserved by PKWARE
};

constexpr GeneralPurposeBitFlags operator &(GeneralPurposeBitFlags a, GeneralPurposeBitFlags b)
{   return static_cast<GeneralPurposeBitFlags>(static_cast<uint16_t>(a) & static_cast<uint16_t>(b));
}

constexpr GeneralPurposeBitFlags operator |(GeneralPurposeBitFlags a, GeneralPurposeBitFlags b)
{   return static_cast<GeneralPurposeBitFlags>(static_cast<uint16_t>(a) | static_cast<uint16_t>(b));
}

// if any of these are set, then fail.
constexpr static const GeneralPurposeBitFlags UnsupportedFlagsMask =
    GeneralPurposeBitFlags::UNSUPPORTED_0  |
    GeneralPurposeBitFlags::UNSUPPORTED_6  |
    GeneralPurposeBitFlags::UNSUPPORTED_12 |
    GeneralPurposeBitFlags::UNSUPPORTED_13 |
    GeneralPurposeBitFlags::UNSUPPORTED_14 |
    GeneralPurposeBitFlags::UNSUPPORTED_15;

/*  FROM APPNOTE.TXT section 4.5.3:
    If one of the size or offset fields in the Local or Central directory
    record is too small to hold the required data, a Zip64 extended information 
    record is created.  The order of the fields in the zip64 extended 
    information record is fixed, but the fields MUST only appear if the 
    corresponding Local or Central directory record field is set to 0xFFFF 
    or 0xFFFFFFFF.

    Note: all fields stored in Intel low-byte/high-byte order.
*/
//////////////////////////////////////////////////////////////////////////////////////////////
//                                  Zip64ExtendedInformation                                //
//////////////////////////////////////////////////////////////////////////////////////////////
class Zip64ExtendedInformation final : public Meta::StructuredObject<
    Meta::Field2Bytes,  // 0 - tag for the "extra" block type               2 bytes(0x0001)
    Meta::Field2Bytes,  // 1 - size of this "extra" block                   2 bytes
    Meta::Field8Bytes,  // 2 - Original uncompressed file size              8 bytes
                        // No point in validating these as it is actually 
                        // possible to have a 0-byte file... Who knew.
    Meta::Field8Bytes,  // 3 - Compressed file size                         8 bytes
                        // No point in validating these as it is actually 
                        // possible to have a 0-byte file... Who knew.    
    Meta::Field8Bytes   // 4 - Offset of local header record                8 bytes
    //Meta::Field4Bytes // 5 - number of the disk on which the file starts  4 bytes -- ITS A FAAKEE!
>
{
public:
    void Read(const ComPtr<IStream>& stream)
    {
        StreamBase::Read(stream, &Field<0>().value);
        Meta::ExactValueValidation<std::uint32_t>(Field<0>().value, static_cast<std::uint32_t>(HeaderIDs::Zip64ExtendedInfo));

        StreamBase::Read(stream, &Field<1>().value);
        Meta::OnlyEitherValueValidation<std::uint32_t>(Field<1>().value, 24, 28);
        
        StreamBase::Read(stream, &Field<2>().value);

        StreamBase::Read(stream, &Field<3>().value);

        StreamBase::Read(stream, &Field<4>().value);
        ThrowErrorIfNot(Error::ZipBadExtendedData, Field<4>().value < m_start.QuadPart, "invalid relative header offset");
    }

    Zip64ExtendedInformation(ULARGE_INTEGER start) : m_start(start) {}
 
    std::uint64_t GetUncompressedSize()         noexcept { return Field<2>().value; }
    void SetUncompressedSize(std::uint64_t v)   noexcept { Field<2>().value = v; }
    std::uint64_t GetCompressedSize()           noexcept { return Field<3>().value; }
    void SetCompressedSize(std::uint64_t v)     noexcept { Field<3>().value = v; }
    std::uint64_t GetRelativeOffset()           noexcept { return Field<4>().value; }
    void SetRelativeOffset(std::uint64_t v)     noexcept { Field<4>().value = v; }

private:
    ULARGE_INTEGER  m_start;
};

/*  TODO: Implement large file support.
This type currently represents a "zip32" Central Directory Header.  We need to create a new field type (offset)
to replace the type for fields 8 & 9 whose implementation is determined by the version needed to extract (field 2)'s
value.

This type would replace its existing compressed & uncompressed sizes properties (encapsulated in fields 8 & 9)
with a 64-bit value version of those methods:
std::uint64_t GetCompressedSize()   { ...
std::uint64_t GetUncompressedSize() { ...
void SetCompressedSize  (std::uint64_t...
void SetUncompressedSize(std::uint64_t...

The underlying implementation of these methods would validate the resulting in/out values and pass the correct
static_casted value to the new meta object type (offset) which would then hold the correct value, as well
as handle the correct sizes w.r.t. (de)serialization.

As-is I don't believe that we "need" this for now, so keeping the implementation "simpler" is probably the correct
answer for now.
*/
//////////////////////////////////////////////////////////////////////////////////////////////
//                              CentralDirectoryFileHeader                                  //
//////////////////////////////////////////////////////////////////////////////////////////////
class CentralDirectoryFileHeader final : public Meta::StructuredObject<
    Meta::Field4Bytes, // 0 - central file header signature   4 bytes(0x02014b50)
    Meta::Field2Bytes, // 1 - version made by                 2 bytes
    Meta::Field2Bytes, // 2 - version needed to extract       2 bytes
    Meta::Field2Bytes, // 3 - general purpose bit flag        2 bytes
    Meta::Field2Bytes, // 4 - compression method              2 bytes
    Meta::Field2Bytes, // 5 - last mod file time              2 bytes
    Meta::Field2Bytes, // 6 - last mod file date              2 bytes
    Meta::Field4Bytes, // 7 - crc - 32                        4 bytes
    Meta::Field4Bytes, // 8 - compressed size                 4 bytes
    Meta::Field4Bytes, // 9 - uncompressed size               4 bytes
    Meta::Field2Bytes, //10 - file name length                2 bytes
    Meta::Field2Bytes, //11 - extra field length              2 bytes
    Meta::Field2Bytes, //12 - file comment length             2 bytes
    Meta::Field2Bytes, //13 - disk number start               2 bytes
    Meta::Field2Bytes, //14 - internal file attributes        2 bytes
    Meta::Field4Bytes, //15 - external file attributes        4 bytes
    Meta::Field4Bytes, //16 - relative offset of local header 4 bytes
    Meta::FieldNBytes, //17 - file name(variable size)
    Meta::FieldNBytes, //18 - extra field(variable size)
    Meta::FieldNBytes  //19 - file comment(variable size)
    >
{
public:
    void Read(const ComPtr<IStream>& stream)
    {
        StreamBase::Read(stream, &Field<0>().value);
        Meta::ExactValueValidation<std::uint32_t>(Field<0>().value, static_cast<std::uint32_t>(Signatures::CentralFileHeader));

        StreamBase::Read(stream, &Field<1>().value);

        StreamBase::Read(stream, &Field<2>().value);

        StreamBase::Read(stream, &Field<3>().value);
        ThrowErrorIfNot(Error::ZipCentralDirectoryHeader,
            0 == (Field<3>().value & static_cast<std::uint16_t>(UnsupportedFlagsMask)),
            "unsupported flag(s) specified");

        StreamBase::Read(stream, &Field<4>().value);
        Meta::OnlyEitherValueValidation<std::uint16_t>(Field<4>().value,  static_cast<std::uint16_t>(CompressionType::Deflate),
            static_cast<std::uint16_t>(CompressionType::Store));

        StreamBase::Read(stream, &Field<5>().value);
        StreamBase::Read(stream, &Field<6>().value);
        StreamBase::Read(stream, &Field<7>().value);
        StreamBase::Read(stream, &Field<8>().value);
        StreamBase::Read(stream, &Field<9>().value);

        StreamBase::Read(stream, &Field<10>().value);
        ThrowErrorIfNot(Error::ZipCentralDirectoryHeader, (Field<10>().value != 0), "unsupported file name size");
        if (Field<10>().value !=0) {Field<17>().value.resize(Field<10>().value, 0); }

        StreamBase::Read(stream, &Field<11>().value);
        if (Field<11>().value != 0) { Field<18>().value.resize(Field<11>().value, 0); }

        StreamBase::Read(stream, &Field<12>().value);
        Meta::ExactValueValidation<std::uint32_t>(Field<12>().value, 0);

        StreamBase::Read(stream, &Field<13>().value);
        Meta::ExactValueValidation<std::uint32_t>(Field<13>().value, 0);

        StreamBase::Read(stream, &Field<14>().value);

        StreamBase::Read(stream, &Field<15>().value);
        StreamBase::Read(stream, &Field<16>().value);
        ULARGE_INTEGER pos = {0};
        ThrowHrIfFailed(stream->Seek({0}, StreamBase::Reference::CURRENT, &pos));
        if (!GetIsZip64())
        {
            ThrowErrorIf(Error::ZipCentralDirectoryHeader, (Field<16>().value >= pos.QuadPart), "invalid relative header offset");
        }
        else
        {
            ThrowErrorIf(Error::ZipCentralDirectoryHeader, (Field<16>().value != 0xFFFFFFFF), "invalid zip64 local header offset");
        }

        if (Field<17>().Size())
        {
            ThrowHrIfFailed(stream->Read(reinterpret_cast<void*>(Field<17>().value.data()), static_cast<ULONG>(Field<17>().Size()), nullptr));
        }

        if (Field<18>().Size())
        {
            ThrowHrIfFailed(stream->Read(reinterpret_cast<void*>(Field<18>().value.data()), static_cast<ULONG>(Field<18>().Size()), nullptr));
        }
        // Only process for Zip64ExtendedInformation
        if (Field<18>().Size() > 2 && Field<18>().value[0] == 0x01 && Field<18>().value[1] == 0x00)
        {
            LARGE_INTEGER zero = {0};
            ThrowHrIfFailed(stream->Seek(zero, StreamBase::Reference::CURRENT, &pos));
            auto vectorStream = ComPtr<IStream>::Make<VectorStream>(&Field<18>().value);
            m_extendedInfo = std::make_unique<Zip64ExtendedInformation>(pos);
            ThrowErrorIfNot(Error::ZipCentralDirectoryHeader, (Field<18>().Size() >= m_extendedInfo->Size()), "Unexpected extended info size");
            m_extendedInfo->Read(vectorStream.Get());
        }

        if (Field<19>().Size())
        {
            ThrowHrIfFailed(stream->Read(reinterpret_cast<void*>(Field<19>().value.data()), static_cast<ULONG>(Field<19>().Size()), nullptr));
        }
    }

    CentralDirectoryFileHeader(bool isZip64) : m_isZip64(isZip64)
    {
        SetSignature(static_cast<std::uint32_t>(Signatures::CentralFileHeader));
        SetVersionMadeBy(static_cast<std::uint16_t>(ZipVersions::Zip64FormatExtension));
        SetVersionNeededToExtract(static_cast<std::uint16_t>(ZipVersions::Zip32DefaultVersion));  // only set to Zip64FormatExtension iff required!
        SetLastModFileDate(static_cast<std::uint16_t>(MagicNumbers::FileDate));
        SetLastModFileTime(static_cast<std::uint16_t>(MagicNumbers::FileTime));
        SetExtraFieldLength(0);
        SetFileCommentLength(0);
        SetDiskNumberStart(0);
        SetInternalFileAttributes(0);
        SetExternalFileAttributes(0);
    }

    bool IsGeneralPurposeBitSet() noexcept
    {   return ((GetGeneralPurposeBitFlags() & GeneralPurposeBitFlags::GeneralPurposeBit) == GeneralPurposeBitFlags::GeneralPurposeBit);
    }

    std::uint16_t GetVersionNeededToExtract()           noexcept { return Field<2>().value; }
    GeneralPurposeBitFlags GetGeneralPurposeBitFlags()  noexcept { return static_cast<GeneralPurposeBitFlags>(Field<3>().value); }
    void SetGeneralPurposeBitFlags(std::uint16_t value) noexcept { Field<3>().value = value; }
    std::uint16_t GetCompressionMethod()                noexcept { return Field<4>().value; }
    void SetCompressionMethod(std::uint16_t value)      noexcept { Field<4>().value= value; }
    std::uint32_t GetCrc32()                            noexcept { return Field<7>().value; }
    void SetCrc(std::uint32_t value)                    noexcept { Field<7>().value = value; }

    std::uint64_t GetCompressedSize() noexcept
    {
        if (!m_extendedInfo.get()) {
            return static_cast<std::uint64_t>(Field<8>().value);
        }
        return m_extendedInfo->GetCompressedSize();
    }

    // TODO: on-demand create m_extendedInfo?
    void SetCompressedSize(std::uint32_t value) noexcept { Field<8>().value = value; }

    std::uint64_t GetUncompressedSize() noexcept
    {
        if (!m_extendedInfo.get()) {
            return static_cast<std::uint64_t>(Field<9>().value);
        }
        return m_extendedInfo->GetUncompressedSize();
    }

    // TODO: on-demand create m_extendedInfo?
    void SetUncompressedSize(std::uint32_t value) noexcept { Field<9>().value = value; }

    std::uint64_t GetRelativeOffsetOfLocalHeader() noexcept
    {
        if (!m_extendedInfo.get()) {
            return static_cast<std::uint64_t>(Field<16>().value);
        }
        return m_extendedInfo->GetRelativeOffset();
    }

    // TODO: on-demand create m_extendedInfo?
    void SetRelativeOffsetOfLocalHeader(std::uint32_t value) noexcept { Field<16>().value = value; }

    std::string GetFileName()
    {
        auto data = Field<17>().value;
        return std::string(data.begin(), data.end());
    }

    void SetFileName(std::string name)
    {
        auto data = Field<17>().value;
        data.resize(name.size());
        data.assign(name.begin(), name.end());
        SetFileNameLength(static_cast<std::uint16_t>(name.size()));
    }

    inline bool GetIsZip64() noexcept { return m_isZip64; }

private:
    void SetSignature(std::uint32_t value)              noexcept { Field<0>().value = value; }
    void SetVersionMadeBy(std::uint16_t value)          noexcept { Field<1>().value = value; }
    void SetVersionNeededToExtract(std::uint16_t value) noexcept { Field<2>().value = value; }
    void SetLastModFileTime(std::uint16_t value)        noexcept { Field<5>().value = value; }
    void SetLastModFileDate(std::uint16_t value)        noexcept { Field<6>().value = value; }
    void SetFileNameLength(std::uint16_t value)         noexcept { Field<10>().value = value; }
    void SetExtraFieldLength(std::uint16_t value)       noexcept { Field<11>().value = value; }
    std::uint16_t GetExtraFieldLength()                 noexcept { return Field<11>().value;  }
    void SetFileCommentLength(std::uint16_t value)      noexcept { Field<12>().value = value; }
    void SetDiskNumberStart(std::uint16_t value)        noexcept { Field<13>().value = value; }
    void SetInternalFileAttributes(std::uint16_t value) noexcept { Field<14>().value = value; }
    void SetExternalFileAttributes(std::uint16_t value) noexcept { Field<15>().value = value; }

    std::unique_ptr<Zip64ExtendedInformation> m_extendedInfo;
    bool m_isZip64 = false;
};//class CentralDirectoryFileHeader

//////////////////////////////////////////////////////////////////////////////////////////////
//                                  LocalFileHeader                                         //
//////////////////////////////////////////////////////////////////////////////////////////////
class LocalFileHeader final : public Meta::StructuredObject< 
    Meta::Field4Bytes,  // 0 - local file header signature     4 bytes(0x04034b50)
    Meta::Field2Bytes,  // 1 - version needed to extract       2 bytes
    Meta::Field2Bytes,  // 2 - general purpose bit flag        2 bytes
    Meta::Field2Bytes,  // 3 - compression method              2 bytes
    Meta::Field2Bytes,  // 4 - last mod file time              2 bytes
    Meta::Field2Bytes,  // 5 - last mod file date              2 bytes
    Meta::Field4Bytes,  // 6 - crc - 32                        4 bytes
    Meta::Field4Bytes,  // 7 - compressed size                 4 bytes
    Meta::Field4Bytes,  // 8 - uncompressed size               4 bytes
    Meta::Field2Bytes,  // 9 - file name length                2 bytes
    Meta::Field2Bytes,  // 10- extra field length              2 bytes
    Meta::FieldNBytes,  // 11- file name                       (variable size)
    Meta::FieldNBytes   // 12- extra field                     (variable size)
>
{
public:
    void Read(const ComPtr<IStream> &stream)
    {
        StreamBase::Read(stream, &Field<0>().value);
        Meta::ExactValueValidation<std::uint32_t>( Field<0>().value, static_cast<std::uint32_t>(Signatures::LocalFileHeader));

        StreamBase::Read(stream, &Field<1>().value);
        Meta::OnlyEitherValueValidation<std::uint16_t>(Field<1>().value, static_cast<std::uint16_t>(ZipVersions::Zip32DefaultVersion),
                                                  static_cast<std::uint16_t>(ZipVersions::Zip64FormatExtension));

        StreamBase::Read(stream, &Field<2>().value);
        ThrowErrorIfNot(Error::ZipLocalFileHeader, ((Field<2>().value & static_cast<std::uint16_t>(UnsupportedFlagsMask)) == 0), "unsupported flag(s) specified");
        ThrowErrorIfNot(Error::ZipLocalFileHeader, (IsGeneralPurposeBitSet() == m_directoryEntry->IsGeneralPurposeBitSet()), "inconsistent general purpose bits specified");

        StreamBase::Read(stream, &Field<3>().value);
        Meta::OnlyEitherValueValidation<std::uint16_t>(Field<3>().value, static_cast<std::uint16_t>(CompressionType::Deflate),
                                                  static_cast<std::uint16_t>(CompressionType::Store));

        StreamBase::Read(stream, &Field<4>().value);
        StreamBase::Read(stream, &Field<5>().value);
        StreamBase::Read(stream, &Field<6>().value);
        ThrowErrorIfNot(Error::ZipLocalFileHeader, (!IsGeneralPurposeBitSet() || (Field<6>().value == 0)), "Invalid Zip CRC");

        StreamBase::Read(stream, &Field<7>().value);
        ThrowErrorIfNot(Error::ZipLocalFileHeader, (!IsGeneralPurposeBitSet() || (Field<7>().value == 0)), "Invalid Zip compressed size");

        StreamBase::Read(stream, &Field<8>().value);

        StreamBase::Read(stream, &Field<9>().value);
        ThrowErrorIfNot(Error::ZipLocalFileHeader, (Field<9>().value != 0), "unsupported file name size");
        Field<11>().value.resize(GetFileNameLength(), 0);

        StreamBase::Read(stream, &Field<10>().value);
        // Even if we don't validate them, we need to read the extra field
        if (Field<10>().value != 0) {Field<12>().value.resize(Field<10>().value, 0); }

        if (Field<11>().Size())
        {
            ThrowHrIfFailed(stream->Read(reinterpret_cast<void*>(Field<11>().value.data()), static_cast<ULONG>(Field<11>().Size()), nullptr));
        }

        if (Field<12>().Size())
        {
            ThrowHrIfFailed(stream->Read(reinterpret_cast<void*>(Field<12>().value.data()), static_cast<ULONG>(Field<12>().Size()), nullptr));
        }
    }

    LocalFileHeader(std::shared_ptr<CentralDirectoryFileHeader> directoryEntry) : m_isZip64(directoryEntry->GetIsZip64()), m_directoryEntry(directoryEntry)
    {
    }

    bool IsGeneralPurposeBitSet() noexcept
    {
        return ((GetGeneralPurposeBitFlags() & GeneralPurposeBitFlags::GeneralPurposeBit) == GeneralPurposeBitFlags::GeneralPurposeBit);
    }

    GeneralPurposeBitFlags GetGeneralPurposeBitFlags() noexcept { return static_cast<GeneralPurposeBitFlags>(Field<2>().value); }
    CompressionType GetCompressionType() noexcept { return static_cast<CompressionType>(Field<3>().value); }

    std::uint64_t GetCompressedSize() noexcept
    {
        return IsGeneralPurposeBitSet() ? m_directoryEntry->GetCompressedSize() : static_cast<std::uint64_t>(Field<7>().value);
    }

    std::uint64_t GetUncompressedSize() noexcept
    {   return IsGeneralPurposeBitSet() ? m_directoryEntry->GetUncompressedSize() : static_cast<std::uint64_t>(Field<8>().value);
    }

    std::uint16_t GetFileNameLength()                  noexcept { return Field<9>().value;  }
    std::uint16_t GetExtraFieldLength()                noexcept { return Field<10>().value; }
    void SetGeneralPurposeBitFlag(std::uint16_t value) noexcept { Field<2>().value = value;  }
    void SetCompressedSize(std::uint32_t value)        noexcept { Field<7>().value = value;  }
    void SetUncompressedSize(std::uint32_t value)      noexcept { Field<8>().value = value;  }
    void SetFileNameLength(std::uint16_t value)        noexcept { Field<9>().value = value;  }
    void SetExtraFieldLength(std::uint16_t value)      noexcept { Field<10>().value = value; }

    std::string GetFileName()
    {
        auto data = Field<11>().value;
        return std::string(data.begin(), data.end());
    }

    void SetFileName(std::string name)
    {
        auto data = Field<11>().value;
        data.resize(name.size());
        data.assign(name.begin(), name.end());
        SetFileNameLength(static_cast<std::uint16_t>(name.size()));
    }
protected:
    bool                                        m_isZip64        = false;
    std::shared_ptr<CentralDirectoryFileHeader> m_directoryEntry = nullptr;
}; //class LocalFileHeader

//////////////////////////////////////////////////////////////////////////////////////////////
//                              Zip64EndOfCentralDirectoryRecord                            //
//////////////////////////////////////////////////////////////////////////////////////////////
class Zip64EndOfCentralDirectoryRecord final : public Meta::StructuredObject< 
    Meta::Field4Bytes, // 0 - zip64 end of central dir signature                            4 bytes(0x06064b50)
    Meta::Field8Bytes, // 1 - size of zip64 end of central directory record                 8 bytes
    Meta::Field2Bytes, // 2 - version made by                                               2 bytes
    Meta::Field2Bytes, // 3 - version needed to extract                                     2 bytes
    Meta::Field4Bytes, // 4 - number of this disk                                           4 bytes
    Meta::Field4Bytes, // 5 - number of the disk with the start of the central directory    4 bytes
    Meta::Field8Bytes, // 6 - total number of entries in the central directory on this disk 8 bytes
    Meta::Field8Bytes, // 7 - total number of entries in the central directory              8 bytes
    Meta::Field8Bytes, // 8 - size of the central directory                                 8 bytes
    Meta::Field8Bytes, // 9 - offset of start of central directory with respect to the
                       //     starting disk number                                          8 bytes
    Meta::FieldNBytes  //10 - zip64 extensible data sector                                  (variable size)
    >
{
public:
    void Read(const ComPtr<IStream>& stream)
    {
        StreamBase::Read(stream, &Field<0>().value);
        Meta::ExactValueValidation<std::uint32_t>(Field<0>().value, static_cast<std::uint32_t>(Signatures::Zip64EndOfCD));

        StreamBase::Read(stream, &Field<1>().value);
        //4.3.14.1 The value stored into the "size of zip64 end of central
        //    directory record" should be the size of the remaining
        //    record and should not include the leading 12 bytes.
        ThrowErrorIfNot(Error::Zip64EOCDRecord, (Field<1>().value == (this->Size() - 12)), "invalid size of zip64 EOCD");

        StreamBase::Read(stream, &Field<2>().value);
        Meta::ExactValueValidation<std::uint16_t>(Field<2>().value, static_cast<std::uint16_t>(ZipVersions::Zip64FormatExtension));

        StreamBase::Read(stream, &Field<3>().value);
        Meta::ExactValueValidation<std::uint16_t>(Field<3>().value, static_cast<std::uint16_t>(ZipVersions::Zip64FormatExtension));

        StreamBase::Read(stream, &Field<4>().value);
        Meta::ExactValueValidation<std::uint32_t>(Field<4>().value, 0);

        StreamBase::Read(stream, &Field<5>().value);
        Meta::ExactValueValidation<std::uint32_t>(Field<5>().value, 0);

        StreamBase::Read(stream, &Field<6>().value);
        Meta::NotValueValidation<std::uint64_t>(Field<6>().value, 0);

        StreamBase::Read(stream, &Field<7>().value);
        Meta::NotValueValidation<std::uint64_t>(Field<7>().value, 0);
        ThrowErrorIfNot(Error::Zip64EOCDRecord, (Field<7>().value == this->GetTotalNumberOfEntries()), "invalid total number of entries");

        ULARGE_INTEGER pos = {0};
        ThrowHrIfFailed(stream->Seek({0}, StreamBase::Reference::CURRENT, &pos));
        StreamBase::Read(stream, &Field<8>().value);
        ThrowErrorIfNot(Error::Zip64EOCDRecord, ((Field<8>().value != 0) && (Field<8>().value < pos.QuadPart)), "invalid size of central directory");

        ThrowHrIfFailed(stream->Seek({0}, StreamBase::Reference::CURRENT, &pos));
        StreamBase::Read(stream, &Field<9>().value);
        ThrowErrorIfNot(Error::Zip64EOCDRecord, ((Field<9>().value != 0) && (Field<9>().value < pos.QuadPart)), "invalid size of central directory");

        if (Field<10>().Size())
        {
            ThrowHrIfFailed(stream->Read(reinterpret_cast<void*>(Field<10>().value.data()), static_cast<ULONG>(Field<10>().Size()), nullptr));
        }
    }

    Zip64EndOfCentralDirectoryRecord()
    {
        SetSignature(static_cast<std::uint32_t>(Signatures::Zip64EndOfCD));
        SetGetSizeOfZip64CDRecord(this->Size() - 12);
        SetVersionMadeBy(static_cast<std::uint16_t>(ZipVersions::Zip64FormatExtension));
        SetVersionNeededToExtract(static_cast<std::uint16_t>(ZipVersions::Zip64FormatExtension));
        SetNumberOfThisDisk(0);
        SetTotalNumberOfEntries(0);
    }

    std::uint64_t GetTotalNumberOfEntries() noexcept { return Field<6>().value; }

    void SetTotalNumberOfEntries(std::uint64_t value) noexcept
    {
        Field<6>().value = value;
        Field<7>().value = value;
    }

    std::uint64_t GetSizeOfCD()                         noexcept { return Field<8>().value; }
    void SetSizeOfCD(std::uint64_t value)               noexcept { Field<8>().value = value; }
    std::uint64_t GetOffsetStartOfCD()                  noexcept { return Field<9>().value; }
    void SetOffsetfStartOfCD(std::uint64_t value)       noexcept { Field<9>().value = value; }

private:
    void SetSignature(std::uint32_t value)              noexcept { Field<0>().value = value; }
    void SetGetSizeOfZip64CDRecord(std::uint64_t value) noexcept { Field<1>().value = value; }
    void SetVersionMadeBy(std::uint16_t value)          noexcept { Field<2>().value = value; }
    void SetVersionNeededToExtract(std::uint16_t value) noexcept { Field<3>().value = value; }
    void SetNumberOfThisDisk(std::uint32_t value)       noexcept { Field<4>().value = value; }

}; //class Zip64EndOfCentralDirectoryRecord

//////////////////////////////////////////////////////////////////////////////////////////////
//                          Zip64EndOfCentralDirectoryLocator                               //
//////////////////////////////////////////////////////////////////////////////////////////////
class Zip64EndOfCentralDirectoryLocator final : public Meta::StructuredObject<
    Meta::Field4Bytes,  // 0 - zip64 end of central dir locator signature        4 bytes(0x07064b50)
    Meta::Field4Bytes,  // 1 - number of the disk with the start of the zip64
                        //     end of central directory                          4 bytes
    Meta::Field8Bytes,  // 2 - relative offset of the zip64 end of central
                        //     directory record                                  8 bytes
    Meta::Field4Bytes   // 3 - total number of disks                             4 bytes
    >
{
public:
    void Read(const ComPtr<IStream>& stream)
    {
        StreamBase::Read(stream, &Field<0>().value);
        Meta::ExactValueValidation<std::uint32_t>(Field<0>().value, static_cast<std::uint32_t>(Signatures::Zip64EndOfCDLocator));

        StreamBase::Read(stream, &Field<1>().value);
        Meta::ExactValueValidation<std::uint32_t>(Field<1>().value, 0);

        ULARGE_INTEGER pos = {0};
        StreamBase::Read(stream, &Field<2>().value);
        ThrowHrIfFailed(stream->Seek({0}, StreamBase::Reference::CURRENT, &pos));
        ThrowErrorIfNot(Error::Zip64EOCDLocator, ((Field<2>().value != 0) && (Field<2>().value < pos.QuadPart)), "Invalid relative offset");

        StreamBase::Read(stream, &Field<3>().value);
        Meta::ExactValueValidation<std::uint32_t>(Field<3>().value, 1);
    }

    Zip64EndOfCentralDirectoryLocator()
    {
        // set smart defaults.
        SetSignature(static_cast<std::uint32_t>(Signatures::Zip64EndOfCDLocator));
        SetNumberOfDisk(0);
        SetTotalNumberOfDisks(1);
    }

    std::uint64_t GetRelativeOffset()               noexcept { return Field<2>().value; }
    void SetRelativeOffset(std::uint64_t value)     noexcept { Field<2>().value = value; }

private:
    void SetSignature(std::uint32_t value)          noexcept { Field<0>().value = value; }
    void SetNumberOfDisk(std::uint32_t value)       noexcept { Field<1>().value = value; }
    void SetTotalNumberOfDisks(std::uint32_t value) noexcept { Field<3>().value = value; }
}; //class Zip64EndOfCentralDirectoryLocator

//////////////////////////////////////////////////////////////////////////////////////////////
//                              EndOfCentralDirectoryRecord                                 //
//////////////////////////////////////////////////////////////////////////////////////////////
class EndCentralDirectoryRecord final : public Meta::StructuredObject<
    Meta::Field4Bytes,  // 0 - end of central dir signature              4 bytes  (0x06054b50)
    Meta::Field2Bytes,  // 1 - number of this disk                       2 bytes
    Meta::Field2Bytes,  // 2 - number of the disk with the start of the
                        //     central directory                         2 bytes
    Meta::Field2Bytes,  // 3 - total number of entries in the central
                        //     directory on this disk                    2 bytes
    Meta::Field2Bytes,  // 4 - total number of entries in the central
                        //     directory                                 2 bytes
    Meta::Field4Bytes,  // 5 - size of the central directory             4 bytes
    Meta::Field4Bytes,  // 6 - offset of start of central directory with
                        //     respect to the starting disk number       4 bytes
    Meta::Field2Bytes,  // 7 - .ZIP file comment length                  2 bytes
    Meta::FieldNBytes   // 8 - .ZIP file comment                         (variable size)
    >
{
public:
    void Read(const ComPtr<IStream>& stream)
    {
        StreamBase::Read(stream, &Field<0>().value);
        Meta::ExactValueValidation<std::uint32_t>(Field<0>().value, static_cast<std::uint32_t>(Signatures::EndOfCentralDirectory));

        StreamBase::Read(stream, &Field<1>().value);
        Meta::OnlyEitherValueValidation<std::uint32_t>(Field<1>().value, 0, 0xFFFF);

        StreamBase::Read(stream, &Field<2>().value);
        Meta::OnlyEitherValueValidation<std::uint32_t>(Field<2>().value, 0, 0xFFFF);
        ThrowErrorIf(Error::ZipEOCDRecord, (Field<1>().value != Field<2>().value), "field missmatch");
        m_isZip64 = (0xFFFF == Field<2>().value);

        StreamBase::Read(stream, &Field<3>().value);
        if (Field<3>().value != 0 && Field<3>().value != 0xFFFF)
        {   m_archiveHasZip64Locator = false;
        }

        StreamBase::Read(stream, &Field<4>().value);
        ThrowErrorIf(Error::ZipEOCDRecord, (Field<3>().value != Field<4>().value), "field missmatch");

        StreamBase::Read(stream, &Field<5>().value);
        StreamBase::Read(stream, &Field<6>().value);

        if(m_archiveHasZip64Locator)
        {
            ThrowErrorIf(Error::ZipEOCDRecord, ((Field<5>().value != 0) && (Field<5>().value != 0xFFFFFFFF)),
                "unsupported size of central directory");
            ThrowErrorIf(Error::ZipEOCDRecord, ((Field<6>().value != 0) && (Field<6>().value != 0xFFFFFFFF)),
                "unsupported offset of start of central directory");
        }

        StreamBase::Read(stream, &Field<7>().value);
        Meta::ExactValueValidation<std::uint32_t>(Field<7>().value, 0);

        if (Field<8>().Size())
        {
           ThrowHrIfFailed(stream->Read(reinterpret_cast<void*>(Field<8>().value.data()), static_cast<ULONG>(Field<8>().Size()), nullptr));
        }
    }

    EndCentralDirectoryRecord()
    {
        SetSignature(static_cast<std::uint32_t>(Signatures::EndOfCentralDirectory));
        SetNumberOfDisk(0);
        SetDiskStart(0);
        // by default, the next 12 bytes need to be: FFFF FFFF  FFFF FFFF  FFFF FFFF
        SetTotalNumberOfEntries          (std::numeric_limits<std::uint16_t>::max());
        SetTotalEntriesInCentralDirectory(std::numeric_limits<std::uint16_t>::max());
        SetSizeOfCentralDirectory        (std::numeric_limits<std::uint32_t>::max());
        SetOffsetOfCentralDirectory      (std::numeric_limits<std::uint32_t>::max());
        // last 2 bytes need to be : 00
        SetCommentLength(0);
    }
    
    bool GetArchiveHasZip64Locator()                            noexcept { return m_archiveHasZip64Locator; }
    bool GetIsZip64()                                           noexcept { return m_isZip64; }
    std::uint64_t GetNumberOfCentralDirectoryEntries()          noexcept { return static_cast<std::uint64_t>(Field<3>().value); }
    std::uint64_t GetStartOfCentralDirectory()                  noexcept { return static_cast<std::uint64_t>(Field<6>().value); }

private:
    bool m_isZip64 = false;
    bool m_archiveHasZip64Locator = true;

    void SetSignature(std::uint32_t value)                      noexcept { Field<0>().value = value; }
    void SetNumberOfDisk(std::uint16_t value)                   noexcept { Field<1>().value = value; }
    void SetDiskStart(std::uint16_t value)                      noexcept { Field<2>().value = value; }
    void SetTotalNumberOfEntries(std::uint16_t value)           noexcept { Field<3>().value = value; }
    void SetTotalEntriesInCentralDirectory(std::uint16_t value) noexcept { Field<4>().value = value; }
    void SetSizeOfCentralDirectory(std::uint32_t value)         noexcept { Field<5>().value = value; }
    void SetOffsetOfCentralDirectory(std::uint32_t value)       noexcept { Field<6>().value = value; }
    std::uint32_t GetOffsetOfCentralDirectory()                 noexcept { return Field<6>().value;  }
    void SetCommentLength(std::uint16_t value)                  noexcept { Field<7>().value = value; }
};//class EndCentralDirectoryRecord

//////////////////////////////////////////////////////////////////////////////////////////////
//                              ZipObject member implementation                             //
//////////////////////////////////////////////////////////////////////////////////////////////
std::vector<std::string> ZipObject::GetFileNames(FileNameOptions)
{
    std::vector<std::string> result;
    std::for_each(m_streams.begin(), m_streams.end(), [&result](auto it)
    {
        result.push_back(it.first);
    });
    return result;
}

ComPtr<IStream> ZipObject::GetFile(const std::string& fileName)
{   // TODO: Make this on-demand populate m_streams and then pull from there.
    auto result = m_streams.find(fileName);
    if (result == m_streams.end())
    {
        return ComPtr<IStream>();
    }        
    return result->second;
}

std::string ZipObject::GetFileName()
{
    return m_stream.As<IStreamInternal>()->GetName();
}

ZipObject::ZipObject(IMsixFactory* appxFactory, const ComPtr<IStream>& stream) : m_factory(appxFactory), m_stream(stream)
{   // Confirm that the file IS the correct format
    EndCentralDirectoryRecord endCentralDirectoryRecord;
    LARGE_INTEGER pos = {0};
    pos.QuadPart = -1 * endCentralDirectoryRecord.Size();
    ThrowHrIfFailed(m_stream->Seek(pos, StreamBase::Reference::END, nullptr));
    endCentralDirectoryRecord.Read(m_stream.Get());

    // find where the zip central directory exists.
    std::uint64_t offsetStartOfCD = 0;
    std::uint64_t totalNumberOfEntries = 0;
    Zip64EndOfCentralDirectoryLocator zip64Locator;
    if (!endCentralDirectoryRecord.GetArchiveHasZip64Locator())
    {
        offsetStartOfCD      = endCentralDirectoryRecord.GetStartOfCentralDirectory();
        totalNumberOfEntries = endCentralDirectoryRecord.GetNumberOfCentralDirectoryEntries();
    }
    else
    {   // Make sure that we have a zip64 end of central directory locator            
        pos.QuadPart = -1*(endCentralDirectoryRecord.Size() + zip64Locator.Size());
        ThrowHrIfFailed(m_stream->Seek(pos, StreamBase::Reference::END, nullptr));
        zip64Locator.Read(m_stream.Get());

        // now read the end of zip central directory record
        Zip64EndOfCentralDirectoryRecord zip64EndOfCentralDirectory;
        pos.QuadPart = zip64Locator.GetRelativeOffset();
        ThrowHrIfFailed(m_stream->Seek(pos, StreamBase::Reference::START, nullptr));
        zip64EndOfCentralDirectory.Read(m_stream.Get());            
        offsetStartOfCD = zip64EndOfCentralDirectory.GetOffsetStartOfCD();
        totalNumberOfEntries = zip64EndOfCentralDirectory.GetTotalNumberOfEntries();
    }

    // read the zip central directory
    std::map<std::string, std::shared_ptr<CentralDirectoryFileHeader>> centralDirectory;
    pos.QuadPart = offsetStartOfCD;
    ThrowHrIfFailed(m_stream->Seek(pos, StreamBase::Reference::START, nullptr));
    for (std::uint32_t index = 0; index < totalNumberOfEntries; index++)
    {
        auto centralFileHeader = std::make_shared<CentralDirectoryFileHeader>(endCentralDirectoryRecord.GetIsZip64());
        centralFileHeader->Read(m_stream.Get());
        // TODO: ensure that there are no collisions on name!
        centralDirectory.insert(std::make_pair(centralFileHeader->GetFileName(), centralFileHeader));
    }

    if (endCentralDirectoryRecord.GetArchiveHasZip64Locator())
    {   // We should have no data between the end of the last central directory header and the start of the EoCD
        ULARGE_INTEGER uPos = {0};
        ThrowHrIfFailed(m_stream->Seek({0}, StreamBase::Reference::CURRENT, &uPos));
        ThrowErrorIfNot(Error::ZipHiddenData, (uPos.QuadPart == zip64Locator.GetRelativeOffset()), "hidden data unsupported");
    }

    // TODO: change population of m_streams into cache semantics and move into ZipObject::GetFile
    // Read the file repository
    for (const auto& centralFileHeader : centralDirectory)
    {
        pos.QuadPart = centralFileHeader.second->GetRelativeOffsetOfLocalHeader();
        ThrowHrIfFailed(m_stream->Seek(pos, MSIX::StreamBase::Reference::START, nullptr));
        auto localFileHeader = std::make_shared<LocalFileHeader>(centralFileHeader.second);
        localFileHeader->Read(m_stream.Get());

        auto fileStream = ComPtr<IStream>::Make<ZipFileStream>(
            centralFileHeader.second->GetFileName(),
            "TODO: Implement", // TODO: put value from content type 
            localFileHeader->GetCompressionType() == CompressionType::Deflate,
            centralFileHeader.second->GetRelativeOffsetOfLocalHeader() + localFileHeader->Size(),
            localFileHeader->GetCompressedSize(),
            m_stream
            );

        if (localFileHeader->GetCompressionType() == CompressionType::Deflate)
        {
            fileStream = ComPtr<IStream>::Make<InflateStream>(std::move(fileStream), localFileHeader->GetUncompressedSize());
        }

        m_streams.insert(std::make_pair(centralFileHeader.second->GetFileName(), std::move(fileStream)));
    }
} // ZipObject::ZipObject
} // namespace MSIX
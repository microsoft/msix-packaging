//
//  Copyright (C) 2017 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
// 
#include "Exceptions.hpp"
#include "ComHelper.hpp"
#include "StreamBase.hpp"
#include "ObjectBase.hpp"
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

inline constexpr operator std::uint16_t(const ZipVersions& value) { return static_cast<std::uint16_t>(value); }

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

inline constexpr operator std::uint32_t(const Signatures& value) { return static_cast<std::uint32_t>(value); }

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

inline constexpr GeneralPurposeBitFlags operator &(GeneralPurposeBitFlags a, GeneralPurposeBitFlags b)
{   return static_cast<GeneralPurposeBitFlags>(static_cast<uint16_t>(a) & static_cast<uint16_t>(b));
}

inline constexpr GeneralPurposeBitFlags operator |(GeneralPurposeBitFlags a, GeneralPurposeBitFlags b)
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

//////////////////////////////////////////////////////////////////////////////////////////////
//                              General Zip validation policies                             //
//////////////////////////////////////////////////////////////////////////////////////////////
class OffsetOrSize32bit : public Meta::FieldBase<OffsetOrSize32bit,   std::uint32_t, Meta::OnlyEitherValueValidation<OffsetOrSize32bit, 0, 0xFFFFFFFF>> {};
class OffsetOrSize64bit : public Meta::FieldBase<OffsetOrSize64bit,   std::uint64_t, Meta::InjectedValidation<OffsetOrSize64bit >> {};
class HowCompressed     : public Meta::FieldBase<HowCompressed,       std::uint16_t, Meta::OnlyEitherValueValidation<HowCompressed, CompressionType::Deflate, CompressionType::Store>> {};
class FieldMustBeEmpty  : public Meta::FieldNBytes<FieldMustBeEmpty,  Meta::InvalidFieldValidation<FieldMustBeEmpty>>{};
class VarFieldLength    : public Meta::FieldBase<VarFieldLength,      std::uint16_t, Meta::InjectedValidation<VarFieldLength>> {};
class VarFieldLenZero   : public Meta::FieldBase<VarFieldLenZero,     std::uint16_t, Meta::ExactValueValidation<VarFieldLenZero, 0>> {};
class InjectedVal2Bytes : public Meta::FieldBase<InjectedVal2Bytes,   std::uint16_t, Meta::InjectedValidation<InjectedVal2Bytes>> {};
class InjectedVal4Bytes : public Meta::FieldBase<InjectedVal4Bytes,   std::uint32_t, Meta::InjectedValidation<InjectedVal4Bytes>> {};
class InjectedVal8Bytes : public Meta::FieldBase<InjectedVal8Bytes,   std::uint64_t, Meta::InjectedValidation<InjectedVal8Bytes>> {};

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
class Z64ExtInfoHeader  : public Meta::FieldBase<Z64ExtInfoHeader,    std::uint16_t, Meta::ExactValueValidation<Z64ExtInfoHeader, HeaderIDs::Zip64ExtendedInfo>> {};
class Z64ExtInfoSize    : public Meta::FieldBase<Z64ExtInfosize,      std::uint16_t, Meta::OnlyEitherValueValidation<Z64ExtInfoSize, 24, 28>> {};

class Zip64ExtendedInformation : public Meta::StructuredObject<
    Z64ExtInfoHeader,   // 0 - tag for the "extra" block type               2 bytes(0x0001)
    Z64ExtInfoSize,     // 1 - size of this "extra" block                   2 bytes
    Meta::Field8Bytes,  // 2 - Original uncompressed file size              8 bytes
                        // No point in validating these as it is actually 
                        // possible to have a 0-byte file... Who knew.
    Meta::Field8Bytes,  // 3 - Compressed file size                         8 bytes
                        // No point in validating these as it is actually 
                        // possible to have a 0-byte file... Who knew.    
    InjectedVal8Bytes   // 4 - Offset of local header record                8 bytes
    //Meta::Field4Bytes // 5 - number of the disk on which the file starts  4 bytes -- ITS A FAAKEE!
>
{
public:
    void ValidateField(size_t field) override 
    {
        ULARGE_INTEGER pos = {0};        
        switch (field)
        {
        case 4:
            ThrowErrorIfNot(Error::ZipBadExtendedData, ((Field<4>().value < m_start.QuadPart)), "invalid relative header offset");         
            break;
        default:
            UNEXPECTED;
        }
    }

    Zip64ExtendedInformation(ULARGE_INTEGER start, IStream* stream) : m_start(start), m_stream(stream)
    {
        ConfigureField<4>();
    }

    std::uint64_t GetUncompressedSize()   const noexcept { return Field<2>().value; }
    void SetUncompressedSize(std::uint64_t v)   noexcept { Field<2>().value = v; }
    std::uint64_t GetCompressedSize()     const noexcept { return Field<3>().value; }
    void SetCompressedSize(std::uint64_t v)     noexcept { Field<3>().value = v; }
    std::uint64_t GetRelativeOffset()     const noexcept { return Field<4>().value; }
    void SetRelativeOffset(std::uint64_t v)     noexcept { Field<4>().value = v; }

private:
    ULARGE_INTEGER  m_start;
    ComPtr<IStream> m_stream;
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
class CDFH_Header       : public Meta::FieldBase<CDFH_Header,         std::uint32_t, Meta::ExactValueValidation<CDFH_Header, Signatures::CentralFileHeader>> {};
class CDFH_DiskNumber   : public Meta::FieldBase<CDFH_DiskNumber,     std::uint16_t, Meta::ExactValueValidation<CDFH_DiskNumber, 0>> {};
class CDFH_ExtraField   : public Meta::FieldNBytes<CDFH_ExtraField,   Meta::InjectedValidation<CDFH_ExtraField>> {};
class CDFH_GPBit        : public Meta::FiedlBase<CDFH_GPBit,          std::uint16_t, Meta::VirtualValidation<CDFH_GPBit, 0>>
{
public:
    void Validate(std::size_t, CDFH_GPBit* self) override {
        ThrowErrorIfNot(Error::ZipCentralDirectoryHeader,
            ((self->Field<3>().value  & static_cast<std::uint16_t>(UnsupportedFlagsMask)) == 0),
            "unsupported flag(s) specified");
        };
    }
};

class CentralDirectoryFileHeader : public Meta::StructuredObject<
    CDFH_Header,       // 0 - central file header signature   4 bytes(0x02014b50)
    Meta::Field2Bytes, // 1 - version made by                 2 bytes
    Meta::Field2Bytes, // 2 - version needed to extract       2 bytes
    CDFH_GPBit,        // 3 - general purpose bit flag        2 bytes
    HowCompressed,     // 4 - compression method              2 bytes
    Meta::Field2Bytes, // 5 - last mod file time              2 bytes
    Meta::Field2Bytes, // 6 - last mod file date              2 bytes
    Meta::Field4Bytes, // 7 - crc - 32                        4 bytes
    Meta::Field4Bytes, // 8 - compressed size                 4 bytes
    Meta::Field4Bytes, // 9 - uncompressed size               4 bytes
    InjectedVal2Bytes, //10 - file name length                2 bytes
    InjectedVal2Bytes, //11 - extra field length              2 bytes
    VarFieldLenZero,   //12 - file comment length             2 bytes
    CDFH_DiskNumber,   //13 - disk number start               2 bytes
    Meta::Field2Bytes, //14 - internal file attributes        2 bytes
    Meta::Field4Bytes, //15 - external file attributes        4 bytes
    InjectedVal4Bytes, //16 - relative offset of local header 4 bytes
    Meta::FieldNBytes, //17 - file name(variable size)
    CDFH_ExtraField,   //18 - extra field(variable size)
    FieldMustBeEmpty   //19 - file comment(variable size)
    >
{
public:
    void ValidateField(size_t field)
    {
        ULARGE_INTEGER pos = {0};

        switch(field)
        {
        case 10:
            ThrowErrorIfNot(Error::ZipCentralDirectoryHeader, (Field<10>().value != 0), "unsupported file name size");
            Field<17>().value.resize(Field<10>().value, 0);
            break;
        case 11:
            if (Field<11>().value != 0) { Field<18>().value.resize(Field<11>().value, 0); }  
            break;
        case 16:
            ThrowHrIfFailed(m_stream->Seek({0}, StreamBase::Reference::CURRENT, &pos));
            if (!GetIsZip64())
            {   ThrowErrorIf(Error::ZipCentralDirectoryHeader, (Field<16>().value >= pos.QuadPart), "invalid relative header offset");
            }
            else
            {   ThrowErrorIf(Error::ZipCentralDirectoryHeader, (Field<16>().value != 0xFFFFFFFF), "invalid zip64 local header offset");
            }        
            break;
        case 18:
            // Only process for Zip64ExtendedInformation
            if (Field<18>().value.size() > 2 && Field<18>().value[0] == 0x01 && Field<18>().value[1] == 0x00)
            {
                LARGE_INTEGER zero = {0};
                ThrowHrIfFailed(m_stream->Seek(zero, StreamBase::Reference::CURRENT, &pos));
                auto vectorStream = ComPtr<IStream>::Make<VectorStream>(&Field<18>().value);
                m_extendedInfo = std::make_unique<Zip64ExtendedInformation>(pos, vectorStream.Get());
                ThrowErrorIfNot(Error::ZipCentralDirectoryHeader,(Field<18>().value.size() >= m_extendedInfo->Size()),"Unexpected extended info size");
                m_extendedInfo->Read(vectorStream.Get());
            }
            break;
        default:
            UNEXPECTED;
        }
    }

    CentralDirectoryFileHeader(bool isZip64, IStream* s) : m_isZip64(isZip64), m_stream(s)
    {
        ConfigureField<10>();
        ConfigureField<11>();
        ConfigureField<16>();
        ConfigureField<18>();

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

    inline bool IsGeneralPurposeBitSet() const noexcept
    {   return ((GetGeneralPurposeBitFlags() & GeneralPurposeBitFlags::GeneralPurposeBit) == GeneralPurposeBitFlags::GeneralPurposeBit);
    }

    inline std::uint16_t GetVersionNeededToExtract()          const noexcept { return Field<2>().value; }
    inline GeneralPurposeBitFlags GetGeneralPurposeBitFlags() const noexcept { return static_cast<GeneralPurposeBitFlags>(Field<3>().value); }
    inline void SetGeneralPurposeBitFlags(std::uint16_t value)      noexcept { Field<3>().value = value; }
    inline std::uint16_t GetCompressionMethod()               const noexcept { return Field<4>().value; }
    inline void SetCompressionMethod(std::uint16_t value)           noexcept { Field<4>().value= value; }
    inline std::uint32_t GetCrc32()                           const noexcept { return Field<7>().value; }
    inline void SetCrc(std::uint32_t value)                         noexcept { Field<7>().value = value; }

    std::uint64_t GetCompressedSize() const noexcept
    {
        if (!m_extendedInfo.get()) {
            return static_cast<std::uint64_t>(Field<8>().value);
        }
        return m_extendedInfo->GetCompressedSize();
    }

    // TODO: on-demand create m_extendedInfo?
    inline void SetCompressedSize(std::uint32_t value) noexcept { Field<8>().value = value; }

    std::uint64_t GetUncompressedSize() const noexcept
    {
        if (!m_extendedInfo.get()) {
            return static_cast<std::uint64_t>(Field<9>().value);
        }
        return m_extendedInfo->GetUncompressedSize();
    }

    // TODO: on-demand create m_extendedInfo?
    inline void SetUncompressedSize(std::uint32_t value) noexcept { Field<9>().value = value; }

    std::uint64_t GetRelativeOffsetOfLocalHeader() const noexcept
    {
        if (!m_extendedInfo.get()) {
            return static_cast<std::uint64_t>(Field<16>().value);
        }
        return m_extendedInfo->GetRelativeOffset();
    }

    // TODO: on-demand create m_extendedInfo?
    inline void SetRelativeOffsetOfLocalHeader(std::uint32_t value) noexcept { Field<16>().value = value; }

    std::string GetFileName() const
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

    inline bool GetIsZip64() const noexcept { return m_isZip64; }

private:
    inline void SetSignature(std::uint32_t value)              noexcept { Field<0>().value = value; }
    inline void SetVersionMadeBy(std::uint16_t value)          noexcept { Field<1>().value = value; }
    inline void SetVersionNeededToExtract(std::uint16_t value) noexcept { Field<2>().value = value; }
    inline void SetLastModFileTime(std::uint16_t value)        noexcept { Field<5>().value = value; }
    inline void SetLastModFileDate(std::uint16_t value)        noexcept { Field<6>().value = value; }
    inline void SetFileNameLength(std::uint16_t value)         noexcept { Field<10>().value = value; }
    inline void SetExtraFieldLength(std::uint16_t value)       noexcept { Field<11>().value = value; }
    inline std::uint16_t GetExtraFieldLength()           const noexcept { return Field<11>().value;  }
    inline void SetFileCommentLength(std::uint16_t value)      noexcept { Field<12>().value = value; }
    inline void SetDiskNumberStart(std::uint16_t value)        noexcept { Field<13>().value = value; }
    inline void SetInternalFileAttributes(std::uint16_t value) noexcept { Field<14>().value = value; }
    inline void SetExternalFileAttributes(std::uint16_t value) noexcept { Field<15>().value = value; }

    std::unique_ptr<Zip64ExtendedInformation> m_extendedInfo;
    ComPtr<IStream> m_stream;
    bool m_isZip64 = false;
};//class CentralDirectoryFileHeader

//////////////////////////////////////////////////////////////////////////////////////////////
//                                  LocalFileHeader                                         //
//////////////////////////////////////////////////////////////////////////////////////////////
class LFH_Header        : public Meta::FieldBase<LFH_Header,          std::uint32_t, Meta::ExactValueValidation<LFH_Header, Signatures::LocalFileHeader>> {};
class LFH_VersionNeeded : public Meta::FieldBase<LFH_VersionNeeded,   std::uint16_t, Meta::OnlyEitherValueValidation<LFH_VersionNeeded, ZipVersions::Zip32DefaultVersion, ZipVersions::Zip64FormatExtension>> {};

class LocalFileHeader : public Meta::StructuredObject<LocalFileHeader, 
    LFH_Header,         // 0 - local file header signature     4 bytes(0x04034b50)
    LFH_VersionNeeded,  // 1 - version needed to extract       2 bytes
    InjectedVal2Bytes,  // 2 - general purpose bit flag        2 bytes
    HowCompressed,      // 3 - compression method              2 bytes
    Meta::Field2Bytes,  // 4 - last mod file time              2 bytes
    Meta::Field2Bytes,  // 5 - last mod file date              2 bytes
    InjectedVal4Bytes,  // 6 - crc - 32                        4 bytes
    InjectedVal4Bytes,  // 7 - compressed size                 4 bytes
    Meta::Field4Bytes,  // 8 - uncompressed size               4 bytes
    InjectedVal2Bytes,  // 9 - file name length                2 bytes
    InjectedVal2Bytes,  // 10- extra field length              2 bytes
    Meta::FieldNBytes,  // 11- file name                       (variable size)
    Meta::FieldNBytes   // 12- extra field                     (variable size)
>
{
public:
    void ValidateField(size_t field)
    {
        switch (field)
        {
        case 2:
            ThrowErrorIfNot(Error::ZipLocalFileHeader, ((Field<2>().value & static_cast<std::uint16_t>(UnsupportedFlagsMask)) == 0), "unsupported flag(s) specified");
            ThrowErrorIfNot(Error::ZipLocalFileHeader, (IsGeneralPurposeBitSet() == m_directoryEntry->IsGeneralPurposeBitSet()), "inconsistent general purpose bits specified");        
            break;
        case 6:
            ThrowErrorIfNot(Error::ZipLocalFileHeader, (!IsGeneralPurposeBitSet() || (Field<6>().value == 0)), "Invalid Zip CRC");
            break;
        case 7:
            ThrowErrorIfNot(Error::ZipLocalFileHeader, (!IsGeneralPurposeBitSet() || (Field<7>().value == 0)), "Invalid Zip CRC");
            break;
        case 9:
            ThrowErrorIfNot(Error::ZipLocalFileHeader, (Field<9>.value != 0), "unsupported file name size");
            Field<11>().value.resize(GetFileNameLength(), 0);
            break;
        case 10:
            // Even if we don't validate them, we need to read the extra field
            if (Field<10>().value != 0) { Field<12>().value.resize(Field<10>().value,0); }
            break;
        default:
            UNEXPECTED;
        }
    }

    LocalFileHeader(std::shared_ptr<CentralDirectoryFileHeader> directoryEntry) : m_isZip64(directoryEntry->GetIsZip64()), m_directoryEntry(directoryEntry)
    {
        ConfigureField<2>();
        ConfigureField<6>();
        ConfigureField<7>();
        ConfigureField<9>();
        ConfigureField<10>();
    }

    inline bool IsGeneralPurposeBitSet() const noexcept
    {   return ((GetGeneralPurposeBitFlags() & GeneralPurposeBitFlags::GeneralPurposeBit) == GeneralPurposeBitFlags::GeneralPurposeBit);
    }

    inline GeneralPurposeBitFlags GetGeneralPurposeBitFlags() const noexcept { return static_cast<GeneralPurposeBitFlags>(Field<2>().value); }
    inline CompressionType GetCompressionType() const noexcept { return static_cast<CompressionType>(Field<3>().value); }

    inline std::uint64_t GetCompressedSize() const noexcept
    {   return IsGeneralPurposeBitSet() ? m_directoryEntry->GetCompressedSize() : static_cast<std::uint64_t>(Field<7>().value);
    }

    inline std::uint64_t GetUncompressedSize() const noexcept
    {   return IsGeneralPurposeBitSet() ? m_directoryEntry->GetUncompressedSize() : static_cast<std::uint64_t>(Field<8>().value);
    }

    inline std::uint16_t GetFileNameLength()                  const noexcept { return Field<9>().value;  }
    inline std::uint16_t GetExtraFieldLength()                const noexcept { return Field<10>().value; }

    inline void SetGeneralPurposeBitFlag(std::uint16_t value) const noexcept { Field<2>().value = value;  }
    inline void SetCompressedSize(std::uint32_t value)        const noexcept { Field<7>().value = value;  }
    inline void SetUncompressedSize(std::uint32_t value)      const noexcept { Field<8>().value = value;  }
    inline void SetFileNameLength(std::uint16_t value)        const noexcept { Field<9>().value = value;  }
    inline void SetExtraFieldLength(std::uint16_t value)      const noexcept { Field<10>().value = value; }

    std::string GetFileName() const
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
class Z64DiskNumber     : public Meta::FieldBase<Z64DiskNumber,       std::uint32_t, Meta::ExactValueValidation<Z64DiskNumber, 0>> {};
class Z64EOCDRecord     : public Meta::FieldBase<Z64EOCDRecord,       std::uint32_t, Meta::ExactValueValidation<Z64EOCDRecord, Signatures::Zip64EndOfCD>> {};
class Z64EOCDRVersion   : public Meta::FieldBase<Z64EOCDRVersion,     std::uint16_t, Meta::ExactValueValidation<Z64EOCDRVersion, ZipVersions::Zip64FormatExtension>> {};
class Z64EOCDRCount     : public Meta::FiledBase<Z64EOCDRCount,       std::uint64_t, Meta::NotValueValidation<Z64EOCDRCount, 0>> {};

class Zip64EndOfCentralDirectoryRecord : public Meta::StructuredObject<Zip64EndOfCentralDirectoryRecord, 
    Z64EOCDRecord,     // 0 - zip64 end of central dir signature                            4 bytes(0x06064b50)
    InjectedVal8Bytes, // 1 - size of zip64 end of central directory record                 8 bytes
    Z64EOCDRVersion,   // 2 - version made by                                               2 bytes
    Z64EOCDRVersion,   // 3 - version needed to extract                                     2 bytes
    Z64DiskNumber,     // 4 - number of this disk                                           4 bytes
    Z64DiskNumber,     // 5 - number of the disk with the start of the central directory    4 bytes
    Z64EOCDRCount,     // 6 - total number of entries in the central directory on this disk 8 bytes
    Z64EOCDRCount,     // 7 - total number of entries in the central directory              8 bytes
    InjectedVal8Bytes, // 8 - size of the central directory                                 8 bytes
    InjectedVal8Bytes, // 9 - offset of start of central directory with respect to the
                       //     starting disk number                                          8 bytes
    FieldMustBeEmpty   //10 - zip64 extensible data sector                                  (variable size)
    >
{
public:
    void ValidateField(size_t field) override
    {
        ULARGE_INTEGER pos = {0};
        ThrowHrIfFailed(m_stream->Seek({0}, StreamBase::Reference::CURRENT, &pos));
        
        switch (field)
        {
        case 1:
            //4.3.14.1 The value stored into the "size of zip64 end of central
            //    directory record" should be the size of the remaining
            //    record and should not include the leading 12 bytes.
            ThrowErrorIfNot(Error::Zip64EOCDRecord, (Field<1>().value == (this->Size() - 12)), "invalid size of zip64 EOCD");
            break;
        case 8:
            ThrowErrorIfNot(Error::Zip64EOCDRecord, ((Field<8>().value != 0) && (Field<8>().value < pos.QuadPart)), "invalid size of central directory");
            break;
        case 9:
            ThrowErrorIfNot(Error::Zip64EOCDRecord, ((Field<9>().value != 0) && (Field<9>().value < pos.QuadPart)), "invalid size of central directory");
            break;
        default:
            UNEXPECTED;
        }
    }

    void Validate() override
    {
        ThrowErrorIfNot(Error::Zip64EOCDRecord, (Field<7>().value == this->GetTotalNumberOfEntries()), "invalid total number of entries");
    }

    Zip64EndOfCentralDirectoryRecord(const ComPtr<IStream>& s) : m_stream(s)
    {
        ConfigureField<1>(); // TODO: can we make this a static value instead?
        ConfigureField<8>();
        ConfigureField<9>();

        SetSignature(static_cast<std::uint32_t>(Signatures::Zip64EndOfCD));
        SetGetSizeOfZip64CDRecord(this->Size() - 12);
        SetVersionMadeBy(static_cast<std::uint16_t>(ZipVersions::Zip64FormatExtension));
        SetVersionNeededToExtract(static_cast<std::uint16_t>(ZipVersions::Zip64FormatExtension));
        SetNumberOfThisDisk(0);
        SetTotalNumberOfEntries(0);
    }

    inline std::uint64_t GetTotalNumberOfEntries() const noexcept { return Field<6>().value; }

    void SetTotalNumberOfEntries(std::uint64_t value) noexcept
    {
        Field<6>().value = value;
        Field<7>().value = value;
    }

    inline std::uint64_t GetSizeOfCD()                   const noexcept { return Field<8>().value; }
    inline void SetSizeOfCD(std::uint64_t value)               noexcept { Field<8>().value = value; }
    inline std::uint64_t GetOffsetStartOfCD()            const noexcept { return Field<9>().value; }
    inline void SetOffsetfStartOfCD(std::uint64_t value)       noexcept { Field<9>().value = value; }

private:
    inline void SetSignature(std::uint32_t value)              noexcept { Field<0>().value = value; }
    inline void SetGetSizeOfZip64CDRecord(std::uint64_t value) noexcept { Field<1>().value = value; }
    inline void SetVersionMadeBy(std::uint16_t value)          noexcept { Field<2>().value = value; }
    inline void SetVersionNeededToExtract(std::uint16_t value) noexcept { Field<3>().value = value; }
    inline void SetNumberOfThisDisk(std::uint32_t value)       noexcept { Field<4>().value = value; }

    ComPtr<IStream> m_stream = nullptr;
}; //class Zip64EndOfCentralDirectoryRecord

//////////////////////////////////////////////////////////////////////////////////////////////
//                          Zip64EndOfCentralDirectoryLocator                               //
//////////////////////////////////////////////////////////////////////////////////////////////
class Z64EOCDLocator    : public Meta::FieldBase<Z64EOCDLocator,      std::uint32_t, Meta::ExactValueValidation<Z64EOCDLocator, Signatures::Zip64EndOfCDLocator>> {};
class Z64NumberOfDisks  : public Meta::FieldBase<Z64NumberOfDisks,    std::uint32_t, Meta::ExactValueValidation<Z64NumberOfDisks, 1>> {};

class Zip64EndOfCentralDirectoryLocator : public Meta::StructuredObject<Zip64EndOfCentralDirectoryLocator,
    Z64EOCDLocator,     // 0 - zip64 end of central dir locator signature        4 bytes(0x07064b50)
    Z64NumberDisks,     // 1 - number of the disk with the start of the zip64
                        //     end of central directory                          4 bytes
    OffsetOrSize64bit,  // 2 - relative offset of the zip64 end of central
                        //     directory record                                  8 bytes
    Z64NumberOfDisks    // 3 - total number of disks                             4 bytes
    >
{
public:
    void ValidateField(size_t field) override
    {
        switch (field)
        {
        case 2:
            ULARGE_INTEGER pos = {0};
            ThrowHrIfFailed(m_stream->Seek({0}, StreamBase::Reference::CURRENT, &pos));
            ThrowErrorIfNot(Error::Zip64EOCDLocator, ((v != 0) && (v < pos.QuadPart)), "Invalid relative offset");            
        break;
        default:
            UNEXPECTED;
        }
    }

    Zip64EndOfCentralDirectoryLocator(const ComPtr<IStream>& s) : m_stream(s)
    {
        ConfigureField<2>();   
        // set smart defaults.
        SetSignature(static_cast<std::uint32_t>(Signatures::Zip64EndOfCDLocator));
        SetNumberOfDisk(0);
        SetTotalNumberOfDisks(1);
    }

    inline std::uint64_t GetRelativeOffset()         const noexcept { return Field<2>().value; }
    inline void SetRelativeOffset(std::uint64_t value)     noexcept { Field<2>().value = value; }

private:
    inline void SetSignature(std::uint32_t value)          noexcept { Field<0>().value = value; }
    inline void SetNumberOfDisk(std::uint32_t value)       noexcept { Field<1>().value = value; }
    inline void SetTotalNumberOfDisks(std::uint32_t value) noexcept { Field<3>().value = value; }

    ComPtr<IStream> m_stream;
}; //class Zip64EndOfCentralDirectoryLocator

//////////////////////////////////////////////////////////////////////////////////////////////
//                              EndOfCentralDirectoryRecord                                 //
//////////////////////////////////////////////////////////////////////////////////////////////
class EOCDSignature     : public Meta::FieldBase<EOCDSignature,       std::uint32_t, Meta::ExactValueValidation<EOCDSignature, Signatures::EndOfCentralDirectory>> {};
class EOCDNumberDisks   : public Meta::FieldBase<EOCDNumberDisks,     std::uint16_t, Meta::OnlyEitherValueValidation<EOCDNumberDisks,   0, 0xFFFF>> {};

class EndOfCentralDirectoryRecord : public Meta::StructuredObject<EndOfCentralDirectoryRecord,
    EOCDSignature,      // 0 - end of central dir signature              4 bytes  (0x06054b50)
    EOCDNumberDisks,    // 1 - number of this disk                       2 bytes
    EOCDNumberDisks,    // 2 - number of the disk with the start of the
                        //     central directory                         2 bytes
    Field2Bytes,        // 3 - total number of entries in the central
                        //     directory on this disk                    2 bytes
    Field2Bytes,        // 4 - total number of entries in the central
                        //     directory                                 2 bytes
    Field4Bytes,        // 5 - size of the central directory             4 bytes
    Field4Bytes,        // 6 - offset of start of central directory with
                        //     respect to the starting disk number       4 bytes
    VarFieldLenZero,    // 7 - .ZIP file comment length                  2 bytes
    FieldMustBeEmpty    // 8 - .ZIP file comment                         (variable size)
    >
{
public:
    void Validate() override
    {
        if (Field<1>().value != Field<2>().value) { throw std::invalid_argument("field missmatch"); }
        if (Field<3>().value != Field<4>().value) { throw std::invalid_argument("field missmatch"); }

        m_isZip64 = (0xFFFF == Field<2>().value);
        if (Field<3>().value != 0 && Field<3>().value != 0xFFFF)
        {   m_archiveHasZip64Locator = false;
        }
        if(m_archiveHasZip64Locator)
        {
            if((Field<5>().value != 0) && (Field<5>().value != 0xFFFFFFFF)) {
                throw std::invalid_argument("unsupported size of central directory");
            }
            if ((Field<6>().value != 0) && (Field<6>().value != 0xFFFFFFFF)) {
                throw std::invalid_argument("unsupported offset of start of central directory");
            }
        }
    }
    
    EndOfCentralDirectoryRecord()
    {
        SetSignature(Signatures::EndOfCentralDirectory);
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
    
    inline bool GetArchiveHasZip64Locator()                      const noexcept { return m_archiveHasZip64Locator; }
    inline bool GetIsZip64()                                     const noexcept { return m_isZip64; }
    inline std::uint64_t GetNumberOfCentralDirectoryEntries()    const noexcept { return static_cast<std::uint64_t>(Field<3>().value); }
    inline std::uint64_t GetStartOfCentralDirectory()            const noexcept { return static_cast<std::uint64_t>(Field<6>().value); }

private:
    bool m_isZip64 = false;
    bool m_archiveHasZip64Locator = true;
    
    inline void SetSignature(std::uint32_t value)                      noexcept { Field<0>().value = value; }
    inline void SetNumberOfDisk(std::uint16_t value)                   noexcept { Field<1>().value = value; }
    inline void SetDiskStart(std::uint16_t value)                      noexcept { Field<2>().value = value; }
    inline void SetTotalNumberOfEntries(std::uint16_t value)           noexcept { Field<3>().value = value; }
    inline void SetTotalEntriesInCentralDirectory(std::uint16_t value) noexcept { Field<4>().value = value; }
    inline void SetSizeOfCentralDirectory(std::uint32_t value)         noexcept { Field<5>().value = value; }
    inline void SetOffsetOfCentralDirectory(std::uint32_t value)       noexcept { Field<6>().value = value; }
    inline std::uint32_t GetOffsetOfCentralDirectory()           const noexcept { return Field<6>().value;  }
    inline void SetCommentLength(std::uint16_t value)                  noexcept { Field<7>().value = value; }
};//class EndOfCentralDirectoryRecord

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

ZipObject::ZipObject(IMSIXFactory* appxFactory, const ComPtr<IStream>& stream) : m_factory(appxFactory), m_stream(stream)
{   // Confirm that the file IS the correct format
    EndCentralDirectoryRecord endCentralDirectoryRecord;
    LARGE_INTEGER pos = {0};
    pos.QuadPart = -1 * endCentralDirectoryRecord.Size();
    ThrowHrIfFailed(m_stream->Seek(pos, StreamBase::Reference::END, nullptr));
    endCentralDirectoryRecord.Read(m_stream.Get());

    // find where the zip central directory exists.
    std::uint64_t offsetStartOfCD = 0;
    std::uint64_t totalNumberOfEntries = 0;
    Zip64EndOfCentralDirectoryLocator zip64Locator(m_stream.Get());        
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
        Zip64EndOfCentralDirectoryRecord zip64EndOfCentralDirectory(m_stream);
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
        auto centralFileHeader = std::make_shared<CentralDirectoryFileHeader>(endCentralDirectoryRecord.GetIsZip64(), m_stream.Get());
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
            m_factory,
            localFileHeader->GetCompressionType() == CompressionType::Deflate,
            centralFileHeader.second->GetRelativeOffsetOfLocalHeader() + localFileHeader->Size(),
            localFileHeader->GetCompressedSize(),                
            m_stream
            );

        if (localFileHeader->GetCompressionType() == CompressionType::Deflate)
        {
            fileStream = ComPtr<IStream>::Make<InflateStream>(fileStream.Get(), localFileHeader->GetUncompressedSize());
        }

        m_streams.insert(std::make_pair(centralFileHeader.second->GetFileName(), std::move(fileStream)));
    }
} // ZipObject::ZipObject
} // namespace MSIX
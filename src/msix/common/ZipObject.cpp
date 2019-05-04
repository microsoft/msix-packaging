//
//  Copyright (C) 2017 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
// 
#include "Exceptions.hpp"
#include "StreamBase.hpp"
#include "ObjectBase.hpp"
#include "ComHelper.hpp"
#include "ZipObject.hpp"
#include "VectorStream.hpp"
#include "MsixFeatureSelector.hpp"

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

// Hat tip to the people at Facebook.  Timestamp for files in ZIP archive 
// format held constant to make pack/unpack deterministic
enum class MagicNumbers : std::uint16_t
{
    FileTime = 0x6B60,  // kudos to those know this
    FileDate = 0xA2B1,  // :)
};

// if any of these are set, then fail.
constexpr static const GeneralPurposeBitFlags UnsupportedFlagsMask =
    GeneralPurposeBitFlags::UNSUPPORTED_0  |
    GeneralPurposeBitFlags::UNSUPPORTED_6  |
    GeneralPurposeBitFlags::UNSUPPORTED_12 |
    GeneralPurposeBitFlags::UNSUPPORTED_13 |
    GeneralPurposeBitFlags::UNSUPPORTED_14 |
    GeneralPurposeBitFlags::UNSUPPORTED_15;

// Note: all fields stored in Intel low-byte/high-byte order.

// FROM APPNOTE.TXT section 4.5.3:
// If one of the size or offset fields in the Local or Central directory
// record is too small to hold the required data, a Zip64 extended information 
// record is created.  The order of the fields in the zip64 extended 
// information record is fixed, but the fields MUST only appear if the 
// corresponding Local or Central directory record field is set to 0xFFFF 
// or 0xFFFFFFFF.
//////////////////////////////////////////////////////////////////////////////////////////////
//                                  Zip64ExtendedInformation                                //
//////////////////////////////////////////////////////////////////////////////////////////////
Zip64ExtendedInformation::Zip64ExtendedInformation()
{
    SetSignature(static_cast<std::uint16_t>(HeaderIDs::Zip64ExtendedInfo));
    // Should be 0x18, but do it this way if we end up field 5
    SetSize(static_cast<std::uint16_t>(Size() - Field<0>().Size() - Field<1>().Size()));
}

void Zip64ExtendedInformation::Initialize(std::uint64_t uncompressedSize, std::uint64_t compressedSize, std::uint64_t relativeOffset)
{
    THROW_IF_PACK_NOT_ENABLED
    SetUncompressedSize(uncompressedSize);
    SetCompressedSize(compressedSize);
    SetRelativeOffsetOfLocalHeader(relativeOffset);
}

void Zip64ExtendedInformation::Read(const ComPtr<IStream>& stream, ULARGE_INTEGER start)
{
    StreamBase::Read(stream, &Field<0>().value);
    Meta::ExactValueValidation<std::uint32_t>(Field<0>().value, static_cast<std::uint32_t>(HeaderIDs::Zip64ExtendedInfo));

    StreamBase::Read(stream, &Field<1>().value);
    Meta::OnlyEitherValueValidation<std::uint32_t>(Field<1>().value, 24, 28);
        
    StreamBase::Read(stream, &Field<2>().value);
    StreamBase::Read(stream, &Field<3>().value);

    StreamBase::Read(stream, &Field<4>().value);
    ThrowErrorIfNot(Error::ZipBadExtendedData, Field<4>().value < start.QuadPart, "invalid relative header offset");
}

//////////////////////////////////////////////////////////////////////////////////////////////
//                              CentralDirectoryFileHeader                                  //
//////////////////////////////////////////////////////////////////////////////////////////////
CentralDirectoryFileHeader::CentralDirectoryFileHeader()
{
    SetSignature(static_cast<std::uint32_t>(Signatures::CentralFileHeader));
    SetVersionMadeBy(static_cast<std::uint16_t>(ZipVersions::Zip64FormatExtension));
    SetVersionNeededToExtract(static_cast<std::uint16_t>(ZipVersions::Zip64FormatExtension)); // we always do zip64
    SetGeneralPurposeBitFlags(static_cast<std::uint16_t>(GeneralPurposeBitFlags::GeneralPurposeBit));
    SetLastModFileDate(static_cast<std::uint16_t>(MagicNumbers::FileDate)); // TODO: figure out how to convert to msdos time
    SetLastModFileTime(static_cast<std::uint16_t>(MagicNumbers::FileTime));
    SetCompressedSize(std::numeric_limits<std::uint32_t>::max()); // always use zip64
    SetUncompressedSize(std::numeric_limits<std::uint32_t>::max()); // always use zip64
    SetExtraFieldLength(0);
    SetFileCommentLength(0);
    SetDiskNumberStart(0);
    SetInternalFileAttributes(0);
    SetExternalFileAttributes(0);
    SetRelativeOffsetOfLocalHeader(std::numeric_limits<std::uint32_t>::max()); // always use zip64
}

void CentralDirectoryFileHeader::Initialize(std::string& name, std::uint32_t crc, std::uint64_t compressedSize, std::uint64_t uncompressedSize,
    std::uint64_t relativeOffset,  std::uint16_t compressionMethod)
{
    THROW_IF_PACK_NOT_ENABLED
    SetCompressionMethod(compressionMethod);
    SetCrc(crc);
    SetFileName(name);
    SetExtraField(compressedSize, uncompressedSize, relativeOffset);
}

void CentralDirectoryFileHeader::Read(const ComPtr<IStream>& stream, bool isZip64)
{
    m_isZip64 = isZip64;
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
    if (m_isZip64)
    {
        ThrowErrorIf(Error::ZipCentralDirectoryHeader, (Field<16>().value != 0xFFFFFFFF), "invalid zip64 local header offset");
    }
    else
    {
        ThrowErrorIf(Error::ZipCentralDirectoryHeader, (Field<16>().value >= pos.QuadPart), "invalid relative header offset");
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
        ThrowErrorIfNot(Error::ZipCentralDirectoryHeader, (Field<18>().Size() >= m_extendedInfo.Size()), "Unexpected extended info size");
        m_extendedInfo.Read(vectorStream.Get(), pos);
    }

    if (Field<19>().Size())
    {
        ThrowHrIfFailed(stream->Read(reinterpret_cast<void*>(Field<19>().value.data()), static_cast<ULONG>(Field<19>().Size()), nullptr));
    }
}

//////////////////////////////////////////////////////////////////////////////////////////////
//                                  LocalFileHeader                                         //
//////////////////////////////////////////////////////////////////////////////////////////////
LocalFileHeader::LocalFileHeader()
{
    SetSignature(static_cast<std::uint32_t>(Signatures::LocalFileHeader));
    SetVersionNeededToExtract(static_cast<std::uint16_t>(ZipVersions::Zip64FormatExtension)); // always zip64
    SetGeneralPurposeBitFlags(static_cast<std::uint16_t>(GeneralPurposeBitFlags::GeneralPurposeBit));
    SetLastModFileTime(static_cast<std::uint16_t>(MagicNumbers::FileTime)); // TODO: figure out how to convert to msdos time
    SetLastModFileDate(static_cast<std::uint16_t>(MagicNumbers::FileDate));
    SetCrc(0);
    SetCompressedSize(0);
    SetUncompressedSize(0);
    SetExtraFieldLength(0);
}

void LocalFileHeader::Initialize(std::string& name, bool isCompressed)
{
    THROW_IF_PACK_NOT_ENABLED
    auto compressMethod = (isCompressed) ? CompressionType::Deflate : CompressionType::Store; 
    SetCompressionMethod(static_cast<std::uint16_t>(compressMethod));
    SetFileName(name);
}

void LocalFileHeader::Read(const ComPtr<IStream> &stream, CentralDirectoryFileHeader& directoryEntry)
{
    StreamBase::Read(stream, &Field<0>().value);
    Meta::ExactValueValidation<std::uint32_t>( Field<0>().value, static_cast<std::uint32_t>(Signatures::LocalFileHeader));

    StreamBase::Read(stream, &Field<1>().value);
    Meta::OnlyEitherValueValidation<std::uint16_t>(Field<1>().value, static_cast<std::uint16_t>(ZipVersions::Zip32DefaultVersion),
                                              static_cast<std::uint16_t>(ZipVersions::Zip64FormatExtension));

    StreamBase::Read(stream, &Field<2>().value);
    ThrowErrorIfNot(Error::ZipLocalFileHeader, ((Field<2>().value & static_cast<std::uint16_t>(UnsupportedFlagsMask)) == 0), "unsupported flag(s) specified");
    ThrowErrorIfNot(Error::ZipLocalFileHeader, (IsGeneralPurposeBitSet() == directoryEntry.IsGeneralPurposeBitSet()), "inconsistent general purpose bits specified");

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

//////////////////////////////////////////////////////////////////////////////////////////////
//                              Zip64EndOfCentralDirectoryRecord                            //
//////////////////////////////////////////////////////////////////////////////////////////////
Zip64EndOfCentralDirectoryRecord::Zip64EndOfCentralDirectoryRecord()
{
    SetSignature(static_cast<std::uint32_t>(Signatures::Zip64EndOfCD));
    // We not use Field<10> so there's no variable data
    SetSizeOfZip64CDRecord(static_cast<std::uint64_t>(Size() - Field<0>().Size() - Field<1>().Size()));
    SetVersionMadeBy(static_cast<std::uint16_t>(ZipVersions::Zip64FormatExtension));
    SetVersionNeededToExtract(static_cast<std::uint16_t>(ZipVersions::Zip64FormatExtension));
    SetNumberOfThisDisk(0);
    SetNumberOfTheDiskWithStartOfCD(0);
}

void Zip64EndOfCentralDirectoryRecord::Read(const ComPtr<IStream>& stream)
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
    ThrowErrorIfNot(Error::Zip64EOCDRecord, (Field<7>().value == GetTotalNumberOfEntries()), "invalid total number of entries");

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

//////////////////////////////////////////////////////////////////////////////////////////////
//                          Zip64EndOfCentralDirectoryLocator                               //
//////////////////////////////////////////////////////////////////////////////////////////////
Zip64EndOfCentralDirectoryLocator::Zip64EndOfCentralDirectoryLocator()
{
    SetSignature(static_cast<std::uint32_t>(Signatures::Zip64EndOfCDLocator));
    SetNumberOfDisk(0);
    SetTotalNumberOfDisks(1);
}

void Zip64EndOfCentralDirectoryLocator::Read(const ComPtr<IStream>& stream)
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

//////////////////////////////////////////////////////////////////////////////////////////////
//                              EndOfCentralDirectoryRecord                                 //
//////////////////////////////////////////////////////////////////////////////////////////////
EndCentralDirectoryRecord::EndCentralDirectoryRecord()
{
    SetSignature(static_cast<std::uint32_t>(Signatures::EndOfCentralDirectory));
    SetNumberOfDisk(0);
    SetDiskStart(0);
    // always use zip64
    SetTotalNumberOfEntries(std::numeric_limits<std::uint16_t>::max());
    SetTotalEntriesInCentralDirectory(std::numeric_limits<std::uint16_t>::max());
    SetSizeOfCentralDirectory(std::numeric_limits<std::uint32_t>::max());
    SetOffsetOfCentralDirectory(std::numeric_limits<std::uint32_t>::max());
    SetCommentLength(0);
}

void EndCentralDirectoryRecord::Read(const ComPtr<IStream>& stream)
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

} // namespace MSIX

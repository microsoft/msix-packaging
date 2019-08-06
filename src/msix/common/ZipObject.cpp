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
#include "ZipFileStream.hpp"
#include "InflateStream.hpp"
#include "RangeStream.hpp"
#include "TimeHelpers.hpp"

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
    SetSize(NonOptionalSize);
}

void Zip64ExtendedInformation::Read(const ComPtr<IStream>& stream, ULARGE_INTEGER start, uint32_t uncompressedSize, uint32_t compressedSize, uint32_t offset, uint16_t disk)
{
    StreamBase::Read(stream, &Field<0>());
    Meta::ExactValueValidation<std::uint32_t>(Field<0>(), static_cast<std::uint32_t>(HeaderIDs::Zip64ExtendedInfo));

    StreamBase::Read(stream, &Field<1>());
    // The incoming stream will be just the extended info, and our size should match it minus the fixed bytes
    Meta::ExactValueValidation<std::uint32_t>(Field<1>(), static_cast<uint32_t>(stream.As<IStreamInternal>()->GetSize() - NonOptionalSize));

    if (IsValueInExtendedInfo(uncompressedSize))
    {
        StreamBase::Read(stream, &Field<2>());
    }

    if (IsValueInExtendedInfo(compressedSize))
    {
        StreamBase::Read(stream, &Field<3>());
    }

    if (IsValueInExtendedInfo(offset))
    {
        StreamBase::Read(stream, &Field<4>());
        ThrowErrorIfNot(Error::ZipBadExtendedData, Field<4>().get() < start.QuadPart, "invalid relative header offset");
    }

    if (IsValueInExtendedInfo(disk))
    {
        StreamBase::Read(stream, &Field<5>());
    }
}

//////////////////////////////////////////////////////////////////////////////////////////////
//                              CentralDirectoryFileHeader                                  //
//////////////////////////////////////////////////////////////////////////////////////////////
CentralDirectoryFileHeader::CentralDirectoryFileHeader()
{
    SetSignature(static_cast<std::uint32_t>(Signatures::CentralFileHeader));
    SetVersionMadeBy(static_cast<std::uint16_t>(ZipVersions::Zip64FormatExtension));
    SetVersionNeededToExtract(static_cast<std::uint16_t>(ZipVersions::Zip32DefaultVersion));
    SetGeneralPurposeBitFlags(0);

    MsDosDateAndTime msDosDateAndTime;
    SetLastModFileDate(msDosDateAndTime.GetDosDate());
    SetLastModFileTime(msDosDateAndTime.GetDosTime());

    SetCompressedSize(0);
    SetUncompressedSize(0);
    SetExtraFieldLength(0);
    SetFileCommentLength(0);
    SetDiskNumberStart(0);
    SetInternalFileAttributes(0);
    SetExternalFileAttributes(0);
    SetRelativeOffsetOfLocalHeader(0);
}

void CentralDirectoryFileHeader::SetData(const std::string& name, std::uint32_t crc, std::uint64_t compressedSize, std::uint64_t uncompressedSize,
    std::uint64_t relativeOffset,  std::uint16_t compressionMethod, bool forceDataDescriptor)
{
    THROW_IF_PACK_NOT_ENABLED
    SetCompressionMethod(compressionMethod);
    SetCrc(crc);
    SetFileName(name);

    // Set these values that may end up in the extra field info; then make sure to update it
    SetCompressedSize(compressedSize);
    SetUncompressedSize(uncompressedSize);
    SetRelativeOffsetOfLocalHeader(relativeOffset);
    UpdateExtraField();

    if (forceDataDescriptor ||
        compressedSize > MaxSizeToNotUseDataDescriptor ||
        uncompressedSize > MaxSizeToNotUseDataDescriptor)
    {
        SetGeneralPurposeBitFlags(static_cast<std::uint16_t>(GeneralPurposeBitFlags::DataDescriptor));
        SetVersionNeededToExtract(static_cast<std::uint16_t>(ZipVersions::Zip64FormatExtension));
    }
}

void CentralDirectoryFileHeader::Read(const ComPtr<IStream>& stream, bool isZip64)
{
    m_isZip64 = isZip64;
    StreamBase::Read(stream, &Field<0>());
    Meta::ExactValueValidation<std::uint32_t>(Field<0>(), static_cast<std::uint32_t>(Signatures::CentralFileHeader));

    StreamBase::Read(stream, &Field<1>());
    StreamBase::Read(stream, &Field<2>());

    StreamBase::Read(stream, &Field<3>());
    ThrowErrorIfNot(Error::ZipCentralDirectoryHeader,
        0 == (Field<3>().get() & static_cast<std::uint16_t>(UnsupportedFlagsMask)),
        "unsupported flag(s) specified");

    StreamBase::Read(stream, &Field<4>());
    Meta::OnlyEitherValueValidation<std::uint16_t>(Field<4>(),  static_cast<std::uint16_t>(CompressionType::Deflate),
        static_cast<std::uint16_t>(CompressionType::Store));

    StreamBase::Read(stream, &Field<5>());
    StreamBase::Read(stream, &Field<6>());
    StreamBase::Read(stream, &Field<7>());
    StreamBase::Read(stream, &Field<8>());
    StreamBase::Read(stream, &Field<9>());

    StreamBase::Read(stream, &Field<10>());
    ThrowErrorIfNot(Error::ZipCentralDirectoryHeader, (Field<10>().get() != 0), "unsupported file name size");
    if (Field<10>().get() != 0) {Field<17>().get().resize(Field<10>().get(), 0); }

    StreamBase::Read(stream, &Field<11>());
    if (Field<11>().get() != 0) { Field<18>().get().resize(Field<11>().get(), 0); }

    StreamBase::Read(stream, &Field<12>());
    Meta::ExactValueValidation<std::uint32_t>(Field<12>(), 0);

    StreamBase::Read(stream, &Field<13>());
    Meta::ExactValueValidation<std::uint32_t>(Field<13>(), 0);

    StreamBase::Read(stream, &Field<14>());

    StreamBase::Read(stream, &Field<15>());
    StreamBase::Read(stream, &Field<16>());
    ULARGE_INTEGER pos = {0};
    ThrowHrIfFailed(stream->Seek({0}, StreamBase::Reference::CURRENT, &pos));
    if (!m_isZip64 || !IsValueInExtendedInfo(Field<16>()))
    {
        ThrowErrorIf(Error::ZipCentralDirectoryHeader, (Field<16>().get() >= pos.QuadPart), "invalid relative header offset");
    }

    if (Field<17>().Size())
    {
        ThrowHrIfFailed(stream->Read(reinterpret_cast<void*>(Field<17>().get().data()), static_cast<ULONG>(Field<17>().Size()), nullptr));
    }

    if (Field<18>().Size())
    {
        ThrowHrIfFailed(stream->Read(reinterpret_cast<void*>(Field<18>().get().data()), static_cast<ULONG>(Field<18>().Size()), nullptr));
    }
    // Only process for Zip64ExtendedInformation
    if (Field<18>().Size() > 2 && Field<18>().get()[0] == 0x01 && Field<18>().get()[1] == 0x00)
    {
        LARGE_INTEGER zero = {0};
        ThrowHrIfFailed(stream->Seek(zero, StreamBase::Reference::CURRENT, &pos));
        auto vectorStream = ComPtr<IStream>::Make<VectorStream>(&Field<18>());
        m_extendedInfo.Read(vectorStream.Get(), pos, Field<9>(), Field<8>(), Field<16>(), Field<13>());
    }

    if (Field<19>().Size())
    {
        ThrowHrIfFailed(stream->Read(reinterpret_cast<void*>(Field<19>().get().data()), static_cast<ULONG>(Field<19>().Size()), nullptr));
    }
}

//////////////////////////////////////////////////////////////////////////////////////////////
//                                  LocalFileHeader                                         //
//////////////////////////////////////////////////////////////////////////////////////////////
LocalFileHeader::LocalFileHeader()
{
    SetSignature(static_cast<std::uint32_t>(Signatures::LocalFileHeader));
    SetVersionNeededToExtract(static_cast<std::uint16_t>(ZipVersions::Zip64FormatExtension)); // deafult to zip64
    SetGeneralPurposeBitFlags(static_cast<std::uint16_t>(GeneralPurposeBitFlags::DataDescriptor));

    MsDosDateAndTime msDosDateAndTime;
    SetLastModFileDate(msDosDateAndTime.GetDosDate());
    SetLastModFileTime(msDosDateAndTime.GetDosTime());

    SetCrc(0);
    SetCompressedSize(0);
    SetUncompressedSize(0);
    SetExtraFieldLength(0);
}

void LocalFileHeader::SetData(const std::string& name, bool isCompressed)
{
    THROW_IF_PACK_NOT_ENABLED
    auto compressMethod = (isCompressed) ? CompressionType::Deflate : CompressionType::Store; 
    SetCompressionMethod(static_cast<std::uint16_t>(compressMethod));
    SetFileName(name);
}

void LocalFileHeader::SetData(std::uint32_t crc, std::uint64_t compressedSize, std::uint64_t uncompressedSize)
{
    THROW_IF_PACK_NOT_ENABLED
    SetVersionNeededToExtract(static_cast<std::uint16_t>(ZipVersions::Zip32DefaultVersion));
    SetGeneralPurposeBitFlags(static_cast<std::uint16_t>(GetGeneralPurposeBitFlags() & ~GeneralPurposeBitFlags::DataDescriptor));
    SetCrc(crc);
    SetCompressedSize(static_cast<uint32_t>(compressedSize));
    SetUncompressedSize(static_cast<uint32_t>(uncompressedSize));
}

void LocalFileHeader::Read(const ComPtr<IStream> &stream, CentralDirectoryFileHeader& directoryEntry)
{
    StreamBase::Read(stream, &Field<0>());
    Meta::ExactValueValidation<std::uint32_t>(Field<0>(), static_cast<std::uint32_t>(Signatures::LocalFileHeader));

    StreamBase::Read(stream, &Field<1>());
    Meta::OnlyEitherValueValidation<std::uint16_t>(Field<1>(), static_cast<std::uint16_t>(ZipVersions::Zip32DefaultVersion),
                                              static_cast<std::uint16_t>(ZipVersions::Zip64FormatExtension));

    StreamBase::Read(stream, &Field<2>());
    ThrowErrorIfNot(Error::ZipLocalFileHeader, ((Field<2>().get() & static_cast<std::uint16_t>(UnsupportedFlagsMask)) == 0), "unsupported flag(s) specified");
    ThrowErrorIfNot(Error::ZipLocalFileHeader, (IsDataDescriptorBitSet() == directoryEntry.IsDataDescriptorBitSet()), "inconsistent general purpose bits specified");

    StreamBase::Read(stream, &Field<3>());
    Meta::OnlyEitherValueValidation<std::uint16_t>(Field<3>(), static_cast<std::uint16_t>(CompressionType::Deflate),
                                              static_cast<std::uint16_t>(CompressionType::Store));

    StreamBase::Read(stream, &Field<4>());
    StreamBase::Read(stream, &Field<5>());
    StreamBase::Read(stream, &Field<6>());
    ThrowErrorIfNot(Error::ZipLocalFileHeader, (!IsDataDescriptorBitSet() || (Field<6>().get() == 0)), "Invalid Zip CRC");

    StreamBase::Read(stream, &Field<7>());
    ThrowErrorIfNot(Error::ZipLocalFileHeader, (!IsDataDescriptorBitSet() || (Field<7>().get() == 0)), "Invalid Zip compressed size");

    StreamBase::Read(stream, &Field<8>());

    StreamBase::Read(stream, &Field<9>());
    ThrowErrorIfNot(Error::ZipLocalFileHeader, (Field<9>().get() != 0), "unsupported file name size");
    Field<11>().get().resize(GetFileNameLength(), 0);

    StreamBase::Read(stream, &Field<10>());
    // Even if we don't validate them, we need to read the extra field
    if (Field<10>().get() != 0) {Field<12>().get().resize(Field<10>().get(), 0); }

    if (Field<11>().Size())
    {
        ThrowHrIfFailed(stream->Read(reinterpret_cast<void*>(Field<11>().get().data()), static_cast<ULONG>(Field<11>().Size()), nullptr));
    }

    if (Field<12>().Size())
    {
        ThrowHrIfFailed(stream->Read(reinterpret_cast<void*>(Field<12>().get().data()), static_cast<ULONG>(Field<12>().Size()), nullptr));
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

void Zip64EndOfCentralDirectoryRecord::SetData(std::uint64_t numCentralDirs, std::uint64_t sizeCentralDir, std::uint64_t offsetStartCentralDirectory)
{
    SetTotalNumberOfEntriesDisk(numCentralDirs);
    SetSizeOfCD(sizeCentralDir);
    SetOffsetfStartOfCD(offsetStartCentralDirectory);
}

void Zip64EndOfCentralDirectoryRecord::Read(const ComPtr<IStream>& stream)
{
    StreamBase::Read(stream, &Field<0>());
    Meta::ExactValueValidation<std::uint32_t>(Field<0>(), static_cast<std::uint32_t>(Signatures::Zip64EndOfCD));

    StreamBase::Read(stream, &Field<1>());
    //4.3.14.1 The value stored into the "size of zip64 end of central
    //    directory record" should be the size of the remaining
    //    record and should not include the leading 12 bytes.
    ThrowErrorIfNot(Error::Zip64EOCDRecord, (Field<1>().get() == (this->Size() - 12)), "invalid size of zip64 EOCD");

    StreamBase::Read(stream, &Field<2>());
    Meta::ExactValueValidation<std::uint16_t>(Field<2>(), static_cast<std::uint16_t>(ZipVersions::Zip64FormatExtension));

    StreamBase::Read(stream, &Field<3>());
    Meta::ExactValueValidation<std::uint16_t>(Field<3>(), static_cast<std::uint16_t>(ZipVersions::Zip64FormatExtension));

    StreamBase::Read(stream, &Field<4>());
    Meta::ExactValueValidation<std::uint32_t>(Field<4>(), 0);

    StreamBase::Read(stream, &Field<5>());
    Meta::ExactValueValidation<std::uint32_t>(Field<5>(), 0);

    StreamBase::Read(stream, &Field<6>());
    Meta::NotValueValidation<std::uint64_t>(Field<6>(), 0);

    StreamBase::Read(stream, &Field<7>());
    Meta::NotValueValidation<std::uint64_t>(Field<7>(), 0);
    ThrowErrorIfNot(Error::Zip64EOCDRecord, (Field<7>().get() == GetTotalNumberOfEntries()), "invalid total number of entries");

    ULARGE_INTEGER pos = {0};
    ThrowHrIfFailed(stream->Seek({0}, StreamBase::Reference::CURRENT, &pos));
    StreamBase::Read(stream, &Field<8>());
    ThrowErrorIfNot(Error::Zip64EOCDRecord, ((Field<8>().get() != 0) && (Field<8>().get() < pos.QuadPart)), "invalid size of central directory");

    ThrowHrIfFailed(stream->Seek({0}, StreamBase::Reference::CURRENT, &pos));
    StreamBase::Read(stream, &Field<9>());
    ThrowErrorIfNot(Error::Zip64EOCDRecord, ((Field<9>().get() != 0) && (Field<9>().get() < pos.QuadPart)), "invalid size of central directory");

    if (Field<10>().Size())
    {
        ThrowHrIfFailed(stream->Read(reinterpret_cast<void*>(Field<10>().get().data()), static_cast<ULONG>(Field<10>().Size()), nullptr));
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

void Zip64EndOfCentralDirectoryLocator::SetData(std::uint64_t zip64EndCdrOffset)
{
    SetRelativeOffset(zip64EndCdrOffset);
}

void Zip64EndOfCentralDirectoryLocator::Read(const ComPtr<IStream>& stream)
{
    StreamBase::Read(stream, &Field<0>());
    Meta::ExactValueValidation<std::uint32_t>(Field<0>(), static_cast<std::uint32_t>(Signatures::Zip64EndOfCDLocator));

    StreamBase::Read(stream, &Field<1>());
    Meta::ExactValueValidation<std::uint32_t>(Field<1>(), 0);

    ULARGE_INTEGER pos = {0};
    StreamBase::Read(stream, &Field<2>());
    ThrowHrIfFailed(stream->Seek({0}, StreamBase::Reference::CURRENT, &pos));
    ThrowErrorIfNot(Error::Zip64EOCDLocator, ((Field<2>().get() != 0) && (Field<2>().get() < pos.QuadPart)), "Invalid relative offset");

    StreamBase::Read(stream, &Field<3>());
    Meta::ExactValueValidation<std::uint32_t>(Field<3>(), 1);
}

//////////////////////////////////////////////////////////////////////////////////////////////
//                              EndOfCentralDirectoryRecord                                 //
//////////////////////////////////////////////////////////////////////////////////////////////
EndCentralDirectoryRecord::EndCentralDirectoryRecord()
{
    SetSignature(static_cast<std::uint32_t>(Signatures::EndOfCentralDirectory));
    // Always use zip64
    SetNumberOfDisk(0);
    SetDiskStart(0);
    SetTotalNumberOfEntries(std::numeric_limits<std::uint16_t>::max());
    SetTotalEntriesInCentralDirectory(std::numeric_limits<std::uint16_t>::max());
    SetSizeOfCentralDirectory(std::numeric_limits<std::uint32_t>::max());
    SetOffsetOfCentralDirectory(std::numeric_limits<std::uint32_t>::max());
    SetCommentLength(0);
}

void EndCentralDirectoryRecord::Read(const ComPtr<IStream>& stream)
{
    StreamBase::Read(stream, &Field<0>());
    Meta::ExactValueValidation<std::uint32_t>(Field<0>(), static_cast<std::uint32_t>(Signatures::EndOfCentralDirectory));

    StreamBase::Read(stream, &Field<1>());
    Meta::OnlyEitherValueValidation<std::uint32_t>(Field<1>(), 0, 0xFFFF);

    StreamBase::Read(stream, &Field<2>());
    Meta::OnlyEitherValueValidation<std::uint32_t>(Field<2>(), 0, 0xFFFF);
    ThrowErrorIf(Error::ZipEOCDRecord, (Field<1>().get() != Field<2>().get()), "field missmatch");

    StreamBase::Read(stream, &Field<3>());
    StreamBase::Read(stream, &Field<4>());
    ThrowErrorIf(Error::ZipEOCDRecord, (Field<3>().get() != Field<4>().get()), "field missmatch");

    StreamBase::Read(stream, &Field<5>());
    StreamBase::Read(stream, &Field<6>());

    m_isZip64 = (
        IsValueInExtendedInfo(Field<1>()) ||
        IsValueInExtendedInfo(Field<2>()) ||
        IsValueInExtendedInfo(Field<3>()) ||
        IsValueInExtendedInfo(Field<4>()) ||
        IsValueInExtendedInfo(Field<5>()) ||
        IsValueInExtendedInfo(Field<6>()));

    if (m_isZip64)
    {
        ThrowErrorIf(Error::ZipEOCDRecord, ((Field<5>().get() != 0) && (Field<5>().get() != 0xFFFFFFFF)),
            "unsupported size of central directory");
        ThrowErrorIf(Error::ZipEOCDRecord, ((Field<6>().get() != 0) && (Field<6>().get() != 0xFFFFFFFF)),
            "unsupported offset of start of central directory");
    }

    StreamBase::Read(stream, &Field<7>());
    Meta::ExactValueValidation<std::uint32_t>(Field<7>(), 0);

    if (Field<8>().Size())
    {
       ThrowHrIfFailed(stream->Read(reinterpret_cast<void*>(Field<8>().get().data()), static_cast<ULONG>(Field<8>().Size()), nullptr));
    }
}

//////////////////////////////////////////////////////////////////////////////////////////////
//                                       ZipObject                                          //
//////////////////////////////////////////////////////////////////////////////////////////////
ZipObject::ZipObject(IStream* stream, bool readStream) : m_stream(stream)
{
    // Used by the writer to create an empty zip object
    if (!readStream)
    {
        return;
    }

    LARGE_INTEGER pos = { 0 };
    pos.QuadPart = m_endCentralDirectoryRecord.Size();
    pos.QuadPart *= -1;
    ThrowHrIfFailed(m_stream->Seek(pos, StreamBase::Reference::END, nullptr));
    m_endCentralDirectoryRecord.Read(m_stream.Get());

    // find where the zip central directory exists.
    std::uint64_t offsetStartOfCD = 0;
    std::uint64_t totalNumberOfEntries = 0;
    if (!m_endCentralDirectoryRecord.GetIsZip64())
    {
        offsetStartOfCD = m_endCentralDirectoryRecord.GetStartOfCentralDirectory();
        totalNumberOfEntries = m_endCentralDirectoryRecord.GetNumberOfCentralDirectoryEntries();
    }
    else
    {   // Make sure that we have a zip64 end of central directory locator
        pos.QuadPart = m_endCentralDirectoryRecord.Size() + m_zip64Locator.Size();
        pos.QuadPart *= -1;
        ThrowHrIfFailed(m_stream->Seek(pos, StreamBase::Reference::END, nullptr));
        m_zip64Locator.Read(m_stream.Get());

        // now read the end of zip central directory record
        pos.QuadPart = m_zip64Locator.GetRelativeOffset();
        ThrowHrIfFailed(m_stream->Seek(pos, StreamBase::Reference::START, nullptr));
        m_zip64EndOfCentralDirectory.Read(m_stream.Get());
        offsetStartOfCD = m_zip64EndOfCentralDirectory.GetOffsetStartOfCD();
        totalNumberOfEntries = m_zip64EndOfCentralDirectory.GetTotalNumberOfEntries();
    }

    // read the zip central directory
    pos.QuadPart = offsetStartOfCD;
    ThrowHrIfFailed(m_stream->Seek(pos, StreamBase::Reference::START, nullptr));
    for (std::uint32_t index = 0; index < totalNumberOfEntries; index++)
    {
        auto centralFileHeader = CentralDirectoryFileHeader();
        centralFileHeader.Read(m_stream.Get(), m_endCentralDirectoryRecord.GetIsZip64());
        // TODO: ensure that there are no collisions on name!
        m_centralDirectories.emplace_back(std::make_pair(centralFileHeader.GetFileName(), std::move(centralFileHeader)));
    }

    if (m_endCentralDirectoryRecord.GetIsZip64())
    {   // We should have no data between the end of the last central directory header and the start of the EoCD
        ULARGE_INTEGER uPos = { 0 };
        ThrowHrIfFailed(m_stream->Seek({ 0 }, StreamBase::Reference::CURRENT, &uPos));
        ThrowErrorIfNot(Error::ZipHiddenData, (uPos.QuadPart == m_zip64Locator.GetRelativeOffset()), "hidden data unsupported");
    }
}

// IStoreageObject
std::vector<std::string> ZipObject::GetFileNames(FileNameOptions)
{
    std::vector<std::string> result;
    for (const auto& cd : m_centralDirectories)
    {
        result.push_back(cd.first);
    }
    return result;
}

// ZipObject::GetFile has cache semantics. If not found on m_streams, get the file from the central directories.
// Not finding a file is non-fatal
ComPtr<IStream> ZipObject::GetFile(const std::string& fileName)
{
    auto result = m_streams.find(fileName);
    if (result == m_streams.end())
    {
        // Find the central directory item in question
        CentralDirectoryFileHeader* targetCDptr = nullptr;
        for (auto& cd : m_centralDirectories)
        {
            if (cd.first == fileName)
            {
                targetCDptr = &cd.second;
            }
        }
        if (!targetCDptr)
        {
            return {};
        }
        CentralDirectoryFileHeader& targetCD = *targetCDptr;

        LARGE_INTEGER pos = { 0 };
        pos.QuadPart = targetCD.GetRelativeOffsetOfLocalHeader();
        ThrowHrIfFailed(m_stream->Seek(pos, MSIX::StreamBase::Reference::START, nullptr));
        LocalFileHeader lfh = LocalFileHeader();
        lfh.Read(m_stream.Get(), targetCD);

        auto fileStream = ComPtr<IStream>::Make<ZipFileStream>(
            fileName,
            targetCD.GetCompressionMethod() == CompressionType::Deflate,
            targetCD.GetRelativeOffsetOfLocalHeader() + lfh.Size(),
            targetCD.GetCompressedSize(),
            m_stream.Get()
            );

        if (targetCD.GetCompressionMethod() == CompressionType::Deflate)
        {
            fileStream = ComPtr<IStream>::Make<InflateStream>(std::move(fileStream), targetCD.GetUncompressedSize());
        }
        ComPtr<IStream> result(fileStream);
        m_streams.insert(std::make_pair(fileName, std::move(fileStream)));
        return result;
    }
    return result->second;
}

std::string ZipObject::GetFileName()
{
    return m_stream.As<IStreamInternal>()->GetName();
}

ComPtr<IStream> ZipObject::GetStream()
{
    return m_stream;
}

MSIX::EndCentralDirectoryRecord& ZipObject::GetEndCentralDirectoryRecord()
{
    return m_endCentralDirectoryRecord;
}

MSIX::Zip64EndOfCentralDirectoryLocator& ZipObject::GetZip64Locator()
{
    return m_zip64Locator;
}

MSIX::Zip64EndOfCentralDirectoryRecord& ZipObject::GetZip64EndOfCentralDirectory()
{
    return m_zip64EndOfCentralDirectory;
}

std::vector<std::pair<std::string, CentralDirectoryFileHeader>>& ZipObject::GetCentralDirectories()
{
    return m_centralDirectories;
}

MSIX::ComPtr<IStream> ZipObject::GetEntireZipFileStream(const std::string& fileName)
{
    // Find the file in the CD
    CentralDirectoryFileHeader* targetCDptr = nullptr;
    for (auto& cd : m_centralDirectories)
    {
        if (cd.first == fileName)
        {
            targetCDptr = &cd.second;
        }
    }
    if (!targetCDptr)
    {
        return {};
    }
    CentralDirectoryFileHeader& targetCD = *targetCDptr;

    LARGE_INTEGER pos = { 0 };
    pos.QuadPart = targetCD.GetRelativeOffsetOfLocalHeader();
    ThrowHrIfFailed(m_stream->Seek(pos, MSIX::StreamBase::Reference::START, nullptr));
    LocalFileHeader lfh = LocalFileHeader();
    lfh.Read(m_stream.Get(), targetCD);

    uint64_t streamSize = lfh.Size() + targetCD.GetCompressedSize() + (lfh.IsDataDescriptorBitSet() ? DataDescriptor{}.Size() : 0);

    return ComPtr<IStream>::Make<RangeStream>(targetCD.GetRelativeOffsetOfLocalHeader(), streamSize, m_stream.Get());
}

} // namespace MSIX

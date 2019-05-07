//
//  Copyright (C) 2019 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
//
#pragma once

#include "Exceptions.hpp"
#include "ComHelper.hpp"
#include "StorageObject.hpp"
#include "AppxFactory.hpp"
#include "ObjectBase.hpp"

#include <vector>
#include <map>
#include <memory>

namespace MSIX {

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
    {
        return static_cast<GeneralPurposeBitFlags>(static_cast<uint16_t>(a) & static_cast<uint16_t>(b));
    }

    constexpr GeneralPurposeBitFlags operator |(GeneralPurposeBitFlags a, GeneralPurposeBitFlags b)
    {
        return static_cast<GeneralPurposeBitFlags>(static_cast<uint16_t>(a) | static_cast<uint16_t>(b));
    }

    enum class CompressionType : std::uint16_t
    {
        Store = 0,
        Deflate = 8,
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
        Zip64ExtendedInformation();

        void SetData(std::uint64_t uncompressedSize, std::uint64_t compressedSize, std::uint64_t relativeOffset);

        void Read(const ComPtr<IStream>& stream, ULARGE_INTEGER start);

        std::uint64_t GetUncompressedSize()            noexcept { return Field<2>().value; }
        std::uint64_t GetCompressedSize()              noexcept { return Field<3>().value; }
        std::uint64_t GetRelativeOffsetOfLocalHeader() noexcept { return Field<4>().value; }

    protected:
        void SetSignature(std::uint16_t value)        noexcept { Field<0>().value = value; }
        void SetSize(std::uint16_t value)             noexcept { Field<1>().value = value; }
        void SetUncompressedSize(std::uint64_t value) noexcept { Field<2>().value = value; }
        void SetCompressedSize(std::uint64_t value)   noexcept { Field<3>().value = value; }
        void SetRelativeOffsetOfLocalHeader(std::uint64_t value)        noexcept { Field<4>().value = value; }
    };

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
        Meta::FieldNBytes  //19 - file comment(variable size) NOT USED
        >
    {
    public:
        CentralDirectoryFileHeader();

        void SetData(std::string& name, std::uint32_t crc, std::uint64_t compressedSize,
            std::uint64_t uncompressedSize, std::uint64_t relativeOffset,  std::uint16_t compressionMethod);

        void Read(const ComPtr<IStream>& stream, bool isZip64);

        bool IsGeneralPurposeBitSet() noexcept
        {
            return ((static_cast<GeneralPurposeBitFlags>(Field<3>().value) & GeneralPurposeBitFlags::GeneralPurposeBit) == GeneralPurposeBitFlags::GeneralPurposeBit);
        }

        CompressionType GetCompressionMethod() noexcept { return static_cast<CompressionType>(Field<4>().value); }

        std::uint64_t GetCompressedSize() noexcept
        {
            if (m_isZip64)
            {
                return m_extendedInfo.GetCompressedSize();
            }
            return static_cast<std::uint64_t>(Field<8>().value);
        }

        std::uint64_t GetUncompressedSize() noexcept
        {
            if (m_isZip64)
            {
                return m_extendedInfo.GetUncompressedSize();
            }
            return static_cast<std::uint64_t>(Field<9>().value);
        }

        std::uint64_t GetRelativeOffsetOfLocalHeader() noexcept
        {
            if (m_isZip64)
            {
                return m_extendedInfo.GetRelativeOffsetOfLocalHeader();

            }
            return static_cast<std::uint64_t>(Field<16>().value);
        }

        std::string GetFileName()
        {
            auto data = Field<17>().value;
            return std::string(data.begin(), data.end());
        }

    protected:
        void SetSignature(std::uint32_t value)                   noexcept { Field<0>().value = value; }
        void SetVersionMadeBy(std::uint16_t value)               noexcept { Field<1>().value = value; }
        void SetVersionNeededToExtract(std::uint16_t value)      noexcept { Field<2>().value = value; }
        void SetGeneralPurposeBitFlags(std::uint16_t value)      noexcept { Field<3>().value = value; }
        void SetCompressionMethod(std::uint16_t value)           noexcept { Field<4>().value = value; }
        void SetLastModFileTime(std::uint16_t value)             noexcept { Field<5>().value = value; }
        void SetLastModFileDate(std::uint16_t value)             noexcept { Field<6>().value = value; }
        void SetCrc(std::uint32_t value)                         noexcept { Field<7>().value = value; }
        void SetCompressedSize(std::uint32_t value)              noexcept { Field<8>().value = value; }
        void SetUncompressedSize(std::uint32_t value)            noexcept { Field<9>().value = value; }
        void SetFileNameLength(std::uint16_t value)              noexcept { Field<10>().value = value; }
        void SetExtraFieldLength(std::uint16_t value)            noexcept { Field<11>().value = value; }
        void SetFileCommentLength(std::uint16_t value)           noexcept { Field<12>().value = value; }
        void SetDiskNumberStart(std::uint16_t value)             noexcept { Field<13>().value = value; }
        void SetInternalFileAttributes(std::uint16_t value)      noexcept { Field<14>().value = value; }
        void SetExternalFileAttributes(std::uint16_t value)      noexcept { Field<15>().value = value; }
        void SetRelativeOffsetOfLocalHeader(std::uint32_t value) noexcept { Field<16>().value = value; }
        void SetFileName(std::string& name)
        {
            auto data = Field<17>().value;
            data.resize(name.size());
            data.assign(name.begin(), name.end());
            SetFileNameLength(static_cast<std::uint16_t>(name.size()));
        }
        void SetExtraField(std::uint64_t compressedSize, std::uint64_t uncompressedSize, std::uint64_t relativeOffset)
        {
            m_extendedInfo.SetData(compressedSize, uncompressedSize, relativeOffset);
            SetExtraFieldLength(static_cast<std::uint16_t>(m_extendedInfo.Size()));
            Field<18>().value = m_extendedInfo.GetBytes();
        }

        Zip64ExtendedInformation m_extendedInfo;
        bool m_isZip64 = true;
    };

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
        Meta::FieldNBytes   // 12- extra field                     (variable size) NOT USED
    >
    {
    public:
        LocalFileHeader();

        void SetData(std::string& name, bool isCompressed);

        void Read(const ComPtr<IStream>& stream, CentralDirectoryFileHeader& directoryEntry);

        std::uint16_t GetCompressionMethod() noexcept { return Field<3>().value; }
        std::uint16_t GetFileNameLength()    noexcept { return Field<9>().value;  }
        std::string GetFileName()
        {
            auto data = Field<11>().value;
            return std::string(data.begin(), data.end());
        }

    protected:
        bool IsGeneralPurposeBitSet() noexcept
        {
            return ((static_cast<GeneralPurposeBitFlags>(Field<2>().value) & GeneralPurposeBitFlags::GeneralPurposeBit) == GeneralPurposeBitFlags::GeneralPurposeBit);
        }

        void SetSignature(std::uint32_t value)              noexcept { Field<0>().value = value; }
        void SetVersionNeededToExtract(std::uint16_t value) noexcept { Field<1>().value = value; }
        void SetGeneralPurposeBitFlags(std::uint16_t value) noexcept { Field<2>().value = value; }
        void SetCompressionMethod(std::uint16_t value)      noexcept { Field<3>().value = value; }
        void SetLastModFileTime(std::uint16_t value)        noexcept { Field<4>().value = value; }
        void SetLastModFileDate(std::uint16_t value)        noexcept { Field<5>().value = value; }
        void SetCrc(std::uint32_t value)                    noexcept { Field<6>().value = value; }
        void SetCompressedSize(std::uint32_t value)         noexcept { Field<7>().value = value; }
        void SetUncompressedSize(std::uint32_t value)       noexcept { Field<8>().value = value; }
        void SetFileNameLength(std::uint16_t value)         noexcept { Field<9>().value = value; }
        void SetExtraFieldLength(std::uint16_t value)       noexcept { Field<10>().value = value; }
        void SetFileName(std::string& name)
        {
            auto data = Field<11>().value;
            data.resize(name.size());
            data.assign(name.begin(), name.end());
            SetFileNameLength(static_cast<std::uint16_t>(name.size()));
        }
    };

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
        Meta::FieldNBytes  //10 - zip64 extensible data sector                                  (variable size) NOT USED
        >
    {
    public:
        Zip64EndOfCentralDirectoryRecord();

        void SetData(std::uint64_t numCentralDirs, std::uint64_t sizeCentralDir, std::uint64_t offsetStartCentralDirectory);

        void Read(const ComPtr<IStream>& stream);

        std::uint64_t GetTotalNumberOfEntries() noexcept { return Field<6>().value; }
        std::uint64_t GetOffsetStartOfCD()      noexcept { return Field<9>().value; }

    protected:
        void SetSignature(std::uint32_t value)                    noexcept { Field<0>().value = value; }
        void SetSizeOfZip64CDRecord(std::uint64_t value)          noexcept { Field<1>().value = value; }
        void SetVersionMadeBy(std::uint16_t value)                noexcept { Field<2>().value = value; }
        void SetVersionNeededToExtract(std::uint16_t value)       noexcept { Field<3>().value = value; }
        void SetNumberOfThisDisk(std::uint32_t value)             noexcept { Field<4>().value = value; }
        void SetNumberOfTheDiskWithStartOfCD(std::uint32_t value) noexcept { Field<5>().value = value; }
        void SetTotalNumberOfEntriesDisk(std::uint64_t value) noexcept
        {
            Field<6>().value = value;
            Field<7>().value = value;
        }
        void SetSizeOfCD(std::uint64_t value)         noexcept { Field<8>().value = value; }
        void SetOffsetfStartOfCD(std::uint64_t value) noexcept { Field<9>().value = value; }
    };

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
        Zip64EndOfCentralDirectoryLocator();

        void SetData(std::uint64_t zip64EndCdrOffset);

        void Read(const ComPtr<IStream>& stream);

        std::uint64_t GetRelativeOffset()           noexcept { return Field<2>().value; }

    protected:
        void SetSignature(std::uint32_t value)          noexcept { Field<0>().value = value; }
        void SetNumberOfDisk(std::uint32_t value)       noexcept { Field<1>().value = value; }
        void SetRelativeOffset(std::uint64_t value)     noexcept { Field<2>().value = value; }
        void SetTotalNumberOfDisks(std::uint32_t value) noexcept { Field<3>().value = value; }
    };

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
        EndCentralDirectoryRecord();

        void Read(const ComPtr<IStream>& stream);

        bool GetArchiveHasZip64Locator() noexcept { return m_archiveHasZip64Locator; }
        bool GetIsZip64()                noexcept { return m_isZip64; }

        std::uint64_t GetNumberOfCentralDirectoryEntries() noexcept { return static_cast<std::uint64_t>(Field<3>().value); }
        std::uint64_t GetStartOfCentralDirectory()         noexcept { return static_cast<std::uint64_t>(Field<6>().value); }

    protected:
        void SetSignature(std::uint32_t value)                      noexcept { Field<0>().value = value; }
        void SetNumberOfDisk(std::uint16_t value)                   noexcept { Field<1>().value = value; }
        void SetDiskStart(std::uint16_t value)                      noexcept { Field<2>().value = value; }
        void SetTotalNumberOfEntries(std::uint16_t value)           noexcept { Field<3>().value = value; }
        void SetTotalEntriesInCentralDirectory(std::uint16_t value) noexcept { Field<4>().value = value; }
        void SetSizeOfCentralDirectory(std::uint32_t value)         noexcept { Field<5>().value = value; }
        void SetOffsetOfCentralDirectory(std::uint32_t value)       noexcept { Field<6>().value = value; }
        void SetCommentLength(std::uint16_t value)                  noexcept { Field<7>().value = value; }

        bool m_isZip64 = true;
        bool m_archiveHasZip64Locator = true;
    };

    class ZipObject
    {
    public:
        ZipObject(const ComPtr<IStream>& stream) : m_stream(stream) {}
        ZipObject(const ComPtr<IStorageObject>& storageObject);

    protected:
        EndCentralDirectoryRecord m_endCentralDirectoryRecord;
        Zip64EndOfCentralDirectoryLocator m_zip64Locator;
        Zip64EndOfCentralDirectoryRecord m_zip64EndOfCentralDirectory;
        std::map<std::string, CentralDirectoryFileHeader> m_centralDirectories;
        ComPtr<IStream> m_stream;
    };
}

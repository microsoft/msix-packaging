//
//  Copyright (C) 2019 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
//
#pragma once

#include "Exceptions.hpp"
#include "ComHelper.hpp"
#include "ObjectBase.hpp"

#include <string>

namespace MSIX {

    enum class ZipVersions : std::uint16_t
    {
        Zip32DefaultVersion = 20,
        Zip64FormatExtension = 45,
    };

    enum class GeneralPurposeBitFlags : std::uint16_t
    {
        UNSUPPORTED_0 = 0x0001,         // Bit 0: If set, indicates that the file is encrypted.

        Deflate_MaxCompress = 0x0002,   // Maximum compression (-exx/-ex), otherwise, normal compression (-en)
        Deflate_FastCompress = 0x0004,  // Fast (-ef), if Max+Fast then SuperFast (-es) compression

        DataDescriptor = 0x0008,        // the field's crc-32 compressed and uncompressed sizes = 0 in the local header
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

    constexpr GeneralPurposeBitFlags operator ~(GeneralPurposeBitFlags a)
    {
        return static_cast<GeneralPurposeBitFlags>(~static_cast<uint16_t>(a));
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

    constexpr uint64_t MaxSizeToNotUseDataDescriptor = static_cast<uint64_t>(std::numeric_limits<std::uint32_t>::max() - 1);

    template <typename T>
    inline bool IsValueInExtendedInfo(T value) noexcept
    {
        return (value == std::numeric_limits<T>::max());
    }

    template <typename T>
    inline bool IsValueInExtendedInfo(const Meta::FieldBase<T>& field) noexcept
    {
        return IsValueInExtendedInfo(field.get());
    }

    class Zip64ExtendedInformation final : public Meta::StructuredObject<
        Meta::Field2Bytes,          // 0 - tag for the "extra" block type               2 bytes(0x0001)
        Meta::Field2Bytes,          // 1 - size of this "extra" block                   2 bytes
        Meta::OptionalField8Bytes,  // 2 - Original uncompressed file size              8 bytes
        Meta::OptionalField8Bytes,  // 3 - Compressed file size                         8 bytes
        Meta::OptionalField8Bytes,  // 4 - Offset of local header record                8 bytes
        Meta::OptionalField4Bytes   // 5 - number of the disk on which the file starts  4 bytes
    >
    {
    public:
        Zip64ExtendedInformation();

        // The incoming values are those from the central directory record. Their value there determines
        // whether we attempt to read them here.
        void Read(const ComPtr<IStream>& stream, ULARGE_INTEGER start, uint32_t uncompressedSize, uint32_t compressedSize, uint32_t offset, uint16_t disk);

        std::uint64_t GetUncompressedSize() const               { return Field<2>(); }
        std::uint64_t GetCompressedSize() const                 { return Field<3>(); }
        std::uint64_t GetRelativeOffsetOfLocalHeader() const    { return Field<4>(); }
        std::uint32_t GetDiskStartNumber() const                { return Field<5>(); }

        void SetUncompressedSize(std::uint64_t value) noexcept { Field<2>() = value; }
        void SetCompressedSize(std::uint64_t value)   noexcept { Field<3>() = value; }
        void SetRelativeOffsetOfLocalHeader(std::uint64_t value)        noexcept { Field<4>() = value; }

        bool HasAnySet() const
        {
            return (Field<2>() || Field<3>() || Field<4>() || Field<5>());
        }

        std::vector<std::uint8_t> GetBytes()
        {
            SetSize(static_cast<uint16_t>(Size() - NonOptionalSize));
            return StructuredObject::GetBytes();
        }

    protected:
        constexpr static size_t NonOptionalSize = 4;

        void SetSignature(std::uint16_t value)        noexcept { Field<0>() = value; }
        void SetSize(std::uint16_t value)             noexcept { Field<1>() = value; }
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

        void SetData(const std::string& name, std::uint32_t crc, std::uint64_t compressedSize,
            std::uint64_t uncompressedSize, std::uint64_t relativeOffset,  std::uint16_t compressionMethod, bool forceDataDescriptor);

        void Read(const ComPtr<IStream>& stream, bool isZip64);

        GeneralPurposeBitFlags GetGeneralPurposeBitFlags() const noexcept { return static_cast<GeneralPurposeBitFlags>(Field<3>().get()); }

        bool IsGeneralPurposeBitSet() const noexcept
        {
            return ((GetGeneralPurposeBitFlags() & GeneralPurposeBitFlags::DataDescriptor) == GeneralPurposeBitFlags::DataDescriptor);
        }

        CompressionType GetCompressionMethod() const noexcept { return static_cast<CompressionType>(Field<4>().get()); }

        std::uint64_t GetCompressedSize() const noexcept
        {
            if (IsValueInExtendedInfo(Field<8>()))
            {
                return m_extendedInfo.GetCompressedSize();
            }
            return static_cast<std::uint64_t>(Field<8>().get());
        }

        std::uint64_t GetUncompressedSize() const noexcept
        {
            if (IsValueInExtendedInfo(Field<9>()))
            {
                return m_extendedInfo.GetUncompressedSize();
            }
            return static_cast<std::uint64_t>(Field<9>().get());
        }

        std::uint64_t GetRelativeOffsetOfLocalHeader() const noexcept
        {
            if (IsValueInExtendedInfo(Field<16>()))
            {
                return m_extendedInfo.GetRelativeOffsetOfLocalHeader();

            }
            return static_cast<std::uint64_t>(Field<16>().get());
        }

        std::string GetFileName() const
        {
            auto data = Field<17>().get();
            return std::string(data.begin(), data.end());
        }

    protected:
        void SetSignature(std::uint32_t value)                   noexcept { Field<0>() = value; }
        void SetVersionMadeBy(std::uint16_t value)               noexcept { Field<1>() = value; }
        void SetVersionNeededToExtract(std::uint16_t value)      noexcept { Field<2>() = value; }
        void SetGeneralPurposeBitFlags(std::uint16_t value)      noexcept { Field<3>() = value; }
        void SetCompressionMethod(std::uint16_t value)           noexcept { Field<4>() = value; }
        void SetLastModFileTime(std::uint16_t value)             noexcept { Field<5>() = value; }
        void SetLastModFileDate(std::uint16_t value)             noexcept { Field<6>() = value; }
        void SetCrc(std::uint32_t value)                         noexcept { Field<7>() = value; }
        void SetFileNameLength(std::uint16_t value)              noexcept { Field<10>() = value; }
        void SetExtraFieldLength(std::uint16_t value)            noexcept { Field<11>() = value; }
        void SetFileCommentLength(std::uint16_t value)           noexcept { Field<12>() = value; }
        void SetDiskNumberStart(std::uint16_t value)             noexcept { Field<13>() = value; }
        void SetInternalFileAttributes(std::uint16_t value)      noexcept { Field<14>() = value; }
        void SetExternalFileAttributes(std::uint16_t value)      noexcept { Field<15>() = value; }

        // Values that might appear in the extended info (minus disk, which we will never set there)
        void SetCompressedSize(std::uint64_t value) noexcept
        {
            if (value > MaxSizeToNotUseDataDescriptor)
            {
                m_extendedInfo.SetCompressedSize(value);
                Field<8>() = std::numeric_limits<uint32_t>::max();
            }
            else
            {
                Field<8>() = static_cast<uint32_t>(value);
            }
        }

        void SetUncompressedSize(std::uint64_t value)noexcept
        {
            if (value > MaxSizeToNotUseDataDescriptor)
            {
                m_extendedInfo.SetUncompressedSize(value);
                Field<9>() = std::numeric_limits<uint32_t>::max();
            }
            else
            {
                Field<9>() = static_cast<uint32_t>(value);
            }
        }

        void SetRelativeOffsetOfLocalHeader(std::uint64_t value) noexcept
        {
            if (value > MaxSizeToNotUseDataDescriptor)
            {
                m_extendedInfo.SetRelativeOffsetOfLocalHeader(value);
                Field<16>() = std::numeric_limits<uint32_t>::max();
            }
            else
            {
                Field<16>() = static_cast<uint32_t>(value);
            }
        }

        void SetFileName(const std::string& name)
        {
            SetFileNameLength(static_cast<std::uint16_t>(name.size()));
            Field<17>().get().resize(name.size(), 0);
            std::copy(name.begin(), name.end(), Field<17>().get().begin());
        }

        void UpdateExtraField()
        {
            if (m_extendedInfo.HasAnySet())
            {
                SetVersionNeededToExtract(static_cast<std::uint16_t>(ZipVersions::Zip64FormatExtension));
                SetExtraFieldLength(static_cast<std::uint16_t>(m_extendedInfo.Size()));
                Field<18>().get() = m_extendedInfo.GetBytes();
            }
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

        void SetData(const std::string& name, bool isCompressed);
        void SetData(std::uint32_t crc, std::uint64_t compressedSize, std::uint64_t uncompressedSize);

        void Read(const ComPtr<IStream>& stream, CentralDirectoryFileHeader& directoryEntry);

        GeneralPurposeBitFlags GetGeneralPurposeBitFlags() const noexcept { return static_cast<GeneralPurposeBitFlags>(Field<2>().get()); }
        std::uint16_t GetCompressionMethod() const noexcept { return Field<3>(); }
        std::uint16_t GetFileNameLength() const noexcept    { return Field<9>();  }
        std::string GetFileName() const
        {
            auto data = Field<11>().get();
            return std::string(data.begin(), data.end());
        }

    protected:
        bool IsGeneralPurposeBitSet() const noexcept
        {
            return ((GetGeneralPurposeBitFlags() & GeneralPurposeBitFlags::DataDescriptor) == GeneralPurposeBitFlags::DataDescriptor);
        }

        void SetSignature(std::uint32_t value)              noexcept { Field<0>() = value; }
        void SetVersionNeededToExtract(std::uint16_t value) noexcept { Field<1>() = value; }
        void SetGeneralPurposeBitFlags(std::uint16_t value) noexcept { Field<2>() = value; }
        void SetCompressionMethod(std::uint16_t value)      noexcept { Field<3>() = value; }
        void SetLastModFileTime(std::uint16_t value)        noexcept { Field<4>() = value; }
        void SetLastModFileDate(std::uint16_t value)        noexcept { Field<5>() = value; }
        void SetCrc(std::uint32_t value)                    noexcept { Field<6>() = value; }
        void SetCompressedSize(std::uint32_t value)         noexcept { Field<7>() = value; }
        void SetUncompressedSize(std::uint32_t value)       noexcept { Field<8>() = value; }
        void SetFileNameLength(std::uint16_t value)         noexcept { Field<9>() = value; }
        void SetExtraFieldLength(std::uint16_t value)       noexcept { Field<10>() = value; }
        void SetFileName(const std::string& name)
        {
            SetFileNameLength(static_cast<std::uint16_t>(name.size()));
            Field<11>().get().resize(name.size(), 0);
            std::copy(name.begin(), name.end(), Field<11>().get().begin());
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

        std::uint64_t GetTotalNumberOfEntries() const noexcept  { return Field<6>(); }
        std::uint64_t GetOffsetStartOfCD() const noexcept       { return Field<9>(); }

    protected:
        void SetSignature(std::uint32_t value)                    noexcept { Field<0>() = value; }
        void SetSizeOfZip64CDRecord(std::uint64_t value)          noexcept { Field<1>() = value; }
        void SetVersionMadeBy(std::uint16_t value)                noexcept { Field<2>() = value; }
        void SetVersionNeededToExtract(std::uint16_t value)       noexcept { Field<3>() = value; }
        void SetNumberOfThisDisk(std::uint32_t value)             noexcept { Field<4>() = value; }
        void SetNumberOfTheDiskWithStartOfCD(std::uint32_t value) noexcept { Field<5>() = value; }
        void SetTotalNumberOfEntriesDisk(std::uint64_t value) noexcept
        {
            Field<6>() = value;
            Field<7>() = value;
        }
        void SetSizeOfCD(std::uint64_t value)         noexcept { Field<8>() = value; }
        void SetOffsetfStartOfCD(std::uint64_t value) noexcept { Field<9>() = value; }
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

        std::uint64_t GetRelativeOffset() const noexcept { return Field<2>(); }

    protected:
        void SetSignature(std::uint32_t value)          noexcept { Field<0>() = value; }
        void SetNumberOfDisk(std::uint32_t value)       noexcept { Field<1>() = value; }
        void SetRelativeOffset(std::uint64_t value)     noexcept { Field<2>() = value; }
        void SetTotalNumberOfDisks(std::uint32_t value) noexcept { Field<3>() = value; }
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

        bool GetIsZip64() const noexcept { return m_isZip64; }

        std::uint64_t GetNumberOfCentralDirectoryEntries() noexcept { return static_cast<std::uint64_t>(Field<3>().get()); }
        std::uint64_t GetStartOfCentralDirectory()         noexcept { return static_cast<std::uint64_t>(Field<6>().get()); }

    protected:
        void SetSignature(std::uint32_t value)                      noexcept { Field<0>() = value; }
        void SetNumberOfDisk(std::uint16_t value)                   noexcept { Field<1>() = value; }
        void SetDiskStart(std::uint16_t value)                      noexcept { Field<2>() = value; }
        void SetTotalNumberOfEntries(std::uint16_t value)           noexcept { Field<3>() = value; }
        void SetTotalEntriesInCentralDirectory(std::uint16_t value) noexcept { Field<4>() = value; }
        void SetSizeOfCentralDirectory(std::uint32_t value)         noexcept { Field<5>() = value; }
        void SetOffsetOfCentralDirectory(std::uint32_t value)       noexcept { Field<6>() = value; }
        void SetCommentLength(std::uint16_t value)                  noexcept { Field<7>() = value; }

        bool m_isZip64 = true;
    };
}

#include "Exceptions.hpp"
#include "StreamBase.hpp"
#include "ObjectBase.hpp"
#include "ZipObject.hpp"
#include "ZipFileStream.hpp"
#include "InflateStream.hpp"

#include <memory>
#include <string>
#include <limits>
#include <functional>
#include <algorithm>

namespace xPlat {

    /* Zip File Structure
    [LocalFileHeader 1]
    [encryption header 1]
    [file data 1]
    [data descriptor 1]
    .
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

    // from ZIP file format specification detailed in AppNote.txt
    enum class Signatures : std::uint32_t
    {
        LocalFileHeader = 0x04034b50,
        DataDescriptor = 0x08074b50,
        CentralFileHeader = 0x02014b50,
        Zip64EndOfCD = 0x06064b50,
        Zip64EndOfCDLocator = 0x07064b50,
        EndOfCentralDirectory = 0x06054b50,
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

    inline constexpr GeneralPurposeBitFlags operator &(GeneralPurposeBitFlags a, GeneralPurposeBitFlags b)
    {
        return static_cast<GeneralPurposeBitFlags>(static_cast<uint16_t>(a) & static_cast<uint16_t>(b));
    }

    inline constexpr GeneralPurposeBitFlags operator |(GeneralPurposeBitFlags a, GeneralPurposeBitFlags b)
    {
        return static_cast<GeneralPurposeBitFlags>(static_cast<uint16_t>(a) | static_cast<uint16_t>(b));
    }

    // if any of these are set, then fail.
    constexpr static const GeneralPurposeBitFlags UnsupportedFlagsMask =
        GeneralPurposeBitFlags::UNSUPPORTED_0  |
        GeneralPurposeBitFlags::UNSUPPORTED_6  |
        GeneralPurposeBitFlags::UNSUPPORTED_12 |
        GeneralPurposeBitFlags::UNSUPPORTED_13 |
        GeneralPurposeBitFlags::UNSUPPORTED_14 |
        GeneralPurposeBitFlags::UNSUPPORTED_15;

    /*
    class DataDescriptor : public Meta::StructuredObject
    {
    public:
        std::uint32_t GetCrc32() { return *Meta::Object::GetValue<std::uint32_t>(Field(0)); }
        void SetCrc32(std::uint32_t value) { Meta::Object::SetValue(Field(0), value); }

        std::uint32_t GetCompressedSize() { return *Meta::Object::GetValue<std::uint32_t>(Field(1)); }
        void SetCompressedSize(std::uint32_t value) { Meta::Object::SetValue(Field(1), value); }

        std::uint32_t GetUncompressedSize() { return *Meta::Object::GetValue<std::uint32_t>(Field(2)); }
        void SetUncompressedSize(std::uint32_t value) { Meta::Object::SetValue(Field(2), value); }

        DataDescriptor() : Meta::StructuredObject(
        {
            // 0 - crc - 32                          4 bytes    
            std::make_shared<Meta::Field4Bytes>([](std::uint32_t& v) {}),
            // 1 - compressed size                 4 bytes
            std::make_shared<Meta::Field4Bytes>([](std::uint32_t& v) {}),
            // 2 - uncompressed size               4 bytes
            std::make_shared<Meta::Field4Bytes>([](std::uint32_t& v) {})

        })
        {
        }
    };//class DataDescriptor
    */

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
    class CentralDirectoryFileHeader : public Meta::StructuredObject<
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
        CentralDirectoryFileHeader(StreamBase* s) : m_stream(s)
        {
            // 0 - central file header signature   4 bytes(0x02014b50)
            Field<0>().validation = [](std::uint32_t& v)
            {   if (v != static_cast<std::uint32_t>(Signatures::CentralFileHeader))
                {   throw ZipException("signature mismatch", ZipException::Error::InvalidCentralDirectoryHeader);
                }
            };
            // 1 - version made by                 2 bytes
            Field<1>().validation = [](std::uint16_t& v)
            {   if (v != static_cast<std::uint16_t>(ZipVersions::Zip64FormatExtension))
                {   throw ZipException("unsupported version made by", ZipException::Error::InvalidCentralDirectoryHeader);
                }
            };
            // 2 - version needed to extract       2 bytes
            Field<2>().validation = [](std::uint16_t& v)
            {   if ((v != static_cast<std::uint16_t>(ZipVersions::Zip32DefaultVersion)) &&
                    (v != static_cast<std::uint16_t>(ZipVersions::Zip64FormatExtension))
                    )
                {   throw ZipException("unsupported version needed to extract", ZipException::Error::InvalidCentralDirectoryHeader);
                }
            };
            // 3 - general purpose bit flag        2 bytes
            Field<3>().validation = [](std::uint16_t& v)
            {   if ((v & static_cast<std::uint16_t>(UnsupportedFlagsMask)) != 0)
                {   throw ZipException("unsupported flag(s) specified", ZipException::Error::InvalidCentralDirectoryHeader);
                }
            };
            // 4 - compression method              2 bytes
            Field<4>().validation = [](std::uint16_t& v)
            {   if ((v != static_cast<std::uint16_t>(CompressionType::Store)) &&
                    (v != static_cast<std::uint16_t>(CompressionType::Deflate))
                    )
                {   throw ZipException("unsupported compression method", ZipException::Error::InvalidCentralDirectoryHeader);
                }
            };
            // 5 - last mod file time              2 bytes
            // 6 - last mod file date              2 bytes
            // 7 - crc - 32                        4 bytes
            // 8 - compressed size                 4 bytes
            // 9 - uncompressed size               4 bytes
            //10 - file name length                2 bytes
            Field<10>().validation = [&](std::uint16_t& v)
            {   if (v == 0)
                {   throw ZipException("unsupported file name size", ZipException::Error::InvalidCentralDirectoryHeader);
                }
                Field<17>().value.resize(v,0);
            };
            //11 - extra field length              2 bytes
            Field<11>().validation = [&](std::uint16_t& v)
            {   if (v != 0)
                {   throw ZipException("unsupported extra field size", ZipException::Error::InvalidCentralDirectoryHeader);
                }
                Field<18>().value.resize(v,0);
            };
            //12 - file comment length             2 bytes
            Field<12>().validation = [&](std::uint16_t& v)
            {   if (v != 0)
                {   throw ZipException("unsupported file comment size", ZipException::Error::InvalidCentralDirectoryHeader);
                }
            };
            //13 - disk number start               2 bytes
            Field<13>().validation = [](std::uint16_t& v)
            {   if (v != 0)
                {   throw ZipException("unsupported disk number start", ZipException::Error::InvalidCentralDirectoryHeader);
                }
            };
            //14 - internal file attributes        2 bytes
            Field<14>().validation = [](std::uint16_t& v)
            {   if (v != 0)
                {   throw ZipException("unsupported internal file attributes", ZipException::Error::InvalidCentralDirectoryHeader);
                }
            };
            //15 - external file attributes        4 bytes
            //16 - relative offset of local header 4 bytes
            Field<16>().validation = [&](std::uint32_t& v)
            {   if (v >= m_stream->Ftell())
                {   throw ZipException("invalid relative offset", ZipException::Error::InvalidCentralDirectoryHeader);
                }
            };
            //17 - file name(variable size)
            //18 - extra field(variable size)
            //19 - file comment(variable size)
            
            SetSignature(static_cast<std::uint32_t>(Signatures::CentralFileHeader));
  
            SetVersionMadeBy(static_cast<std::uint16_t>(ZipVersions::Zip64FormatExtension));
            // only set to Zip64FormatExtension iff required!
            SetVersionNeededToExtract(static_cast<std::uint16_t>(ZipVersions::Zip32DefaultVersion));    
            SetLastModFileDate(static_cast<std::uint16_t>(MagicNumbers::FileDate));
            SetLastModFileTime(static_cast<std::uint16_t>(MagicNumbers::FileTime));
            SetExtraFieldLength(0);
            SetFileCommentLength(0);
            SetDiskNumberStart(0);
            SetInternalFileAttributes(0);
            SetExternalFileAttributes(0);
        }

        bool IsGeneralPurposeBitSet()
        {
            return ((GetGeneralPurposeBitFlags() & GeneralPurposeBitFlags::GeneralPurposeBit) == GeneralPurposeBitFlags::GeneralPurposeBit);
        }

        std::uint16_t GetVersionNeededToExtract()                { return Field<2>().value; }

        GeneralPurposeBitFlags GetGeneralPurposeBitFlags()       { return static_cast<GeneralPurposeBitFlags>(Field<3>().value); }
        void SetGeneralPurposeBitFlags(std::uint16_t value)      { Field<3>().value = value; }

        std::uint16_t GetCompressionMethod()                     { return Field<4>().value; }
        void SetCompressionMethod(std::uint16_t value)           { Field<4>().value= value; }

        std::uint32_t GetCrc32()                                 { return Field<7>().value; }
        void SetCrc(std::uint32_t value)                         { Field<7>().value = value; }

        std::uint32_t GetCompressedSize()                        { return Field<8>().value; }
        void SetCompressedSize(std::uint32_t value)              { Field<8>().value = value; }

        std::uint32_t GetUncompressedSize()                      { return Field<9>().value; }
        void SetUncompressedSize(std::uint32_t value)            { Field<9>().value = value; }

        std::uint32_t GetRelativeOffsetOfLocalHeader()           { return Field<16>().value; }
        void SetRelativeOffsetOfLocalHeader(std::uint32_t value) { Field<16>().value = value; }

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

    private:
        void SetSignature(std::uint32_t value)              { Field<0>().value = value; }
        void SetVersionMadeBy(std::uint16_t value)          { Field<1>().value = value; }
        void SetVersionNeededToExtract(std::uint16_t value) { Field<2>().value = value; }

        void SetLastModFileTime(std::uint16_t value)        { Field<5>().value = value; }
        void SetLastModFileDate(std::uint16_t value)        { Field<6>().value = value; }

        void SetFileNameLength(std::uint16_t value)         { Field<10>().value = value; }
        void SetExtraFieldLength(std::uint16_t value)       { Field<11>().value = value; }
        void SetFileCommentLength(std::uint16_t value)      { Field<12>().value = value; }
        void SetDiskNumberStart(std::uint16_t value)        { Field<13>().value = value; }
        void SetInternalFileAttributes(std::uint16_t value) { Field<14>().value = value; }
        void SetExternalFileAttributes(std::uint16_t value) { Field<15>().value = value; }

        StreamBase* m_stream = nullptr;
    };//class CentralDirectoryFileHeader

    class LocalFileHeader : public Meta::StructuredObject<
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
        LocalFileHeader(std::shared_ptr<CentralDirectoryFileHeader> directoryEntry) : m_directoryEntry(directoryEntry)
        {
            // 0 - local file header signature     4 bytes(0x04034b50)
            Field<0>().validation = [](std::uint32_t& v)
            {   if (v != static_cast<std::uint32_t>(Signatures::LocalFileHeader))
                {   throw ZipException("file header does not match signature", ZipException::Error::InvalidLocalFileHeader);
                }
            };
            // 1 - version needed to extract       2 bytes
            Field<1>().validation = [](std::uint16_t& v)
            {   if ((v != static_cast<std::uint16_t>(ZipVersions::Zip32DefaultVersion)) &&
                    (v != static_cast<std::uint16_t>(ZipVersions::Zip64FormatExtension))
                )
                {   throw ZipException("unsupported version needed to extract", ZipException::Error::InvalidLocalFileHeader);
                }
            };
            // 2 - general purpose bit flag        2 bytes
            Field<2>().validation = [&](std::uint16_t& v)
            {   if ((v & static_cast<std::uint16_t>(UnsupportedFlagsMask)) != 0)
                {   throw ZipException("unsupported flag(s) specified", ZipException::Error::InvalidLocalFileHeader);
                }
                if (IsGeneralPurposeBitSet() != m_directoryEntry->IsGeneralPurposeBitSet())
                {   throw ZipException("inconsistent general purpose bits specified", ZipException::Error::InvalidLocalFileHeader);
                }
            };
            // 3 - compression method              2 bytes
            Field<3>().validation = [](std::uint16_t& v)
            {   if ((v != static_cast<std::uint16_t>(CompressionType::Store)) &&
                    (v != static_cast<std::uint16_t>(CompressionType::Deflate))
                    )
                {   throw ZipException("unsupported compression method", ZipException::Error::InvalidLocalFileHeader);
                }
            };
            // 4 - last mod file time              2 bytes
            // 5 - last mod file date              2 bytes
            // 6 - crc - 32                        4 bytes
            Field<6>().validation = [&](std::uint32_t& v)
            {   if (IsGeneralPurposeBitSet() && (v != 0))
                {   throw ZipException("Invalid CRC", ZipException::Error::InvalidLocalFileHeader);
                }
            };
            // 7 - compressed size                 4 bytes
            Field<7>().validation = [&](std::uint32_t& v)
            {   if (IsGeneralPurposeBitSet() && (v != 0))
                {   throw ZipException("Invalid compressed size", ZipException::Error::InvalidLocalFileHeader);
                }
            };
            // 8 - uncompressed size               4 bytes
            Field<8>().validation = [&](std::uint32_t& v)
            {   if (IsGeneralPurposeBitSet() && (v != 0))
                {   throw ZipException("Invalid uncompressed size", ZipException::Error::InvalidLocalFileHeader);
                }
            };
            // 9 - file name length                2 bytes
            Field<9>().validation = [&](std::uint16_t& v)
            {   if (v == 0)
                {   throw ZipException("unsupported file name size", ZipException::Error::InvalidLocalFileHeader);
                }
                Field<11>().value.resize(GetFileNameLength(), 0);
            };
            // 10- extra field length              2 bytes
            Field<10>().validation = [&](std::uint16_t& v)
            {   if (v != 0)
                {   throw ZipException("unsupported extra field size", ZipException::Error::InvalidLocalFileHeader);
                }
                Field<12>().value.resize(GetExtraFieldLength(), 0);
            };
            // 11- file name (variable size)
            // 12- extra field (variable size)
        }

        bool IsGeneralPurposeBitSet()
        {
            return ((GetGeneralPurposeBitFlags() & GeneralPurposeBitFlags::GeneralPurposeBit) == GeneralPurposeBitFlags::GeneralPurposeBit);
        }

        GeneralPurposeBitFlags GetGeneralPurposeBitFlags() { return static_cast<GeneralPurposeBitFlags>(Field<2>().value); }

        CompressionType GetCompressionType() { return static_cast<CompressionType>(Field<3>().value); }

        std::uint32_t GetCompressedSize()
        {
            if (IsGeneralPurposeBitSet())
            {
                return m_directoryEntry->GetCompressedSize();
            }
            return Field<7>().value;
        }

        std::uint32_t GetUncompressedSize() 
        {
            if (IsGeneralPurposeBitSet())
            {
                return m_directoryEntry->GetUncompressedSize();
            }
            return Field<8>().value;
        }

        std::uint16_t GetFileNameLength()   { return Field<9>().value;  }
        std::uint16_t GetExtraFieldLength() { return Field<10>().value; }

        void SetGeneralPurposeBitFlag(std::uint16_t value)  { Field<2>().value = value;  }
        void SetCompressedSize(std::uint32_t value)         { Field<7>().value = value;  }
        void SetUncompressedSize(std::uint32_t value)       { Field<8>().value = value;  }
        void SetFileNameLength(std::uint16_t value)         { Field<9>().value = value;  }
        void SetExtraFieldLength(std::uint16_t value)       { Field<10>().value = value; }

        std::string   GetFileName()
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

        std::shared_ptr<CentralDirectoryFileHeader> m_directoryEntry = nullptr;
    }; //class LocalFileHeader

    class Zip64EndOfCentralDirectoryRecord : public Meta::StructuredObject<
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
        Zip64EndOfCentralDirectoryRecord(StreamBase* s) : m_stream(s)
        {
            // 0 - zip64 end of central dir signature 4 bytes(0x06064b50)
            Field<0>().validation = [](std::uint32_t& v)
            {   if (v != static_cast<std::uint32_t>(Signatures::Zip64EndOfCD))
                {   throw ZipException("end of zip64 central directory does not match signature", ZipException::Error::InvalidHeader);
                }
            };
            // 1 - size of zip64 end of central directory record 8 bytes
            Field<1>().validation = [&](std::uint64_t& v)
            {   //4.3.14.1 The value stored into the "size of zip64 end of central
                //    directory record" should be the size of the remaining
                //    record and should not include the leading 12 bytes.
                auto size = this->Size() - 12;
                if (v != size)
                {   throw ZipException("invalid size of zip64 EOCD", ZipException::Error::InvalidZip64CentralDirectoryRecord);
                }
            };
            // 2 - version made by                 2 bytes
            Field<2>().validation = [](std::uint16_t& v)
            {   if (v != static_cast<std::uint16_t>(ZipVersions::Zip64FormatExtension))
                {   throw ZipException("invalid zip64 EOCD version made by", ZipException::Error::InvalidZip64CentralDirectoryRecord);
                }
            };
            // 3 - version needed to extract       2 bytes
            Field<3>().validation = [](std::uint16_t& v)
            {   if (v != static_cast<std::uint16_t>(ZipVersions::Zip64FormatExtension))
                {   throw ZipException("invalid zip64 EOCD version to extract", ZipException::Error::InvalidZip64CentralDirectoryRecord);
                }
            };
            // 4 - number of this disk             4 bytes
            Field<4>().validation = [](std::uint32_t& v)
            {   if (v != 0)
                {   throw ZipException("invalid disk number", ZipException::Error::InvalidZip64CentralDirectoryRecord);
                }
            };
            // 5 - number of the disk with the start of the central directory  4 bytes
            Field<5>().validation = [](std::uint32_t& v)
            {   if (v != 0)
                {   throw ZipException("invalid disk index", ZipException::Error::InvalidZip64CentralDirectoryRecord);
                }
            };
            // 6 - total number of entries in the central directory on this disk  8 bytes
            Field<6>().validation = [](std::uint64_t& v)
            {   if (v == 0)
                {   throw ZipException("invalid number of entries", ZipException::Error::InvalidZip64CentralDirectoryRecord);
                }
            };
            // 7 - total number of entries in the central directory 8 bytes
            Field<7>().validation = [&](std::uint64_t& v)
            {   if (v != this->GetTotalNumberOfEntries())
                {   throw ZipException("invalid total number of entries", ZipException::Error::InvalidZip64CentralDirectoryRecord);
                }
            };
            // 8 - size of the central directory   8 bytes
            Field<8>().validation = [&](std::uint64_t& v)
            {   // TODO: tighten up this validation
                if ((v == 0) ||
                    (v >= m_stream->Ftell()))
                {   throw ZipException("invalid size of central directory", ZipException::Error::InvalidZip64CentralDirectoryRecord);
                }
            };
            // 9 - offset of start of central directory with respect to the starting disk number        8 bytes
            Field<9>().validation = [&](std::uint64_t& v)
            {   // TODO: tighten up this validation
                if ((v == 0) ||
                    (v >= m_stream->Ftell()))
                {   throw ZipException("invalid start of central directory", ZipException::Error::InvalidZip64CentralDirectoryRecord);
                }
            };
            //10 - zip64 extensible data sector(variable size)
            Field<10>().validation = [](std::vector<std::uint8_t>& data)
            {   if (data.size() != 0)
                {   throw ZipException("unsupported extensible data", ZipException::Error::InvalidZip64CentralDirectoryRecord);
                }
            };

            SetSignature(static_cast<std::uint32_t>(Signatures::Zip64EndOfCD));
            SetGetSizeOfZip64CDRecord(this->Size() - 12);
            SetVersionMadeBy(static_cast<std::uint16_t>(ZipVersions::Zip64FormatExtension));
            SetVersionNeededToExtract(static_cast<std::uint16_t>(ZipVersions::Zip64FormatExtension));
            SetNumberOfThisDisk(0);
            SetTotalNumberOfEntries(0);
            Field<10>().value.resize(0);
        }

        std::uint64_t GetTotalNumberOfEntries() { return Field<6>().value; }

        void SetTotalNumberOfEntries(std::uint64_t value)
        {
            Field<6>().value = value;
            Field<7>().value = value;
        }

        std::uint64_t GetSizeOfCD()                     { return Field<8>().value; }
        void SetSizeOfCD(std::uint64_t value)           { Field<8>().value = value; }
        std::uint64_t GetOffsetfStartOfCD()             { return Field<9>().value; }
        void SetOffsetfStartOfCD(std::uint64_t value)   { Field<9>().value = value; }

    private:
        void SetSignature(std::uint32_t value)              { Field<0>().value = value; }
        void SetGetSizeOfZip64CDRecord(std::uint64_t value) { Field<1>().value = value; }
        void SetVersionMadeBy(std::uint16_t value)          { Field<2>().value = value; }
        void SetVersionNeededToExtract(std::uint16_t value) { Field<3>().value = value; }
        void SetNumberOfThisDisk(std::uint32_t value)       { Field<4>().value = value; }

        StreamBase* m_stream = nullptr;
    }; //class Zip64EndOfCentralDirectoryRecord

    class Zip64EndOfCentralDirectoryLocator : public Meta::StructuredObject<
        Meta::Field4Bytes, // 0 - zip64 end of central dir locator signature        4 bytes(0x07064b50)
        Meta::Field4Bytes, // 1 - number of the disk with the start of the zip64
                           //     end of central directory                          4 bytes
        Meta::Field8Bytes, // 2 - relative offset of the zip64 end of central
                           //     directory record                                  8 bytes
        Meta::Field4Bytes  // 3 - total number of disks                             4 bytes
        >
    {
    public:
        Zip64EndOfCentralDirectoryLocator(StreamBase* s) : m_stream(s)
        {
            // 0 - zip64 end of central dir locator signature 4 bytes(0x07064b50)
            Field<0>().validation = [](std::uint32_t& v)
            {   if (v != static_cast<std::uint32_t>(Signatures::Zip64EndOfCDLocator))
                {   throw ZipException("end of central directory locator does not match signature", ZipException::Error::InvalidHeader);
                }
            };
            // 1 - number of the disk with the start of the zip64 end of central directory               4 bytes
            Field<1>().validation = [](std::uint32_t& v)
            {   if (v != 0)
                {   throw ZipException("Invalid disk number", ZipException::Error::InvalidZip64CentralDirectoryLocator);
                }
            };
            // 2 - relative offset of the zip64 end of central directory record 8 bytes
            Field<2>().validation = [&](std::uint64_t& v)
            {   if ((v == 0) ||
                    (v >= m_stream->Ftell()))
                {   throw ZipException("Invalid relative offset", ZipException::Error::InvalidZip64CentralDirectoryLocator);
                }
            };
            // 3 - total number of disks           4 bytes
            Field<3>().validation = [](std::uint32_t& v)
            {   if (v != 1)
                {   throw ZipException("Invalid total number of disks", ZipException::Error::InvalidZip64CentralDirectoryLocator);
                }
            };

            SetSignature(static_cast<std::uint32_t>(Signatures::Zip64EndOfCDLocator));
            SetNumberOfDisk(0);
            SetTotalNumberOfDisks(1);
        }

        std::uint64_t GetRelativeOffset()           { return Field<2>().value; }
        void SetRelativeOffset(std::uint64_t value) { Field<2>().value = value; }

    private:
        void SetSignature(std::uint32_t value)          { Field<0>().value = value; }
        void SetNumberOfDisk(std::uint32_t value)       { Field<1>().value = value; }
        void SetTotalNumberOfDisks(std::uint32_t value) { Field<3>().value = value; }

        StreamBase* m_stream = nullptr;
    }; //class Zip64EndOfCentralDirectoryLocator

    class EndCentralDirectoryRecord : public Meta::StructuredObject<
        Meta::Field4Bytes, // 0 - end of central dir signature              4 bytes  (0x06054b50)
        Meta::Field2Bytes, // 1 - number of this disk                       2 bytes
        Meta::Field2Bytes, // 2 - number of the disk with the start of the
                           //     central directory                         2 bytes
        Meta::Field2Bytes, // 3 - total number of entries in the central
                           //     directory on this disk                    2 bytes
        Meta::Field2Bytes, // 4 - total number of entries in the central
                           //     directory                                 2 bytes
        Meta::Field4Bytes, // 5 - size of the central directory             4 bytes
        Meta::Field4Bytes, // 6 - offset of start of central directory with
                           //     respect to the starting disk number       4 bytes
        Meta::Field2Bytes, // 7 - .ZIP file comment length                  2 bytes
        Meta::FieldNBytes  // 8 - .ZIP file comment                         (variable size)
        >
    {
    public:
        EndCentralDirectoryRecord()
        {
            // 0 - end of central dir signature    4 bytes  (0x06054b50)
            Field<0>().validation = [](std::uint32_t& v)
            {   if (v != static_cast<std::uint32_t>(Signatures::EndOfCentralDirectory))
                {   throw ZipException("invalid signiture", ZipException::Error::InvalidEndOfCentralDirectoryRecord);
                }
            };
            // 1 - number of this disk             2 bytes
            Field<1>().validation = [](std::uint16_t& v)
            {   if (v != 0)
                {   throw ZipException("unsupported disk number", ZipException::Error::InvalidEndOfCentralDirectoryRecord);
                }
            };
            // 2 - number of the disk with the start of the central directory  2 bytes
            Field<2>().validation = [](std::uint16_t& v)
            {   if (v != 0)
                {   throw ZipException("unsupported EoCDR disk number", ZipException::Error::InvalidEndOfCentralDirectoryRecord);
                }
            };
            // 3 - total number of entries in the central directory on this disk  2 bytes
            Field<3>().validation = [](std::uint16_t& v)
            {   if (v != std::numeric_limits<std::uint16_t>::max())
                {   throw ZipException("unsupported total number of entries on this disk", ZipException::Error::InvalidEndOfCentralDirectoryRecord);
                }
            };
            // 4 - total number of entries in the central directory           2 bytes
            Field<4>().validation = [](std::uint16_t& v)
            {   if (v != std::numeric_limits<std::uint16_t>::max())
                {   throw ZipException("unsupported total number of entries", ZipException::Error::InvalidEndOfCentralDirectoryRecord);
                }
            };
            // 5 - size of the central directory   4 bytes
            Field<5>().validation = [](std::uint32_t& v)
            {   if (v != std::numeric_limits<std::uint32_t>::max())
                {   throw ZipException("unsupported size of central directory", ZipException::Error::InvalidEndOfCentralDirectoryRecord);
                }
            };
            // 6 - offset of start of central directory with respect to the starting disk number        4 bytes
            Field<6>().validation = [](std::uint32_t& v)
            {   if (v != std::numeric_limits<std::uint32_t>::max())
                {   throw ZipException("unsupported offset of start of central directory", ZipException::Error::InvalidEndOfCentralDirectoryRecord);
                }
            };
            // 7 - .ZIP file comment length        2 bytes
            Field<7>().validation = [&](std::uint16_t& v)
            {   if (v != 0)
                {   throw ZipException("Zip comment unsupported", ZipException::Error::InvalidEndOfCentralDirectoryRecord);
                }
            };
            // 8 - .ZIP file comment       (variable size)
            Field<8>().validation = [](std::vector<std::uint8_t>& data)
            {   if (data.size() != 0)
                {   throw ZipException("Zip comment unsupported", ZipException::Error::InvalidEndOfCentralDirectoryRecord);
                }
            };

            SetSignature(static_cast<std::uint32_t>(Signatures::EndOfCentralDirectory));
            SetNumberOfDisk(0);
            SetDiskStart(0);
            // next 12 bytes need to be: FFFF FFFF  FFFF FFFF  FFFF FFFF
            SetTotalNumberOfEntries          (std::numeric_limits<std::uint16_t>::max());
            SetTotalEntriesInCentralDirectory(std::numeric_limits<std::uint16_t>::max());
            SetSizeOfCentralDirectory        (std::numeric_limits<std::uint32_t>::max());
            SetOffsetOfCentralDirectory      (std::numeric_limits<std::uint32_t>::max());
            // last 2 bytes need to be : 00
            SetCommentLength(0);
        }

    private:
        void SetSignature(std::uint32_t value)                      { Field<0>().value = value; }
        void SetNumberOfDisk(std::uint16_t value)                   { Field<1>().value = value; }
        void SetDiskStart(std::uint16_t value)                      { Field<2>().value = value; }
        void SetTotalNumberOfEntries(std::uint16_t value)           { Field<3>().value = value; }
        void SetTotalEntriesInCentralDirectory(std::uint16_t value) { Field<4>().value = value; }
        void SetSizeOfCentralDirectory(std::uint32_t value)         { Field<5>().value = value; }
        void SetOffsetOfCentralDirectory(std::uint32_t value)       { Field<6>().value = value; }
        void SetCommentLength(std::uint16_t value)                  { Field<7>().value = value; }

    };//class EndOfCentralDirectoryRecord

    std::vector<std::string> ZipObject::GetFileNames()
    {
        std::vector<std::string> result;
        std::for_each(m_streams.begin(), m_streams.end(), [&](auto it)
        {
            result.push_back(it.first);
        });
        return result;
    }

    std::shared_ptr<StreamBase> ZipObject::GetFile(const std::string& fileName)
    {
        return m_streams[fileName];
    }

    void ZipObject::RemoveFile(const std::string& fileName)
    {
        throw NotImplementedException();
    }

    std::shared_ptr<StreamBase> ZipObject::OpenFile(const std::string& fileName, FileStream::Mode mode)
    {
        throw NotImplementedException();
    }

    void ZipObject::CommitChanges()
    {
        throw NotImplementedException();
    }

    std::string ZipObject::GetPathSeparator() { return "/"; }

    ZipObject::ZipObject(std::unique_ptr<StreamBase>&& stream) : m_stream(std::move(stream))
    {
        // Confirm that the file IS the correct format
        EndCentralDirectoryRecord endCentralDirectoryRecord;
        m_stream->Seek(-1 * endCentralDirectoryRecord.Size(), StreamBase::Reference::END);
        endCentralDirectoryRecord.Read(m_stream.get());

        // find where the zip central directory exists.
        Zip64EndOfCentralDirectoryLocator zip64Locator(m_stream.get());
        m_stream->Seek(-1*(endCentralDirectoryRecord.Size() + zip64Locator.Size()), StreamBase::Reference::END);
        zip64Locator.Read(m_stream.get());

        // now read the zip central directory
        Zip64EndOfCentralDirectoryRecord zip64EndOfCentralDirectory(m_stream.get());
        m_stream->Seek(zip64Locator.GetRelativeOffset(), StreamBase::Reference::START);
        zip64EndOfCentralDirectory.Read(m_stream.get());

        // read the zip central directory
        m_stream->Seek(zip64EndOfCentralDirectory.GetOffsetfStartOfCD(), StreamBase::Reference::START);
        for (std::uint32_t index = 0; index < zip64EndOfCentralDirectory.GetTotalNumberOfEntries(); index++)
        {
            auto centralFileHeader = std::make_shared<CentralDirectoryFileHeader>(m_stream.get());
            centralFileHeader->Read(m_stream.get());
            // TODO: ensure that there are no collisions on name!
            m_centralDirectory.insert(std::make_pair(centralFileHeader->GetFileName(), centralFileHeader));
        }

        // We should have no data between the end of the last central directory header and the start of the EoCD
        if (m_stream->Ftell() != zip64Locator.GetRelativeOffset())
        {   throw ZipException("hidden data unsupported", ZipException::Error::HiddenDataBetweenLastCDHandEoCD);
        }

        // read the file repository
        for (const auto& centralFileHeader : m_centralDirectory)
        {
            m_stream->Seek(centralFileHeader.second->GetRelativeOffsetOfLocalHeader(), xPlat::StreamBase::Reference::START);
            auto localFileHeader = std::make_shared<LocalFileHeader>(centralFileHeader.second);
            localFileHeader->Read(m_stream.get());
            m_fileRepository.insert(std::make_pair(
                centralFileHeader.second->GetRelativeOffsetOfLocalHeader(),
                localFileHeader));

            std::shared_ptr<StreamBase> fileStream = std::make_shared<ZipFileStream>(
                centralFileHeader.second->GetRelativeOffsetOfLocalHeader() + localFileHeader->Size(),
                localFileHeader->GetCompressedSize(),
                localFileHeader->GetCompressionType() == CompressionType::Deflate,
                m_stream.get()
                );

            if (localFileHeader->GetCompressionType() == CompressionType::Deflate)
            {
                fileStream = std::make_shared<InflateStream>(std::move(fileStream), localFileHeader->GetUncompressedSize());
            }

            m_streams.insert(std::make_pair(centralFileHeader.second->GetFileName(), fileStream));
        }
    } // ZipObject::ZipObject
} // namespace xPlat

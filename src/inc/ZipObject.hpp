#pragma once

#include "Exceptions.hpp"
#include "StreamBase.hpp"
#include "ObjectBase.hpp"
#include "ZipFileStream.hpp"

#include <vector>
#include <map>
#include <memory>

namespace xPlat {

    class ZipException : public ExceptionBase
    {
    public:
        enum class Error : std::uint32_t
        {
            InvalidHeader                       = 1,
            FieldOutOfRange                     = 2,
            InvalidEndOfCentralDirectoryRecord  = 3,
            InvalidZip64CentralDirectoryLocator = 4,
            InvalidZip64CentralDirectoryRecord  = 5,
            InvalidCentralDirectoryHeader       = 6,
            HiddenDataBetweenLastCDHandEoCD     = 7,
        };

        ZipException(std::string message, Error error) : reason(message), ExceptionBase(ExceptionBase::Facility::ZIP)
        {
            SetLastError(static_cast<std::uint32_t>(error));
        }
        std::string reason;
    };

    enum class ZipVersions : std::uint16_t
    {
        Zip32DefaultVersion = 20,
        Zip64FormatExtension = 45,
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
        UNSUPPORTED_0           = 0x0001, // Bit 0: If set, indicates that the file is encrypted.

        Deflate_MaxCompress     = 0x0002, // Maximum compression (-exx/-ex), otherwise, normal compression (-en)
        Deflate_FastCompress    = 0x0004, // Fast (-ef), if Max+Fast then SuperFast (-es) compression

        CRC32_SizesZero         = 0x0008,  // the field's crc-32 compressed and uncompressed sizes = 0 in the local header
                                           // the correct values are put in the data descriptor immediately following the
                                           // compressed data.
        EnhancedDeflate         = 0x0010,
        CompressedPatchedData   = 0x0020,
        UNSUPPORTED_6           = 0x0040, // Strong encryption.

        UnUsed_7                = 0x0080, // currently unused
        UnUsed_8                = 0x0100, // currently unused
        UnUsed_9                = 0x0200, // currently unused
        UnUsed_10               = 0x0400, // currently unused

        EncodingMustUseUTF8     = 0x0800, // Language encoding flag (EFS).  File name and comments fields MUST be encoded UTF-8

        UNSUPPORTED_12          = 0x1000, // Reserved by PKWARE for enhanced compression
        UNSUPPORTED_13          = 0x2000, // Set when encrypting the Central Directory
        UNSUPPORTED_14          = 0x4000, // Reserved by PKWARE
        UNSUPPORTED_15          = 0x8000, // Reserved by PKWARE
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

    class CentralDirectoryFileHeader : public Meta::StructuredObject
    {
        /*  TODO: Implement large file support.
        This type currently represents a "zip32" Central Directory Header.  We need to refactor this into 3 types:
        a "header" type that reads all the way down to field 7 (CRC-32) as-is, but that replaces fields 8 & 9 with a
        new meta object type (ambiguous?) whose implementation is determined by the version needed to extract (field 2)'s
        value.

        This type would replace its existing compressed & uncompressed sizes properties (encapsulated in fields 8 & 9)
        with a 64-bit value version of those methods:
            std::uint64_t GetCompressedSize()   { ...
            std::uint64_t GetUncompressedSize() { ...
            void SetCompressedSize  (std::uint64_t...
            void SetUncompressedSize(std::uint64_t...

        The underlying implementation of these methods would validate the resulting in/out values and pass the correct
        static_casted value to the new meta object type (ambiguous?) which would then hold the correct value, as well
        as handle the correct sizes w.r.t. (de)serialization.

        As-is I don't believe that we "need" this for now, so keeping the implementation "simpler" is probably the correct
        answer for now.
        */
    public:
        virtual ~CentralDirectoryFileHeader() {}

        CentralDirectoryFileHeader(StreamBase* s) : stream(s), Meta::StructuredObject(
        {
            // 0 - central file header signature   4 bytes(0x02014b50)
            std::make_shared<Meta::Field4Bytes>(s, [](std::uint32_t& v)
            {   if (v != static_cast<std::uint32_t>(Signatures::CentralFileHeader))
                {   throw ZipException("signature mismatch", ZipException::Error::InvalidCentralDirectoryHeader);
                }
            }),
            // 1 - version made by                 2 bytes
            std::make_shared<Meta::Field2Bytes>(s, [](std::uint16_t& v)
            {   if (v != static_cast<std::uint16_t>(ZipVersions::Zip64FormatExtension))
                {   throw ZipException("unsupported version made by", ZipException::Error::InvalidCentralDirectoryHeader);
                }
            }),
            // 2 - version needed to extract       2 bytes
            std::make_shared<Meta::Field2Bytes>(s, [](std::uint16_t& v)
            {   if ((v != static_cast<std::uint16_t>(ZipVersions::Zip32DefaultVersion)) && 
                    (v != static_cast<std::uint16_t>(ZipVersions::Zip64FormatExtension))
                )
                {   throw ZipException("unsupported version needed to extract", ZipException::Error::InvalidCentralDirectoryHeader);
                }
            }),
            // 3 - general purpose bit flag        2 bytes
            std::make_shared<Meta::Field2Bytes>(s, [](std::uint16_t& v)
            {   if ((v & static_cast<std::uint16_t>(UnsupportedFlagsMask)) != 0)
                {   throw ZipException("unsupported flag(s) specified", ZipException::Error::InvalidCentralDirectoryHeader);
                }
            }),
            // 4 - compression method              2 bytes
            std::make_shared<Meta::Field2Bytes>(s, [](std::uint16_t& v)
            {   if ((v != static_cast<std::uint16_t>(CompressionType::Store)) && 
                    (v != static_cast<std::uint16_t>(CompressionType::Deflate))
                )
                {   throw ZipException("unsupported compression method", ZipException::Error::InvalidCentralDirectoryHeader);
                }
            }),
            // 5 - last mod file time              2 bytes
            std::make_shared<Meta::Field2Bytes>(s, [](std::uint16_t& v) {}),
            // 6 - last mod file date              2 bytes
            std::make_shared<Meta::Field2Bytes>(s, [](std::uint16_t& v) {}),
            // 7 - crc - 32                          4 bytes
            std::make_shared<Meta::Field4Bytes>(s,[&](std::uint32_t& v) {}),
            // 8 - compressed size                 4 bytes
            std::make_shared<Meta::Field4Bytes>(s, [](std::uint32_t& v) {}),
            // 9 - uncompressed size               4 bytes
            std::make_shared<Meta::Field4Bytes>(s, [](std::uint32_t& v) {}),
            //10 - file name length                2 bytes
            std::make_shared<Meta::Field2Bytes>(s, [&](std::uint16_t& v)
            {   if (v > std::numeric_limits<std::uint16_t>::max())
                {   throw ZipException("file name exceeds max size", ZipException::Error::InvalidCentralDirectoryHeader);
                }
                Meta::Object::GetValue<std::vector<std::uint8_t>>(Field(17))->resize(v, 0);
            }),
            //11 - extra field length              2 bytes
            std::make_shared<Meta::Field2Bytes>(s, [&](std::uint16_t& v)
            {   if (v > std::numeric_limits<std::uint16_t>::max())
                {   throw ZipException("file name exceeds max size", ZipException::Error::InvalidCentralDirectoryHeader);
                }
                Meta::Object::GetValue<std::vector<std::uint8_t>>(Field(18))->resize(v, 0);
            }),
            //12 - file comment length             2 bytes
            std::make_shared<Meta::Field2Bytes>(s, [&](std::uint16_t& v)
            {   if (v > std::numeric_limits<std::uint16_t>::max())
                {   throw ZipException("file comment exceeds max size", ZipException::Error::InvalidCentralDirectoryHeader);
                }
                Meta::Object::GetValue<std::vector<std::uint8_t>>(Field(19))->resize(v, 0);
            }),
            //13 - disk number start               2 bytes
            std::make_shared<Meta::Field2Bytes>(s, [](std::uint16_t& v)
            {   if (v != 0)
                {   throw ZipException("unsupported disk number start", ZipException::Error::InvalidCentralDirectoryHeader);
                }
            }),
            //14 - internal file attributes        2 bytes
            std::make_shared<Meta::Field2Bytes>(s, [](std::uint16_t& v)
            {   if (v != 0)
                {   throw ZipException("unsupported internal file attributes", ZipException::Error::InvalidCentralDirectoryHeader);
                }
            }),
            //15 - external file attributes        4 bytes
            std::make_shared<Meta::Field4Bytes>(s,[](std::uint32_t& v)
            {   if (v != 0)
                {   //throw ZipException("unsupported external file attributes", ZipException::Error::InvalidCentralDirectoryHeader);
                }
            }),
            //16 - relative offset of local header 4 bytes
            std::make_shared<Meta::Field4Bytes>(s,[&](std::uint32_t& v)
            {   if (v >= stream->Ftell())
                {   throw ZipException("invalid relative offset", ZipException::Error::InvalidCentralDirectoryHeader);
                }
            }),
            //17 - file name(variable size)
            std::make_shared<Meta::FieldNBytes>(s, [](std::vector<std::uint8_t>& data) {}),
            //18 - extra field(variable size)
            std::make_shared<Meta::FieldNBytes>(s, [](std::vector<std::uint8_t>& data) {}),
            //19 - file comment(variable size)
            std::make_shared<Meta::FieldNBytes>(s, [](std::vector<std::uint8_t>& data) {})
        })
        {/*constructor*/
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

        std::uint16_t GetVersionNeededToExtract() { return *Meta::Object::GetValue<std::uint16_t>(Field(2)); }

        GeneralPurposeBitFlags GetGeneralPurposeBitFlag() { return static_cast<GeneralPurposeBitFlags>(*Meta::Object::GetValue<std::uint16_t>(Field(3))); }
        void SetGeneralPurposeBitFlag(std::uint16_t value) { Meta::Object::SetValue(Field(3), value); }

        std::uint16_t GetCompressionMethod() { return *Meta::Object::GetValue<std::uint16_t>(Field(4)); }
        void SetCompressionMethod(std::uint16_t value) { Meta::Object::SetValue(Field(4), value); }

        std::uint32_t GetCrc32() { return *Meta::Object::GetValue<std::uint32_t>(Field(7)); }
        void SetCrc(std::uint32_t value) { Meta::Object::SetValue(Field(7), value); }

        std::uint32_t GetCompressedSize() { return *Meta::Object::GetValue<std::uint32_t>(Field(8)); }
        void SetCompressedSize(std::uint32_t value) { Meta::Object::SetValue(Field(8), value); }

        std::uint32_t GetUncompressedSize() { return *Meta::Object::GetValue<std::uint32_t>(Field(9)); }
        void SetUncompressedSize(std::uint32_t value) { Meta::Object::SetValue(Field(9), value); }

        std::uint32_t GetRelativeOffsetOfLocalHeader() { return *Meta::Object::GetValue<std::uint32_t>(Field(16)); }
        void SetRelativeOffsetOfLocalHeader(std::uint32_t value) { Meta::Object::SetValue(Field(16), value); }

        std::string GetFileName()
        {
            auto data = Meta::Object::GetValue<std::vector<std::uint8_t>>(Field(17));
            return std::string(data->begin(), data->end());
        }

        void SetFileName(std::string name)
        {
            auto data = Meta::Object::GetValue<std::vector<std::uint8_t>>(Field(17));
            data->resize(name.size());
            data->assign(name.begin(), name.end());
            SetFileNameLength(static_cast<std::uint16_t>(name.size()));
        }

    private:
        void SetSignature(std::uint32_t value)              { Meta::Object::SetValue(Field(0), value); }
        void SetVersionMadeBy(std::uint16_t value)          { Meta::Object::SetValue(Field(1), value); }
        void SetVersionNeededToExtract(std::uint16_t value) { Meta::Object::SetValue(Field(2), value); }

        void SetLastModFileTime(std::uint16_t value)        { Meta::Object::SetValue(Field(5), value); }
        void SetLastModFileDate(std::uint16_t value)        { Meta::Object::SetValue(Field(6), value); }

        void SetFileNameLength(std::uint16_t value)         { Meta::Object::SetValue(Field(10), value); }
        void SetExtraFieldLength(std::uint16_t value)       { Meta::Object::SetValue(Field(11), value); }
        void SetFileCommentLength(std::uint16_t value)      { Meta::Object::SetValue(Field(12), value); }
        void SetDiskNumberStart(std::uint16_t value)        { Meta::Object::SetValue(Field(13), value); }
        void SetInternalFileAttributes(std::uint16_t value) { Meta::Object::SetValue(Field(14), value); }
        void SetExternalFileAttributes(std::uint16_t value) { Meta::Object::SetValue(Field(15), value); }

        StreamBase* stream = nullptr;
    };//class CentralDirectoryFileHeader

    // This represents a raw stream over a.zip file.
    class ZipObject
    {
    public:
        ZipObject(StreamPtr&& stream) : m_stream(std::move(stream)) { }

        void Read();

        std::vector<std::string> GetFileNames();

    protected:
        StreamPtr m_stream;
        std::map<std::string, std::shared_ptr<ZipFileStream>>                m_streams;
        std::map<std::string, std::shared_ptr<CentralDirectoryFileHeader>>   m_centralDirectory;

        // TODO: change to uint64_t when adding full zip64 support
        //std::map<std::uint32_t, std::shared_ptr<LocalFileHeader>>   fileRepository;

    };//class ZipObject
}
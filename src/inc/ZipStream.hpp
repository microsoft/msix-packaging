
#include "Exceptions.hpp"
#include "StreamBase.hpp"
#include "ObjectBase.hpp"

#include <string>
#include <limits>
#include <functional>

namespace xPlat {

    class ZipException : public ExceptionBase
    {
    public:
        enum Error : std::uint32_t
        {
            InvalidHeader = 1,
            FieldOutOfRange = 2,
        };

        ZipException(std::string message, Error error) : reason(message), ExceptionBase(ExceptionBase::Facility::ZIP)
        {
            SetLastError(error);
        }

        std::string reason;
    };

    namespace Meta
    {
        class FieldNBytes : public FieldBase<std::vector<std::uint8_t>>
        {
        public:
            using Lambda = std::function<void(std::vector<std::uint8_t>& v)>;
            FieldNBytes(StreamBase& stream, Lambda validator) : FieldBase(stream, validator) {}

            size_t Size()   { return Value().size(); }

            virtual void Write()
            {
                Validate();
                stream.Write(Size(), static_cast<std::uint8_t>(const_cast<T>(Value().data())));
            }

            virtual void Read()
            {
                stream.Read(Size(), static_cast<std::uint8_t>(const_cast<T>(Value().data())));
                Validate();
            }
        };
    }

    // This represents a raw stream over a.zip file.
    class ZipStream : public StreamBase
    {
        enum MagicNumbers
        {
            Zip64MinimumVersion = 45,
            Zip32DefaultVersion = 20
        };

        // from ZIP file format specification detailed in AppNote.txt
        enum Signatures : std::uint32_t
        {
            LocalFileHeader = 0x04034b50,
            DataDescriptor = 0x08074b50,
            CentralFileHeader = 0x02014b50,
            Zip64EndOfCD = 0x06064b50,
            Zip64EndOfCDLocator = 0x07064b50,
            EndOfCentralDirectory = 0x06054b50,
        };

        enum CompressionType : std::uint16_t
        {
            Store = 0,
            Deflate = 8,
        };

        class LocalFileHeader : public StructuredObject
        {
        public:
            std::uint16_t GetFileNameLength()   { return Field(9).Value<std::uint16_t>(); }
            void SetFileNameLength(std::uint16_t value) { Field(9).SetValue(value); }

            std::uint16_t GetExtraFieldLength() { return Field(10).Value<std::uint16_t>(); }
            void SetExtraFieldLength(std::uint16_t value) { Field(10).SetValue(value); }

            std::uint32_t GetCompressedSize() { return Field(7).Value<std::uint32_t>(); }
            void SetCompressedSize(std::uint32_t value) { Field(7).SetValue(value); }

            std::uint32_t GetUncompressedSize() { return Field(8).Value<std::uint32_t>(); }
            void SetUncompressedSize(std::uint32_t value) { Field(8).SetValue(value); }

            std::string   GetFileName() {
                auto data = Field(11).Value<std::vector<std::uint8_t>>();
                return std::string(data.begin(), data.end());
            }

            void SetFileName(std::string name)
            {
                auto data = Field(11).Value<std::vector<std::uint8_t>>();
                data.resize(name.size());
                data.assign(name.begin(), name.end());
                SetFileNameLength(static_cast<std::uint16_t>(name.size()));
            }

            LocalFileHeader(StreamBase& stream) : StructuredObject(
            {
                // 0 - local file header signature     4 bytes(0x04034b50)
                Meta::Field4Bytes(stream, [](std::uint32_t& v)
                {   if (v != Signatures::LocalFileHeader)
                    {   throw ZipException("file header does not match signature", ZipException::Error::InvalidHeader);
                    }
                }),
                // 1 - version needed to extract       2 bytes
                Meta::Field2Bytes(stream, [](std::uint16_t& v) {}),
                // 2 - general purpose bit flag        2 bytes
                Meta::Field2Bytes(stream, [](std::uint16_t& v) {}),
                // 3 - compression method              2 bytes
                Meta::Field2Bytes(stream, [](std::uint16_t& v) {}),
                // 4 - last mod file time              2 bytes
                Meta::Field2Bytes(stream, [](std::uint16_t& v) {}),
                // 5 - last mod file date              2 bytes
                Meta::Field2Bytes(stream, [](std::uint16_t& v) {}),
                // 6 - crc - 32                        4 bytes
                Meta::Field4Bytes(stream, [](std::uint32_t& v) {}),
                // 7 - compressed size                 4 bytes
                Meta::Field4Bytes(stream, [](std::uint32_t& v) {}),
                // 8 - uncompressed size               4 bytes
                Meta::Field4Bytes(stream, [](std::uint32_t& v) {}),
                // 9 - file name length                2 bytes
                Meta::Field2Bytes(stream, [this](std::uint16_t& v)
                {
                    if (GetFileNameLength() > std::numeric_limits<std::uint16_t>::max())
                    {
                        throw ZipException("file name field exceeds max size", ZipException::Error::FieldOutOfRange);
                    }
                    Field(11).Value<std::vector<std::uint8_t>>().resize(GetFileNameLength(), 0);
                }),
                // 10- extra field length              2 bytes
                Meta::Field2Bytes(stream, [this](std::uint16_t& v)
                {
                    if (GetExtraFieldLength() > std::numeric_limits<std::uint16_t>::max())
                    {
                        throw ZipException("extra field exceeds max size", ZipException::Error::FieldOutOfRange);
                    }
                    Field(12).Value<std::vector<std::uint8_t>>().resize(GetExtraFieldLength());
                }),
                // 11- file name (variable size)
                Meta::FieldNBytes(stream, [](std::vector<std::uint8_t>& data) {}),
                // 12- extra field (variable size)
                Meta::FieldNBytes(stream, [](std::vector<std::uint8_t>& data) {})
            })
            {
                //Field(0).SetValue(static_cast<std::uint32_t>(Signatures::LocalFileHeader));
            }
        };

    };
}
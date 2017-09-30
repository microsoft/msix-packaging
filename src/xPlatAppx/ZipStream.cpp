#include "Exceptions.hpp"
#include "StreamBase.hpp"
#include "ObjectBase.hpp"
#include "ZipStream.hpp"

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



    class LocalFileHeader : public Meta::StructuredObject
    {
    public:
        std::uint16_t GetFileNameLength() { return *Meta::Object::GetValue<std::uint16_t>(Field(9)); }
        void SetFileNameLength(std::uint16_t value) { Meta::Object::SetValue(Field(9), value); }

        std::uint16_t GetExtraFieldLength() { return *Meta::Object::GetValue<std::uint16_t>(Field(10)); }
        void SetExtraFieldLength(std::uint16_t value) { Meta::Object::SetValue(Field(10), value); }

        std::uint32_t GetCompressedSize() { return *Meta::Object::GetValue<std::uint32_t>(Field(7)); }
        void SetCompressedSize(std::uint32_t value) { Meta::Object::SetValue(Field(7), value); }

        std::uint32_t GetUncompressedSize() { return *Meta::Object::GetValue<std::uint32_t>(Field(8)); }
        void SetUncompressedSize(std::uint32_t value) { Meta::Object::SetValue(Field(8), value); }

        std::string   GetFileName() {
            auto data = *Meta::Object::GetValue<std::vector<std::uint8_t>>(Field(11));
            return std::string(data.begin(), data.end());
        }

        void SetFileName(std::string name)
        {
            auto data = *Meta::Object::GetValue<std::vector<std::uint8_t>>(Field(11));
            data.resize(name.size());
            data.assign(name.begin(), name.end());
            SetFileNameLength(static_cast<std::uint16_t>(name.size()));
        }

        LocalFileHeader(StreamBase* stream) : Meta::StructuredObject(
        {
            // 0 - local file header signature     4 bytes(0x04034b50)
            std::make_shared<Meta::Field4Bytes>(stream, [](std::uint32_t& v)
            {
                if (v != Signatures::LocalFileHeader)
                {
                    throw ZipException("file header does not match signature", ZipException::Error::InvalidHeader);
                }
            }),
            // 1 - version needed to extract       2 bytes
            std::make_shared<Meta::Field2Bytes>(stream, [](std::uint16_t& v) {}),
            // 2 - general purpose bit flag        2 bytes
            std::make_shared<Meta::Field2Bytes>(stream, [](std::uint16_t& v) {}),
            // 3 - compression method              2 bytes
            std::make_shared<Meta::Field2Bytes>(stream, [](std::uint16_t& v) {}),
            // 4 - last mod file time              2 bytes
            std::make_shared<Meta::Field2Bytes>(stream, [](std::uint16_t& v) {}),
            // 5 - last mod file date              2 bytes
            std::make_shared<Meta::Field2Bytes>(stream, [](std::uint16_t& v) {}),
            // 6 - crc - 32                        4 bytes
            std::make_shared<Meta::Field4Bytes>(stream, [](std::uint32_t& v) {}),
            // 7 - compressed size                 4 bytes
            std::make_shared<Meta::Field4Bytes>(stream, [](std::uint32_t& v) {}),
            // 8 - uncompressed size               4 bytes
            std::make_shared<Meta::Field4Bytes>(stream, [](std::uint32_t& v) {}),
            // 9 - file name length                2 bytes
            std::make_shared<Meta::Field2Bytes>(stream, [this](std::uint16_t& v)
            {
                if (GetFileNameLength() > std::numeric_limits<std::uint16_t>::max())
                {
                    throw ZipException("file name field exceeds max size", ZipException::Error::FieldOutOfRange);
                }
                Meta::Object::GetValue<std::vector<std::uint8_t>>(Field(11))->resize(GetFileNameLength(), 0);
            }),
            // 10- extra field length              2 bytes
            std::make_shared<Meta::Field2Bytes>(stream, [this](std::uint16_t& v)
            {
                if (GetExtraFieldLength() > std::numeric_limits<std::uint16_t>::max())
                {
                    throw ZipException("extra field exceeds max size", ZipException::Error::FieldOutOfRange);
                }
                Meta::Object::GetValue<std::vector<std::uint8_t>>(Field(12))->resize(GetExtraFieldLength(), 0);
            }),
            // 11- file name (variable size)
            std::make_shared<Meta::FieldNBytes>(stream, [](std::vector<std::uint8_t>& data) {}),
            // 12- extra field (variable size)
            std::make_shared<Meta::FieldNBytes>(stream, [](std::vector<std::uint8_t>& data) {})
        })
        {/*constructor*/
        }
    }; //class LocalFileHeader

    class DataDescriptor : public Meta::StructuredObject
    {
    public:
        std::uint32_t GetCrc32() { return *Meta::Object::GetValue<std::uint32_t>(Field(0)); }
        void SetCrc32(std::uint32_t value) { Meta::Object::SetValue(Field(0), value); }

        std::uint32_t GetCompressedSize() { return *Meta::Object::GetValue<std::uint32_t>(Field(1)); }
        void SetCompressedSize(std::uint32_t value) { Meta::Object::SetValue(Field(1), value); }

        std::uint32_t GetUncompressedSize() { return *Meta::Object::GetValue<std::uint32_t>(Field(2)); }
        void SetUncompressedSize(std::uint32_t value) { Meta::Object::SetValue(Field(2), value); }

        DataDescriptor(StreamBase* stream) : Meta::StructuredObject(
        {
            // 0 - crc - 32                          4 bytes    
            std::make_shared<Meta::Field4Bytes>(stream, [](std::uint32_t& v) {}),
            // 1 - compressed size                 4 bytes
            std::make_shared<Meta::Field4Bytes>(stream, [](std::uint32_t& v) {}),
            // 2 - uncompressed size               4 bytes
            std::make_shared<Meta::Field4Bytes>(stream, [](std::uint32_t& v) {})

        })
        {/*constructor*/
        }
    };//class DataDescriptor

    class Zip64EndOfCentralDirectoryRecord : public Meta::StructuredObject
    {
    public:
        Zip64EndOfCentralDirectoryRecord(StreamBase* s) : stream(s), Meta::StructuredObject(
        {
            // 0 - zip64 end of central dir signature 4 bytes(0x06064b50)
            std::make_shared<Meta::Field4Bytes>(s,[](std::uint32_t& v)
            {   if (v != Signatures::Zip64EndOfCD)
                {   throw ZipException("end of zip64 central directory does not match signature", ZipException::Error::InvalidHeader);
                }
            }),
            // 1 - size of zip64 end of central directory record 8 bytes
            std::make_shared<Meta::Field8Bytes>(s,[&](std::uint64_t& v)
            {   //4.3.14.1 The value stored into the "size of zip64 end of central
                //    directory record" should be the size of the remaining
                //    record and should not include the leading 12 bytes.
                auto size = this->Size() - 12;
                if (v != size)
                {   throw ZipException("invalid size of zip64 EOCD", ZipException::Error::InvalidZip64CentralDirectoryRecord);
                }
            }),
            // 2 - version made by                 2 bytes
            std::make_shared<Meta::Field2Bytes>(s,[](std::uint16_t& v)
            {   if (v != ZipVersions::Zip64FormatExtension)
                {   throw ZipException("invalid zip64 EOCD version made by", ZipException::Error::InvalidZip64CentralDirectoryRecord);
                }
            }),
            // 3 - version needed to extract       2 bytes
            std::make_shared<Meta::Field2Bytes>(s,[](std::uint16_t& v)
            {   if (v != ZipVersions::Zip64FormatExtension)
                {   throw ZipException("invalid zip64 EOCD version to extract", ZipException::Error::InvalidZip64CentralDirectoryRecord);
                }
            }),
            // 4 - number of this disk             4 bytes
            std::make_shared<Meta::Field4Bytes>(s,[](std::uint32_t& v)
            {   if (v != 0)
                {   throw ZipException("invalid disk number", ZipException::Error::InvalidZip64CentralDirectoryRecord);
                }
            }),
            // 5 - number of the disk with the start of the central directory  4 bytes
            std::make_shared<Meta::Field4Bytes>(s,[](std::uint32_t& v)
            {   if (v != 0)
                {   throw ZipException("invalid disk index", ZipException::Error::InvalidZip64CentralDirectoryRecord);
                }
            }),
            // 6 - total number of entries in the central directory on this disk  8 bytes
            std::make_shared<Meta::Field8Bytes>(s,[](std::uint64_t& v)
            {   if (v == 0)
                {   throw ZipException("invalid number of entries", ZipException::Error::InvalidZip64CentralDirectoryRecord);
                }
            }),
            // 7 - total number of entries in the central directory 8 bytes
            std::make_shared<Meta::Field8Bytes>(s,[&](std::uint64_t& v)
            {   if (v != this->GetTotalNumberOfEntries())
                {   throw ZipException("invalid total number of entries", ZipException::Error::InvalidZip64CentralDirectoryRecord);
                }
            }),
            // 8 - size of the central directory   8 bytes
            std::make_shared<Meta::Field8Bytes>(s,[&](std::uint64_t& v)
            {   // TODO: tighten up this validation
                if ((v == 0) ||
                    (v >= stream->Ftell()))
                {   throw ZipException("invalid size of central directory", ZipException::Error::InvalidZip64CentralDirectoryRecord);
                }
            }),
            // 9 - offset of start of central directory with respect to the starting disk number        8 bytes
            std::make_shared<Meta::Field8Bytes>(s,[&](std::uint64_t& v)
            {   // TODO: tighten up this validation
                if ((v == 0) ||
                    (v >= stream->Ftell()))
                {   throw ZipException("invalid start of central directory", ZipException::Error::InvalidZip64CentralDirectoryRecord);
                }
            }),
            //10 - zip64 extensible data sector(variable size)
            std::make_shared<Meta::FieldNBytes>(s, [](std::vector<std::uint8_t>& data)
            {   if (data.size() != 0)
                {   throw ZipException("unsupported extensible data", ZipException::Error::InvalidZip64CentralDirectoryRecord);
                }
            })
        })
        {
            SetSignature(Signatures::Zip64EndOfCD);
            SetGetSizeOfZip64CDRecord(this->Size() - 12);
            SetVersionMadeBy(ZipVersions::Zip64FormatExtension);
            SetVersionNeededToExtract(ZipVersions::Zip64FormatExtension);
            SetNumberOfThisDisk(0);
            SetTotalNumberOfEntries(0);
            Meta::Object::GetValue<std::vector<std::uint8_t>>(Field(10))->resize(0);
        }/*constructor*/

        std::uint64_t GetTotalNumberOfEntries() { return *Meta::Object::GetValue<std::uint64_t>(Field(6)); }
        void SetTotalNumberOfEntries(std::uint64_t value)
        {
            Meta::Object::SetValue(Field(6), value);
            Meta::Object::SetValue(Field(7), value);
        }

        std::uint64_t GetSizeOfCD() { return *Meta::Object::GetValue<std::uint64_t>(Field(8)); }
        void SetSizeOfCD(std::uint64_t value) { Meta::Object::SetValue(Field(8), value); }

        std::uint64_t GetOffsetfStartOfCD() { return *Meta::Object::GetValue<std::uint64_t>(Field(9)); }
        void SetOffsetfStartOfCD(std::uint64_t value) { Meta::Object::SetValue(Field(9), value); }

    private:
        void SetSignature(std::uint32_t value)              { Meta::Object::SetValue(Field(0), value); }
        void SetGetSizeOfZip64CDRecord(std::uint64_t value) { Meta::Object::SetValue(Field(1), value); }
        void SetVersionMadeBy(std::uint16_t value)          { Meta::Object::SetValue(Field(2), value); }
        void SetVersionNeededToExtract(std::uint16_t value) { Meta::Object::SetValue(Field(3), value); }
        void SetNumberOfThisDisk(std::uint32_t value)       { Meta::Object::SetValue(Field(4), value); }

        StreamBase* stream = nullptr;
    }; //class Zip64EndOfCentralDirectoryRecord

    class Zip64EndOfCentralDirectoryLocator : public Meta::StructuredObject
    {
    public:
        Zip64EndOfCentralDirectoryLocator(StreamBase* s) : stream(s), Meta::StructuredObject(
        {
            // 0 - zip64 end of central dir locator signature 4 bytes(0x07064b50)
            std::make_shared<Meta::Field4Bytes>(s, [](std::uint32_t& v)
            {   if (v != Signatures::Zip64EndOfCDLocator)
                {   throw ZipException("end of central directory locator does not match signature", ZipException::Error::InvalidHeader);
                }
            }),
            // 1 - number of the disk with the start of the zip64 end of central directory               4 bytes
            std::make_shared<Meta::Field4Bytes>(s, [](std::uint32_t& v)
            {   if (v != 0)
                {   throw ZipException("Invalid disk number", ZipException::InvalidZip64CentralDirectoryLocator);
                }
            }),
            // 2 - relative offset of the zip64 end of central directory record 8 bytes
            std::make_shared<Meta::Field8Bytes>(s, [&](std::uint64_t& v)
            {   if ((v == 0) ||
                    (v >= stream->Ftell()))
                {   throw ZipException("Invalid relative offset", ZipException::InvalidZip64CentralDirectoryLocator);
                }
            }),
            // 3 - total number of disks           4 bytes
            std::make_shared<Meta::Field4Bytes>(s, [](std::uint32_t& v)
            {   if (v != 1)
                {   throw ZipException("Invalid total number of disks", ZipException::InvalidZip64CentralDirectoryLocator);
                }
            })
        })
        {/*constructor*/
            SetSignature(Signatures::Zip64EndOfCDLocator);
            SetNumberOfDisk(0);
            SetTotalNumberOfDisks(1);
        }

        std::uint64_t GetRelativeOffset() { return *Meta::Object::GetValue<std::uint64_t>(Field(2)); }
        void SetRelativeOffset(std::uint64_t value) { Meta::Object::SetValue(Field(2), value); }

    private:
        void SetSignature(std::uint32_t value) { Meta::Object::SetValue(Field(0), value); }
        void SetNumberOfDisk(std::uint32_t value) { Meta::Object::SetValue(Field(1), value); }
        void SetTotalNumberOfDisks(std::uint32_t value) { Meta::Object::SetValue(Field(3), value); }

        StreamBase* stream = nullptr;
    }; //class Zip64EndOfCentralDirectoryLocator

    class EndCentralDirectoryRecord : public Meta::StructuredObject
    {
    public:
        EndCentralDirectoryRecord(StreamBase* stream) : Meta::StructuredObject(
        {
            // 0 - end of central dir signature    4 bytes  (0x06054b50)
            std::make_shared<Meta::Field4Bytes>(stream, [](std::uint32_t& v)
            {   if (v != Signatures::EndOfCentralDirectory)
                {   throw ZipException("invalid signiture", ZipException::Error::InvalidEndOfCentralDirectoryRecord);
                }
            }),
            // 1 - number of this disk             2 bytes
            std::make_shared<Meta::Field2Bytes>(stream, [](std::uint16_t& v)
            {   if (v != 0)
                {   throw ZipException("unsupported disk number", ZipException::Error::InvalidEndOfCentralDirectoryRecord);
                }
            }),
            // 2 - number of the disk with the start of the central directory  2 bytes
            std::make_shared<Meta::Field2Bytes>(stream, [](std::uint16_t& v)
            {   if (v != 0)
                {   throw ZipException("unsupported EoCDR disk number", ZipException::Error::InvalidEndOfCentralDirectoryRecord);
                }
            }),
            // 3 - total number of entries in the central directory on this disk  2 bytes
            std::make_shared<Meta::Field2Bytes>(stream, [](std::uint16_t& v)
            {   if (v != std::numeric_limits<std::uint16_t>::max())
                {
                throw ZipException("unsupported total number of entries on this disk", ZipException::Error::InvalidEndOfCentralDirectoryRecord);
                }
            }),
            // 4 - total number of entries in the central directory           2 bytes
            std::make_shared<Meta::Field2Bytes>(stream, [](std::uint16_t& v)
            {   if (v != std::numeric_limits<std::uint16_t>::max())
                {   throw ZipException("unsupported total number of entries", ZipException::Error::InvalidEndOfCentralDirectoryRecord);
                }
            }),
            // 5 - size of the central directory   4 bytes
            std::make_shared<Meta::Field4Bytes>(stream, [](std::uint32_t& v)
            {   if (v != std::numeric_limits<std::uint32_t>::max())
                {   throw ZipException("unsupported size of central directory", ZipException::Error::InvalidEndOfCentralDirectoryRecord);
                }
            }),
            // 6 - offset of start of central directory with respect to the starting disk number        4 bytes
            std::make_shared<Meta::Field4Bytes>(stream, [](std::uint32_t& v)
            {   if (v != std::numeric_limits<std::uint32_t>::max())
                {   throw ZipException("unsupported offset of start of central directory", ZipException::Error::InvalidEndOfCentralDirectoryRecord);
                }
            }),
            // 7 - .ZIP file comment length        2 bytes
            std::make_shared<Meta::Field2Bytes>(stream, [this](std::uint16_t& v)
            {   if (v != 0)
                {   throw ZipException("Zip comment unsupported", ZipException::Error::InvalidEndOfCentralDirectoryRecord);
                }
            }),
            // 8 - .ZIP file comment       (variable size)
            std::make_shared<Meta::FieldNBytes>(stream, [](std::vector<std::uint8_t>& data)
            {   if (data.size() != 0)
                {   throw ZipException("Zip comment unsupported", ZipException::Error::InvalidEndOfCentralDirectoryRecord);
                }
            })
        })
        {/*constructor*/
            SetSignature(Signatures::EndOfCentralDirectory);
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
        void SetSignature(std::uint32_t value) { Meta::Object::SetValue(Field(0), value); }
        void SetNumberOfDisk(std::uint16_t value) { Meta::Object::SetValue(Field(1), value); }
        void SetDiskStart(std::uint16_t value) { Meta::Object::SetValue(Field(2), value); }
        void SetTotalNumberOfEntries(std::uint16_t value) { Meta::Object::SetValue(Field(3), value); }
        void SetTotalEntriesInCentralDirectory(std::uint16_t value) { Meta::Object::SetValue(Field(4), value); }
        void SetSizeOfCentralDirectory(std::uint32_t value) { Meta::Object::SetValue(Field(5), value); }
        void SetOffsetOfCentralDirectory(std::uint32_t value) { Meta::Object::SetValue(Field(6), value); }
        void SetCommentLength(std::uint16_t value) { Meta::Object::SetValue(Field(7), value); }

    };//class EndOfCentralDirectoryRecord

    void ZipStream::Read()
    {
        // Confirm that the file IS the correct format
        EndCentralDirectoryRecord endCentralDirectoryRecord(stream.get());
        stream->Seek(-1 * endCentralDirectoryRecord.Size(), StreamBase::Reference::END);
        endCentralDirectoryRecord.Read();

        // find where the zip central directory exists.
        Zip64EndOfCentralDirectoryLocator zip64Locator(stream.get());
        stream->Seek(-1*(endCentralDirectoryRecord.Size() + zip64Locator.Size()), StreamBase::Reference::END);
        zip64Locator.Read();

        // now read the zip central directory
        Zip64EndOfCentralDirectoryRecord zip64EndOfCentralDirectory(stream.get());
        stream->Seek(zip64Locator.GetRelativeOffset(), StreamBase::Reference::START);
        zip64EndOfCentralDirectory.Read();
        stream->Seek(zip64EndOfCentralDirectory.GetOffsetfStartOfCD(), StreamBase::Reference::START);
        for (std::uint32_t index = 0; index < zip64EndOfCentralDirectory.GetTotalNumberOfEntries(); index++)
        {
            auto centralFileHeader = std::make_shared<CentralDirectoryFileHeader>(stream.get());
            centralFileHeader->Read();
            centralDirectory.insert(std::make_pair(centralFileHeader->GetFileName(), centralFileHeader));
        }

        // We should have no data between the end of the last central directory header and the start of the EoCD
        if (stream->Ftell() != zip64Locator.GetRelativeOffset())
        {   throw ZipException("hidden data unsupported", ZipException::Error::HiddenDataBetweenLastCDHandEoCD);
        }
    }

    std::vector<std::string> ZipStream::GetFileNames()
    {
        std::vector<std::string> result;
        std::for_each(streams.begin(), streams.end(), [&](auto it)
        {
            result.push_back(it.first);
        });
        return result;
    }

} // namespace xPlat
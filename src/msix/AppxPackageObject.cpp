//
//  Copyright (C) 2017 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
// 
#include "AppxPackaging.hpp"
#include "Exceptions.hpp"
#include "ComHelper.hpp"
#include "StreamBase.hpp"
#include "StorageObject.hpp"
#include "AppxPackageObject.hpp"
#include "UnicodeConversion.hpp"
#include "IXml.hpp"
#include "MSIXResource.hpp"

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <functional>
#include <limits>

namespace MSIX {

    // names of footprint files.
    #define APPXBLOCKMAP_XML  "AppxBlockMap.xml"
    #define APPXMANIFEST_XML  "AppxManifest.xml"
    #define CODEINTEGRITY_CAT "AppxMetadata/CodeIntegrity.cat"
    #define APPXSIGNATURE_P7X "AppxSignature.p7x"
    #define CONTENT_TYPES_XML "[Content_Types].xml"

    static const std::map<APPX_FOOTPRINT_FILE_TYPE, std::string> footprintFiles = 
    {
        {APPX_FOOTPRINT_FILE_TYPE_MANIFEST,         APPXMANIFEST_XML},
        {APPX_FOOTPRINT_FILE_TYPE_BLOCKMAP,         APPXBLOCKMAP_XML},
        {APPX_FOOTPRINT_FILE_TYPE_SIGNATURE,        APPXSIGNATURE_P7X},
        {APPX_FOOTPRINT_FILE_TYPE_CODEINTEGRITY,    CODEINTEGRITY_CAT},
    };

    static const std::uint8_t PercentangeEncodingTableSize = 0x5E;
    static const std::vector<std::string> PercentangeEncoding =
    {   "", "", "", "", "", "", "", "",
        "", "", "", "", "", "", "", "",
        "", "", "", "", "", "", "", "",
        "", "", "", "", "", "", "", "",
        "%20", "%21", "", "%23", "%24", "%25", "%26", "%27", // [space] ! # $ % & '
        "%28", "%29", "", "%2B", "%2C", "", "", "", // ( ) + ,
        "", "", "", "", "", "", "", "",
        "", "", "", "%3B",   "", "%3D", "", "",   // ; =
        "%40",   "", "", "", "", "", "", "", // @
        "", "", "", "", "", "", "", "",
        "", "", "", "", "", "", "", "",
        "", "", "", "%5B", "", "%5D" // [ ]
    };

    static const std::map<std::string, char> EncodingToChar = 
    {   {"20", ' '}, {"21", '!'}, {"23", '#'},  {"24", '$'},
        {"25", '%'}, {"26", '&'}, {"27", '\''}, {"28", '('},
        {"29", ')'}, {"25", '+'}, {"2B", '%'},  {"2C", ','},
        {"3B", ';'}, {"3D", '='}, {"40", '@'},  {"5B", '['},
        {"5D", ']'}
    };

    static std::string EncodeFileName(std::string fileName)
    {
        std::string result;
        for (std::uint32_t position = 0; position < fileName.length(); ++position)
        {   std::uint8_t index = static_cast<std::uint8_t>(fileName[position]);
            if(fileName[position] < PercentangeEncodingTableSize && index < PercentangeEncoding.size() && !PercentangeEncoding[index].empty())
            {   result += PercentangeEncoding[index];
            }
            else if (fileName[position] == '\\') // Remove Windows file separator.
            {   result += '/';
            }
            else
            {   result += fileName[position];
            }
        }
        return result;
    }

    static std::string DecodeFileName(std::string fileName)
    {
        std::string result;
        for (std::uint32_t i = 0; i < fileName.length(); ++i)
        {   if(fileName[i] == '%')
            {   const auto& found = EncodingToChar.find(fileName.substr(i+1, 2));
                ThrowErrorIf(Error::UnknownFileNameEncoding, (found == EncodingToChar.end()), fileName.c_str())
                result += found->second;                
                i += 2;
            }
            else
            {   result += fileName[i];
            }
        }
        return result;
    }

    AppxPackageId::AppxPackageId(
        const std::string& name,
        const std::string& version,
        const std::string& resourceId,
        const std::string& architecture,
        const std::string& publisher) :
        Name(name), Version(version), ResourceId(resourceId), Architecture(architecture), Publisher(publisher)
    {
        // This should go away once the schema validation is on
        // Only name, publisher and version are required
        ThrowErrorIf(Error::AppxManifestSemanticError, (Name.empty() || Version.empty() || Publisher.empty()), "Invalid Identity element");

        // TODO: calculate the publisher hash from the publisher value.
    }

    AppxManifestObject::AppxManifestObject(IXmlFactory* factory, ComPtr<IStream>& stream) : m_stream(stream)
    {
        size_t identitiesFound = 0;
        auto dom = factory->CreateDomFromStream(XmlContentType::AppxManifestXml, stream);
        dom->ForEachElementIn(dom->GetDocument().Get(), XmlQueryName::Package_Identity,
            [&](IXmlElement* identityNode){
                identitiesFound++;
                ThrowErrorIf(Error::AppxManifestSemanticError, (identitiesFound > 1), "There must be only one Identity element at most in AppxManifest.xml");

                const auto& name           = identityNode->GetAttributeValue(XmlAttributeName::Package_Identity_Name);
                const auto& architecture   = identityNode->GetAttributeValue(XmlAttributeName::Package_Identity_ProcessorArchitecture);
                const auto& publisher      = identityNode->GetAttributeValue(XmlAttributeName::Package_Identity_Publisher);
                const auto& version        = identityNode->GetAttributeValue(XmlAttributeName::Package_Identity_Version);
                const auto& resourceId     = identityNode->GetAttributeValue(XmlAttributeName::Package_Identity_ResourceId);

                m_packageId = std::make_unique<AppxPackageId>(name, version, resourceId, architecture, publisher);
                return true;             
            }
        );
        // Have to check for this semantically as not all validating parsers can validate this via schema
        ThrowErrorIf(Error::AppxManifestSemanticError, (identitiesFound == 0), "No Identity element in AppxManifest.xml");
    }

    AppxPackageObject::AppxPackageObject(IMSIXFactory* factory, MSIX_VALIDATION_OPTION validation, IStorageObject* container) :
        m_factory(factory),
        m_validation(validation),
        m_container(container)
    {
        ComPtr<IXmlFactory> xmlFactory;
        ThrowHrIfFailed(factory->QueryInterface(UuidOfImpl<IXmlFactory>::iid, reinterpret_cast<void**>(&xmlFactory)));        

        // 1. Get the appx signature from the container and parse it
        // TODO: pass validation flags and other necessary goodness through.
        auto file = m_container->GetFile(APPXSIGNATURE_P7X);
        if ((validation & MSIX_VALIDATION_OPTION_SKIPSIGNATURE) == 0)
        {   ThrowErrorIfNot(Error::MissingAppxSignatureP7X, (file.first), "AppxSignature.p7x not in archive!");
        }

        m_appxSignature = ComPtr<IVerifierObject>::Make<AppxSignatureObject>(factory, validation, file.second);

        // 2. Get content type using signature object for validation
        file = m_container->GetFile(CONTENT_TYPES_XML);
        ThrowErrorIfNot(Error::MissingContentTypesXML, (file.first), "[Content_Types].xml not in archive!");
        MSIX::ComPtr<IStream> stream = m_appxSignature->GetValidationStream(CONTENT_TYPES_XML, file.second);        
        auto contentType = xmlFactory->CreateDomFromStream(XmlContentType::ContentTypeXml, stream);

        // 3. Get blockmap object using signature object for validation        
        file = m_container->GetFile(APPXBLOCKMAP_XML);
        ThrowErrorIfNot(Error::MissingAppxBlockMapXML, (file.first), "AppxBlockMap.xml not in archive!");
        stream = m_appxSignature->GetValidationStream(APPXBLOCKMAP_XML, file.second);
        m_appxBlockMap = ComPtr<IVerifierObject>::Make<AppxBlockMapObject>(factory, stream);

        // 4. Get manifest object using blockmap object for validation
        // TODO: pass validation flags and other necessary goodness through.
        file = m_container->GetFile(APPXMANIFEST_XML);
        ThrowErrorIfNot(Error::MissingAppxManifestXML, (file.second), "AppxManifest.xml not in archive!");
        stream = m_appxBlockMap->GetValidationStream(APPXMANIFEST_XML, file.second);
        m_appxManifest = ComPtr<IVerifierObject>::Make<AppxManifestObject>(xmlFactory.Get(), stream);
        
        if ((validation & MSIX_VALIDATION_OPTION_SKIPSIGNATURE) == 0)
        {
            std::string reason = "Publisher mismatch: '" + m_appxManifest->GetPublisher() + "' != '" + m_appxSignature->GetPublisher() + "'";
            ThrowErrorIfNot(Error::PublisherMismatch,
                (0 == m_appxManifest->GetPublisher().compare(m_appxSignature->GetPublisher())), reason.c_str());
        }

        struct Config
        {
            using lambda = std::function<MSIX::ComPtr<IStream>()>;
            Config(lambda f) : GetValidationStream(f) {}
            lambda GetValidationStream;
        };

        std::map<std::string, Config> footPrintFileNames = {
            { APPXBLOCKMAP_XML,  Config([&](){ m_footprintFiles.push_back(APPXBLOCKMAP_XML);  return m_appxBlockMap->GetStream();})  },
            { APPXMANIFEST_XML,  Config([&](){ m_footprintFiles.push_back(APPXMANIFEST_XML);  return m_appxManifest->GetStream();})  },
            { APPXSIGNATURE_P7X, Config([&](){ if (m_appxSignature->GetStream().Get()){m_footprintFiles.push_back(APPXSIGNATURE_P7X);} return m_appxSignature->GetStream();}) },
            { CODEINTEGRITY_CAT, Config([&](){ m_footprintFiles.push_back(CODEINTEGRITY_CAT); return m_appxSignature->GetValidationStream(CODEINTEGRITY_CAT, std::move(m_container->GetFile(CODEINTEGRITY_CAT).second));}) },
            { CONTENT_TYPES_XML, Config([&]()->IStream*{ return nullptr;}) }, // content types is never implicitly unpacked
        };

        // 5. Ensure that the stream collection contains streams wired up for their appropriate validation
        // and partition the container's file names into footprint and payload files.  First by going through
        // the footprint files, and then by going through the payload files.
        auto filesToProcess = m_container->GetFileNames(FileNameOptions::All);
        for (const auto& fileName : m_container->GetFileNames(FileNameOptions::FootPrintOnly))
        {   auto footPrintFile = footPrintFileNames.find(fileName);
            if (footPrintFile != footPrintFileNames.end())
            {   m_streams[fileName] = footPrintFile->second.GetValidationStream();
                filesToProcess.erase(std::remove(filesToProcess.begin(), filesToProcess.end(), fileName), filesToProcess.end());
            }
        }
        
        auto blockMapInternal = m_appxBlockMap.As<IAppxBlockMapInternal>();
        for (const auto& fileName : blockMapInternal->GetFileNames())
        {   auto footPrintFile = footPrintFileNames.find(fileName);
            if (footPrintFile == footPrintFileNames.end())
            {   std::string containerFileName = EncodeFileName(fileName);
                m_payloadFiles.push_back(containerFileName);
                auto fileStream = m_container->GetFile(containerFileName);
                ThrowErrorIfNot(Error::FileNotFound, (fileStream.first), "File described in blockmap not contained in OPC container");

                // Verify file in OPC and BlockMap
                ComPtr<IAppxFile> appxFile;
                ThrowHrIfFailed(fileStream.second->QueryInterface(UuidOfImpl<IAppxFile>::iid, reinterpret_cast<void**>(&appxFile)));
                APPX_COMPRESSION_OPTION compressionOpt;
                ThrowHrIfFailed(appxFile->GetCompressionOption(&compressionOpt));
                bool isUncompressed = (compressionOpt == APPX_COMPRESSION_OPTION_NONE);
                
                ComPtr<IAppxFileInternal> appxfileInternal;
                ThrowHrIfFailed(fileStream.second->QueryInterface(UuidOfImpl<IAppxFileInternal>::iid, reinterpret_cast<void**>(&appxfileInternal)));
                auto sizeOnZip = appxfileInternal->GetCompressSize();

                auto blocks = blockMapInternal->GetBlocks(fileName);
                std::uint64_t blocksSize = 0;
                for(auto& block : blocks)
                {   // For Block elements that don't have a Size attribute, we always set its size as BLOCKMAP_BLOCK_SIZE
                    // (even for the last one). The Size attribute isn't specified if the file is not compressed.
                    ThrowErrorIf(Error::BlockMapSemanticError, isUncompressed && (block.compressedSize != BLOCKMAP_BLOCK_SIZE),
                        "An uncompressed file has a size attribute in its Block elements");
                    blocksSize += block.compressedSize;
                }

                if(isUncompressed)
                {   UINT64 blockMapFileSize;
                    auto blockMapFile = blockMapInternal->GetFile(fileName);
                    ThrowHrIfFailed(blockMapFile->GetUncompressedSize(&blockMapFileSize));
                    ThrowErrorIf(Error::BlockMapSemanticError, (blockMapFileSize != sizeOnZip ),
                        "Uncompressed size of the file in the block map and the OPC container don't match");
                }
                else
                {   // From Windows code:
                    // The file item is compressed. There are 2 cases here:
                    // 1. The compressed size of the file is the same as the total size of all compressed blocks.
                    // 2. The compressed size of the file is 2 bytes more than the total size of all compressed blocks.
                    // It depends on how the block compression is done. MakeAppx block compression implementation will end up 
                    // with case 2. However, we shouldn't block the first case since it is totally valid and 3rd party
                    // implementation may end up with it.
                    // The reason we created compressed file item with 2 extra bytes (03 00) is because we use Z_FULL_FLUSH 
                    // flag to compress every block. If we use Z_FINISH flag to compress the last block, these 2 extra bytes will
                    // not be generated. The AddBlock()-->... -->AddBlock()-->Close() pattern in OPC push stack prevents the
                    // deflator from knowing whether the current block is the last block. So it cannot use Z_FINISH flag for
                    // the last block of the file. Note that removing the 2 extra bytes from the compressed file data will make
                    // it invalid when consumed by popular zip tools like WinZip and ShellZip. So they are required for the 
                    // packages we created.
                    ThrowErrorIfNot(Error::BlockMapSemanticError,
                        (blocksSize == sizeOnZip ) // case 1
                        || (blocksSize == sizeOnZip - 2), // case 2
                        "Compressed size of the file in the block map and the OPC container don't match");
                }
                m_streams[containerFileName] = m_appxBlockMap->GetValidationStream(fileName, fileStream.second);
                filesToProcess.erase(std::remove(filesToProcess.begin(), filesToProcess.end(), containerFileName), filesToProcess.end());
            }
        }
        // If the map is not empty, there's a file in the container that didn't go to the footprint or payload
        // files. (eg. payload file missing in the AppxBlockMap.xml)
        ThrowErrorIfNot(Error::BlockMapSemanticError, (filesToProcess.empty()), "Payload file not described in AppxBlockMap.xml");
    }

    void AppxPackageObject::Unpack(MSIX_PACKUNPACK_OPTION options, IStorageObject* to)
    {
        auto fileNames = GetFileNames(FileNameOptions::All);
        for (const auto& fileName : fileNames)
        {
            std::string targetName;
            if (options & MSIX_PACKUNPACK_OPTION_CREATEPACKAGESUBFOLDER)
            {   NOTIMPLEMENTED
                //targetName = GetAppxManifest()->GetPackageFullName() + to->GetPathSeparator() + fileName;
            }
            else
            {   targetName = DecodeFileName(fileName);
            }

            auto targetFile = to->OpenFile(targetName, MSIX::FileStream::Mode::WRITE_UPDATE);
            auto sourceFile = GetFile(fileName).second;

            ULARGE_INTEGER bytesCount = {0};
            bytesCount.QuadPart = std::numeric_limits<std::uint64_t>::max();
            ThrowHrIfFailed(sourceFile->CopyTo(targetFile, bytesCount, nullptr, nullptr));
        }
    }

    std::string AppxPackageObject::GetPathSeparator() { return "/"; }

    std::vector<std::string> AppxPackageObject::GetFileNames(FileNameOptions options)
    {
        std::vector<std::string> result;

        if ((options & FileNameOptions::FootPrintOnly) == FileNameOptions::FootPrintOnly)
        {
            result.insert(result.end(), m_footprintFiles.begin(), m_footprintFiles.end());
        }
        if ((options & FileNameOptions::PayloadOnly) == FileNameOptions::PayloadOnly)
        {
            result.insert(result.end(), m_payloadFiles.begin(), m_payloadFiles.end());
        }
        return result;
    }

    std::pair<bool,IStream*> AppxPackageObject::GetFile(const std::string& fileName)
    {
        auto result = m_streams.find(fileName);
        if (result == m_streams.end())
        {
            return std::make_pair(false, nullptr);
        }
        return std::make_pair(true, result->second.Get());
    }

    void AppxPackageObject::RemoveFile(const std::string& fileName)
    {
        // TODO: Implement
        NOTIMPLEMENTED
    }

    IStream* AppxPackageObject::OpenFile(const std::string& fileName, MSIX::FileStream::Mode mode)
    {
        // TODO: Implement
        NOTIMPLEMENTED
    }

    void AppxPackageObject::CommitChanges()
    {
        // TODO: Implement
        NOTIMPLEMENTED
    }

    // IAppxPackageReader
    HRESULT STDMETHODCALLTYPE AppxPackageObject::GetBlockMap(IAppxBlockMapReader** blockMapReader)
    {
        return static_cast<HRESULT>(Error::NotImplemented);
    }
   
    HRESULT STDMETHODCALLTYPE AppxPackageObject::GetFootprintFile(APPX_FOOTPRINT_FILE_TYPE type, IAppxFile** file)
    {
        return MSIX::ResultOf([&]() {
            ThrowErrorIf(Error::InvalidParameter, (file == nullptr || *file != nullptr), "bad pointer");
            auto footprint = footprintFiles.find(type);
            ThrowErrorIf(Error::FileNotFound, (footprint == footprintFiles.end()), "unknown footprint file type");
            ComPtr<IStream> stream = GetFile(footprint->second).second;
            ThrowErrorIf(Error::FileNotFound, (stream.Get() == nullptr), "requested footprint file not in package")
            // Clients expect the stream's pointer to be at the start of the file!
            ThrowHrIfFailed(stream->Seek({0}, StreamBase::Reference::START, nullptr)); 
            auto result = stream.As<IAppxFile>();
            *file = result.Detach();
            return static_cast<HRESULT>(Error::OK);
        });
    }

    HRESULT STDMETHODCALLTYPE AppxPackageObject::GetPayloadFile(LPCWSTR fileName, IAppxFile** file)
    {
        return MSIX::ResultOf([&]() {
            ThrowErrorIf(Error::InvalidParameter, (fileName == nullptr || file == nullptr || *file != nullptr), "bad pointer");
            std::string name = utf16_to_utf8(fileName);
            ComPtr<IStream> stream = GetFile(name).second;
            ThrowErrorIf(Error::FileNotFound, (stream.Get() == nullptr), "requested file not in package")
            // Clients expect the stream's pointer to be at the start of the file!
            ThrowHrIfFailed(stream->Seek({0}, StreamBase::Reference::START, nullptr)); 
            auto result = stream.As<IAppxFile>();
            *file = result.Detach();
            return static_cast<HRESULT>(Error::OK);
        });
    }

    HRESULT STDMETHODCALLTYPE AppxPackageObject::GetPayloadFiles(IAppxFilesEnumerator** filesEnumerator)
    {
        return MSIX::ResultOf([&]() {
            ThrowErrorIf(Error::InvalidParameter,(filesEnumerator == nullptr || *filesEnumerator != nullptr), "bad pointer");

            ComPtr<IStorageObject> storage;
            ThrowHrIfFailed(QueryInterface(UuidOfImpl<IStorageObject>::iid, reinterpret_cast<void**>(&storage)));
            auto result = ComPtr<IAppxFilesEnumerator>::Make<AppxFilesEnumerator>(storage.Get());
            *filesEnumerator = result.Detach();
            return static_cast<HRESULT>(Error::OK);
        });
    }

    HRESULT STDMETHODCALLTYPE AppxPackageObject::GetManifest(IAppxManifestReader** manifestReader)
    {
        return static_cast<HRESULT>(Error::NotImplemented);
    }
}
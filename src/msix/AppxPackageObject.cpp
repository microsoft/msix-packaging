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

    static std::string DecodeFileName(const std::string& fileName)
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

    AppxManifestObject::AppxManifestObject(IXmlFactory* factory, const ComPtr<IStream>& stream) : m_stream(stream)
    {      
        struct _context
        {
            AppxManifestObject* self;
            size_t identitiesFound;
        };
        _context context = {};
        context.self            = this;
        context.identitiesFound = 0;

        auto dom = factory->CreateDomFromStream(XmlContentType::AppxManifestXml, stream);
        dom->ForEachElementIn(dom->GetDocument().Get(), XmlQueryName::Package_Identity, static_cast<void*>(&context), 
            [](void* c, IXmlElement* identityNode){
                _context* context = reinterpret_cast<_context*>(c);
                context->identitiesFound++;
                ThrowErrorIf(Error::AppxManifestSemanticError, (context->identitiesFound > 1), "There must be only one Identity element at most in AppxManifest.xml");

                const auto& name           = identityNode->GetAttributeValue(XmlAttributeName::Package_Identity_Name);
                const auto& architecture   = identityNode->GetAttributeValue(XmlAttributeName::Package_Identity_ProcessorArchitecture);
                const auto& publisher      = identityNode->GetAttributeValue(XmlAttributeName::Package_Identity_Publisher);
                const auto& version        = identityNode->GetAttributeValue(XmlAttributeName::Package_Identity_Version);
                const auto& resourceId     = identityNode->GetAttributeValue(XmlAttributeName::Package_Identity_ResourceId);

                context->self->m_packageId = std::make_unique<AppxPackageId>(name, version, resourceId, architecture, publisher);
                return true;             
            }
        );
        // Have to check for this semantically as not all validating parsers can validate this via schema
        ThrowErrorIf(Error::AppxManifestSemanticError, (context.identitiesFound == 0), "No Identity element in AppxManifest.xml");
    }

    AppxPackageObject::AppxPackageObject(IMSIXFactory* factory, MSIX_VALIDATION_OPTION validation, const ComPtr<IStorageObject>& container) :
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
        {   ThrowErrorIf(Error::MissingAppxSignatureP7X, (nullptr == file.Get()), "AppxSignature.p7x not in archive!");
        }

        m_appxSignature = ComPtr<IVerifierObject>::Make<AppxSignatureObject>(factory, validation, file);

        // 2. Get content type using signature object for validation
        file = m_container->GetFile(CONTENT_TYPES_XML);
        ThrowErrorIf(Error::MissingContentTypesXML, (nullptr == file.Get()), "[Content_Types].xml not in archive!");
        ComPtr<IStream> stream = m_appxSignature->GetValidationStream(CONTENT_TYPES_XML, file);        
        auto contentType = xmlFactory->CreateDomFromStream(XmlContentType::ContentTypeXml, stream);

        // 3. Get blockmap object using signature object for validation        
        file = m_container->GetFile(APPXBLOCKMAP_XML);
        ThrowErrorIf(Error::MissingAppxBlockMapXML, (nullptr == file.Get()), "AppxBlockMap.xml not in archive!");
        stream = m_appxSignature->GetValidationStream(APPXBLOCKMAP_XML, file);
        m_appxBlockMap = ComPtr<IVerifierObject>::Make<AppxBlockMapObject>(factory, stream);

        // 4. Get manifest object using blockmap object for validation
        // TODO: pass validation flags and other necessary goodness through.
        file = m_container->GetFile(APPXMANIFEST_XML);
        ThrowErrorIf(Error::MissingAppxManifestXML, (nullptr == file.Get()), "AppxManifest.xml not in archive!");
        stream = m_appxBlockMap->GetValidationStream(APPXMANIFEST_XML, file);
        m_appxManifest = ComPtr<IVerifierObject>::Make<AppxManifestObject>(xmlFactory.Get(), stream);
        
        if ((validation & MSIX_VALIDATION_OPTION_SKIPSIGNATURE) == 0)
        {
            std::string reason = "Publisher mismatch: '" + m_appxManifest->GetPublisher() + "' != '" + m_appxSignature->GetPublisher() + "'";
            ThrowErrorIfNot(Error::PublisherMismatch,
                (0 == m_appxManifest->GetPublisher().compare(m_appxSignature->GetPublisher())), reason.c_str());
        }

        struct Config
        {
            typedef ComPtr<IStream> (*lambda)(AppxPackageObject* self);
            Config(lambda f) : GetValidationStream(f) {}
            lambda GetValidationStream;
        };

        std::map<std::string, Config> footPrintFileNames = {
            { APPXBLOCKMAP_XML,  Config([](AppxPackageObject* self){ self->m_footprintFiles.push_back(APPXBLOCKMAP_XML);  return self->m_appxBlockMap->GetStream();})  },
            { APPXMANIFEST_XML,  Config([](AppxPackageObject* self){ self->m_footprintFiles.push_back(APPXMANIFEST_XML);  return self->m_appxManifest->GetStream();})  },
            { APPXSIGNATURE_P7X, Config([](AppxPackageObject* self){ if (self->m_appxSignature->GetStream().Get()){self->m_footprintFiles.push_back(APPXSIGNATURE_P7X);} return self->m_appxSignature->GetStream();}) },
            { CODEINTEGRITY_CAT, Config([](AppxPackageObject* self){ self->m_footprintFiles.push_back(CODEINTEGRITY_CAT); auto file = self->m_container->GetFile(CODEINTEGRITY_CAT); return self->m_appxSignature->GetValidationStream(CODEINTEGRITY_CAT, file);}) },
            { CONTENT_TYPES_XML, Config([](AppxPackageObject*)->ComPtr<IStream>{ return ComPtr<IStream>();}) }, // content types is never implicitly unpacked
        };

        // 5. Ensure that the stream collection contains streams wired up for their appropriate validation
        // and partition the container's file names into footprint and payload files.  First by going through
        // the footprint files, and then by going through the payload files.
        auto filesToProcess = m_container->GetFileNames(FileNameOptions::All);
        for (const auto& fileName : m_container->GetFileNames(FileNameOptions::FootPrintOnly))
        {   auto footPrintFile = footPrintFileNames.find(fileName);
            if (footPrintFile != footPrintFileNames.end())
            {   m_streams[fileName] = footPrintFile->second.GetValidationStream(this);
                filesToProcess.erase(std::remove(filesToProcess.begin(), filesToProcess.end(), fileName), filesToProcess.end());
            }
        }
        
        auto blockMapStorage = m_appxBlockMap.As<IStorageObject>();
        for (const auto& fileName : blockMapStorage->GetFileNames(FileNameOptions::PayloadOnly))
        {   auto footPrintFile = footPrintFileNames.find(fileName);
            if (footPrintFile == footPrintFileNames.end())
            {   std::string containerFileName = EncodeFileName(fileName);
                m_payloadFiles.push_back(containerFileName);
                auto fileStream = m_container->GetFile(containerFileName);
                ThrowErrorIfNot(Error::FileNotFound, (fileStream.Get()), "File described in blockmap not contained in OPC container");
                m_streams[containerFileName] = m_appxBlockMap->GetValidationStream(fileName, fileStream);
                filesToProcess.erase(std::remove(filesToProcess.begin(), filesToProcess.end(), containerFileName), filesToProcess.end());
            }
        }
        // If the map is not empty, there's a file in the container that didn't go to the footprint or payload
        // files. (eg. payload file missing in the AppxBlockMap.xml)
        ThrowErrorIfNot(Error::BlockMapSemanticError, (filesToProcess.empty()), "Payload file not described in AppxBlockMap.xml");
    }

    void AppxPackageObject::Unpack(MSIX_PACKUNPACK_OPTION options, const ComPtr<IStorageObject>& to)
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
            auto sourceFile = GetFile(fileName);

            ULARGE_INTEGER bytesCount = {0};
            bytesCount.QuadPart = std::numeric_limits<std::uint64_t>::max();
            ThrowHrIfFailed(sourceFile->CopyTo(targetFile.Get(), bytesCount, nullptr, nullptr));
        }
    }

    const char* AppxPackageObject::GetPathSeparator() { return "/"; }

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

    ComPtr<IStream> AppxPackageObject::GetFile(const std::string& fileName)
    {
        auto result = m_streams.find(fileName);
        if (result == m_streams.end())
        {
            return ComPtr<IStream>();
        }
        return result->second;
    }

    void AppxPackageObject::RemoveFile(const std::string& fileName)
    {
        // TODO: Implement
        NOTIMPLEMENTED
    }

    ComPtr<IStream> AppxPackageObject::OpenFile(const std::string& fileName, MSIX::FileStream::Mode mode)
    {
        NOTIMPLEMENTED
        return ComPtr<IStream>();
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
            ComPtr<IStream> stream = GetFile(footprint->second);
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
            ComPtr<IStream> stream = GetFile(name);
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
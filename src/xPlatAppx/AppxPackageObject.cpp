#include "AppxPackaging.hpp"
#include "Exceptions.hpp"
#include "ComHelper.hpp"
#include "StreamBase.hpp"
#include "StorageObject.hpp"
#include "AppxPackageObject.hpp"
#include "UnicodeConversion.hpp"
#include "ContentTypesSchemas.hpp"

#include "xercesc/util/XMLString.hpp"
#include "xercesc/parsers/XercesDOMParser.hpp"

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <functional>
#include <limits>

XERCES_CPP_NAMESPACE_USE

namespace xPlat {

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
            {   auto found = EncodingToChar.find(fileName.substr(i+1, 2));
                if (found != EncodingToChar.end())
                {   result += found->second;
                }
                else
                {   throw Exception(Error::AppxUnknownFileNameEncoding, fileName);
                }
                i += 2;
            }
            else
            {   result += fileName[i];
            }
        }
        return result;
    }

    static std::string GetAttributeValue(DOMElement* element, std::string attributeName)
    {
        XercesXMLChPtr nameAttr(XMLString::transcode(attributeName.c_str()));
        XercesCharPtr value(XMLString::transcode(element->getAttribute(nameAttr.Get())));
        return std::string(value.Get());
    }


    AppxPackageId::AppxPackageId(
        const std::string& name,
        const std::string& version,
        const std::string& resourceId,
        const std::string& architecture,
        const std::string& publisher) :
        Name(name), Version(version), ResourceId(resourceId), Architecture(architecture), PublisherHash(publisher)
    {
        // This should go away once the schema validation is on
        // Only name, publisher and version are required
        ThrowErrorIf(Error::AppxManifestSemanticError, (Name.empty() || Version.empty() || PublisherHash.empty()), "Invalid Identity element");
    }

    AppxManifestObject::AppxManifestObject(ComPtr<IStream>& stream) : m_stream(stream)
    {
        // TODO: pass schemas to validate AppxManifest. This only validates that is a well-formed xml
        auto dom = ComPtr<IXmlObject>::Make<XmlObject>(stream);

        // Get Identity
        XercesXMLChPtr identityXPath(XMLString::transcode("/Package/Identity"));
        XercesPtr<DOMXPathNSResolver> resolver(dom->Document()->createNSResolver(dom->Document()->getDocumentElement()));
        XercesPtr<DOMXPathResult> identityResult(dom->Document()->evaluate(
            identityXPath.Get(),
            dom->Document()->getDocumentElement(),
            resolver.Get(),
            DOMXPathResult::ORDERED_NODE_SNAPSHOT_TYPE,
            nullptr));

        // This should go away once the schema validation is on
        ThrowErrorIf(Error::AppxManifestSemanticError, (identityResult->getSnapshotLength() == 0), "No Identity element in AppxManifest.xml");
        ThrowErrorIf(Error::AppxManifestSemanticError, (identityResult->getSnapshotLength() > 1), "There must be only one Identity element at most in AppxManifest.xml");

        auto identityNode = static_cast<DOMElement*>(identityResult->getNodeValue());
        auto name = GetAttributeValue(identityNode, "Name");
        auto architecture = GetAttributeValue(identityNode, "ProcessorArchitecture");
        auto publisher = GetAttributeValue(identityNode, "Publisher");
        auto version = GetAttributeValue(identityNode, "Version");
        auto resourceId = GetAttributeValue(identityNode, "ResourceId");
        m_packageId = std::make_unique<AppxPackageId>(name, version, resourceId, architecture, publisher);
    }

    AppxPackageObject::AppxPackageObject(IxPlatFactory* factory, APPX_VALIDATION_OPTION validation, IStorageObject* container) :
        m_factory(factory),
        m_validation(validation),
        m_container(container)
    {
        // 1. Get the appx signature from the container and parse it
        // TODO: pass validation flags and other necessary goodness through.
        m_appxSignature = ComPtr<IVerifierObject>::Make<AppxSignatureObject>(validation, 
            ((validation & APPX_VALIDATION_OPTION_SKIPSIGNATURE) == 0) ? m_container->GetFile(APPXSIGNATURE_P7X) : nullptr
        );

        if ((validation & APPX_VALIDATION_OPTION_SKIPSIGNATURE) == 0)
        {   ThrowErrorIfNot(Error::AppxMissingSignatureP7X, (m_appxSignature->HasStream()), "AppxSignature.p7x not in archive!");
        }

        // 2. Get content type using signature object for validation
        // TODO: switch underlying type of m_contentType to something more specific.
        auto temp = m_appxSignature->GetValidationStream(CONTENT_TYPES_XML, m_container->GetFile(CONTENT_TYPES_XML));
        m_contentType = ComPtr<IVerifierObject>::Make<XmlObject>(temp, &contentTypesSchema);
        ThrowErrorIfNot(Error::AppxMissingContentTypesXML, (m_contentType->HasStream()), "[Content_Types].xml not in archive!");

        // 3. Get blockmap object using signature object for validation
        temp = m_appxSignature->GetValidationStream(APPXBLOCKMAP_XML, m_container->GetFile(APPXBLOCKMAP_XML));
        m_appxBlockMap = ComPtr<IVerifierObject>::Make<AppxBlockMapObject>(factory, temp);
        ThrowErrorIfNot(Error::AppxMissingBlockMapXML, (m_appxBlockMap->HasStream()), "AppxBlockMap.xml not in archive!");

        // 4. Get manifest object using blockmap object for validation
        // TODO: pass validation flags and other necessary goodness through.
        temp = m_appxBlockMap->GetValidationStream(APPXMANIFEST_XML, m_container->GetFile(APPXMANIFEST_XML));
        m_appxManifest = ComPtr<IVerifierObject>::Make<AppxManifestObject>(temp);
        ThrowErrorIfNot(Error::AppxMissingAppxManifestXML, (m_appxBlockMap->HasStream()), "AppxManifest.xml not in archive!");

        struct Config
        {
            using lambda = std::function<xPlat::ComPtr<IStream>()>;
            Config(lambda f) : GetValidationStream(f) {}
            lambda GetValidationStream;
        };

        std::map<std::string, Config> footPrintFileNames = {
            { APPXBLOCKMAP_XML,  Config([&](){ m_footprintFiles.push_back(APPXBLOCKMAP_XML);  return m_appxBlockMap->GetStream();})  },
            { APPXMANIFEST_XML,  Config([&](){ m_footprintFiles.push_back(APPXMANIFEST_XML);  return m_appxManifest->GetStream();})  },
            { APPXSIGNATURE_P7X, Config([&](){ if (m_appxSignature->GetStream().Get()){m_footprintFiles.push_back(APPXSIGNATURE_P7X);} return m_appxSignature->GetStream();}) },
            { CODEINTEGRITY_CAT, Config([&](){ m_footprintFiles.push_back(CODEINTEGRITY_CAT); return m_appxSignature->GetValidationStream(CODEINTEGRITY_CAT, std::move(m_container->GetFile(CODEINTEGRITY_CAT)));}) },
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
        
        auto blockMapStorage = m_appxBlockMap.As<IStorageObject>();
        for (const auto& fileName : blockMapStorage->GetFileNames(FileNameOptions::PayloadOnly))
        {   auto footPrintFile = footPrintFileNames.find(fileName);
            if (footPrintFile == footPrintFileNames.end())
            {   std::string containerFileName = EncodeFileName(fileName);
                m_payloadFiles.push_back(containerFileName);
                m_streams[containerFileName] = m_appxBlockMap->GetValidationStream(fileName, m_container->GetFile(containerFileName));
                filesToProcess.erase(std::remove(filesToProcess.begin(), filesToProcess.end(), containerFileName), filesToProcess.end());
            }
        }
        // If the map is not empty, there's a file in the container that didn't go to the footprint or payload
        // files. (eg. payload file missing in the AppxBlockMap.xml)
        ThrowErrorIfNot(Error::BlockMapSemanticError, (filesToProcess.empty()), "Package not valid!");
    }

    void AppxPackageObject::Pack(APPX_PACKUNPACK_OPTION options, const std::string& certFile, IStorageObject* from)
    {
        // TODO: Implement
        throw Exception(Error::NotImplemented);
    }

    void AppxPackageObject::Unpack(APPX_PACKUNPACK_OPTION options, IStorageObject* to)
    {
        auto fileNames = GetFileNames(FileNameOptions::All);
        for (const auto& fileName : fileNames)
        {
            std::string targetName;
            if (options & APPX_PACKUNPACK_OPTION_CREATEPACKAGESUBFOLDER)
            {   throw Exception(Error::NotImplemented);
                //targetName = GetAppxManifest()->GetPackageFullName() + to->GetPathSeparator() + fileName;
            }
            else
            {   targetName = DecodeFileName(fileName);
            }

            auto targetFile = to->OpenFile(targetName, xPlat::FileStream::Mode::WRITE_UPDATE);
            auto sourceFile = GetFile(fileName);

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

    IStream* AppxPackageObject::GetFile(const std::string& fileName)
    {
        // TODO: add input validation.
        return m_streams[fileName].Get();
    }

    void AppxPackageObject::RemoveFile(const std::string& fileName)
    {
        // TODO: Implement
        throw Exception(Error::NotImplemented);
    }

    IStream* AppxPackageObject::OpenFile(const std::string& fileName, xPlat::FileStream::Mode mode)
    {
        // TODO: Implement
        throw Exception(Error::NotImplemented);
    }

    void AppxPackageObject::CommitChanges()
    {
        // TODO: Implement
        throw Exception(Error::NotImplemented);
    }

    // IAppxPackageReader
    HRESULT STDMETHODCALLTYPE AppxPackageObject::GetBlockMap(IAppxBlockMapReader** blockMapReader)
    {
        return xPlat::ResultOf([&]() {
            // TODO: Implement
            throw Exception(Error::NotImplemented);
        });
    }
   
    HRESULT STDMETHODCALLTYPE AppxPackageObject::GetFootprintFile(APPX_FOOTPRINT_FILE_TYPE type, IAppxFile** file)
    {
        return xPlat::ResultOf([&]() {
            ThrowErrorIf(Error::InvalidParameter, (file == nullptr || *file != nullptr), "bad pointer");
            auto footprint = footprintFiles.find(type);
            ThrowErrorIf(Error::FileNotFound, (footprint == footprintFiles.end()), "unknown footprint file type");
            ComPtr<IStream> stream = GetFile(footprint->second);
            ThrowErrorIf(Error::FileNotFound, (stream.Get() == nullptr), "requested footprint file not in package")
            // Clients expect the stream's pointer to be at the start of the file!
            ThrowHrIfFailed(stream->Seek({0}, StreamBase::Reference::START, nullptr)); 
            auto result = stream.As<IAppxFile>();
            *file = result.Detach();
        });
    }

    HRESULT STDMETHODCALLTYPE AppxPackageObject::GetPayloadFile(LPCWSTR fileName, IAppxFile** file)
    {
        return xPlat::ResultOf([&]() {
            ThrowErrorIf(Error::InvalidParameter, (fileName == nullptr || file == nullptr || *file != nullptr), "bad pointer");
            std::string name = utf16_to_utf8(fileName);
            ComPtr<IStream> stream = GetFile(name);
            ThrowErrorIf(Error::FileNotFound, (stream.Get() == nullptr), "requested file not in package")
            // Clients expect the stream's pointer to be at the start of the file!
            ThrowHrIfFailed(stream->Seek({0}, StreamBase::Reference::START, nullptr)); 
            auto result = stream.As<IAppxFile>();
            *file = result.Detach();
        });
    }

    HRESULT STDMETHODCALLTYPE AppxPackageObject::GetPayloadFiles(IAppxFilesEnumerator** filesEnumerator)
    {
        return xPlat::ResultOf([&]() {
            ThrowErrorIf(Error::InvalidParameter,(filesEnumerator == nullptr || *filesEnumerator != nullptr), "bad pointer");

            ComPtr<IStorageObject> storage;
            ThrowHrIfFailed(QueryInterface(UuidOfImpl<IStorageObject>::iid, reinterpret_cast<void**>(&storage)));
            auto result = ComPtr<IAppxFilesEnumerator>::Make<AppxFilesEnumerator>(storage.Get());
            *filesEnumerator = result.Detach();
        });
    }

    HRESULT STDMETHODCALLTYPE AppxPackageObject::GetManifest(IAppxManifestReader** manifestReader)
    {
        return xPlat::ResultOf([&]() {
            // TODO: Implement
            throw Exception(Error::NotImplemented);
        });
    }
}
#include "AppxBlockMapObject.hpp"
#include "AppxBlockMapSchemas.hpp"
#include "xercesc/framework/XMLGrammarPoolImpl.hpp"
#include "xercesc/parsers/XercesDOMParser.hpp"
#include "xercesc/framework/MemBufInputSource.hpp"
#include "xercesc/util/XMLString.hpp"
#include <algorithm>
#include <iterator>

/* Example XML:
<?xml version="1.0" encoding="UTF-8"?>
<BlockMap HashMethod="http://www.w3.org/2001/04/xmlenc#sha256" xmlns="http://schemas.microsoft.com/appx/2010/blockmap">
...
<File LfhSize="65" Size="187761" Name="Assets\video_offline_demo_page1.jpg">
	<Block Hash="NQL/PSheCSB3yZzKyZ6nHbsfzJt1EZJxOXLllMVvtEI="/>
	<Block Hash="2Udxo8Nwie7rvy4g0T5yfz9qccDNMVWh2mfMD1YCQao="/>
	<Block Hash="MmXnlptT/u+ilMKCIriWR49k99rBqwXKO3s60zGwZKg="/>
</File>
...
<File LfhSize="57" Size="47352" Name="Resources\Fonts\SegMVR2.ttf">
    <Block Size="27777" Hash="LGaGnk3EtFymriM9cRmeX7eZI+b2hpwOIlJIXdeE1ik="/>
</File>
...
</BlockMap>
*/

XERCES_CPP_NAMESPACE_USE


namespace xPlat {
    AppxBlockMapObject::AppxBlockMapObject(IxPlatFactory* factory, IStream* stream) :
        m_factory(factory)
    {
        // Create buffer from stream
        LARGE_INTEGER start = { 0 };
        ULARGE_INTEGER end = { 0 };
        ThrowHrIfFailed(stream->Seek(start, StreamBase::Reference::END, &end));
        ThrowHrIfFailed(stream->Seek(start, StreamBase::Reference::START, nullptr));

        std::uint32_t streamSize = end.u.LowPart;
        std::vector<std::uint8_t> buffer(streamSize);
        ULONG actualRead = 0;
        ThrowHrIfFailed(stream->Read(buffer.data(), streamSize, &actualRead));
        ThrowErrorIf(Error::FileRead, (actualRead != streamSize), "read error");

        std::unique_ptr<MemBufInputSource> appxBlockMapFile = std::make_unique<MemBufInputSource>(
            reinterpret_cast<const XMLByte*>(&buffer[0]), actualRead, "XML File");

        // Create parser and grammar pool
        std::unique_ptr<XMLGrammarPool> grammarPool =
            std::make_unique<XMLGrammarPoolImpl>(XMLPlatformUtils::fgMemoryManager);
        std::unique_ptr<XercesDOMParser> parser =
            std::make_unique<XercesDOMParser>(nullptr, XMLPlatformUtils::fgMemoryManager, grammarPool.get());
        parser->cacheGrammarFromParse(true);
        parser->setValidationScheme(XercesDOMParser::Val_Always);
        parser->setDoSchema(true);
        parser->setDoNamespaces(true);
        parser->setHandleMultipleImports(true);
        parser->setValidationSchemaFullChecking(true);

        // Add block map schemas
        std::unique_ptr<MemBufInputSource> blockMapSchema = std::make_unique<MemBufInputSource>(
            reinterpret_cast<const XMLByte*>(blockMapSchemaRaw.c_str()),
            blockMapSchemaRaw.length(),
            "Xerces BlockMap");
        std::unique_ptr<MemBufInputSource> blockMapSchema2015 = std::make_unique<MemBufInputSource>(
            reinterpret_cast<const XMLByte*>(blockMapSchema2015Raw.c_str()),
            blockMapSchema2015Raw.length(),
            "Xerces BlockMap 2015");
        std::unique_ptr<MemBufInputSource> blockMapSchema2017 = std::make_unique<MemBufInputSource>(
            reinterpret_cast<const XMLByte*>(blockMapSchema2017Raw.c_str()),
            blockMapSchema2017Raw.length(),
            "Xerces BlockMap 2017");
        parser->loadGrammar(*blockMapSchema, Grammar::GrammarType::SchemaGrammarType, true);
        parser->loadGrammar(*blockMapSchema2015, Grammar::GrammarType::SchemaGrammarType, true);
        parser->loadGrammar(*blockMapSchema2017, Grammar::GrammarType::SchemaGrammarType, true);

        // Set the error handler for the parser
        std::unique_ptr<ErrorHandler> errorHandler = std::make_unique<ParsingException>();
        parser->setErrorHandler(errorHandler.get());

        parser->parse(*appxBlockMapFile);
        m_document = ComPtr<IXmlObject>::Make<XmlObject>(stream, parser->adoptDocument());

        CreateBlockMapFiles(m_document->Document());

        return;
    }

    void AppxBlockMapObject::CreateBlockMapFiles(XERCES_CPP_NAMESPACE::DOMDocument* dom)
    {
        XercesPtr<XMLCh> fileXPath(XMLString::transcode("/BlockMap/File"));

        XercesPtr<DOMXPathNSResolver> resolver(dom->createNSResolver(dom->getDocumentElement()));
        XercesPtr<DOMXPathResult> fileResult = dom->evaluate(
            fileXPath.Get(),
            dom->getDocumentElement(),
            resolver.Get(),
            DOMXPathResult::ORDERED_NODE_SNAPSHOT_TYPE,
            nullptr);

        // Create IAppxBlockMapFile and IAppxBlockMapBlock vector per file
        for (XMLSize_t i = 0; i < fileResult->getSnapshotLength(); i++)
        {
            fileResult->snapshotItem(i);
            xPlat::ComPtr<IAppxBlockMapFile>::Make<AppxBlockMapFile>(
                static_cast<DOMElement*>(fileResult->getNodeValue()),
                dom);
        }
        return;
    }

    IStream* AppxBlockMapObject::GetValidationStream(const std::string& part, IStream* stream)
    {
        // TODO: Implement -- for now, just pass through.
        return stream;
    }

    HRESULT STDMETHODCALLTYPE AppxBlockMapObject::GetFile(LPCWSTR filename, IAppxBlockMapFile **file)
    {
        return ResultOf([&]{
            ThrowErrorIf(Error::InvalidParameter, (
                filename == nullptr || *filename == '\0' || file == nullptr || *file != nullptr
            ), "bad pointer");
            std::string name = utf16_to_utf8(filename);
            auto index = m_blockMapfiles.find(name);
            ThrowErrorIf(Error::FileNotFound, (index == m_blockMapfiles.end()), "named file not in blockmap");
            *file = index->second.Get();
            (*file)->AddRef();
        });
    }

    HRESULT STDMETHODCALLTYPE AppxBlockMapObject::GetFiles(IAppxBlockMapFilesEnumerator **enumerator)
    {
        return ResultOf([&]{
            ThrowErrorIf(Error::InvalidParameter, (enumerator == nullptr || *enumerator != nullptr), "bad pointer");

            std::vector<std::string> fileNames(m_blockMapfiles.size());
            std::transform(
                m_blockMapfiles.begin(),
                m_blockMapfiles.end(),
                std::back_inserter(fileNames),
                [](auto keyValuePair){ return keyValuePair.first; }
            );

            ComPtr<IAppxBlockMapReader> self;
            ThrowHrIfFailed(QueryInterface(UuidOfImpl<IAppxBlockMapReader>::iid, reinterpret_cast<void**>(&self)));

            *enumerator = ComPtr<IAppxBlockMapFilesEnumerator>::Make<AppxBlockMapFilesEnumerator>(self.Get(), std::move(fileNames)).Detach();
        });
    }

    HRESULT STDMETHODCALLTYPE AppxBlockMapObject::GetHashMethod(IUri **hashMethod)
    {   // Ultimately, this IUri object represents the HashMethod attribute in the blockmap:
        return ResultOf([&]{
            // TODO: Implement...
            throw Exception(Error::NotImplemented);
        });
    }

    HRESULT STDMETHODCALLTYPE AppxBlockMapObject::GetStream(IStream **blockMapStream)
    {
        return ResultOf([&]{
            ThrowErrorIf(Error::InvalidParameter, (blockMapStream == nullptr || *blockMapStream != nullptr), "bad pointer");
            auto stream = GetStream();
            LARGE_INTEGER li{0};
            ThrowHrIfFailed(stream->Seek(li, StreamBase::Reference::START, nullptr));
            stream->AddRef();
            *blockMapStream = stream;
        });
    }

    AppxBlockMapFile::AppxBlockMapFile(DOMElement* fileElement, XERCES_CPP_NAMESPACE::DOMDocument* dom)
    {
        SetName(fileElement);
        SetLocalFileHeaderSize(fileElement);
        SetUncompressedSize(fileElement);

        std::cout << "File: " << m_name << std::endl;
        std::cout << "\tLfhSize: " << m_localFileHeaderSize << "\tSize: " << m_uncompressedSize << std::endl;

        // Get blocks elements
        XercesPtr<XMLCh> blockXPath(XMLString::transcode("./Block"));
        XercesPtr<DOMXPathNSResolver> resolver(dom->createNSResolver(dom->getDocumentElement()));
        XercesPtr<DOMXPathResult> blockResult = dom->evaluate(
            blockXPath.Get(),
            fileElement,
            resolver.Get(),
            DOMXPathResult::ORDERED_NODE_SNAPSHOT_TYPE,
            nullptr);

        for (XMLSize_t j = 0; j < blockResult->getSnapshotLength(); j++)
        {
            blockResult->snapshotItem(j);
            // MOVE TO BLOCKMAPOBJECT
            DOMElement* blockElement = static_cast<DOMElement*>(blockResult->getNodeValue());
            XercesPtr<XMLCh> nameAttr(XMLString::transcode("Hash"));
            XercesPtr<char> name(XMLString::transcode(blockElement->getAttribute(nameAttr.Get())));
            std::string attributeValue(name.Get());
            std::cout << "\t\t" << attributeValue << std::endl;
        }
    }
}
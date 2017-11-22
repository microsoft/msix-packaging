#include "AppxBlockMapObject.hpp"
#include "AppxBlockMapSchemas.hpp"
#include "xercesc/framework/MemBufInputSource.hpp"
#include "xercesc/framework/XMLGrammarPoolImpl.hpp"
#include "xercesc/parsers/XercesDOMParser.hpp"
#include "xercesc/util/Base64.hpp"
#include "xercesc/util/XMLString.hpp"
#include <algorithm>
#include <iterator>
#include "BlockMapStream.hpp"

/* Example XML:
<?xml version="1.0" encoding="UTF-8"?>
<BlockMap HashMethod="http://www.w3.org/2001/04/xmlenc#sha256" xmlns="http://schemas.microsoft.com/appx/2010/blockmap">
...
<File Name="assets\icon150.png" Size="0" LfhSize="48"/>
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

    static std::uint32_t GetLocalFileHeaderSize(XERCES_CPP_NAMESPACE::DOMElement* element)
    {
        XercesPtr<XMLCh> nameAttr(XERCES_CPP_NAMESPACE::XMLString::transcode("LfhSize"));
        XercesPtr<char> name(XERCES_CPP_NAMESPACE::XMLString::transcode(element->getAttribute(nameAttr.Get())));
        std::string attributeValue(name.Get());
        bool hasValue = !attributeValue.empty();
        std::uint32_t value = 0;
        if (hasValue) { value = static_cast<std::uint32_t>(std::stoul(attributeValue)); }
        return value;        
    }

    static std::string GetName(XERCES_CPP_NAMESPACE::DOMElement* element)
    {
        XercesPtr<XMLCh> nameAttr(XERCES_CPP_NAMESPACE::XMLString::transcode("Name"));
        XercesPtr<char> name(XERCES_CPP_NAMESPACE::XMLString::transcode(element->getAttribute(nameAttr.Get())));
        return std::string (name.Get());
    }

    static std::uint64_t GetSize(XERCES_CPP_NAMESPACE::DOMElement* element)
    {
        XercesPtr<XMLCh> nameAttr(XERCES_CPP_NAMESPACE::XMLString::transcode("Size"));
        XercesPtr<char> name(XERCES_CPP_NAMESPACE::XMLString::transcode(element->getAttribute(nameAttr.Get())));
        std::string attributeValue(name.Get());
        std::uint64_t value = (64*1024); // size of block not always specified, in which case, it's 64k
        if (!attributeValue.empty())
        {   value = static_cast<std::uint64_t>(std::stoull(attributeValue));
        }
        return value;
    }

    static std::vector<std::uint8_t> GetDigestData(XERCES_CPP_NAMESPACE::DOMElement* element)
    {
        XercesPtr<XMLCh> nameAttr(XMLString::transcode("Hash"));
        XMLSize_t len = 0;
        XercesPtr<XMLByte> decodedData(XERCES_CPP_NAMESPACE::Base64::decodeToXMLByte(
            element->getAttribute(nameAttr.Get()), 
            &len));
        std::vector<std::uint8_t> result(len);
        for(XMLSize_t index=0; index < len; index++)
        {   result[index] = static_cast<std::uint8_t>(decodedData.Get()[index]);
        }
        return result;
    }

    static Block GetBlock(XERCES_CPP_NAMESPACE::DOMElement* element)
    {
        Block result {0};
        result.size = static_cast<std::uint32_t>(GetSize(element));
        result.hash = GetDigestData(element);
        return result;
    }
        
    AppxBlockMapObject::AppxBlockMapObject(IxPlatFactory* factory, IStream* stream) : m_factory(factory), m_stream(stream)
    {
        auto dom = ComPtr<IXmlObject>::Make<XmlObject>(stream, &blockMapSchema);
        // Create xPath query over blockmap file.
        XercesPtr<XMLCh> fileXPath(XMLString::transcode("/BlockMap/File"));
        XercesPtr<DOMXPathNSResolver> resolver(dom->Document()->createNSResolver(dom->Document()->getDocumentElement()));
        XercesPtr<DOMXPathResult> fileResult = dom->Document()->evaluate(
            fileXPath.Get(),
            dom->Document()->getDocumentElement(),
            resolver.Get(),
            DOMXPathResult::ORDERED_NODE_SNAPSHOT_TYPE,
            nullptr);

        // Create IAppxBlockMapFiles
        for (XMLSize_t i = 0; i < fileResult->getSnapshotLength(); i++)
        {
            fileResult->snapshotItem(i);
            auto fileNode = static_cast<DOMElement*>(fileResult->getNodeValue());

            // Get blocks elements
            XercesPtr<XMLCh> blockXPath(XMLString::transcode("./Block"));            
            XercesPtr<DOMXPathResult> blockResult = dom->Document()->evaluate(
                blockXPath.Get(),
                fileNode,
                resolver.Get(),
                DOMXPathResult::ORDERED_NODE_SNAPSHOT_TYPE,
                nullptr);

            // get all the blocks for the file.
            std::vector<Block> blocks(blockResult->getSnapshotLength());                
            for (XMLSize_t j = 0; j < blockResult->getSnapshotLength(); j++)
            {
                blockResult->snapshotItem(j);
                auto blockNode = static_cast<DOMElement*>(blockResult->getNodeValue());
                blocks[j] = GetBlock(blockNode);
            }    

            auto name = GetName(fileNode);
            auto existing = m_blockMap.find(name);
            ThrowErrorIf(Error::BlockMapSemanticError, (existing != m_blockMap.end()), "duplicate file name specified.");
            m_blockMap.insert(std::make_pair(name, std::move(blocks)));

            m_blockMapfiles.insert(std::make_pair(name,
                ComPtr<IAppxBlockMapFile>::Make<AppxBlockMapFile>(
                    factory,
                    &(m_blockMap[name]),
                    GetLocalFileHeaderSize(fileNode),
                    name,
                    GetSize(fileNode))));
        }        
    }

    IStream* AppxBlockMapObject::GetValidationStream(const std::string& part, IStream* stream)
    {
        ThrowErrorIf(Error::InvalidParameter, (part.empty() || stream == nullptr), "bad input");
        auto item = m_blockMap.find(part);
        ThrowErrorIf(Error::BlockMapSemanticError, item == m_blockMap.end(), "file not tracked by blockmap");
        return ComPtr<IStream>::Make<BlockMapStream>(stream, item->second).Detach();
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
}
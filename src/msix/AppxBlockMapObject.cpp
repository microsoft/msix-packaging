#include "AppxBlockMapObject.hpp"
#include <algorithm>
#include <iterator>
#include "IXml.hpp"
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

namespace MSIX {

    template <class T>
    static T GetNumber(IXmlElement* element, XmlAttributeName attribute, T defaultValue)
    {
        std::string attributeValue = element->GetAttributeValue(attribute);
        bool hasValue = !attributeValue.empty();
        T value = defaultValue;
        if (hasValue) { value = static_cast<T>(std::stoul(attributeValue)); }
        return value;        
    }

    static Block GetBlock(IXmlElement* element)
    {
        Block result {0};
        result.compressedSize = GetNumber<std::uint64_t>(element, XmlAttributeName::BlockMap_File_Block_Size, BLOCKMAP_BLOCK_SIZE);
        result.hash = element->GetBase64DecodedAttributeValue(XmlAttributeName::BlockMap_File_Block_Hash);
        return result;
    }

    AppxBlockMapObject::AppxBlockMapObject(IMSIXFactory* factory, ComPtr<IStream>& stream) : m_factory(factory), m_stream(stream)
    {
        ComPtr<IXmlFactory> xmlFactory;
        ThrowHrIfFailed(factory->QueryInterface(UuidOfImpl<IXmlFactory>::iid, reinterpret_cast<void**>(&xmlFactory)));        
        auto dom = xmlFactory->CreateDomFromStream(XmlContentType::AppxBlockMapXml, stream);
        size_t countFilesFound = 0;
        dom->ForEachElementIn(dom->GetDocument().Get(), XmlQueryName::BlockMap_File, [&](IXmlElement* fileNode)
        {
            auto name = fileNode->GetAttributeValue(XmlAttributeName::BlockMap_File_Name);
            ThrowErrorIf(Error::BlockMapSemanticError, (name == "[Content_Types].xml"), "[Content_Types].xml cannot be in the AppxBlockMap.xml file");
            ThrowErrorIf(Error::BlockMapSemanticError, (m_blockMap.find(name) != m_blockMap.end()), "duplicate file name specified.");

            std::vector<Block> blocks;
            size_t countBlocks = 0;
            dom->ForEachElementIn(fileNode, XmlQueryName::BlockMap_File_Block, [&](IXmlElement* blockNode)
            {
                blocks.push_back(GetBlock(blockNode));
                countBlocks++;    
                return true;
            });

            std::uint64_t sizeAttribute = GetNumber<std::uint64_t>(fileNode, XmlAttributeName::BlockMap_File_Block_Size, BLOCKMAP_BLOCK_SIZE);
            ThrowErrorIf(Error::BlockMapSemanticError, (0 == countBlocks && 0 != sizeAttribute), "If size is non-zero, then there must be 1+ blocks.");
            
            m_blockMap.insert(std::make_pair(name, std::move(blocks)));
            m_blockMapfiles.insert(std::make_pair(name,
                ComPtr<IAppxBlockMapFile>::Make<AppxBlockMapFile>(
                    factory,
                    &(m_blockMap[name]),
                    GetNumber<std::uint32_t>(fileNode, XmlAttributeName::BlockMap_File_LocalFileHeaderSize, 0),
                    name,
                    sizeAttribute
                )));
            countFilesFound++;    
            return true;            
        });
        ThrowErrorIf(Error::BlockMapSemanticError, (0 == countFilesFound), "Empty AppxBlockMap.xml");
    }

    MSIX::ComPtr<IStream> AppxBlockMapObject::GetValidationStream(const std::string& part, IStream* stream)
    {
        ThrowErrorIf(Error::InvalidParameter, (part.empty() || stream == nullptr), "bad input");
        auto item = m_blockMap.find(part);
        ThrowErrorIf(Error::BlockMapSemanticError, item == m_blockMap.end(), "file not tracked by blockmap");
        return ComPtr<IStream>::Make<BlockMapStream>(m_factory, part, stream, item->second);
    }

    HRESULT STDMETHODCALLTYPE AppxBlockMapObject::GetFile(LPCWSTR filename, IAppxBlockMapFile **file)
    {
        return ResultOf([&]{
            ThrowErrorIf(Error::InvalidParameter, (
                filename == nullptr || *filename == '\0' || file == nullptr || *file != nullptr
            ), "bad pointer");
            auto fileStream = GetFile(utf16_to_utf8(filename));
            ThrowErrorIfNot(Error::InvalidParameter, (fileStream.first), "file not found!");
            MSIX::ComPtr<IStream> stream = fileStream.second;
            *file = stream.As<IAppxBlockMapFile>().Detach();
            return static_cast<HRESULT>(Error::OK);
        });
    }

    HRESULT STDMETHODCALLTYPE AppxBlockMapObject::GetFiles(IAppxBlockMapFilesEnumerator **enumerator)
    {
        return ResultOf([&]{
            ThrowErrorIf(Error::InvalidParameter, (enumerator == nullptr || *enumerator != nullptr), "bad pointer");
            ComPtr<IAppxBlockMapReader> self;
            ThrowHrIfFailed(QueryInterface(UuidOfImpl<IAppxBlockMapReader>::iid, reinterpret_cast<void**>(&self)));
            *enumerator = ComPtr<IAppxBlockMapFilesEnumerator>::Make<AppxBlockMapFilesEnumerator>(
                self.Get(),
                std::move(GetFileNames(FileNameOptions::All))).Detach();
            return static_cast<HRESULT>(Error::OK);
        });
    }

    HRESULT STDMETHODCALLTYPE AppxBlockMapObject::GetHashMethod(IUri **hashMethod)
    {   // Ultimately, this IUri object represents the HashMethod attribute in the blockmap:
        return static_cast<HRESULT>(Error::NotImplemented);
    }

    HRESULT STDMETHODCALLTYPE AppxBlockMapObject::GetStream(IStream **blockMapStream)
    {
        return ResultOf([&]{
            ThrowErrorIf(Error::InvalidParameter, (blockMapStream == nullptr || *blockMapStream != nullptr), "bad pointer");
            auto stream = GetStream();
            LARGE_INTEGER li{0};
            ThrowHrIfFailed(stream->Seek(li, StreamBase::Reference::START, nullptr));
            *blockMapStream = stream.Detach();
            return static_cast<HRESULT>(Error::OK);
        });
    }

    // IStorageObject methods
    std::string AppxBlockMapObject::GetPathSeparator() { return "\\"; }
    std::vector<std::string> AppxBlockMapObject::GetFileNames(FileNameOptions)
    {
        std::vector<std::string> fileNames;
        std::transform(
            m_blockMapfiles.begin(),
            m_blockMapfiles.end(),
            std::back_inserter(fileNames),
            [](auto keyValuePair){ return keyValuePair.first; }
        );
        return fileNames;
    }

    std::pair<bool,IStream*> AppxBlockMapObject::GetFile(const std::string& fileName)
    {
        auto index = m_blockMapfiles.find(fileName);
        ThrowErrorIf(Error::FileNotFound, (index == m_blockMapfiles.end()), "named file not in blockmap");
        return std::make_pair(true, index->second.As<IStream>().Detach());
    }

    void AppxBlockMapObject::RemoveFile(const std::string& )                           { throw Exception(Error::NotImplemented); }
    IStream* AppxBlockMapObject::OpenFile(const std::string& ,MSIX::FileStream::Mode)  { throw Exception(Error::NotImplemented); }
    void AppxBlockMapObject::CommitChanges()                                           { throw Exception(Error::NotImplemented); }
}
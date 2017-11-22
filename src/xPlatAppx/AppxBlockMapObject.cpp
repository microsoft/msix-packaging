#include "AppxBlockMapObject.hpp"
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

namespace xPlat {
    AppxBlockMapObject::AppxBlockMapObject(IxPlatFactory* factory, IStream* stream) : 
        m_factory(factory)
    {
        m_document = ComPtr<IXmlObject>::Make<XmlObject>(stream);
        // TODO: Implement higher-level validation
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
}
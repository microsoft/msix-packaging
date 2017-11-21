#include "AppxBlockMapObject.hpp"
#include <algorithm>
#include <iterator>

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
    {
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
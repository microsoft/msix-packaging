#include "AppxBlockMapObject.hpp"

namespace xPlat {

    // IAppxBlockMapBlock 
    HRESULT STDMETHODCALLTYPE AppxBlockMapBlock::GetHash(UINT32* bufferSize, BYTE** buffer)
    {
        return xPlat::ResultOf([&]() {
            // TODO: Implement
            throw Exception(Error::NotImplemented);
        });
    }

    HRESULT STDMETHODCALLTYPE AppxBlockMapBlock::GetCompressedSize(UINT32* size)
    {
        return xPlat::ResultOf([&]() {
            // TODO: Implement
            throw Exception(Error::NotImplemented);
        });
    }

    // IAppxBlockMapFile
    HRESULT STDMETHODCALLTYPE AppxBlockMapFile::GetBlocks(IAppxBlockMapBlocksEnumerator** blocks)
    {
        return xPlat::ResultOf([&]() {
            // TODO: Implement
            throw Exception(Error::NotImplemented);
        });
    }

    HRESULT STDMETHODCALLTYPE AppxBlockMapFile::GetLocalFileHeaderSize(UINT32* lfhSize)
    {
        return xPlat::ResultOf([&]() {
            // TODO: Implement
            throw Exception(Error::NotImplemented);
        });
    }

    HRESULT STDMETHODCALLTYPE AppxBlockMapFile::GetName(LPWSTR* name)
    {
        return xPlat::ResultOf([&]() {
            // TODO: Implement
            throw Exception(Error::NotImplemented);
        });
    }

    HRESULT STDMETHODCALLTYPE AppxBlockMapFile::GetUncompressedSize(UINT64* size)
    {
        return xPlat::ResultOf([&]() {
            // TODO: Implement
            throw Exception(Error::NotImplemented);
        });
    }

    HRESULT STDMETHODCALLTYPE AppxBlockMapFile::ValidateFileHash(IStream* fileStream, BOOL* isValid)
    {
        return xPlat::ResultOf([&]() {
            // TODO: Implement
            throw Exception(Error::NotImplemented);
        });
    }

    // IAppxBlockMapFilesEnumerator
    HRESULT STDMETHODCALLTYPE AppxBlockMapFile::GetCurrent(IAppxBlockMapFile** block)
    {
        return xPlat::ResultOf([&]() {
            // TODO: Implement
            throw Exception(Error::NotImplemented);
        });
    }
    
    HRESULT STDMETHODCALLTYPE AppxBlockMapFile::GetHasCurrent(BOOL* hasCurrent)
    {
        return xPlat::ResultOf([&]() {
            // TODO: Implement
            throw Exception(Error::NotImplemented);
        });
    }

    HRESULT STDMETHODCALLTYPE AppxBlockMapFile::MoveNext(BOOL* hasNext)
    {
        return xPlat::ResultOf([&]() {
            // TODO: Implement
            throw Exception(Error::NotImplemented);
        });
    }

    AppxBlockMapObject::AppxBlockMapObject(std::shared_ptr<StreamBase>&& stream)
    {
        // TODO: Implement
    }

    std::shared_ptr<StreamBase> AppxBlockMapObject::GetBlockMapValidationStream(const std::string& file)
    {
        // TODO: Implement
        throw Exception(Error::NotImplemented);
    }

    // IAppxBlockMapBlocksEnumerator
    HRESULT STDMETHODCALLTYPE AppxBlockMapObject::GetCurrent(IAppxBlockMapBlock** block)
    {
        return xPlat::ResultOf([&]() {
            // TODO: Implement
            throw Exception(Error::NotImplemented);
        });
    }

    HRESULT STDMETHODCALLTYPE AppxBlockMapObject::GetHasCurrent(BOOL* hasCurrent)
    {
        return xPlat::ResultOf([&]() {
            // TODO: Implement
            throw Exception(Error::NotImplemented);
        });
    }

    HRESULT STDMETHODCALLTYPE AppxBlockMapObject::MoveNext(BOOL* hasNext)
    {
        return xPlat::ResultOf([&]() {
            // TODO: Implement
            throw Exception(Error::NotImplemented);
        });
    }
}
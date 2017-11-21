#pragma once
#include <string>
#include <map>
#include <vector>

#include "StreamBase.hpp"
#include "VerifierObject.hpp"
#include "AppxPackaging.hpp"
#include "ComHelper.hpp"
#include "UnicodeConversion.hpp"
#include "AppxFactory.hpp"

namespace xPlat {

    class AppxBlockMapBlock : public xPlat::ComClass<AppxBlockMapBlock, IAppxBlockMapBlock>
    {
    public:
        AppxBlockMapBlock(IxPlatFactory* factory, std::uint64_t offset, std::uint32_t size, std::vector<std::uint8_t>* digest) :
            m_factory(factory),
            m_offset(offset),
            m_size(size),
            m_digest(digest)
        {}

        // IAppxBlockMapBlock
        HRESULT STDMETHODCALLTYPE GetHash(UINT32* bufferSize, BYTE** buffer) override
        {
            return ResultOf([&]{
                ThrowHrIfFailed(m_factory->MarshalOutBytes(m_digest, bufferSize, buffer));
            });
        }
        
        HRESULT STDMETHODCALLTYPE GetCompressedSize(UINT32* size) override
        {
            return ResultOf([&]{
                ThrowErrorIf(Error::InvalidParameter, (size == nullptr), "bad pointer");
                *size = static_cast<UINT32>(m_size);                
            });
        }

    private:
        ComPtr<IxPlatFactory> m_factory;
        std::uint64_t m_offset;
        std::uint32_t m_size; // If the block is compressed, means the compressed size of the block. Otherwise, it is the uncompressed size.
        std::vector<std::uint8_t>* m_digest;
    };

    class AppxBlockMapBlocksEnumerator : public xPlat::ComClass<AppxBlockMapBlocksEnumerator, IAppxBlockMapBlocksEnumerator>
    {
    protected:
        std::vector<ComPtr<IAppxBlockMapBlock>>* m_blocks;
        std::size_t                              m_cursor = 0;

    public:
        AppxBlockMapBlocksEnumerator(std::vector<ComPtr<IAppxBlockMapBlock>>* blocks) : 
            m_blocks(blocks)
        {}

        // IAppxBlockMapBlocksEnumerator
        HRESULT STDMETHODCALLTYPE GetCurrent(IAppxBlockMapBlock** block) override
        {
            return ResultOf([&]{
                ThrowErrorIf(Error::InvalidParameter, (block == nullptr || *block != nullptr), "bad pointer");
                *block = m_blocks->at(m_cursor).Get();
                (*block)->AddRef();
            });
        }

        HRESULT STDMETHODCALLTYPE GetHasCurrent(BOOL* hasCurrent) override
        {   return ResultOf([&]{
                ThrowErrorIfNot(Error::InvalidParameter, (hasCurrent), "bad pointer");
                *hasCurrent = (m_cursor != m_files.size()) ? TRUE : FALSE;
            });
        }

        HRESULT STDMETHODCALLTYPE MoveNext(BOOL* hasNext) override      
        {   return ResultOf([&]{
                ThrowErrorIfNot(Error::InvalidParameter, (hasNext), "bad pointer");
                *hasNext = (++m_cursor != m_files.size()) ? TRUE : FALSE;
            });
        }      
    };

    class AppxBlockMapFile : public xPlat::ComClass<AppxBlockMapFile, IAppxBlockMapFile>
    {
    public:
        AppxBlockMapFile(
            IxPlatFactory* factory, 
            std::vector<std::shared_ptr<AppxBlockMapBlock>>* blocks,
            std::uint32_t localFileHeaderSize,
            const std::string& name,
            std::uint64_t uncompressedSize,
        ) : 
            m_factory(factory),
            m_blocks(blocks),
            m_localFileHeaderSize(localFileHeaderSize),
            m_name(name),
            m_uncompressedSize(uncompressedSize)
        {}

        // IAppxBlockMapFile
        HRESULT STDMETHODCALLTYPE GetBlocks(IAppxBlockMapBlocksEnumerator **blocks) override
        {
            return ResultOf([&]{
                ThrowErrorIf(Error::InvalidParameter, (blocks == nullptr || *blocks != nullptr), "bad pointer.");
                *blocks = ComPtr<IAppxBlockMapBlocksEnumerator>::Make<AppxBlockMapBlocksEnumerator>(&m_blocks).Detach();
            });
        }

        HRESULT STDMETHODCALLTYPE GetLocalFileHeaderSize(UINT32* lfhSize) override
        {   // Retrieves the size of the zip local file header of the associated zip file item
            return ResultOf([&]{
                ThrowErrorIf(Error::InvalidParameter, (lfhSize == nullptr), "bad pointer");
                *lfhSize = static_cast<UINT32>(m_localFileHeaderSize);
            });
        }

        HRESULT STDMETHODCALLTYPE GetName(LPWSTR *name) override;
        {
            return ResultOf([&]{
                ThrowHrIfFailed(m_factory->MarshalOutString(m_name, name));
            });
        }
            
        HRESULT STDMETHODCALLTYPE GetUncompressedSize(UINT64 *size) override
        {
            return ResultOf([&]{
                ThrowErrorIf(Error::InvalidParameter, (size == nullptr), "bad pointer");
                *size = static_cast<UINT64>(m_uncompressedSize);
            });
        }
            
        HRESULT STDMETHODCALLTYPE ValidateFileHash(IStream *fileStream, BOOL *isValid) override
        {
            return ResultOf([&]{
                // TODO: Implement...
                throw Exception(Error::NotImplemented);
            });
        }            

    private:        
        std::vector<std::shared_ptr<AppxBlockMapBlock>>* m_blocks;
        ComPtr<IxPlatFactory>   m_factory;
        std::uint32_t           m_localFileHeaderSize;
        std::string             m_name;
        std::uint64_t           m_uncompressedSize;
    };

    class AppxBlockMapFileEnumerator : public xPlat::ComClass<AppxBlockMapFileEnumerator, IAppxBlockMapFilesEnumerator>
    {
    protected:
        ComPtr<IAppxBlockMapReader> m_reader;
        std::vector<std::string>    m_files;
        std::size_t                 m_cursor = 0;

    public:
        AppxBlockMapFileEnumerator(IAppxBlockMapReader* reader, std::vector<std::string>& files) :
            m_reader(reader),
            m_files(files)
        {}

        // IAppxBlockMapFilesEnumerator
        HRESULT STDMETHODCALLTYPE GetCurrent(IAppxBlockMapFile** block) override
        {
            return ResultOf([&]{
                ThrowErrorIf(Error::Unexpected, (m_cursor >= m_files.size()), "index out of range");
                ThrowHrIfFailed(m_reader->GetFile(utf8_to_utf16(m_files.at(m_cursor)).c_str(), block));
            });
        }

        HRESULT STDMETHODCALLTYPE GetHasCurrent(BOOL* hasCurrent) override
        {   return ResultOf([&]{
                ThrowErrorIfNot(Error::InvalidParameter, (hasCurrent), "bad pointer");
                *hasCurrent = (m_cursor != m_files.size()) ? TRUE : FALSE;
            });
        }

        HRESULT STDMETHODCALLTYPE MoveNext(BOOL* hasNext) override      
        {   return ResultOf([&]{
                ThrowErrorIfNot(Error::InvalidParameter, (hasNext), "bad pointer");
                *hasNext = (++m_cursor != m_files.size()) ? TRUE : FALSE;
            });
        }
    };    

    // Object backed by AppxBlockMap.xml
    class AppxBlockMapObject : public VerifierObject, public xPlat::ComClass<AppxBlockMapObject, IAppxBlockMapReader>
    {
    public:
        AppxBlockMapObject(IStream* stream);

        IStream* GetValidationStream(const std::string& part, IStream* stream) override;

        // IAppxBlockMapReader
        HRESULT STDMETHODCALLTYPE GetFile(LPCWSTR filename, IAppxBlockMapFile **file) override;
        HRESULT STDMETHODCALLTYPE GetFiles(IAppxBlockMapFilesEnumerator **enumerator) override;
        HRESULT STDMETHODCALLTYPE GetHashMethod(IUri **hashMethod) override;
        HRESULT STDMETHODCALLTYPE GetStream(IStream **blockMapStream) override;

    protected:
        std::map<std::string, ComPtr<IAppxBlockMapFile>> m_blockMapfiles;
    };
}
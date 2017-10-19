#pragma once
#include <string>
#include <map>
#include <vector>

#include "StreamBase.hpp"
#include "AppxPackaging.hpp"
#include "ComHelper.hpp"

namespace xPlat {

    class AppxBlockMapBlock : public xPlat::ComClass<AppxBlockMapBlock, IAppxBlockMapBlock>
    {
    public:
        AppxBlockMapBlock() :
            m_offset(0),
            m_size(0),
            m_encryptedSize(0),
            m_hashLength(0),
            m_packageOffset(0)
        {}

        // IAppxBlockMapBlock
        HRESULT STDMETHODCALLTYPE GetHash(UINT32* bufferSize, BYTE** buffer);
        HRESULT STDMETHODCALLTYPE GetCompressedSize(UINT32* size);

    private:
        std::uint64_t m_offset;
        std::uint32_t m_size;    // If the block is compressed, means the compressed size of the block. Otherwise, it is the uncompressed size.
        std::uint32_t m_encryptedSize;   // Size of the encrypted block. It is the raw data size of the block if it is encrypted.
        std::uint32_t m_hashLength;
        std::uint32_t m_packageOffset;
        std::vector<unsigned char> hash;
    };

    class AppxBlockMapFile : public xPlat::ComClass<AppxBlockMapFile, IAppxBlockMapFile, IAppxBlockMapFilesEnumerator>
    {
    public:
        // IAppxBlockMapFile
        HRESULT STDMETHODCALLTYPE GetBlocks(IAppxBlockMapBlocksEnumerator **blocks) override;
        HRESULT STDMETHODCALLTYPE GetLocalFileHeaderSize(UINT32* lfhSize) override;
        HRESULT STDMETHODCALLTYPE GetName(LPWSTR *name) override;
        HRESULT STDMETHODCALLTYPE GetUncompressedSize(UINT64 *size) override;
        HRESULT STDMETHODCALLTYPE ValidateFileHash(IStream *fileStream, BOOL *isValid) override;

        // IAppxBlockMapFilesEnumerator
        HRESULT STDMETHODCALLTYPE GetCurrent(IAppxBlockMapFile** block) override;
        HRESULT STDMETHODCALLTYPE GetHasCurrent(BOOL* hasCurrent) override;
        HRESULT STDMETHODCALLTYPE MoveNext(BOOL* hasNext) override;

    private:
        std::vector<std::shared_ptr<AppxBlockMapBlock>> m_blocks;
    };

    // Object backed by AppxBlockMap.xml
    class AppxBlockMapObject : public xPlat::ComClass<AppxBlockMapObject, IAppxBlockMapBlocksEnumerator>
    {
    public:
        AppxBlockMapObject(std::shared_ptr<StreamBase>&& stream);

        std::shared_ptr<StreamBase> GetBlockMapValidationStream(const std::string& file);

        // IAppxBlockMapBlocksEnumerator
        HRESULT STDMETHODCALLTYPE GetCurrent(IAppxBlockMapBlock** block);
        HRESULT STDMETHODCALLTYPE GetHasCurrent(BOOL* hasCurrent);
        HRESULT STDMETHODCALLTYPE MoveNext(BOOL* hasNext);

    protected:
        std::shared_ptr<StreamBase> m_stream;
        std::map<std::string, std::shared_ptr<AppxBlockMapFile>> m_blockMapfiles;
    };
}

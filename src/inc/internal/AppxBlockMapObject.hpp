//
//  Copyright (C) 2017 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
// 
#pragma once
#include <algorithm>
#include <string>
#include <map>
#include <vector>
#include <iterator>

#include "StreamBase.hpp"
#include "VerifierObject.hpp"
#include "AppxPackaging.hpp"
#include "ComHelper.hpp"
#include "UnicodeConversion.hpp"
#include "AppxFactory.hpp"
#include "IXml.hpp"
#include "BlockMapStream.hpp"
#include "Enumerators.hpp"

// internal interface
// {67fed21a-70ef-4175-8f12-415b213ab6d2}
#ifndef WIN32
interface IAppxBlockMapInternal : public IUnknown
#else
#include "unknwn.h"
#include "objidl.h"
class IAppxBlockMapInternal : public IUnknown
#endif
{
public:
    virtual std::vector<std::string> GetFileNames() = 0;
    virtual std::vector<MSIX::Block> GetBlocks(const std::string& fileName) = 0;
    virtual MSIX::ComPtr<IAppxBlockMapFile> GetFile(const std::string& fileName) = 0;
};
MSIX_INTERFACE(IAppxBlockMapInternal, 0x67fed21a,0x70ef,0x4175,0x8f,0x12,0x41,0x5b,0x21,0x3a,0xb6,0xd2);

namespace MSIX {

    class AppxBlockMapBlock final : public MSIX::ComClass<AppxBlockMapBlock, IAppxBlockMapBlock>
    {
    public:
        AppxBlockMapBlock(IMsixFactory* factory, Block* block) :
            m_factory(factory),
            m_block(block)
        {}

        // IAppxBlockMapBlock
        HRESULT STDMETHODCALLTYPE GetHash(UINT32* bufferSize, BYTE** buffer) noexcept override try
        {
            ThrowHrIfFailed(m_factory->MarshalOutBytes(m_block->hash, bufferSize, buffer));
            return static_cast<HRESULT>(Error::OK);
        } CATCH_RETURN();

        HRESULT STDMETHODCALLTYPE GetCompressedSize(UINT32* size) noexcept override try
        {
            ThrowErrorIf(Error::InvalidParameter, (size == nullptr), "bad pointer");
            *size = static_cast<UINT32>(m_block->compressedSize);
            return static_cast<HRESULT>(Error::OK);
        } CATCH_RETURN();

    private:
        IMsixFactory* m_factory;
        Block*        m_block;
    };

    class AppxBlockMapFile final : public MSIX::ComClass<AppxBlockMapFile, IAppxBlockMapFile, IAppxBlockMapFileUtf8 >
    {
    public:
        AppxBlockMapFile(
            IMsixFactory* factory,
            std::vector<Block>* blocks,
            std::uint32_t localFileHeaderSize,
            const std::string& name,
            std::uint64_t uncompressedSize
        ) :
            m_factory(factory),
            m_blocks(blocks),
            m_localFileHeaderSize(localFileHeaderSize),
            m_name(name),
            m_uncompressedSize(uncompressedSize)
        {
        }

        // IAppxBlockMapFile
        HRESULT STDMETHODCALLTYPE GetBlocks(IAppxBlockMapBlocksEnumerator **blocks) noexcept override try
        {
            ThrowErrorIf(Error::InvalidParameter, (blocks == nullptr || *blocks != nullptr), "bad pointer.");
            if (m_blockMapBlocks.empty())
            {   m_blockMapBlocks.reserve(m_blocks->size());
                std::transform(
                    m_blocks->begin(),
                    m_blocks->end(),
                    std::back_inserter(m_blockMapBlocks),
                    [&](auto& item){
                        return ComPtr<IAppxBlockMapBlock>::Make<AppxBlockMapBlock>(m_factory, &item);
                    }
                );
            }
            *blocks = ComPtr<IAppxBlockMapBlocksEnumerator>::
                Make<EnumeratorCom<IAppxBlockMapBlocksEnumerator, IAppxBlockMapBlock>>(m_blockMapBlocks).Detach();
            return static_cast<HRESULT>(Error::OK);
        } CATCH_RETURN();

        HRESULT STDMETHODCALLTYPE GetLocalFileHeaderSize(UINT32* lfhSize) noexcept override try
        {   // Retrieves the size of the zip local file header of the associated zip file item
            ThrowErrorIf(Error::InvalidParameter, (lfhSize == nullptr), "bad pointer");
            *lfhSize = static_cast<UINT32>(m_localFileHeaderSize);
            return static_cast<HRESULT>(Error::OK);
        } CATCH_RETURN();

        HRESULT STDMETHODCALLTYPE GetName(LPWSTR *name) noexcept override try
        {
            ThrowHrIfFailed(m_factory->MarshalOutString(m_name, name));
            return static_cast<HRESULT>(Error::OK);
        } CATCH_RETURN();

        HRESULT STDMETHODCALLTYPE GetUncompressedSize(UINT64 *size) noexcept override try
        {
            ThrowErrorIf(Error::InvalidParameter, (size == nullptr), "bad pointer");
            *size = static_cast<UINT64>(m_uncompressedSize);
            return static_cast<HRESULT>(Error::OK);
        } CATCH_RETURN();

        HRESULT STDMETHODCALLTYPE ValidateFileHash(IStream *fileStream, BOOL *isValid) noexcept override
        {
            return static_cast<HRESULT>(Error::NotImplemented);
        }

        // IAppxBlockMapFileUtf8
        HRESULT STDMETHODCALLTYPE GetName(LPSTR *name) noexcept override try
        {
            ThrowHrIfFailed(m_factory->MarshalOutStringUtf8(m_name, name));
            return static_cast<HRESULT>(Error::OK);
        } CATCH_RETURN();

    private:
        std::vector<ComPtr<IAppxBlockMapBlock>> m_blockMapBlocks;
        std::vector<Block>* m_blocks;
        IMsixFactory*       m_factory;
        std::uint32_t       m_localFileHeaderSize;
        std::string         m_name;
        std::uint64_t       m_uncompressedSize;
    };

    // Object backed by AppxBlockMap.xml
    class AppxBlockMapObject final : public MSIX::ComClass<AppxBlockMapObject, IAppxBlockMapReader, IVerifierObject, IAppxBlockMapInternal, IAppxBlockMapReaderUtf8 >
    {
    public:
        AppxBlockMapObject(IMsixFactory* factory, const ComPtr<IStream>& stream);

        // IVerifierObject
        const std::string& GetPublisher() override { NOTSUPPORTED; }
        bool HasStream() override { return !!m_stream; }
        ComPtr<IStream> GetStream() override { return m_stream; }
        ComPtr<IStream> GetValidationStream(const std::string& part, const ComPtr<IStream>& stream) override;

        // IAppxBlockMapReader
        HRESULT STDMETHODCALLTYPE GetFile(LPCWSTR filename, IAppxBlockMapFile **file) noexcept override;
        HRESULT STDMETHODCALLTYPE GetFiles(IAppxBlockMapFilesEnumerator **enumerator) noexcept override;
        HRESULT STDMETHODCALLTYPE GetHashMethod(IUri **hashMethod) noexcept override;
        HRESULT STDMETHODCALLTYPE GetStream(IStream **blockMapStream) noexcept override;

        // IAppxBlockMapInternal methods
        std::vector<std::string>        GetFileNames() override;
        std::vector<Block>              GetBlocks(const std::string& fileName) override;
        MSIX::ComPtr<IAppxBlockMapFile> GetFile(const std::string& fileName) override;

        // IAppxBlockMapReaderUtf8
        HRESULT STDMETHODCALLTYPE GetFile(LPCSTR filename, IAppxBlockMapFile **file) noexcept override;

    protected:
        std::map<std::string, std::vector<Block>>        m_blockMap;
        std::map<std::string, ComPtr<IAppxBlockMapFile>> m_blockMapFiles;
        IMsixFactory*   m_factory;
        ComPtr<IStream> m_stream;
    };
}
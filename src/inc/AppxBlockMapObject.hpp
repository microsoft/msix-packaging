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

// internal interface
EXTERN_C const IID IID_IAppxBlockMapInternal;
#ifndef WIN32
// {67fed21a-70ef-4175-8f12-415b213ab6d2}
interface IAppxBlockMapInternal : public IUnknown
#else
#include "Unknwn.h"
#include "Objidl.h"
class IAppxBlockMapInternal : public IUnknown
#endif
{
public:
    virtual std::vector<std::string>  GetFileNames() = 0;
    virtual std::vector<MSIX::Block>  GetBlocks(const std::string& fileName) = 0;
    virtual MSIX::ComPtr<IAppxBlockMapFile> GetFile(const std::string& fileName) = 0;
};
SpecializeUuidOfImpl(IAppxBlockMapInternal);

namespace MSIX {

    class AppxBlockMapBlock final : public MSIX::ComClass<AppxBlockMapBlock, IAppxBlockMapBlock>
    {
    public:
        AppxBlockMapBlock(IMSIXFactory* factory, Block* block) :
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
        IMSIXFactory*   m_factory;
        Block*          m_block;
    };

    class AppxBlockMapBlocksEnumerator final : public MSIX::ComClass<AppxBlockMapBlocksEnumerator, IAppxBlockMapBlocksEnumerator>
    {
    protected:
        std::vector<ComPtr<IAppxBlockMapBlock>>* m_blocks;
        std::size_t                              m_cursor = 0;

    public:
        AppxBlockMapBlocksEnumerator(std::vector<ComPtr<IAppxBlockMapBlock>>* blocks) :
            m_blocks(blocks)
        {}

        // IAppxBlockMapBlocksEnumerator
        HRESULT STDMETHODCALLTYPE GetCurrent(IAppxBlockMapBlock** block) noexcept override try
        {
            ThrowErrorIf(Error::InvalidParameter, (block == nullptr || *block != nullptr), "bad pointer");
            *block = m_blocks->at(m_cursor).Get();
            (*block)->AddRef();
            return static_cast<HRESULT>(Error::OK);
        } CATCH_RETURN();

        HRESULT STDMETHODCALLTYPE GetHasCurrent(BOOL* hasCurrent) noexcept override try
        {   
            ThrowErrorIfNot(Error::InvalidParameter, (hasCurrent), "bad pointer");
            *hasCurrent = (m_cursor != m_blocks->size()) ? TRUE : FALSE;
            return static_cast<HRESULT>(Error::OK);
        } CATCH_RETURN();

        HRESULT STDMETHODCALLTYPE MoveNext(BOOL* hasNext) noexcept override try
        {
            ThrowErrorIfNot(Error::InvalidParameter, (hasNext), "bad pointer");
            *hasNext = (++m_cursor != m_blocks->size()) ? TRUE : FALSE;
            return static_cast<HRESULT>(Error::OK);
        } CATCH_RETURN();
    };

    class AppxBlockMapFile final : public MSIX::ComClass<AppxBlockMapFile, IAppxBlockMapFile>
    {
    public:
        AppxBlockMapFile(
            IMSIXFactory* factory,
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
            if (m_blockMapBlocks.empty())
            {   m_blockMapBlocks.reserve(m_blocks->size());
                std::transform(
                    m_blocks->begin(),
                    m_blocks->end(),
                    std::back_inserter(m_blockMapBlocks),
                    [&](auto item){
                        return ComPtr<IAppxBlockMapBlock>::Make<AppxBlockMapBlock>(m_factory, &item);
                    }
                );
            }
            ThrowErrorIf(Error::InvalidParameter, (blocks == nullptr || *blocks != nullptr), "bad pointer.");
            *blocks = ComPtr<IAppxBlockMapBlocksEnumerator>::Make<AppxBlockMapBlocksEnumerator>(&m_blockMapBlocks).Detach();
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

    private:

        std::vector<ComPtr<IAppxBlockMapBlock>> m_blockMapBlocks;
        std::vector<Block>* m_blocks;
        IMSIXFactory*       m_factory;
        std::uint32_t       m_localFileHeaderSize;
        std::string         m_name;
        std::uint64_t       m_uncompressedSize;
    };

    class AppxBlockMapFilesEnumerator final : public MSIX::ComClass<AppxBlockMapFilesEnumerator, IAppxBlockMapFilesEnumerator>
    {
    protected:
        ComPtr<IAppxBlockMapReader> m_reader;
        std::vector<std::string>    m_files;
        std::size_t                 m_cursor = 0;

    public:
        AppxBlockMapFilesEnumerator(const ComPtr<IAppxBlockMapReader>& reader, std::vector<std::string>&& files) :
            m_reader(reader), m_files(files)
        {}

        // IAppxBlockMapFilesEnumerator
        HRESULT STDMETHODCALLTYPE GetCurrent(IAppxBlockMapFile** block) noexcept override try
        {
            ThrowErrorIf(Error::Unexpected, (m_cursor >= m_files.size()), "index out of range");
            ThrowHrIfFailed(m_reader->GetFile(utf8_to_wstring(m_files.at(m_cursor)).c_str(), block));
            return static_cast<HRESULT>(Error::OK);
        } CATCH_RETURN();

        HRESULT STDMETHODCALLTYPE GetHasCurrent(BOOL* hasCurrent) noexcept override try
        {
            ThrowErrorIfNot(Error::InvalidParameter, (hasCurrent), "bad pointer");
            *hasCurrent = (m_cursor != m_files.size()) ? TRUE : FALSE;
            return static_cast<HRESULT>(Error::OK);
        } CATCH_RETURN();

        HRESULT STDMETHODCALLTYPE MoveNext(BOOL* hasNext) noexcept override try
        {
            ThrowErrorIfNot(Error::InvalidParameter, (hasNext), "bad pointer");
            *hasNext = (++m_cursor != m_files.size()) ? TRUE : FALSE;
            return static_cast<HRESULT>(Error::OK);
        } CATCH_RETURN();
    };

    // Object backed by AppxBlockMap.xml
    class AppxBlockMapObject final : public MSIX::ComClass<AppxBlockMapObject, IAppxBlockMapReader, IVerifierObject, IAppxBlockMapInternal>
    {
    public:
        AppxBlockMapObject(IMSIXFactory* factory, const ComPtr<IStream>& stream);

        // IVerifierObject
        const std::string& GetPublisher() override {NOTSUPPORTED;}
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

    protected:
        std::map<std::string, std::vector<Block>>        m_blockMap;
        std::map<std::string, ComPtr<IAppxBlockMapFile>> m_blockMapFiles;
        IMSIXFactory*   m_factory;
        ComPtr<IStream> m_stream;
    };
}
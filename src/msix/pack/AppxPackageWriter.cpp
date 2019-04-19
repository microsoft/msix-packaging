//
//  Copyright (C) 2019 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
// 
#include "AppxPackageWriter.hpp"
#include "MsixErrors.hpp"

#include <string>

namespace MSIX {

    AppxPackageWriter::AppxPackageWriter(IStream* outputStream) : m_outputStream(outputStream)
    {
        m_state = WriterState::Open;
    }

    // IPackageWriter
    void AppxPackageWriter::Pack(const ComPtr<IDirectoryObject>& from)
    {
        ThrowErrorIf(Error::InvalidState, m_state != WriterState::Open, "Invalid package writer state");
        auto fileMap = from->GetFilesByLastModDate();

        for(const auto& file : fileMap)
        {
            // TODO: either create APPX_PACKAGE_WRITER_PAYLOAD_STREAM_UTF8s and call AddPayloadFiles
            // or make the blockmap/zip writer take the information and call it from here, AddPayloadFiles
            // and AddPayloadFile
        }
    }

    // IAppxPackageWriter
    HRESULT STDMETHODCALLTYPE AppxPackageWriter::AddPayloadFile(LPCWSTR fileName, LPCWSTR contentType,
        APPX_COMPRESSION_OPTION compressionOption, IStream *inputStream) noexcept try
    {
        return AddPayloadFile(wstring_to_utf8(fileName).c_str(), wstring_to_utf8(contentType).c_str(), 
            compressionOption, inputStream);
    } CATCH_RETURN();

    HRESULT STDMETHODCALLTYPE AppxPackageWriter::Close(IStream *manifest) noexcept try
    {
        ThrowErrorIf(Error::InvalidState, m_state != WriterState::Open, "Invalid package writer state");
        // TODO: implement
        m_state = WriterState::Closed;
        return static_cast<HRESULT>(Error::OK);
    } CATCH_RETURN();

    // IAppxPackageWriterUtf8
    HRESULT STDMETHODCALLTYPE AppxPackageWriter::AddPayloadFile(LPCSTR fileName, LPCSTR contentType,
        APPX_COMPRESSION_OPTION compressionOption, IStream* inputStream) noexcept try
    {
        ThrowErrorIf(Error::InvalidState, m_state != WriterState::Open, "Invalid package writer state");
        // TODO: implement
        return static_cast<HRESULT>(Error::OK);
    } CATCH_RETURN();

    // IAppxPackageWriter3
    HRESULT STDMETHODCALLTYPE AppxPackageWriter::AddPayloadFiles(UINT32 fileCount,
        APPX_PACKAGE_WRITER_PAYLOAD_STREAM* payloadFiles, UINT64 memoryLimit) noexcept try
    {
        ThrowErrorIf(Error::InvalidState, m_state != WriterState::Open, "Invalid package writer state");
        // TODO: implement
        return static_cast<HRESULT>(Error::OK);
    } CATCH_RETURN();

    // IAppxPackageWriter3Utf8
    HRESULT STDMETHODCALLTYPE AppxPackageWriter::AddPayloadFiles(UINT32 fileCount,
        APPX_PACKAGE_WRITER_PAYLOAD_STREAM_UTF8* payloadFiles, UINT64 memoryLimit) noexcept try
    {
        ThrowErrorIf(Error::InvalidState, m_state != WriterState::Open, "Invalid package writer state");
        // TODO: implement
        return static_cast<HRESULT>(Error::OK);
    } CATCH_RETURN();
}
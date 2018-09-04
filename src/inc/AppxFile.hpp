//
//  Copyright (C) 2017 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
// 
#pragma once

#include <memory>
#include <vector>
#include <algorithm>
#include <iostream>
#include <limits>

#include "MSIXWindows.hpp"
#include "AppxPackaging.hpp"
#include "MSIXFactory.hpp"
#include "Exceptions.hpp"
#include "ComHelper.hpp"
#include "FileStream.hpp"
#include "UnicodeConversion.hpp"
#include "StreamBase.hpp"

namespace MSIX {
    class AppxFile : public ComClass<AppxFile, IAppxFile>
    {
    public:
        AppxFile(IMsixFactory* factory, const std::string& name, const ComPtr<IStream>& stream) : m_factory(factory), m_name(name), m_stream(stream)
        {
            LARGE_INTEGER start = { 0 };
            ULARGE_INTEGER end = { 0 };
            ThrowHrIfFailed(m_stream->Seek(start, StreamBase::Reference::END, &end));
            ThrowHrIfFailed(m_stream->Seek(start, StreamBase::Reference::START, nullptr));
            m_size = end.u.LowPart;
        }

        // IAppxFile methods
        virtual HRESULT STDMETHODCALLTYPE GetCompressionOption(APPX_COMPRESSION_OPTION* compressionOption) noexcept override
        {
            if (compressionOption)
            {
                *compressionOption = APPX_COMPRESSION_OPTION_NONE;
                ComPtr<IStreamInternal> streamInt;
                HRESULT hr = m_stream->QueryInterface(UuidOfImpl<IStreamInternal>::iid, reinterpret_cast<void**>(&streamInt));
                if (SUCCEEDED(hr))
                {
                    *compressionOption = streamInt->IsCompressed() ? APPX_COMPRESSION_OPTION_NORMAL : APPX_COMPRESSION_OPTION_NONE;
                }
            }
            return static_cast<HRESULT>(Error::OK);
        }

        virtual HRESULT STDMETHODCALLTYPE GetContentType(LPWSTR* contentType) noexcept override
        {
            return static_cast<HRESULT>(Error::NotImplemented);
        }

        virtual HRESULT STDMETHODCALLTYPE GetName(LPWSTR* fileName) noexcept override try
        {
            return m_factory->MarshalOutString(m_name, fileName);
        } CATCH_RETURN();

        virtual HRESULT STDMETHODCALLTYPE GetSize(UINT64* size) noexcept override try
        {
            if (size)
            {
                *size = m_size;
            }
            return static_cast<HRESULT>(Error::OK);
        } CATCH_RETURN();

        virtual HRESULT STDMETHODCALLTYPE GetStream(IStream** stream) noexcept override try
        {
            ThrowErrorIf(Error::InvalidParameter, (stream == nullptr || *stream != nullptr), "bad pointer");
            *stream = m_stream.As<IStream>().Detach();
            return static_cast<HRESULT>(Error::OK);
        } CATCH_RETURN();

    protected:
        std::string m_name;
        ComPtr<IStream> m_stream;
        IMsixFactory* m_factory;
        std::uint64_t m_size;
    };
}
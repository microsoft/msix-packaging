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
    class AppxFile : public ComClass<AppxFile, IAppxFile, IAppxFileUtf8>
    {
    public:
        AppxFile(IMsixFactory* factory, const std::string& name, std::function<ComPtr<IStream>()>&& streamFunc)
            : m_factory(factory), m_name(name), m_streamFunc(std::move(streamFunc))
        { }

        // IAppxFile methods
        virtual HRESULT STDMETHODCALLTYPE GetCompressionOption(APPX_COMPRESSION_OPTION* compressionOption) noexcept override
        {
            if (compressionOption)
            {
                *compressionOption = APPX_COMPRESSION_OPTION_NONE;
                ComPtr<IStreamInternal> streamInt;
                HRESULT hr = m_streamFunc()->QueryInterface(UuidOfImpl<IStreamInternal>::iid, reinterpret_cast<void**>(&streamInt));
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
                STATSTG statstg {};
                ThrowHrIfFailed(m_streamFunc()->Stat(&statstg, 0));
                *size = static_cast<uint64_t>(statstg.cbSize.QuadPart);
            }
            return static_cast<HRESULT>(Error::OK);
        } CATCH_RETURN();

        virtual HRESULT STDMETHODCALLTYPE GetStream(IStream** stream) noexcept override try
        {
            ThrowErrorIf(Error::InvalidParameter, (stream == nullptr || *stream != nullptr), "bad pointer");
            *stream = m_streamFunc().As<IStream>().Detach();
            return static_cast<HRESULT>(Error::OK);
        } CATCH_RETURN();

        // IAppxFileUtf8
        virtual HRESULT STDMETHODCALLTYPE GetContentType(LPSTR* contentType) noexcept override
        {
            return static_cast<HRESULT>(Error::NotImplemented);
        }

        virtual HRESULT STDMETHODCALLTYPE GetName(LPSTR* fileName) noexcept override try
        {
            return m_factory->MarshalOutStringUtf8(m_name, fileName);
        } CATCH_RETURN();

    protected:
        std::string m_name;
        std::function<ComPtr<IStream>()> m_streamFunc;
        IMsixFactory* m_factory;
    };
}
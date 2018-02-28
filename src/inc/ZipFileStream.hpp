//
//  Copyright (C) 2017 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
// 
#pragma once
#include "Exceptions.hpp"
#include "ComHelper.hpp"
#include "StreamBase.hpp"
#include "RangeStream.hpp"
#include "AppxFactory.hpp"

#include <string>

namespace MSIX {

    // This represents a raw stream over a file contained in a .zip file.
    class ZipFileStream : public RangeStream
    {
    public:
        // TODO: define what streams to pass in on the .ctor
        ZipFileStream(
            std::string name,
            std::string contentType,
            IMSIXFactory* factory,
            bool isCompressed,
            std::uint64_t offset,
            std::uint64_t size,
            IStream* stream
        ) : m_isCompressed(isCompressed), RangeStream(offset, size, stream), m_name(name), m_contentType(contentType), m_factory(factory)
        {
        }

        HRESULT STDMETHODCALLTYPE GetName(LPWSTR* fileName) override
        {
            return m_factory->MarshalOutString(m_name, fileName);
        }

        HRESULT STDMETHODCALLTYPE GetContentType(LPWSTR* contentType) override
        {
            return m_factory->MarshalOutString(m_contentType, contentType);
        }

        HRESULT STDMETHODCALLTYPE GetCompressionOption(APPX_COMPRESSION_OPTION* compressionOption) override
        {
            if (compressionOption) { *compressionOption = IsCompressed() ? APPX_COMPRESSION_OPTION_NORMAL : APPX_COMPRESSION_OPTION_NONE; }
            return static_cast<HRESULT>(Error::OK);
        }

        inline bool IsCompressed() { return m_isCompressed; }

    protected:
        IMSIXFactory*  m_factory;
        std::string     m_name;
        std::string     m_contentType;
        bool            m_isCompressed = false;
    };
}
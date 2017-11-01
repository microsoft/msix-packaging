#pragma once
#include "Exceptions.hpp"
#include "ComHelper.hpp"
#include "StreamBase.hpp"
#include "RangeStream.hpp"

#include <string>

namespace xPlat {

    // This represents a raw stream over a file contained in a .zip file.
    class ZipFileStream : public RangeStream
    {
    public:
        // TODO: define what streams to pass in on the .ctor
        ZipFileStream(
            bool isCompressed,
            std::uint32_t offset,
            std::uint32_t size,
            IStream* stream
        ) : m_isCompressed(isCompressed), RangeStream(offset, size, stream)
        {
        }

        HRESULT STDMETHODCALLTYPE GetName(LPWSTR* fileName) override
        {
            // TODO: Implement here.
            return static_cast<HRESULT>(Error::NotImplemented);
        }

        HRESULT STDMETHODCALLTYPE GetContentType(LPWSTR* contentType) override
        {
            // TODO: Implement here.
            return static_cast<HRESULT>(Error::NotImplemented);
        }

        HRESULT STDMETHODCALLTYPE GetCompressionOption(APPX_COMPRESSION_OPTION* compressionOption) override
        {
            if (compressionOption) { *compressionOption = IsCompressed() ? APPX_COMPRESSION_OPTION_NORMAL : APPX_COMPRESSION_OPTION_NONE; }
            return static_cast<HRESULT>(Error::OK);
        }

        inline bool IsCompressed() { return m_isCompressed; }

    protected:
        bool m_isCompressed = false;
    };
}
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
    class ZipFileStream final : public RangeStream
    {
    public:
        // TODO: define what streams to pass in on the .ctor
        ZipFileStream(
            std::string name,
            std::string contentType,
            IMsixFactory* factory,
            bool isCompressed,
            std::uint64_t offset,
            std::uint64_t size,
            const ComPtr<IStream>& stream
        ) : m_isCompressed(isCompressed), RangeStream(offset, size, stream), m_name(name), m_contentType(contentType), m_factory(factory), m_compressedSize(size)
        {
        }

        // IStreamInternal
        std::uint64_t GetSizeOnZip() override { return m_compressedSize; }
        bool IsCompressed() override { return m_isCompressed; }
        std::string GetName() override { return m_name; }

    protected:
        IMsixFactory*   m_factory;
        std::string     m_name;
        std::string     m_contentType;
        bool            m_isCompressed = false;
        std::uint64_t   m_compressedSize;
    };
}
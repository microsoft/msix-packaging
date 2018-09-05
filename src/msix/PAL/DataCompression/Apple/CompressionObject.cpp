//
//  Copyright (C) 2017 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
//
#include "ICompressionObject.hpp"
#include "Compression.h"
#include "Exceptions.hpp"

using namespace std;

namespace MSIX {

    class CompressionObject final : public ICompressionObject
    {
    public:
        CompressionObject() = default;

        // ICompressionObject interface
        CompressionStatus Initialize(CompressionOperation operation) noexcept
        {
            m_compressionStream = {0};

            switch (operation)
            {
                case CompressionOperation::Inflate:
                    return GetStatus(compression_stream_init(&m_compressionStream, COMPRESSION_STREAM_DECODE, COMPRESSION_ZLIB));
                    break;
                default:
                    NOTIMPLEMENTED;
            }
        }

        CompressionStatus Inflate() noexcept
        {
            return GetStatus(compression_stream_process(&m_compressionStream, 0));
        }

        CompressionStatus Cleanup() noexcept
        {
            return GetStatus(compression_stream_destroy(&m_compressionStream));
        }

        size_t GetAvailableSourceSize() noexcept
        {
            return m_compressionStream.src_size;
        }

        size_t GetAvailableDestinationSize() noexcept
        {
            return m_compressionStream.dst_size;
        }

        void SetInput(uint8_t* buffer, size_t size) noexcept
        {
            m_compressionStream.src_ptr = buffer;
            m_compressionStream.src_size = size;
        }

        void SetOutput(uint8_t* buffer, size_t size) noexcept
        {
            m_compressionStream.dst_ptr = buffer;
            m_compressionStream.dst_size = size;
        }

    private:
        compression_stream m_compressionStream = {0};

        CompressionStatus GetStatus(compression_status status)
        {
            switch (status)
            {
                case COMPRESSION_STATUS_OK:
                    return CompressionStatus::Ok;
                case COMPRESSION_STATUS_ERROR:
                    return CompressionStatus::Error;
                case COMPRESSION_STATUS_END:
                    return CompressionStatus::End;
                default:
                    NOTSUPPORTED;
            }
        }
    };

    std::unique_ptr<ICompressionObject> CreateCompressionObject()
    {
        return std::make_unique<CompressionObject>();
    }
}
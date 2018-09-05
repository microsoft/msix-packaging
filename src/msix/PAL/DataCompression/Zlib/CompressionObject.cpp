//
//  Copyright (C) 2017 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
//
#include "ICompressionObject.hpp"
#include "Exceptions.hpp"
#ifdef WIN32
#include "zlib.h"
#else
#include <zlib.h>
#endif

using namespace std;

namespace MSIX {

    class CompressionObject final : public ICompressionObject
    {
    public:
        CompressionObject() = default;

        // ICompressionObject interface
        CompressionStatus Initialize(CompressionOperation operation) noexcept
        {
            m_zstrm = { 0 };

            switch (operation)
            {
                case CompressionOperation::Inflate:
                    return GetStatus(inflateInit2(&m_zstrm, -MAX_WBITS));
                    break;
                default:
                    NOTIMPLEMENTED;
            }
        }

        CompressionStatus Inflate() noexcept
        {
            return GetStatus(inflate(&m_zstrm, Z_NO_FLUSH));
        }

        CompressionStatus Cleanup() noexcept
        {
            return GetStatus(inflateEnd(&m_zstrm));
        }

        size_t GetAvailableSourceSize() noexcept
        {
            return m_zstrm.avail_in;
        }

        size_t GetAvailableDestinationSize() noexcept
        {
            return m_zstrm.avail_out;
        }

        void SetInput(uint8_t* buffer, size_t size) noexcept
        {
            m_zstrm.next_in = buffer;
            m_zstrm.avail_in = static_cast<uint32_t>(size);
        }

        void SetOutput(uint8_t* buffer, size_t size) noexcept
        {
            m_zstrm.next_out = buffer;
            m_zstrm.avail_out = static_cast<uint32_t>(size);
        }

    private:
        z_stream        m_zstrm;

        CompressionStatus GetStatus(int status)
        {
            switch (status)
            {
                case Z_OK:
                    return CompressionStatus::Ok;
                case Z_DATA_ERROR:
                case Z_MEM_ERROR:
                case Z_BUF_ERROR:
                case Z_STREAM_ERROR:
                case Z_ERRNO:
                    return CompressionStatus::Error;
                case Z_STREAM_END:
                    return CompressionStatus::End;
                case Z_NEED_DICT:
                    return CompressionStatus::NeedDictionary;
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
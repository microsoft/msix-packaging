//
//  Copyright (C) 2017 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
// 
#pragma once
#include "Exceptions.hpp"
#include "StreamBase.hpp"
#include "ComHelper.hpp"
#include "MsixFeatureSelector.hpp"

#include <vector>
#include <algorithm>

namespace MSIX {

    class VectorStream final : public StreamBase
    {
    public:
        VectorStream(std::vector<std::uint8_t>* data) : m_data(data) {}

        HRESULT STDMETHODCALLTYPE Read(void* buffer, ULONG countBytes, ULONG* bytesRead) noexcept override try
        {
            ULONG amountToRead = std::min(countBytes, static_cast<ULONG>(m_data->size() - m_offset));
            if (amountToRead > 0) { memcpy(buffer, &(m_data->at(m_offset)), amountToRead); }                
            m_offset += amountToRead;
            if (bytesRead) { *bytesRead = amountToRead; }
            return static_cast<HRESULT>(Error::OK);
        } CATCH_RETURN();

        HRESULT STDMETHODCALLTYPE Seek(LARGE_INTEGER move, DWORD origin, ULARGE_INTEGER *newPosition) noexcept override try
        {
            LARGE_INTEGER newPos {0};
            switch (origin)
            {
            case Reference::CURRENT:
                newPos.QuadPart = m_offset + move.QuadPart;
                break;
            case Reference::START:
                newPos.QuadPart = move.QuadPart;
                break;
            case Reference::END:
                newPos.QuadPart = static_cast<std::uint64_t>(m_data->size()) + move.QuadPart;
                break;
            }
            m_offset = std::min(newPos.u.LowPart, static_cast<ULONG>(m_data->size()));
            if (newPosition) { newPosition->QuadPart = newPos.QuadPart; }
            return static_cast<HRESULT>(Error::OK);
        } CATCH_RETURN();

        HRESULT STDMETHODCALLTYPE Write(const void *buffer, ULONG countBytes, ULONG *bytesWritten) noexcept override try
        {
            THROW_IF_PACK_NOT_ENABLED
            // make sure that we can allocate the buffer
            ULONG expected = static_cast<ULONG>(m_data->size()) + countBytes; 
            m_data->resize(expected);
            if (countBytes > 0) { memcpy(&(m_data->at(m_offset)), buffer, countBytes); }
            m_offset = static_cast<ULONG>(m_data->size());
            ThrowErrorIf(Error::FileWrite, expected != m_offset, "Error writing to stream");
            if (bytesWritten) { *bytesWritten = countBytes; }
            return static_cast<HRESULT>(Error::OK);
        } CATCH_RETURN();

        std::uint64_t GetSize() override
        {
            return static_cast<std::uint64_t>(m_data->size());
        }

    protected:
        ULONG m_offset = 0;
        std::vector<std::uint8_t>* m_data;
    };
} // namespace MSIX
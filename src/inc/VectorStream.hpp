#pragma once
#include "Exceptions.hpp"
#include "StreamBase.hpp"
#include "ComHelper.hpp"
#include <vector>
#include <algorithm>

namespace MSIX {

    class VectorStream : public StreamBase
    {
    public:
        VectorStream(std::vector<std::uint8_t>* data) : m_data(data) {}

        HRESULT STDMETHODCALLTYPE Read(void* buffer, ULONG countBytes, ULONG* bytesRead) override
        {
            return ResultOf([&]{
                ULONG amountToRead = std::min(countBytes, static_cast<ULONG>(m_data->size() - m_offset));
                if (amountToRead > 0) { memcpy(buffer, &(m_data->at(m_offset)), amountToRead); }                
                m_offset += amountToRead;
                if (bytesRead) { *bytesRead = amountToRead; }
            });
        }

        HRESULT STDMETHODCALLTYPE Seek(LARGE_INTEGER move, DWORD origin, ULARGE_INTEGER *newPosition) override
        {
            return ResultOf([&]{
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
            });
        }

    protected:
        ULONG m_offset = 0;
        std::vector<std::uint8_t>* m_data;
    };
} // namespace MSIX
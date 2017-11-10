#pragma once
#define NOMINMAX /* windows.h, or more correctly windef.h, defines min as a macro... */
#include "Exceptions.hpp"
#include "StreamBase.hpp"
#include "ComHelper.hpp"
#include "SHA256.hpp"

#include <string>
#include <map>
#include <functional>
#include <algorithm>

namespace xPlat {
  
    // This represents a subset of a Stream
    class HashStream : public StreamBase
    {
    public:
        HashStream(IStream* stream, std::vector<byte>& expectedHash) :
            m_hashesMatch(false),
            m_relativePosition(0)
        {
            m_expectedHash.assign(expectedHash.begin(), expectedHash.end());
            STATSTG statstg = { 0 };
            HRESULT hr = stream->Stat(&statstg, STATFLAG_NONAME);

            //TODO: validate that statstg.cbSize is reasonable here
            if (FAILED(hr) || statstg.cbSize.LowPart == 0)
                throw xPlat::Exception(xPlat::Error::AppxSignatureInvalid); //TODO: better exception

            ULARGE_INTEGER uli;
            LARGE_INTEGER li;
            li.QuadPart = 0;
            hr = stream->Seek(li, STREAM_SEEK_SET, &uli);

            if (FAILED(hr) || uli.QuadPart != 0)
                throw xPlat::Exception(xPlat::Error::AppxSignatureInvalid); //TODO: better exception

            m_cacheBuffer.resize(statstg.cbSize.LowPart);
            ULONG bytesRead = 0;
            hr = stream->Read(m_cacheBuffer.data(), m_cacheBuffer.size(), &bytesRead);

            if (FAILED(hr) || bytesRead != statstg.cbSize.LowPart)
                throw xPlat::Exception(xPlat::Error::AppxSignatureInvalid); //TODO: better exception

            std::vector<std::uint8_t> hash;
            if (!xPlat::SHA256::ComputeHash(m_cacheBuffer.data(), m_cacheBuffer.size(), hash))
                throw xPlat::Exception(xPlat::Error::AppxSignatureInvalid); //TODO: better exception

            if (m_cacheBuffer.size() != hash.size())
                throw xPlat::Exception(xPlat::Error::AppxSignatureInvalid); //TODO: better exception

            m_hashesMatch = (memcmp(m_cacheBuffer.data(), hash.data(), m_cacheBuffer.size()) == 0);

            if (!m_hashesMatch)
                throw xPlat::Exception(xPlat::Error::AppxSignatureInvalid); //TODO: better exception
        }

        HRESULT STDMETHODCALLTYPE Seek(LARGE_INTEGER move, DWORD origin, ULARGE_INTEGER *newPosition) override
        {
            if (m_hashesMatch)
            {
                LARGE_INTEGER newPos = { 0 };
                switch (origin)
                {
                    case Reference::CURRENT:
                        m_relativePosition += move.LowPart;
                        break;
                    case Reference::START:
                        m_relativePosition = move.LowPart;
                        break;
                    case Reference::END:
                        m_relativePosition = m_cacheBuffer.size();
                        break;
                }
                m_relativePosition = std::max((std::uint32_t)0, std::min(m_relativePosition, (std::uint32_t)m_cacheBuffer.size()));
                if (newPosition)
                {
                    newPosition->QuadPart = (std::uint64_t)m_relativePosition;
                }
            }
            else
            {
                throw xPlat::Exception(xPlat::Error::AppxSignatureInvalid); //TODO: better exception
            }
            return S_OK;
        }

        HRESULT STDMETHODCALLTYPE Read(void* buffer, ULONG countBytes, ULONG* actualRead) override
        {
            HRESULT hr = STG_E_INVALIDPOINTER;
            if (buffer)
            {
                if (m_hashesMatch)
                {
                    ULONG bytesToRead = std::min((std::uint32_t)countBytes, (std::uint32_t)m_cacheBuffer.size() - m_relativePosition);
                    if (bytesToRead)
                    {
                        memcpy(buffer, reinterpret_cast<BYTE*>(m_cacheBuffer.data()) + m_relativePosition, bytesToRead);
                    }
                    if (actualRead)
                        *actualRead = bytesToRead;
                    hr = (countBytes = bytesToRead) ? S_OK : S_FALSE;
                }
                else
                {
                    throw xPlat::Exception(xPlat::Error::AppxSignatureInvalid); //TODO: better exception
                }
            }
            return hr;
        }
      
    protected:
        bool m_hashesMatch;
        std::vector<std::uint8_t> m_expectedHash;
        std::vector<std::uint8_t> m_cacheBuffer;
        std::uint32_t m_relativePosition;
    };
}
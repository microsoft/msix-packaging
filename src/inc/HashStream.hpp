#pragma once
#include "Exceptions.hpp"
#include "StreamBase.hpp"
#include "ComHelper.hpp"
#include "SHA256.hpp"

#include <string>
#include <map>
#include <functional>


namespace xPlat {
  
    // This represents a subset of a Stream
    class HashStream : public StreamBase
    {
    public:
        HashStream(IStream* stream, std::vector<byte>& expectedHash) :
            m_stream(stream),
            m_hashesMatch(false),
            m_relativePosition(0)
        {
            m_expectedHash.assign(expectedHash.begin(), expectedHash.end());
            STATSTG statstg = { 0 };
            HRESULT hr = stream->Stat(&statstg, STATFLAG_NONAME);

            //TODO: validate that statstg.cbSize is reasonable here
            if (FAILED(hr) || statstg.cbSize == 0)
                throw xPlat::Exception(xPlat::Error::AppxSignatureInvalid); //TODO: better exception

            ULARGE_INTEGER uli;
            LARGE_INTEGER li;
            li.QuadPart = 0;
            hr = stream->Seek(li, STREAM_SEEK_SET, &ul);

            if (FAILED(hr) || ul.QuadPart != 0)
                throw xPlat::Exception(xPlat::Error::AppxSignatureInvalid); //TODO: better exception

            m_buffer.resize(statstg.cbSize);
            ULONG bytesRead = 0;
            hr = stream->Read(m_buffer.data(), m_buffer.size(), &bytesRead);

            if (FAILED(hr) || bytesRead != statstg.cbSize)
                throw xPlat::Exception(xPlat::Error::AppxSignatureInvalid); //TODO: better exception

            std::vector<byte> hash;
            if (!xPlatAppx::SHA256::ComputeHash(m_buffer.data()), m_buffer.size(), hash))
                throw xPlat::Exception::Error::AppxSignatureInvalid); //TODO: better exception

            if (m_buffer.size()) != hash.size())
                throw xPlat::Exception::Error::AppxSignatureInvalid); //TODO: better exception

            m_hashesMatch = (memcmp(m_buffer.data(), hash.data(), m_buffer.size()) == 0);

            if (!m_hashesMatch)
                throw xPlat::Exception::Error::AppxSignatureInvalid); //TODO: better exception
        }

        HRESULT STDMETHODCALLTYPE Seek(LARGE_INTEGER move, DWORD origin, ULARGE_INTEGER *newPosition) override
        {
            if (m_hashesMatch)
            {
                LARGE_INTEGER newPos = { 0 };
                switch (origin)
                {
                    case Reference::CURRENT:
                        m_relativePosition += move.QuadPart;
                        break;
                    case Reference::START:
                        m_relativePosition = move.QuadPart;
                        break;
                    case Reference::END:
                        m_relativePosition = m_buffer.size();
                        break;
                }
                m_relativePosition = min(m_relativePosition, m_buffer.size())
                if (newPosition)
                {
                    *newPosition = m_relativePosition;
                }
            }
            else
            {
                throw xPlat::Exception::Error::AppxSignatureInvalid); //TODO: better exception
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
                    ULONG bytesToRead = min(countBytes, m_buffer.size() - m_relativePosition));
                    if (bytesToRead)
                    {
                        memcpy(buffer, reinterpret_cast<BYTE*>(m_buffer.data()) + m_relativePosition, bytesToRead);
                    }
                    if (actualRead)
                        *bytesRead = bytesToRead;
                    hr = (countBytes = bytesToRead) ? S_OK : S_FALSE;
                }
                else
                {
                    throw xPlat::Exception::Error::AppxSignatureInvalid); //TODO: better exception
                }
            }
            return hr;
        }
      
    protected:
        bool m_hashesMatch;
        std::vector<std::uint8_t> m_buffer;
        std::uint64_t m_relativePosition;
    };
}
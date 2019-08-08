
#include <Windows.h>
#include <ByteBuffer.hpp>

#include "GeneralUtil.hpp"
#include <TraceLoggingProvider.h>
#include "MsixTraceLoggingProvider.hpp"
#include <assert.h>

namespace MsixCoreLib
{
    void ByteBuffer::Clear()
    {
        if (capacity > minCapacity)
        {
            delete[] this->value.bytes;
            this->value.bytes = NULL;
            this->capacity = 0;
        }
        this->value.length = 0;
    }

    HRESULT ByteBuffer::SetCapacity(
        _In_ ULONG capacity)
    {
        if (capacity > ByteBuffer::MaxLength)
        {
            return HRESULT_FROM_WIN32(ERROR_PARAMETER_QUOTA_EXCEEDED);
        }

        if (capacity != this->capacity)
        {
            if (capacity != 0)
            {
                std::unique_ptr<BYTE[]> newBytes(new APPXCOMMON_NEW_TAG BYTE[capacity]);
                if (newBytes == nullptr)
                {
                    return HRESULT_FROM_WIN32(ERROR_OUTOFMEMORY);
                }
                memcpy_s(newBytes.get(), capacity, this->value.bytes, this->value.length);
                delete[] this->value.bytes;
                this->value.bytes = newBytes.release();
            }
            else
            {
                delete[] this->value.bytes;
                this->value.bytes = NULL;
            }

            this->capacity = capacity;

            if (this->value.length > this->capacity)
            {
                this->value.length = this->capacity;
            }
            else if (this->value.length < this->capacity)
            {
                DebugFillMemory(
                    &this->value.bytes[this->value.length], this->capacity - this->value.length);
            }
        }

        return S_OK;
    }

    ULONG ByteBuffer::GetCapacityFromLength(
        _In_ ULONG length)
    {
        if (length > minCapacity)
        {
            return length;
        }
        ULONG capacity = initialCapacity;
        while (capacity < length)
        {
            capacity = capacity * 2;
        }
        return capacity;
    }

    HRESULT ByteBuffer::SetLength(
        _In_ ULONG length)
    {
        if (length > this->capacity)
        {
            RETURN_IF_FAILED(SetCapacity(length));
        }
        if (this->value.length > length)
        {
            DebugFillMemory(&this->value.bytes[length], this->value.length - length);
        }
        this->value.length = length;
        return NOERROR;
    }

    HRESULT ByteBuffer::SetValue(
        _In_reads_(length) const BYTE* bytes,
        _In_ ULONG length)
    {
        if (length > this->capacity || this->capacity > minCapacity)
        {
            RETURN_IF_FAILED(SetCapacity(GetCapacityFromLength(length)));
        }
        RETURN_IF_FAILED(SetLength(length));
        ::CopyMemory(this->value.bytes, bytes, length);
        return NOERROR;
    }
}

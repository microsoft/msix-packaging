#pragma once
#include <memory>
#include <string>

#include "Exceptions.hpp"
#include "StreamBase.hpp"

namespace xPlat {

    // A base class for objects that are used to verify structured data represented by an underlying stream.
    class VerifierObject
    {
    public:
        VerifierObject(IStream* stream) : m_stream(stream) {}

        inline bool HasStream() { return m_stream.Get() != nullptr; }
        inline IStream* GetStream() { return m_stream.Get(); }       
        
        virtual IStream* GetValidationStream(const std::string& part, IStream* stream) = 0;

    protected:
        ComPtr<IStream> m_stream;        
    };
    
} // namespace xPlat

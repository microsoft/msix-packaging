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
        VerifierObject(std::shared_ptr<StreamBase> stream) : m_stream(std::move(stream)) {}

        inline bool HasStream() { return m_stream != nullptr;}
        inline std::shared_ptr<StreamBase>& GetStream() { return m_stream; }       
        
        virtual std::shared_ptr<StreamBase> GetValidationStream(const std::string& part, std::shared_ptr<StreamBase> stream) = 0;

    protected:
        std::shared_ptr<StreamBase> m_stream = nullptr;        
    };
    
} // namespace xPlat

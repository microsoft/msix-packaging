#pragma once
#include <memory>
#include <string>

#include "Exceptions.hpp"
#include "StreamBase.hpp"

namespace xPlat {

    // internal interface
    EXTERN_C const IID IID_IVerifierObject;   
    #ifndef WIN32
    MIDL_INTERFACE("cb0a105c-3a6c-4e48-9351-377c4dccd890")
    interface IVerifierObject : public IUnknown
    #else
    #include "Unknwn.h"
    #include "Objidl.h"
    class IVerifierObject : public IUnknown
    #endif
    // An internal interface for objects that are used to verify structured data represented by an underlying stream.
    {
    public:
        #ifdef WIN32
        virtual ~IVerifierObject() {}
        #endif            

        virtual bool HasStream() = 0;
        virtual IStream* GetStream() = 0;
        virtual IStream* GetValidationStream(const std::string& part, IStream* stream) = 0;
    };
    
    SpecializeUuidOfImpl(IVerifierObject);

} // namespace xPlat

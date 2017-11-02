#pragma once
#include <memory>
#include <string>
#include <vector>

#include "Exceptions.hpp"
#include "StreamBase.hpp"
#include "VerifierObject.hpp"

// Mandatory for using any feature of Xerces.
#include "xercesc/util/PlatformUtils.hpp"
#include "xercesc/dom/DOM.hpp"

namespace xPlat {

    // XML de-serialization happens during construction, of this object.
    // XML serialization happens through the Write method
    class XmlObject : public VerifierObject
    {
    public:
        // TODO: Implement actual XML validation....
        XmlObject(IStream* stream) : VerifierObject(stream) {}

        // TODO: Implement: Writes the contents of the DOM Document to the underlying stream.
        void Write()
        { 
            throw Exception(Error::NotImplemented);
        }

        // Returns a shared pointer to the DOMDocument representing the contents of this stream
        std::shared_ptr<XERCES_CPP_NAMESPACE::DOMDocument> Document()         { return m_DOMDocument;}

        IStream* GetValidationStream(const std::string& part, IStream* stream) override
        {
            throw Exception(Error::NotSupported);
        }

    protected:
        ComPtr<IStream> m_stream;
        std::shared_ptr<XERCES_CPP_NAMESPACE::DOMImplementation> m_DOMImplementation;
        std::shared_ptr<XERCES_CPP_NAMESPACE::DOMDocument>       m_DOMDocument;
    };

} // namespace xPlat


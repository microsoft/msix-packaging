#pragma once
#include <memory>
#include <string>
#include <vector>

#include "Exceptions.hpp"
#include "StreamBase.hpp"

// Mandatory for using any feature of Xerces.
#include "xercesc/util/PlatformUtils.hpp"
#include "xercesc/dom/DOM.hpp"

namespace xPlat {

    class XmlObject : public StreamBase
    {
    public:
        // TODO: Implement actual XML validation....
        XmlObject(std::shared_ptr<StreamBase>& stream) : m_stream(stream) {}

        // StreamBase interface is largely pass through.
        // XML de-serialization happens during construction, of this object.
        // XML serialization happens through 
        virtual void Write(std::size_t size, const std::uint8_t* bytes)       { throw Exception(Error::NotSupported); }
        virtual std::size_t Read(std::size_t size, const std::uint8_t* bytes) { return m_stream->Read(size, bytes); }
        virtual void Seek(std::uint64_t offset, Reference where)              { m_stream->Seek(offset,where); }
        virtual int Ferror()                                                  { return m_stream->Ferror(); }
        virtual bool Feof()                                                   { return m_stream->Feof(); }
        virtual std::uint64_t Ftell()                                         { return m_stream->Ftell(); }

        // Returns a shared pointer to the DOMDocument representing the contents of this stream
        std::shared_ptr<XERCES_CPP_NAMESPACE::DOMDocument> Document()         { return m_DOMDocument;}

        // TODO: Implement: Writes the contents of the DOM Document to the underlying stream.
        void Write()                                                          { throw Exception(Error::NotImplemented); }

    protected:
        std::shared_ptr<StreamBase> m_stream = nullptr;
        std::shared_ptr<XERCES_CPP_NAMESPACE::DOMImplementation> m_DOMImplementation;
        std::shared_ptr<XERCES_CPP_NAMESPACE::DOMDocument>       m_DOMDocument;
    };

} // namespace xPlat


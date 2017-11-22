#pragma once
#include <memory>
#include <string>
#include <vector>

#include "Exceptions.hpp"
#include "StreamBase.hpp"
#include "VerifierObject.hpp"

// Mandatory for using any feature of Xerces.
#include "xercesc/dom/DOM.hpp"
#include "xercesc/framework/MemBufInputSource.hpp"
#include "xercesc/framework/XMLGrammarPoolImpl.hpp"
#include "xercesc/parsers/XercesDOMParser.hpp"
#include "xercesc/util/PlatformUtils.hpp"
#include "xercesc/util/XMLString.hpp"

// internal interface
EXTERN_C const IID IID_IXmlObject;
#ifndef WIN32
MIDL_INTERFACE("0e7a446e-baf7-44c1-b38a-216bfa18a1a8")
interface IXmlObject : public IUnknown
#else
#include "Unknwn.h"
#include "Objidl.h"
class IXmlObject : public IUnknown
#endif
// An internal interface for XML
{
public:
    #ifdef WIN32
    virtual ~IXmlObject() {}
    #endif

    virtual void Write() = 0;
    virtual XERCES_CPP_NAMESPACE::DOMDocument* Document() = 0;
};

SpecializeUuidOfImpl(IXmlObject);

namespace xPlat {

    // XML de-serialization happens during construction, of this object.
    // XML serialization happens through the Write method
    class XmlObject : public ComClass<XmlObject, IXmlObject, IVerifierObject>
    {
    public:
        XmlObject(IStream* stream, std::map<std::string, std::string>& schemas) :  m_stream(stream) 
        {
            // Create buffer from stream
            LARGE_INTEGER start = { 0 };
            ULARGE_INTEGER end = { 0 };
            ThrowHrIfFailed(stream->Seek(start, StreamBase::Reference::END, &end));
            ThrowHrIfFailed(stream->Seek(start, StreamBase::Reference::START, nullptr));

            std::uint32_t streamSize = end.u.LowPart;
            std::vector<std::uint8_t> buffer(streamSize);
            ULONG actualRead = 0;
            ThrowHrIfFailed(stream->Read(buffer.data(), streamSize, &actualRead));
            ThrowErrorIf(Error::FileRead, (actualRead != streamSize), "read error");

            std::unique_ptr<MemBufInputSource> source = std::make_unique<MemBufInputSource>(
                reinterpret_cast<const XMLByte*>(&buffer[0]), actualRead, "XML File");

            // Create parser and grammar pool
            auto grammarPool = std::make_unique<XMLGrammarPoolImpl>(XMLPlatformUtils::fgMemoryManager);
            m_parser = std::make_unique<XercesDOMParser>(nullptr, XMLPlatformUtils::fgMemoryManager, grammarPool.get());
            
            bool HasSchemas = schemas.begin() != schemas.end();
            m_parser->setValidationScheme(HasSchemas ? XercesDOMParser::Val_Always : XercesDOMParser::Val_None);
            m_parser->cacheGrammarFromParse(HasSchemas);            
            m_parser->setDoSchema(HasSchemas);
            m_parser->setDoNamespaces(HasSchemas);
            m_parser->setHandleMultipleImports(HasSchemas); // TODO: do we need to handle the case where there aren't multiple schemas with the same namespace?
            m_parser->setValidationSchemaFullChecking(HasSchemas);

            // Add schemas
            for (auto index = schemas.begin(); index != schemas.end(); index++)
            {
                auto item = std::make_unique<MemBufInputSource>(
                    reinterpret_cast<const XMLByte*>(index->second->c_str()),
                    index->second->length(),
                    index->first.c_str());
                m_parser->loadGrammar(*item, Grammar::GrammarType::SchemaGrammarType, true);
            }

            // Set the error handler for the parser
            auto errorHandler = std::make_unique<ParsingException>();
            m_parser->setErrorHandler(errorHandler.get());
            m_parser->parse(*source);            
        }

        // IXmlObject
        void Write() override { throw Exception(Error::NotImplemented); }
        XERCES_CPP_NAMESPACE::DOMDocument* Document() override { return parser->getDocument();}

        // IVerifierObject
        bool HasStream() override { return m_stream.Get() != nullptr; }
        IStream* GetStream() override { return m_stream.Get(); }
        IStream* GetValidationStream(const std::string& part, IStream* stream) override
        {
            throw Exception(Error::NotSupported);
        }

    protected:
        std::unique_ptr<XercesDOMParser> m_parser;
        ComPtr<IStream>                  m_stream;
    };

} // namespace xPlat
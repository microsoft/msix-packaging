//
//  Copyright (C) 2017 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
// 
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
#include "xercesc/parsers/AbstractDOMParser.hpp"
#include "xercesc/parsers/XercesDOMParser.hpp"
#include "xercesc/util/PlatformUtils.hpp"
#include "xercesc/util/XMLString.hpp"

// internal interface
EXTERN_C const IID IID_IXmlObject;
#ifndef WIN32
// {0e7a446e-baf7-44c1-b38a-216bfa18a1a8}
interface IXmlObject : public IUnknown
#else
#include "Unknwn.h"
#include "Objidl.h"
class IXmlObject : public IUnknown
#endif
// An internal interface for XML
{
public:
    virtual void Write() = 0;
    virtual XERCES_CPP_NAMESPACE::DOMDocument* Document() = 0;
};

SpecializeUuidOfImpl(IXmlObject);

namespace MSIX {

    // XML de-serialization happens during construction, of this object.
    // XML serialization happens through the Write method
    class XmlObject : public ComClass<XmlObject, IXmlObject, IVerifierObject>
    {
    public:
        XmlObject(ComPtr<IStream>& stream, std::map<std::string, std::string>* schemas = nullptr) :  m_stream(stream)
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

            // move the underlying stream back to the begginning.
            ThrowHrIfFailed(stream->Seek(start, StreamBase::Reference::START, nullptr));

            std::unique_ptr<XERCES_CPP_NAMESPACE::MemBufInputSource> source = std::make_unique<XERCES_CPP_NAMESPACE::MemBufInputSource>(
                reinterpret_cast<const XMLByte*>(&buffer[0]), actualRead, "XML File");

            // Create parser and grammar pool
            auto grammarPool = std::make_unique<XERCES_CPP_NAMESPACE::XMLGrammarPoolImpl>(XERCES_CPP_NAMESPACE::XMLPlatformUtils::fgMemoryManager);
            m_parser = std::make_unique<XERCES_CPP_NAMESPACE::XercesDOMParser>(nullptr, XERCES_CPP_NAMESPACE::XMLPlatformUtils::fgMemoryManager, grammarPool.get());
            
            bool HasSchemas = ((schemas != nullptr) && (schemas->begin() != schemas->end()));
            m_parser->setValidationScheme(HasSchemas ? 
                XERCES_CPP_NAMESPACE::AbstractDOMParser::ValSchemes::Val_Always : 
                XERCES_CPP_NAMESPACE::AbstractDOMParser::ValSchemes::Val_Never
            );
            m_parser->cacheGrammarFromParse(HasSchemas);            
            m_parser->setDoSchema(HasSchemas);
            m_parser->setDoNamespaces(HasSchemas);
            m_parser->setHandleMultipleImports(HasSchemas); // TODO: do we need to handle the case where there aren't multiple schemas with the same namespace?
            m_parser->setValidationSchemaFullChecking(HasSchemas);

            if (HasSchemas)
            {   // Disable DTD and prevent XXE attacks.  See https://www.owasp.org/index.php/XML_External_Entity_(XXE)_Prevention_Cheat_Sheet#libxerces-c for additional details.
                m_parser->setIgnoreCachedDTD(true);
                m_parser->setSkipDTDValidation(true);
                m_parser->setCreateEntityReferenceNodes(false);
            }

            // Add schemas
            if (schemas != nullptr)
            {   for (auto index = schemas->begin(); index != schemas->end(); index++)
                {   auto item = std::make_unique<XERCES_CPP_NAMESPACE::MemBufInputSource>(
                        reinterpret_cast<const XMLByte*>(index->second.c_str()),
                        index->second.length(),
                        index->first.c_str());
                    m_parser->loadGrammar(*item, XERCES_CPP_NAMESPACE::Grammar::GrammarType::SchemaGrammarType, true);
                }                
            }

            // Set the error handler for the parser
            auto errorHandler = std::make_unique<ParsingException>();
            m_parser->setErrorHandler(errorHandler.get());
            m_parser->parse(*source);            
        }

        // IXmlObject
        void Write() override { throw Exception(Error::NotImplemented); }
        XERCES_CPP_NAMESPACE::DOMDocument* Document() override { return m_parser->getDocument();}

        // IVerifierObject
        const std::string& GetPublisher() override { throw Exception(Error::NotSupported); }
        bool HasStream() override { return m_stream.Get() != nullptr; }
        MSIX::ComPtr<IStream> GetStream() override { return m_stream; }
        MSIX::ComPtr<IStream> GetValidationStream(const std::string& part, IStream* stream) override
        {
            throw Exception(Error::NotSupported);
        }

    protected:
        std::unique_ptr<XERCES_CPP_NAMESPACE::XercesDOMParser> m_parser;
        ComPtr<IStream> m_stream;
    };

} // namespace MSIX
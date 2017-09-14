/*
 * Copyright (c) 2001, 2008,
 *     DecisionSoft Limited. All rights reserved.
 * Copyright (c) 2004, 2015 Oracle and/or its affiliates. All rights reserved.
 *     
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef _SCHEMAVALIDATORFILTER_HPP
#define _SCHEMAVALIDATORFILTER_HPP

#include <vector>

#include <xqilla/events/EventHandler.hpp>

#include <xercesc/framework/XMLDocumentHandler.hpp>
#include <xercesc/framework/XMLErrorReporter.hpp>
#include <xercesc/validators/schema/SchemaValidator.hpp>
#include <xercesc/validators/schema/identity/IdentityConstraintHandler.hpp>
#include <xercesc/internal/XMLScanner.hpp>
#include <xercesc/util/RefHash3KeysIdPool.hpp>

class DynamicContext;
class LocationInfo;

#if _XERCES_VERSION < 30000
typedef XMLSSize_t XMLFileLoc;
#endif

/**
 * The SchemaValidatorFilter class will modify the incoming event stream by
 * validating it against the schema definitions found in the DynamicContext's
 * DocumentCache. It will add typing information, as well as default values
 * and attributes, and will normalize values according to the schema normalization
 * rules.
 */
class XQILLA_API SchemaValidatorFilter : public EventFilter,
                                         public XERCES_CPP_NAMESPACE_QUALIFIER XMLScanner,
                                         private XERCES_CPP_NAMESPACE_QUALIFIER XMLDocumentHandler,
                                         private XERCES_CPP_NAMESPACE_QUALIFIER XMLErrorReporter
{
public:
  SchemaValidatorFilter(bool strictValidation, EventHandler *next, XERCES_CPP_NAMESPACE_QUALIFIER GrammarResolver *grammarResolver,
                        XERCES_CPP_NAMESPACE_QUALIFIER MemoryManager *mm, const LocationInfo *info);
  virtual ~SchemaValidatorFilter();

  void reset();

  virtual void startDocumentEvent(const XMLCh *documentURI, const XMLCh *encoding);
  virtual void endDocumentEvent();
  virtual void startElementEvent(const XMLCh *prefix, const XMLCh *uri, const XMLCh *localname);
  virtual void endElementEvent(const XMLCh *prefix, const XMLCh *uri, const XMLCh *localname,
                               const XMLCh *typeURI, const XMLCh *typeName);
  virtual void piEvent(const XMLCh *target, const XMLCh *value);
  virtual void textEvent(const XMLCh *value);
  virtual void textEvent(const XMLCh *chars, unsigned int length);
  virtual void commentEvent(const XMLCh *value);
  virtual void attributeEvent(const XMLCh *prefix, const XMLCh *uri, const XMLCh *localname, const XMLCh *value,
                              const XMLCh *typeURI, const XMLCh *typeName);
  virtual void namespaceEvent(const XMLCh *prefix, const XMLCh *uri);

private:
  // XMLScanner
  virtual const XMLCh *getName() const { return 0; }
  virtual XERCES_CPP_NAMESPACE_QUALIFIER NameIdPool<XERCES_CPP_NAMESPACE_QUALIFIER DTDEntityDecl> *getEntityDeclPool() { return 0; }
  virtual const XERCES_CPP_NAMESPACE_QUALIFIER NameIdPool<XERCES_CPP_NAMESPACE_QUALIFIER DTDEntityDecl> *getEntityDeclPool() const { return 0; }
#if _XERCES_VERSION < 30100
  virtual unsigned int resolveQName(const XMLCh *const qName, XERCES_CPP_NAMESPACE_QUALIFIER XMLBuffer &prefixBufToFill,
                                    const short mode, int &prefixColonPos);
#endif
  virtual void scanDocument(const XERCES_CPP_NAMESPACE_QUALIFIER InputSource &src) {}
  virtual bool scanNext(XERCES_CPP_NAMESPACE_QUALIFIER XMLPScanToken &toFill) { return false; }
  virtual XERCES_CPP_NAMESPACE_QUALIFIER Grammar *loadGrammar(const XERCES_CPP_NAMESPACE_QUALIFIER InputSource &src,
                                                              const short grammarType, const bool toCache) { return 0; }

  virtual void scanCDSection() {}
  virtual void scanCharData(XERCES_CPP_NAMESPACE_QUALIFIER XMLBuffer &toToUse) {}
  virtual EntityExpRes scanEntityRef(const bool inAttVal, XMLCh &firstCh, XMLCh &secondCh,
                                     bool &escaped) { return EntityExp_Returned; }
  virtual void scanDocTypeDecl() {}
  virtual void scanReset(const XERCES_CPP_NAMESPACE_QUALIFIER InputSource &src) {}
  virtual void sendCharData(XERCES_CPP_NAMESPACE_QUALIFIER XMLBuffer &toSend) {}

  // XMLDocumentHandler
  virtual void startDocument() {}
  virtual void endDocument() {}
  virtual void resetDocument() {}
#if _XERCES_VERSION >= 30000
  virtual void startElement(const XERCES_CPP_NAMESPACE_QUALIFIER XMLElementDecl& elemDecl, const unsigned int urlId,
                            const XMLCh* const elemPrefix,
                            const XERCES_CPP_NAMESPACE_QUALIFIER RefVectorOf<XERCES_CPP_NAMESPACE_QUALIFIER XMLAttr>& attrList,
                            const XMLSize_t attrCount, const bool isEmpty, const bool isRoot) {}
  virtual void docCharacters(const XMLCh* const chars, const XMLSize_t length, const bool cdataSection);
  virtual void ignorableWhitespace(const XMLCh* const chars, const XMLSize_t length, const bool cdataSection) {}
#else
  virtual void startElement(const XERCES_CPP_NAMESPACE_QUALIFIER XMLElementDecl& elemDecl, const unsigned int urlId,
                            const XMLCh* const elemPrefix,
                            const XERCES_CPP_NAMESPACE_QUALIFIER RefVectorOf<XERCES_CPP_NAMESPACE_QUALIFIER XMLAttr>& attrList,
                            const unsigned int attrCount, const bool isEmpty, const bool isRoot) {}
  virtual void docCharacters(const XMLCh* const chars, const unsigned int length, const bool cdataSection);
  virtual void ignorableWhitespace(const XMLCh* const chars, const unsigned int length, const bool cdataSection) {}
#endif
  virtual void endElement(const XERCES_CPP_NAMESPACE_QUALIFIER XMLElementDecl& elemDecl, const unsigned int urlId,
                          const bool isRoot, const XMLCh* const elemPrefix=0) {}
  virtual void docComment(const XMLCh* const comment) {}
  virtual void docPI(const XMLCh* const target, const XMLCh* const data) {}
  virtual void startEntityReference(const XERCES_CPP_NAMESPACE_QUALIFIER XMLEntityDecl& entDecl) {}
  virtual void endEntityReference(const XERCES_CPP_NAMESPACE_QUALIFIER XMLEntityDecl& entDecl) {}
  virtual void XMLDecl(const XMLCh* const versionStr, const XMLCh* const encodingStr, const XMLCh* const standaloneStr,
                       const XMLCh* const actualEncodingStr) {}

  // XMLErrorReporter
  virtual void resetErrors() {}
  virtual void error(const unsigned int errCode, const XMLCh* const errDomain, const XMLErrorReporter::ErrTypes type, const XMLCh* const errorText,
                     const XMLCh* const systemId, const XMLCh* const publicId, const XMLFileLoc lineNum, const XMLFileLoc colNum);

private:
  void processStartElement();
  void processAttrs(XERCES_CPP_NAMESPACE_QUALIFIER XMLElementDecl *elemDecl);

  bool switchGrammar(const XMLCh *uri);
  bool laxElementValidation(XERCES_CPP_NAMESPACE_QUALIFIER QName* element,
                            XERCES_CPP_NAMESPACE_QUALIFIER ContentLeafNameTypeVector* cv,
                            const XERCES_CPP_NAMESPACE_QUALIFIER XMLContentModel *cm,
                            unsigned int parentElemDepth);
  bool anyAttributeValidation(XERCES_CPP_NAMESPACE_QUALIFIER SchemaAttDef* attWildCard, unsigned int uriId, bool& skipThisOne, bool& laxThisOne);
  XERCES_CPP_NAMESPACE_QUALIFIER XMLElementDecl *createElementDecl(unsigned int uriId, unsigned int currentScope,
                                                                   bool laxThisOne);

  const LocationInfo *info_;

  XERCES_CPP_NAMESPACE_QUALIFIER SchemaValidator *fSchemaValidator;
  XERCES_CPP_NAMESPACE_QUALIFIER SchemaGrammar *fSchemaGrammar;
  XERCES_CPP_NAMESPACE_QUALIFIER XMLBuffer fContent;
  XERCES_CPP_NAMESPACE_QUALIFIER IdentityConstraintHandler *fICHandler;
  XERCES_CPP_NAMESPACE_QUALIFIER RefHash3KeysIdPool<XERCES_CPP_NAMESPACE_QUALIFIER SchemaElementDecl> *fElemNonDeclPool;

  bool strictValidation_;
  bool errorOccurred_;

  XERCES_CPP_NAMESPACE_QUALIFIER ElemStack::StackElem *parentStack_;
  unsigned int elemDepth_;
  XERCES_CPP_NAMESPACE_QUALIFIER XMLBuffer prefix_;
  XERCES_CPP_NAMESPACE_QUALIFIER XMLBuffer uri_;
  XERCES_CPP_NAMESPACE_QUALIFIER XMLBuffer localname_;
  bool elementToProcess_;
  const XMLCh *xsiType_;
  unsigned int attrCount_;
};

#endif

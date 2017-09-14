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

#ifndef _DOCUMENTCACHEIMPL_HPP
#define _DOCUMENTCACHEIMPL_HPP

#include <xqilla/framework/XQillaExport.hpp>
#include <xqilla/schema/DocumentCache.hpp>

#include <xercesc/framework/XMLDocumentHandler.hpp>
#include <xercesc/framework/XMLEntityHandler.hpp>
#include <xercesc/framework/XMLErrorReporter.hpp>
#include <xercesc/framework/psvi/PSVIHandler.hpp>

class DynamicContext;
class QualifiedName;

XERCES_CPP_NAMESPACE_BEGIN
class XMLGrammarPool;
class GrammarResolver;
class XMLScanner;
XERCES_CPP_NAMESPACE_END

#if _XERCES_VERSION < 30000
typedef XMLSSize_t XMLFileLoc;
#endif

/// The class that performs the parsing of input documents
class XQILLA_API DocumentCacheImpl : public DocumentCache,
                                     private XERCES_CPP_NAMESPACE_QUALIFIER XMLDocumentHandler,
                                     private XERCES_CPP_NAMESPACE_QUALIFIER PSVIHandler,
                                     private XERCES_CPP_NAMESPACE_QUALIFIER XMLEntityHandler,
                                     private XERCES_CPP_NAMESPACE_QUALIFIER XMLErrorReporter
{
public:
  DocumentCacheImpl(XERCES_CPP_NAMESPACE_QUALIFIER MemoryManager* memMgr, XERCES_CPP_NAMESPACE_QUALIFIER XMLGrammarPool* xmlgr = 0);
  DocumentCacheImpl(const DocumentCacheImpl *parent, XERCES_CPP_NAMESPACE_QUALIFIER MemoryManager* memMgr);
  ~DocumentCacheImpl();

  /**
   * Sets the XMLEntityResolver that is used by Xerces when it is used
   * to parse documents. This affects the behaviour of XQilla whenever
   * it retrieves an XML, DTD or XML Schema grammar.
   */
  virtual void setXMLEntityResolver(XERCES_CPP_NAMESPACE_QUALIFIER XMLEntityResolver* const handler);
  virtual XERCES_CPP_NAMESPACE_QUALIFIER XMLEntityResolver* getXMLEntityResolver() const;

  virtual bool getDoPSVI() const { return doPSVI_; }
  virtual void setDoPSVI(bool value) { doPSVI_ = value; }

  virtual Node::Ptr loadDocument(const XMLCh* uri, DynamicContext *context, const QueryPathNode *projection = 0);
  virtual Node::Ptr parseDocument(XERCES_CPP_NAMESPACE_QUALIFIER InputSource &srcToUse, DynamicContext *context, const QueryPathNode *projection = 0);
  virtual void parseDocument(XERCES_CPP_NAMESPACE_QUALIFIER InputSource &srcToUse, EventHandler *handler, DynamicContext *context);

  /*
   * returns true if the type represented by uri:typename is an instance of uriToCheck:typeNameToCheck 
   *
   * ie: to check 
   * xs:integer instance of xs:decimal, 
   * call
   * isTypeOrDerivedFromType("xs", "integer", "xs", "decimal") 
   * (except of course, call with URIs, not prefixes!)
   */ 
  virtual bool isTypeOrDerivedFromType(const XMLCh* const uri, const XMLCh* const typeName, 
                                       const XMLCh* const uriToCheck, const XMLCh* const typeNameToCheck) const; 
  virtual bool isTypeDefined(const XMLCh* const uri, const XMLCh* const typeName) const; 

  virtual void addSchemaLocation(const XMLCh* uri, VectorOfStrings* locations, StaticContext *context, const LocationInfo *location);

  /** helper functions used to map namespace ids found in the SchemaGrammar **/
  virtual unsigned int getSchemaUriId(const XMLCh* uri) const;
  virtual const XMLCh* getSchemaUri(unsigned int id) const;

  virtual XERCES_CPP_NAMESPACE_QUALIFIER GrammarResolver *getGrammarResolver() const;

  virtual XERCES_CPP_NAMESPACE_QUALIFIER DatatypeValidator*  getDatatypeValidator(const XMLCh* uri, const XMLCh* typeName) const;

  virtual XERCES_CPP_NAMESPACE_QUALIFIER ComplexTypeInfo*  getComplexTypeInfo(const XMLCh* uri, const XMLCh* typeName) const;

  virtual XERCES_CPP_NAMESPACE_QUALIFIER SchemaElementDecl* getElementDecl(const XMLCh* elementUri, const XMLCh* elementName) const;

  virtual XERCES_CPP_NAMESPACE_QUALIFIER SchemaAttDef* getAttributeDecl(const XMLCh* attributeUri, const XMLCh* attributeName) const;

  virtual DocumentCache *createDerivedCache(XERCES_CPP_NAMESPACE_QUALIFIER MemoryManager *memMgr) const;

  static XERCES_CPP_NAMESPACE_QUALIFIER InputSource *resolveURI(const XMLCh *uri, const XMLCh *baseUri);

protected:
  void init(XERCES_CPP_NAMESPACE_QUALIFIER XMLGrammarPool *gramPool = 0);

  void loadSchema(const XMLCh* const uri, const XMLCh* location, StaticContext *context, const LocationInfo *info);

  // XMLEntityHandler
  virtual void endInputSource(const XERCES_CPP_NAMESPACE_QUALIFIER InputSource& inputSource);
  virtual bool expandSystemId(const XMLCh* const systemId, XERCES_CPP_NAMESPACE_QUALIFIER XMLBuffer &toFill);
  virtual void resetEntities();
  virtual XERCES_CPP_NAMESPACE_QUALIFIER InputSource* resolveEntity(const XMLCh* const publicId, const XMLCh* const systemId, const XMLCh* const baseURI = 0);
  virtual XERCES_CPP_NAMESPACE_QUALIFIER InputSource* resolveEntity(XERCES_CPP_NAMESPACE_QUALIFIER XMLResourceIdentifier* resourceIdentifier);
  virtual void startInputSource(const XERCES_CPP_NAMESPACE_QUALIFIER InputSource& inputSource);

  // XMLErrorReporter
  virtual void resetErrors();
  virtual void error(const unsigned int errCode, const XMLCh* const errDomain, const XMLErrorReporter::ErrTypes type, const XMLCh* const errorText,
                     const XMLCh* const systemId, const XMLCh* const publicId, const XMLFileLoc lineNum, const XMLFileLoc colNum);

  // XMLDocumentHandler
  virtual void startDocument();
  virtual void endDocument();
  virtual void resetDocument();
#if _XERCES_VERSION >= 30000
  virtual void startElement(const XERCES_CPP_NAMESPACE_QUALIFIER XMLElementDecl& elemDecl, const unsigned int urlId,
                            const XMLCh* const elemPrefix,
                            const XERCES_CPP_NAMESPACE_QUALIFIER RefVectorOf<XERCES_CPP_NAMESPACE_QUALIFIER XMLAttr>& attrList,
                            const XMLSize_t attrCount, const bool isEmpty, const bool isRoot);
  virtual void docCharacters(const XMLCh* const chars, const XMLSize_t length, const bool cdataSection);
  virtual void ignorableWhitespace(const XMLCh* const chars, const XMLSize_t length, const bool cdataSection);
#else
  virtual void startElement(const XERCES_CPP_NAMESPACE_QUALIFIER XMLElementDecl& elemDecl, const unsigned int urlId,
                            const XMLCh* const elemPrefix,
                            const XERCES_CPP_NAMESPACE_QUALIFIER RefVectorOf<XERCES_CPP_NAMESPACE_QUALIFIER XMLAttr>& attrList,
                            const unsigned int attrCount, const bool isEmpty, const bool isRoot);
  virtual void docCharacters(const XMLCh* const chars, const unsigned int length, const bool cdataSection);
  virtual void ignorableWhitespace(const XMLCh* const chars, const unsigned int length, const bool cdataSection);
#endif
  virtual void endElement(const XERCES_CPP_NAMESPACE_QUALIFIER XMLElementDecl& elemDecl, const unsigned int urlId,
                          const bool isRoot, const XMLCh* const elemPrefix=0);
  virtual void docComment(const XMLCh* const comment);
  virtual void docPI(const XMLCh* const target, const XMLCh* const data);
  virtual void startEntityReference(const XERCES_CPP_NAMESPACE_QUALIFIER XMLEntityDecl& entDecl);
  virtual void endEntityReference(const XERCES_CPP_NAMESPACE_QUALIFIER XMLEntityDecl& entDecl);
  virtual void XMLDecl(const XMLCh* const versionStr, const XMLCh* const encodingStr, const XMLCh* const standaloneStr,
                       const XMLCh* const actualEncodingStr);

  // PSVIHandler
  virtual void handleElementPSVI(const XMLCh* const localName, const XMLCh* const uri,
                                 XERCES_CPP_NAMESPACE_QUALIFIER PSVIElement *elementInfo);
  virtual void handlePartialElementPSVI(const XMLCh* const localName, const XMLCh* const uri,
                                        XERCES_CPP_NAMESPACE_QUALIFIER PSVIElement *elementInfo);
  virtual void handleAttributesPSVI(const XMLCh* const localName, const XMLCh* const uri,
                                    XERCES_CPP_NAMESPACE_QUALIFIER PSVIAttributeList *psviAttributes);

protected:
  XERCES_CPP_NAMESPACE_QUALIFIER XMLScanner *getScanner();

  XERCES_CPP_NAMESPACE_QUALIFIER GrammarResolver *grammarResolver_;
  XERCES_CPP_NAMESPACE_QUALIFIER XMLScanner *scanner_;
  XERCES_CPP_NAMESPACE_QUALIFIER XMLEntityResolver *entityResolver_;
  bool doPSVI_;

  LocationInfo location_;
  EventHandler *events_;

  const XERCES_CPP_NAMESPACE_QUALIFIER RefVectorOf<XERCES_CPP_NAMESPACE_QUALIFIER XMLAttr> *attrList_;
  unsigned int attrCount_;
  XERCES_CPP_NAMESPACE_QUALIFIER PSVIElement *elementInfo_;
  bool strictValidation_;

  XERCES_CPP_NAMESPACE_QUALIFIER XMLStringPool* loadedSchemas_;
  XERCES_CPP_NAMESPACE_QUALIFIER XMLBuffer schemaLocations_;
  XERCES_CPP_NAMESPACE_QUALIFIER XMLBuffer noNamespaceSchemaLocation_;
  XERCES_CPP_NAMESPACE_QUALIFIER MemoryManager* memMgr_;
};

#endif

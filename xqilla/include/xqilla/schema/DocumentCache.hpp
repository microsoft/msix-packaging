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

#ifndef _DOCUMENTCACHE_HPP
#define _DOCUMENTCACHE_HPP

#include <vector>

#include <xqilla/framework/XPath2MemoryManager.hpp>
#include <xqilla/items/Node.hpp>

#include <xercesc/util/XercesDefs.hpp>
#include <xercesc/util/XMemory.hpp>

class DynamicContext;
class StaticContext;
class QualifiedName;
class LocationInfo;
class QueryPathNode;

typedef std::vector<const XMLCh*, XQillaAllocator<const XMLCh*> > VectorOfStrings;

XERCES_CPP_NAMESPACE_BEGIN
class DOMDocument;
class DOMNode;
class DatatypeValidator;
class XMLEntityResolver;
class ComplexTypeInfo;
class SchemaElementDecl;
class SchemaAttDef;
class InputSource;
class GrammarResolver;
XERCES_CPP_NAMESPACE_END

///Expression Context is a storage for contexts.
class XQILLA_API DocumentCache : public XERCES_CPP_NAMESPACE_QUALIFIER XMemory
{
public:
  /// untyped
  static const XMLCh g_szUntyped[];

  enum ValidationMode {
    VALIDATION_STRICT = 0,
    VALIDATION_LAX    = 1,
    VALIDATION_SKIP   = 2
  };

  /** virtual destructor */
  virtual ~DocumentCache() {}

  /** load the document from the requested URI */
  virtual Node::Ptr loadDocument(const XMLCh* uri, DynamicContext *context, const QueryPathNode *projection = 0) = 0;
  /** parse the document from the InputSource provided */
  virtual Node::Ptr parseDocument(XERCES_CPP_NAMESPACE_QUALIFIER InputSource &srcToUse, DynamicContext *context, const QueryPathNode *projection = 0) = 0;
  /** parse the document from the InputSource provided, outputing it to the EventHandler provided */
  virtual void parseDocument(XERCES_CPP_NAMESPACE_QUALIFIER InputSource &srcToUse, EventHandler *handler, DynamicContext *context) = 0;

  /**
   * Sets the XMLEntityResolver that is used by Xerces when it is used
   * to parse documents. This affects the behaviour of XQilla whenever
   * it retrieves an XML, DTD or XML Schema grammar.
   */
  virtual void setXMLEntityResolver(XERCES_CPP_NAMESPACE_QUALIFIER XMLEntityResolver* const handler) = 0;
  virtual XERCES_CPP_NAMESPACE_QUALIFIER XMLEntityResolver* getXMLEntityResolver() const = 0;

  virtual bool getDoPSVI() const = 0;
  virtual void setDoPSVI(bool value) = 0;

  /*
   * returns true if the type represented by uri:typename is an instance of uriToCheck:typeNameToCheck 
   *
   * ie: to check 
   * xs:integer instance of xs:decimal, 
   * call
   * isTypeOrDerivedFromType("xs", "integer", "xs", "decimal") 
   * (except of course, call with URIs, not prefixes!)
   */ 
  virtual bool isTypeOrDerivedFromType(const XMLCh* const uri, const XMLCh* const typeName, const XMLCh* const uriToCheck, const XMLCh* const typeNameToCheck) const = 0; 
  virtual bool isTypeDefined(const XMLCh* const uri, const XMLCh* const typeName) const = 0; 

  virtual void addSchemaLocation(const XMLCh* uri, VectorOfStrings* locations, StaticContext *context, const LocationInfo *location) = 0;

  /** helper functions used to map namespace ids found in the SchemaGrammar **/
  virtual unsigned int getSchemaUriId(const XMLCh* uri) const = 0;
  virtual const XMLCh* getSchemaUri(unsigned int id) const = 0;

  virtual XERCES_CPP_NAMESPACE_QUALIFIER GrammarResolver *getGrammarResolver() const = 0;

  virtual XERCES_CPP_NAMESPACE_QUALIFIER DatatypeValidator* getDatatypeValidator(const XMLCh* uri, const XMLCh* typeName) const = 0;

  virtual XERCES_CPP_NAMESPACE_QUALIFIER ComplexTypeInfo*  getComplexTypeInfo(const XMLCh* uri, const XMLCh* typeName) const = 0;

  virtual XERCES_CPP_NAMESPACE_QUALIFIER SchemaElementDecl* getElementDecl(const XMLCh* elementUri, const XMLCh* elementName) const = 0;

  virtual XERCES_CPP_NAMESPACE_QUALIFIER SchemaAttDef* getAttributeDecl(const XMLCh* attributeUri, const XMLCh* attributeName) const = 0;

  virtual DocumentCache *createDerivedCache(XERCES_CPP_NAMESPACE_QUALIFIER MemoryManager *memMgr) const = 0;
};

#endif








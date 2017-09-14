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

#ifndef _STATICCONTEXT_HPP
#define _STATICCONTEXT_HPP

#include <xqilla/framework/XQillaExport.hpp>
#include <xqilla/simple-api/XQilla.hpp>
#include <time.h>
#include <xqilla/schema/DocumentCache.hpp>
#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/util/XMemory.hpp>

class DynamicContext;
class XPath2MemoryManager;
class FuncFactory;
class DatatypeFactory;
class Collation;
class VariableTypeStore;
class ItemFactory;
class StaticType;
class ModuleResolver;
class ExternalFunctionResolver;
class MessageListener;
class ExternalFunction;
class XQillaConfiguration;
class XQUserFunction;
class XQillaNSResolver;

// Copied from ASTNode.hpp
typedef std::vector<ASTNode*,XQillaAllocator<ASTNode*> > VectorOfASTNodes;
typedef std::vector<XQUserFunction*, XQillaAllocator<XQUserFunction*> > UserFunctions;

XERCES_CPP_NAMESPACE_BEGIN
class DOMDocument;
class DOMXPathNSResolver;
class XMLEntityResolver;
XERCES_CPP_NAMESPACE_END

/// The parse time static context interface
class XQILLA_API StaticContext : public XERCES_CPP_NAMESPACE_QUALIFIER XMemory
{
public:
  typedef enum {
      ORDERING_ORDERED,
      ORDERING_UNORDERED
  } NodeSetOrdering;

  typedef enum {
    CONSTRUCTION_MODE_PRESERVE,
    CONSTRUCTION_MODE_STRIP
  } ConstructionMode;

  typedef enum {
    FLWOR_ORDER_EMPTY_GREATEST,
    FLWOR_ORDER_EMPTY_LEAST
  } FLWOROrderingMode;

  typedef enum {
      NONE,
      CASE_INSENSITIVE,
      CASE_SENSITIVE,
      UPPERCASE,
      LOWERCASE
  } FTCaseOption;

  virtual ~StaticContext() {};

  virtual DynamicContext *createModuleContext(XERCES_CPP_NAMESPACE_QUALIFIER MemoryManager *memMgr =
                                              XERCES_CPP_NAMESPACE_QUALIFIER XMLPlatformUtils::fgMemoryManager) const = 0;
  virtual DynamicContext *createDynamicContext(XERCES_CPP_NAMESPACE_QUALIFIER MemoryManager *memMgr =
                                               XERCES_CPP_NAMESPACE_QUALIFIER XMLPlatformUtils::fgMemoryManager) const = 0;

  virtual DynamicContext *createDebugQueryContext(const Item::Ptr &contextItem,
                                                  size_t contextPosition,
                                                  size_t contextSize,
                                                  const VariableStore *variables,
                                                  const XERCES_CPP_NAMESPACE_QUALIFIER DOMXPathNSResolver *nsResolver,
                                                  const XMLCh *defaultElementNS,
                                                  XERCES_CPP_NAMESPACE_QUALIFIER MemoryManager *memMgr =
                                                  XERCES_CPP_NAMESPACE_QUALIFIER XMLPlatformUtils::fgMemoryManager) const = 0;

  /** Returns the configuration object for this context */
  virtual XQillaConfiguration *getConfiguration() const = 0;

  /** Returns the lanuage that this context is for. This value cannot be set, since it is used
      to correctly construct the StaticContext */
  virtual XQilla::Language getLanguage() const = 0;
  
  /** Returns the ItemFactory for this context, which is used to create XQilla items and other objects. */
  virtual ItemFactory *getItemFactory() const = 0;
  /** Sets the ItemFactory for this context */
  virtual void setItemFactory(ItemFactory *factory) = 0;

  /** Get the static type of the context item */
  virtual const StaticType &getContextItemType() const = 0;
  /** Set the static type of the context item */
  virtual void setContextItemType(const StaticType &st) = 0;

  /** Get the current XPath 1.0 compatibility mode */
  virtual bool getXPath1CompatibilityMode() const = 0;
  /** Set the current XPath 1.0 compatibility mode */
  virtual void setXPath1CompatibilityMode(bool newMode) = 0;

  /** Get the NS resolver */
  virtual const XERCES_CPP_NAMESPACE_QUALIFIER DOMXPathNSResolver* getNSResolver() const = 0;
  /** returns the URI that is bound in prefix in the current scope or zero
      length string otherwise */
  virtual const XMLCh* getUriBoundToPrefix(const XMLCh* prefix, const LocationInfo *location = 0) const = 0;
  /** returns the prefix that is bound in uri in the current scope or zero
      length string otherwise */
  virtual const XMLCh* getPrefixBoundToUri(const XMLCh* uri) const = 0;
  /** Set the NS resolver */
  virtual void setNSResolver(const XERCES_CPP_NAMESPACE_QUALIFIER DOMXPathNSResolver* resolver) = 0;
  /** Binds a prefix to a namespace URI */
  virtual void setNamespaceBinding(const XMLCh* prefix, const XMLCh* uri) = 0;

  /** get the value of the default namespace for elements and types */
  virtual const XMLCh* getDefaultElementAndTypeNS() const = 0;
  /** set the value of the default namespace for elements and types */
  virtual void setDefaultElementAndTypeNS(const XMLCh* newNS) = 0;

  /** Return the default namespace for functions */
  virtual const XMLCh* getDefaultFuncNS() const = 0;
  /** Set the default namespace for functions */
  virtual void setDefaultFuncNS(const XMLCh* newNS) = 0;

  /** retrieve the repository for the grammars **/
  virtual DocumentCache* getDocumentCache() const = 0;
  /** sets the repository for the grammars **/
  virtual void setDocumentCache(DocumentCache* docCache) = 0;

  /** returns true if the type represented by uri:typename is an instance of uriToCheck:typeNameToCheck 
   *
   * ie: to check 
   * xs:integer instance of xs:decimal, 
   * call
   * isTypeOrDerivedFromType("xs", "integer", "xs", "decimal") 
   * (except of course, call with URIs, not prefixes!)
   */
  virtual bool isTypeOrDerivedFromType(const XMLCh* const uri, const XMLCh* const typeName, const XMLCh* const uriToCheck, const XMLCh* const typeNameToCheck) const = 0;
  /**
   * Sets the XMLEntityResolver that is used by Xerces when it is used
   * to parse documents. This affects the behaviour of XQilla whenever
   * it retrieves a DTD or XML Schema grammar.
   */
  virtual void setXMLEntityResolver(XERCES_CPP_NAMESPACE_QUALIFIER XMLEntityResolver* const handler) = 0;
  /** Returns the entity resolver currently set */
  virtual XERCES_CPP_NAMESPACE_QUALIFIER XMLEntityResolver* getXMLEntityResolver() const = 0;

  /** Register a callback object for resolving module URIs */
  virtual void setModuleResolver(ModuleResolver *resolver) = 0;
  /** Returns the module resolver currently set */
  virtual ModuleResolver *getModuleResolver() const = 0;
  /* Retrieve a list of locations for the given module URI */
  virtual VectorOfStrings* resolveModuleURI(const XMLCh* uri) const = 0;

  /** add the location for the grammar of a specific namespace **/
  virtual void addSchemaLocation(const XMLCh* uri, VectorOfStrings* locations, const LocationInfo *location = 0) = 0;

  /** get the variable type store */
  virtual VariableTypeStore* getVariableTypeStore() = 0;

  /** adds a template definition to the template tables */
  virtual void addTemplate(XQUserFunction *tp) = 0;
  /** look up a template definition by name */
  virtual const XQUserFunction *lookUpNamedTemplate(const XMLCh *uri, const XMLCh *name) const = 0;
  /** Return a vector of all the templates with patterns */
  virtual const UserFunctions &getTemplateRules() const = 0;

  /** adds a custom function to the function table */
  virtual void addCustomFunction(FuncFactory *func) = 0;
  /** removes a custom function to the function table */
  virtual void removeCustomFunction(FuncFactory *func) = 0;
  /** returns a function object with the given uri, localname and number of arguments triple */
  virtual ASTNode *lookUpFunction(const XMLCh *uri, const XMLCh *name, const VectorOfASTNodes &v, const LocationInfo *location = 0) const = 0;

  /** Register a callback object for resolving external function implementations */
  virtual void setExternalFunctionResolver(ExternalFunctionResolver *resolver) = 0;
  /** Returns the external function resolver currently set */
  virtual ExternalFunctionResolver *getExternalFunctionResolver() const = 0;
  /** adds an external function implementation to the function table */
  virtual void addExternalFunction(const ExternalFunction *func) = 0;
  /** returns an external function implementation for the given uri and localname */
  virtual const ExternalFunction *lookUpExternalFunction(const XMLCh *uri, const XMLCh *name, size_t numArgs) const = 0;

  /** Get the implementation for the specified collation */
  virtual Collation* getCollation(const XMLCh* const URI, const LocationInfo *location = 0) const = 0;
  /** Add a collation */
  virtual void addCollation(Collation* collation) = 0;
  
  /** Get the default collation */
  virtual Collation* getDefaultCollation(const LocationInfo *location = 0) const = 0;
  /** Specify which collation is the default one */
  virtual void setDefaultCollation(const XMLCh* const URI) = 0;

  /** Return the base URI */
  virtual const XMLCh* getBaseURI() const = 0;
  /** Set the base URI */
  virtual void setBaseURI(const XMLCh* newURI) = 0;

  /** Return the ordering method for node sets */
  virtual NodeSetOrdering getNodeSetOrdering() const = 0;
  /** Set the ordering method for node sets */
  virtual void setNodeSetOrdering(NodeSetOrdering newOrder) = 0;

  /** Return the construction mode */
  virtual ConstructionMode getConstructionMode() const = 0;
  /** Set the construction mode */
  virtual void setConstructionMode(ConstructionMode newMode) = 0;

  /** Set the policy for boundary space */
  virtual void setPreserveBoundarySpace(bool value) = 0;
  /** Get the policy for boundary  space */
  virtual bool getPreserveBoundarySpace() const = 0;

  /** Return the default ordering mode for FLWOR blocks */
  virtual FLWOROrderingMode getDefaultFLWOROrderingMode() const = 0;
  /** Set the default ordering mode for FLWOR blocks */
  virtual void setDefaultFLWOROrderingMode(FLWOROrderingMode newMode) = 0;

  /** Set the policy for namespace inheritance */
  virtual void setInheritNamespaces(bool value) = 0;
  /** Get the policy for namespace inheritance */
  virtual bool getInheritNamespaces() const = 0;

  /** Set the policy for namespace copy */
  virtual void setPreserveNamespaces(bool value) = 0;
  /** Get the policy for namespace copy */
  virtual bool getPreserveNamespaces() const = 0;

  /** Set the revalidation mode */
  virtual void setRevalidationMode(DocumentCache::ValidationMode mode) = 0;
  /** Get the revalidation mode */
  virtual DocumentCache::ValidationMode getRevalidationMode() const = 0;

  /** Set the listener for warning and trace messages */
  virtual void setMessageListener(MessageListener *listener) = 0;
  /** Gets the listener for warning and trace messages */
  virtual MessageListener *getMessageListener() const = 0;

  /** Set the module */
  virtual void setModule(XQQuery *module) = 0;
  /** Get the module */
  virtual XQQuery *getModule() const = 0;

  /////////////////////////////////////////
  //  XQilla context specific accessors  //
  /////////////////////////////////////////

  /**
   * Gets whether document projection is enabled.
   *
   * Document projection is an optimisation that uses query analysis
   * to remove sub-trees from documents that aren't needed by the
   * query. This results in substantially smaller memory usage and
   * faster query times, but means that any subsequent querying on the
   * results of an initial query can be wrong.
   *
   * If you are hoping to perform further queries on the results of a
   * query, you should turn document projection off to avoid incorrect
   * results.
   */
  virtual bool getProjection() const = 0;
  /**
   * Sets whether document projection is enabled. Document projection
   * can be enabled or disabled at compile time, as well as overridden
   * at runtime. By default document projection is enabled unless query
   * complilation is unable to guarentee its correctness.
   *
   * Document projection is an optimisation that uses query analysis
   * to remove sub-trees from documents that aren't needed by the
   * query. This results in substantially smaller memory usage and
   * faster query times, but means that any subsequent querying on the
   * results of an initial query can be wrong.
   *
   * If you are hoping to perform further queries on the results of a
   * query, you should turn document projection off to avoid incorrect
   * results.
   */
  virtual void setProjection(bool enabled) = 0;

  virtual bool getDoLintWarnings() const = 0;
  virtual void setDoLintWarnings(bool enabled) = 0;

  /** Return a unique name that can be used for a temporary variable */
  virtual const XMLCh *allocateTempVarName(const XMLCh *prefix = 0) = 0;

  /** Get the memory manager */
  virtual XPath2MemoryManager* getMemoryManager() const = 0;
  /** Set the memory manager to the one given */
  virtual void setMemoryManager(XPath2MemoryManager* memMgr) = 0;

  /** Set default full text query case option. */
  virtual void setFTCase(FTCaseOption option) = 0;
  virtual FTCaseOption getFTCase() = 0;
};

#endif

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

#ifndef XQCONTEXTIMPL_H
#define XQCONTEXTIMPL_H

#include <xqilla/framework/XQillaExport.hpp>
#include <xqilla/context/DynamicContext.hpp>
#include <xqilla/context/impl/VarStoreImpl.hpp>
#include <xqilla/schema/DocumentCache.hpp>
#include <xqilla/ast/ASTNode.hpp>
#include <xqilla/runtime/Sequence.hpp>
#include <xqilla/simple-api/XQilla.hpp>
#include <xqilla/framework/ProxyMemoryManager.hpp>

#include <xercesc/util/ValueHashTableOf.hpp>

class XQillaConfiguration;
class FunctionLookup;

class XQILLA_API XQContextImpl : public DynamicContext
{
public:
  XQContextImpl(XQillaConfiguration *conf, XQilla::Language language,
                XERCES_CPP_NAMESPACE_QUALIFIER MemoryManager* memMgr =
                XERCES_CPP_NAMESPACE_QUALIFIER XMLPlatformUtils::fgMemoryManager);
  ~XQContextImpl();

  virtual DynamicContext *createModuleContext(XERCES_CPP_NAMESPACE_QUALIFIER MemoryManager *memMgr =
                                              XERCES_CPP_NAMESPACE_QUALIFIER XMLPlatformUtils::fgMemoryManager) const;
  virtual DynamicContext *createModuleDynamicContext(const DynamicContext* moduleCtx, XERCES_CPP_NAMESPACE_QUALIFIER MemoryManager *memMgr =
                                                     XERCES_CPP_NAMESPACE_QUALIFIER XMLPlatformUtils::fgMemoryManager) const;
  virtual DynamicContext *createDynamicContext(XERCES_CPP_NAMESPACE_QUALIFIER MemoryManager *memMgr =
                                               XERCES_CPP_NAMESPACE_QUALIFIER XMLPlatformUtils::fgMemoryManager) const;

  virtual DynamicContext *createDebugQueryContext(const Item::Ptr &contextItem,
                                                  size_t contextPosition,
                                                  size_t contextSize,
                                                  const VariableStore *variables,
                                                  const XERCES_CPP_NAMESPACE_QUALIFIER DOMXPathNSResolver *nsResolver,
                                                  const XMLCh *defaultElementNS,
                                                  XERCES_CPP_NAMESPACE_QUALIFIER MemoryManager *memMgr =
                                                  XERCES_CPP_NAMESPACE_QUALIFIER XMLPlatformUtils::fgMemoryManager) const;

  virtual XQillaConfiguration *getConfiguration() const { return _conf; }

  virtual XQilla::Language getLanguage() const;

  //////////////////////////////////////
  // XQuery Static Context Accessors  //
  //////////////////////////////////////

  /**
   * Overrides the inherited method, and disowns any output
   * document that was created with the previous memory manager.
   */
  virtual void setMemoryManager(XPath2MemoryManager *memMgr);

  /** Binds a prefix to a namespace URI */
  virtual void setNamespaceBinding(const XMLCh* prefix, const XMLCh* uri);

  /** Set the policy for boundary space */
  virtual void setPreserveBoundarySpace(bool value);
  /** Get the policy for boundary space */
  virtual bool getPreserveBoundarySpace() const;

  //////////////////////////////////////
  // XQuery Dynamic Context Accessors //
  //////////////////////////////////////

  virtual void testInterrupt() const;

  //////////////////////////////////
  // Dynamic Context Accessors    //
  //////////////////////////////////

  /** Resets the dynamic context, as if it had never been used */
  virtual void clearDynamicContext();

  virtual SequenceBuilder *createSequenceBuilder() const;

  /** Returns the ItemFactory for this context, which is used to create XQilla items and other objects. */
  virtual ItemFactory *getItemFactory() const;
  /** Sets the ItemFactory for this context */
  virtual void setItemFactory(ItemFactory *factory);

  /** Get the context Item */
  virtual Item::Ptr getContextItem() const;
  /** Set the context item to item */
  virtual void setContextItem(const Item::Ptr &item);

  /** Get the context position */
  virtual size_t getContextPosition() const;
  /** Set the context position */
  virtual void setContextPosition(size_t pos);

  /** Get the context size */
  virtual size_t getContextSize() const;
  /** Set the context size */
  virtual void setContextSize(size_t size);

  virtual const VariableStore* getVariableStore() const;
  virtual void setVariableStore(const VariableStore *store);
  virtual const VariableStore* getGlobalVariableStore() const;
  virtual void setGlobalVariableStore(const VariableStore *store);
  virtual void setExternalVariable(const XMLCh *namespaceURI, const XMLCh *name, const Result &value);
  virtual void setExternalVariable(const XMLCh *qname, const Result &value);

  virtual const RegexGroupStore* getRegexGroupStore() const;
  virtual void setRegexGroupStore(const RegexGroupStore *store);

  /** Return the current time */
  virtual time_t getCurrentTime() const;
  /** Set the current time */
  virtual void setCurrentTime(time_t newTime);

  /** Return the implicit timezone for this system.*/
  virtual ATDurationOrDerived::Ptr getImplicitTimezone() const;
  /** Set the implicit timezone */
  virtual void setImplicitTimezone(const ATDurationOrDerived::Ptr &timezoneAsDuration);

  virtual void registerURIResolver(URIResolver *resolver, bool adopt);
  virtual URIResolver *getDefaultURIResolver() const;
  virtual void setDefaultURIResolver(URIResolver *resolver, bool adopt);

  virtual void setDebugListener(DebugListener *listener);
  virtual DebugListener *getDebugListener() const;

  virtual void setStackFrame(const StackFrame *frame);
  virtual const StackFrame *getStackFrame() const;

  virtual Node::Ptr parseDocument(XERCES_CPP_NAMESPACE_QUALIFIER InputSource &srcToUse,
                                  const LocationInfo *location, const QueryPathNode *projection);
  virtual Sequence resolveDocument(const XMLCh* uri, const LocationInfo *location,
                                   const QueryPathNode *projection);
  virtual Sequence resolveCollection(const XMLCh* uri, const LocationInfo *location,
                                     const QueryPathNode *projection);
  virtual Sequence resolveDefaultCollection(const QueryPathNode *projection);
  virtual bool putDocument(const Node::Ptr &document, const XMLCh *uri);

  virtual UpdateFactory *createUpdateFactory() const;

  //////////////////////////////////
  // Static Context Accessors     //
  //////////////////////////////////

  virtual const StaticType &getContextItemType() const;
  virtual void setContextItemType(const StaticType &st);

  virtual bool getXPath1CompatibilityMode() const;
  virtual void setXPath1CompatibilityMode(bool newMode);

  virtual const XERCES_CPP_NAMESPACE_QUALIFIER DOMXPathNSResolver* getNSResolver() const;
  virtual const XMLCh* getUriBoundToPrefix(const XMLCh* prefix, const LocationInfo *location) const;
  virtual const XMLCh* getPrefixBoundToUri(const XMLCh* uri) const;
  virtual void setNSResolver(const XERCES_CPP_NAMESPACE_QUALIFIER DOMXPathNSResolver* resolver);

  virtual const XMLCh* getDefaultElementAndTypeNS() const;
  virtual void setDefaultElementAndTypeNS(const XMLCh* newNS);

  virtual const XMLCh* getDefaultFuncNS() const;
  virtual void setDefaultFuncNS(const XMLCh* newNS);

  virtual DocumentCache* getDocumentCache() const;
  virtual void setDocumentCache(DocumentCache* docCache);
  virtual bool isTypeOrDerivedFromType(const XMLCh* const uri, const XMLCh* const typeName, const XMLCh* const uriToCheck, const XMLCh* const typeNameToCheck) const;

  virtual void setXMLEntityResolver(XERCES_CPP_NAMESPACE_QUALIFIER XMLEntityResolver* const handler);
  virtual XERCES_CPP_NAMESPACE_QUALIFIER XMLEntityResolver* getXMLEntityResolver() const;

  virtual void setModuleResolver(ModuleResolver *resolver);
  virtual ModuleResolver *getModuleResolver() const;
  virtual VectorOfStrings* resolveModuleURI(const XMLCh* uri) const;

  virtual void addSchemaLocation(const XMLCh* uri, VectorOfStrings* locations, const LocationInfo *location);

  virtual void setExternalFunctionResolver(ExternalFunctionResolver *resolver);
  virtual ExternalFunctionResolver *getExternalFunctionResolver() const;

  virtual VariableTypeStore* getVariableTypeStore();

  virtual void addTemplate(XQUserFunction *tp);
  virtual const XQUserFunction *lookUpNamedTemplate(const XMLCh *uri, const XMLCh *name) const;
  virtual const UserFunctions &getTemplateRules() const;

  virtual void addCustomFunction(FuncFactory *func);
  virtual void removeCustomFunction(FuncFactory *func);
  virtual ASTNode *lookUpFunction(const XMLCh *uri, const XMLCh* name, const VectorOfASTNodes &v, const LocationInfo *location) const;

  virtual void addExternalFunction(const ExternalFunction *func);
  virtual const ExternalFunction *lookUpExternalFunction(const XMLCh *uri, const XMLCh *name, size_t numArgs) const;

  /** Get the implementation for the specified collation */
  virtual Collation* getCollation(const XMLCh* const URI, const LocationInfo *location) const;
  /** Add a collation */
  virtual void addCollation(Collation* collation);

  /** Get the default collation */
  virtual Collation* getDefaultCollation(const LocationInfo *location) const;
  /** Specify which collation is the default one */
  virtual void setDefaultCollation(const XMLCh* const URI);

  /** Return the construction mode */
  virtual ConstructionMode getConstructionMode() const;
  /** Set the construction mode */
  virtual void setConstructionMode(ConstructionMode newMode);

  /** Return the ordering method for node sets */
  virtual NodeSetOrdering getNodeSetOrdering() const;
  /** Set the ordering method for node sets */
  virtual void setNodeSetOrdering(NodeSetOrdering newOrder);

  /** Return the base URI */
  virtual const XMLCh* getBaseURI() const;
  /** Set the base URI */
  virtual void setBaseURI(const XMLCh* newURI);

  /** Return the default ordering mode for FLWOR blocks */
  virtual FLWOROrderingMode getDefaultFLWOROrderingMode() const;
  /** Set the default ordering mode for FLWOR blocks */
  virtual void setDefaultFLWOROrderingMode(FLWOROrderingMode newMode);

  /** Set the policy for namespace inheritance */
  virtual void setInheritNamespaces(bool value);
  /** Get the policy for namespace inheritance */
  virtual bool getInheritNamespaces() const;

  /** Set the policy for namespace copy */
  virtual void setPreserveNamespaces(bool value);
  /** Get the policy for namespace copy */
  virtual bool getPreserveNamespaces() const;

  /** Set the revalidation mode */
  virtual void setRevalidationMode(DocumentCache::ValidationMode mode);
  /** Get the revalidation mode */
  virtual DocumentCache::ValidationMode getRevalidationMode() const;

  /** Set the listener for warning and trace messages */
  virtual void setMessageListener(MessageListener *listener);
  /** Gets the listener for warning and trace messages */
  virtual MessageListener *getMessageListener() const;

  virtual void setFTCase(FTCaseOption option) { _ftCaseOption = option; }
  virtual FTCaseOption getFTCase() { return _ftCaseOption; }

  virtual void setModule(XQQuery *module) { _module = module; }
  virtual XQQuery *getModule() const { return _module; }

  /////////////////////////////////////////
  //  XQilla context specific accessors  //
  /////////////////////////////////////////

  virtual bool getProjection() const { return _projection; }
  virtual void setProjection(bool enabled) { _projection = enabled; }

  virtual bool getDoLintWarnings() const { return _lint; }
  virtual void setDoLintWarnings(bool enabled) { _lint = enabled; }

  virtual const XMLCh *allocateTempVarName(const XMLCh *prefix = 0);

  /** Get the memory manager */
  virtual XPath2MemoryManager* getMemoryManager() const;

protected:
  XQillaConfiguration *_conf;
  XQilla::Language _language;

  // The memory manager used to create this context
  XERCES_CPP_NAMESPACE_QUALIFIER MemoryManager* _createdWith;

  // used for managing the memory of objects inside this context
  ProxyMemoryManager _internalMM;

  ////////////////////////////////
  //  Static Context variables  //
  ////////////////////////////////

  /** Static type of the context item */
  StaticType _contextItemType;

  /** XPath 1.0 compatibility mode. This value is true if rules for
   * backward compatibility with XPath Version 1.0 are in effect;
   * otherwise it is false */
  bool _xpath1Compatibility;

  /** In-scope namespaces. This is a set of  (prefix, URI) pairs.
   * The in-scope namespaces are used for resolving prefixes used
   * in QNames within the expression */
  const XERCES_CPP_NAMESPACE_QUALIFIER DOMXPathNSResolver* _nsResolver;

  /** The original (global scope) DOMXPathNSResolver */
  XQillaNSResolver* _globalNSResolver;

  /** Default element/type namespace. This is a namespace URI or "none". This
   * namespace is used for any unprefixed QName appearing in a
   * position where an element or type name is expected. */
  const XMLCh* _defaultElementNS;

  /** Default function namespace. This is a namespace URI.
   * This namespace URI is used for any unprefixed QName appearing
   * as the function name in a function call. */
  const XMLCh* _functionNS;

  /** In-scope schema definitions. This is a generic term for all
   * the element, attribute, and type definitions that are in scope
   * during processing of an expression. */
  DocumentCache* _docCache;
  bool _docCacheOwned;

  /** In-scope variable types. This is a set of  (QName, type) pairs.
   * It defines the set of variables that have been declared and are
   * available for reference within the expression. The QName
   * represents the name of the variable, and the type represents its
   * static data type */
  VariableTypeStore* _varTypeStore;

  /** Stores the available named templates */
  XERCES_CPP_NAMESPACE_QUALIFIER RefHashTableOf<XQUserFunction> _templateNameMap;
  /** Stores the available templates with a pattern */
  UserFunctions _templates; 

  /** In-scope functions. This part of the  static context defines the
   * set of functions that are available to be called from within an
   * expression. Each function is uniquely identified by its QName and
   * its arity (number of parameters). The  static context maps the QName
   * and arity into a function signature and a function definition. The
   * function signature specifies the static types of the function
   * parameters and the function result. */
  FunctionLookup* _functionTable;

  /** In-scope collations. This is a set of  (URI, collation) pairs. It
   * defines the names of the collations that are available for use in
   * function calls that take a collation name as an argument. A collation
   * may be regarded as an object that supports two functions: a function
   * that given a set of strings, returns a sequence containing those strings
   * in sorted order; and a function that given two strings, returns true if
   * they are considered equal, and false if not. */

  std::vector<Collation*, XQillaAllocator<Collation*> > _collations;

  /** Default collation. This is a collation. This collation is used by
   * string comparison functions when no explicit collation is specified. */
  const XMLCh* _defaultCollation;

  /** Construction mode. The construction mode governs the behavior of
   *  element constructors. If construction mode is preserve, the type
   *  of a constructed element node is xs:anyType, and the attributes
   *  and descendants of the constructed element retain their original
   *  types. If construction mode is strip, the type of the constructed
   *  element node and all its descendants is xdt:untyped, and attributes
   *  of the constructed element have type xdt:untypedAtomic */
  ConstructionMode _constructionMode;

  /** Ordering mode, which has the value ordered or unordered, affects
   *  the ordering of the result sequence returned by path expressions,
   *  union, intersect, and except expressions, and FLWOR expressions
   *  that have no order by clause. */
  NodeSetOrdering _ordering;

  /** Default ordering for empty sequences. This component controls whether an 
   *  empty sequence is interpreted as the greatest value or as the least value 
   *  during processing of an order by clause in a FLWOR expression, as described 
   *  in 3.8.3 Order By and Return Clauses.] Its value may be greatest or least. */
  FLWOROrderingMode _flworOrderingMode;

  /** Boundary-space policy. This component controls the processing of boundary 
   *  whitespace  by element constructors, as described in 3.7.1.4 Whitespace 
   *  in Element Content.] Its value may be preserve or strip. */
  bool _bPreserveBoundarySpace;

  /** Copy-namespaces mode. This component controls the namespace bindings 
   *  that are assigned when an existing element node is copied by an element 
   *  constructor, as described in 3.7.1 Direct Element Constructors. Its value 
   *  consists of two parts: preserve or no-preserve, and inherit or no-inherit. */
  bool _bPreserveNamespaces,
       _bInheritNamespaces;

  /** The revalidation mode for XQuery Update */
  DocumentCache::ValidationMode _revalidationMode;

  /** The message listener, for warnings and trace messages */
  MessageListener *_messageListener;

  /** The module */
  XQQuery *_module;

  /** Base URI. This is an absolute URI, used when necessary in the
   * resolution of relative URIs (for example, by the fn:resolve- uri
   * function.) The base URI is always provided by the external environment. */
  const XMLCh* _baseURI;

  /////////////////////////////////
  //  Dynamic Context Variables  //
  /////////////////////////////////

  /** The context item is the item currently being processed. */
  Item::Ptr _contextItem;

  /** The context position is the position of  the context item within
   * the sequence of items currently being processed. */
  size_t _contextPosition;

  /** The context size is the number of items in the sequence of
   * items currently being processed. */
  size_t _contextSize;

  /** In-scope variable values. This is a set of  (QName, Sequence) pairs.
   * It defines the set of variables that have been declared and are
   * available for reference within the expression. The QName
   * represents the name of the variable, and the Sequence represents its
   * value */
  const VariableStore *_varStore;
  const VariableStore *_globalVarStore;
  VarStoreImpl _defaultVarStore;

  const RegexGroupStore *_regexStore;

  /** Current date and time. This information  represents an
   * implementation-dependent point in time during processing of a query
   * or transformation */
  time_t _currentTime;

  /** Implicit timezone. This is the timezone to be used when a date,
   * time, or dateTime value that does not have a timezone is used in a
   * comparison or in any other operation. This value is an instance of
   * xdt:dayTimeDuration */
  ATDurationOrDerived::Ptr _implicitTimezone;

  /** For each atomic type in the in-scope type  definitions, there is 
   * a constructor function in the in-scope functions. Constructor 
   * functions are discussed in 3.10.4 Constructor Functions */
  ItemFactory* _itemFactory;

  ////////////////////////////////////////
  //  XQilla Context specific variables //
  ////////////////////////////////////////

  struct ResolverEntry {
    ResolverEntry() : resolver(0), adopt(false) {}
    ResolverEntry(URIResolver *r, bool a) : resolver(r), adopt(a) {}
    URIResolver *resolver;
    bool adopt;
  };

  /// A stack of URIResolver pointers
  std::vector<ResolverEntry, XQillaAllocator<ResolverEntry> > _resolvers;
  ResolverEntry _defaultResolver;

  ModuleResolver *_moduleResolver;
  ExternalFunctionResolver *_exFuncResolver;

  std::vector<ExternalFunction*, XQillaAllocator<ExternalFunction*> > _exFuncs;

  DebugListener *_debugListener;
  const StackFrame *_stackFrame;

  bool _projection;
  bool _lint;

  unsigned int _tmpVarCounter;

  // used for memory management in expression evaluation
  XPath2MemoryManager* _memMgr;

  //Full text related
  FTCaseOption _ftCaseOption;
};

#endif

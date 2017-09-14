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

#ifndef XQDYNAMICCONTEXTIMPL_H
#define XQDYNAMICCONTEXTIMPL_H

#include <xqilla/context/DynamicContext.hpp>
#include <xqilla/context/impl/VarStoreImpl.hpp>
#include <xqilla/runtime/Sequence.hpp>
#include <xqilla/framework/ProxyMemoryManager.hpp>
#include <xqilla/exceptions/ContextException.hpp>

#include <xercesc/util/ValueHashTableOf.hpp>

class XQILLA_API XQDynamicContextImpl : public DynamicContext
{
public:
  XQDynamicContextImpl(XQillaConfiguration *conf, const StaticContext *staticContext, VarStoreImpl *defaultVarStore,
                       XERCES_CPP_NAMESPACE_QUALIFIER MemoryManager* memMgr);
  ~XQDynamicContextImpl();

  virtual DynamicContext *createModuleContext(XERCES_CPP_NAMESPACE_QUALIFIER MemoryManager *memMgr =
                                              XERCES_CPP_NAMESPACE_QUALIFIER XMLPlatformUtils::fgMemoryManager) const;
  virtual DynamicContext *createModuleDynamicContext(const DynamicContext* moduleCtx, XERCES_CPP_NAMESPACE_QUALIFIER MemoryManager *memMgr =
                                                     XERCES_CPP_NAMESPACE_QUALIFIER XMLPlatformUtils::fgMemoryManager) const;
  virtual DynamicContext *createDynamicContext(XERCES_CPP_NAMESPACE_QUALIFIER MemoryManager *memMgr
                                               = XERCES_CPP_NAMESPACE_QUALIFIER XMLPlatformUtils::fgMemoryManager) const;

  virtual DynamicContext *createDebugQueryContext(const Item::Ptr &contextItem,
                                                  size_t contextPosition,
                                                  size_t contextSize,
                                                  const VariableStore *variables,
                                                  const XERCES_CPP_NAMESPACE_QUALIFIER DOMXPathNSResolver *nsResolver,
                                                  const XMLCh *defaultElementNS,
                                                  XERCES_CPP_NAMESPACE_QUALIFIER MemoryManager *memMgr =
                                                  XERCES_CPP_NAMESPACE_QUALIFIER XMLPlatformUtils::fgMemoryManager) const;

  virtual XQillaConfiguration *getConfiguration() const;

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

  virtual void setModule(XQQuery *module);
  virtual XQQuery *getModule() const;

  virtual void setFTCase(FTCaseOption option);
  virtual FTCaseOption getFTCase();


  /////////////////////////////////////////
  //  XQilla context specific accessors  //
  /////////////////////////////////////////

  virtual bool getProjection() const { return _projection; }
  virtual void setProjection(bool enabled) { _projection = enabled; }

  virtual bool getDoLintWarnings() const;
  virtual void setDoLintWarnings(bool enabled);

  virtual const XMLCh *allocateTempVarName(const XMLCh *prefix = 0);

  /** Get the memory manager */
  virtual XPath2MemoryManager* getMemoryManager() const;

protected:
  XQillaConfiguration *_conf;

  const StaticContext *_staticContext;

  // The memory manager used to create this context
  XERCES_CPP_NAMESPACE_QUALIFIER MemoryManager* _createdWith;

  // used for managing the memory of objects inside this context
  ProxyMemoryManager _internalMM;

  /** In-scope namespaces. This is a set of  (prefix, URI) pairs.
   * The in-scope namespaces are used for resolving prefixes used
   * in QNames within the expression
   * THIS OUGHT TO BE IN THE STATIC CONTEXT - jpcs */
  const XERCES_CPP_NAMESPACE_QUALIFIER DOMXPathNSResolver* _nsResolver;

  /** Default collation. This is a collation. This collation is used by
   * string comparison functions when no explicit collation is specified.
   * THIS OUGHT TO BE IN THE STATIC CONTEXT - jpcs */
  const XMLCh* _defaultCollation;

  /** Default element/type namespace. This is a namespace URI or "none". This
   * namespace is used for any unprefixed QName appearing in a
   * position where an element or type name is expected.
   * THIS OUGHT TO BE IN THE STATIC CONTEXT - jpcs */
  const XMLCh* _defaultElementNS;

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
  VarStoreImpl *_defaultVarStore;
  bool _defaultVarStoreOwned;
  const VariableStore *_varStore;
  const VariableStore *_globalVarStore;

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
  bool _itemFactoryOwned;

  struct ResolverEntry {
    ResolverEntry() : resolver(0), adopt(false) {}
    ResolverEntry(URIResolver *r, bool a) : resolver(r), adopt(a) {}
    URIResolver *resolver;
    bool adopt;
  };

  /// A stack of URIResolver pointers
  std::vector<ResolverEntry, XQillaAllocator<ResolverEntry> > _resolvers;
  ResolverEntry _defaultResolver;

  DebugListener *_debugListener;
  const StackFrame *_stackFrame;

  /** Contains the XMLGrammarPool of the StaticContext, and is used to
   * load xml documents for resolveCollection and resolveDocument */
  DocumentCache* _docCache;

  /** The message listener, for warnings and trace messages */
  MessageListener *_messageListener;

  bool _projection;

  // used for memory management
  XPath2MemoryManager* _memMgr;
};


inline void XQDynamicContextImpl::setContextItemType(const StaticType &)
{ XQThrow2(ContextException,X("XQDynamicContextImpl"), X("You cannot change the static context when using a proxying dynamic context")); }
inline void XQDynamicContextImpl::setXPath1CompatibilityMode(bool newMode)
{ XQThrow2(ContextException,X("XQDynamicContextImpl"), X("You cannot change the static context when using a proxying dynamic context")); }
inline void XQDynamicContextImpl::setDefaultFuncNS(const XMLCh* newNS)
{ XQThrow2(ContextException,X("XQDynamicContextImpl"), X("You cannot change the static context when using a proxying dynamic context")); }
inline void XQDynamicContextImpl::setBaseURI(const XMLCh* newURI)
{ XQThrow2(ContextException,X("XQDynamicContextImpl"), X("You cannot change the static context when using a proxying dynamic context")); }
inline void XQDynamicContextImpl::addTemplate(XQUserFunction *tp)
{ XQThrow2(ContextException,X("XQDynamicContextImpl"), X("You cannot change the static context when using a proxying dynamic context")); }
inline void XQDynamicContextImpl::addCustomFunction(FuncFactory *func)
{ XQThrow2(ContextException,X("XQDynamicContextImpl"), X("You cannot change the static context when using a proxying dynamic context")); }
inline void XQDynamicContextImpl::removeCustomFunction(FuncFactory *func)
{ XQThrow2(ContextException,X("XQDynamicContextImpl"), X("You cannot change the static context when using a proxying dynamic context")); }
inline void XQDynamicContextImpl::addExternalFunction(const ExternalFunction *func)
{ XQThrow2(ContextException,X("XQDynamicContextImpl"), X("You cannot change the static context when using a proxying dynamic context")); }
inline void XQDynamicContextImpl::addCollation(Collation* collation)
{ XQThrow2(ContextException,X("XQDynamicContextImpl"), X("You cannot change the static context when using a proxying dynamic context")); }
inline void XQDynamicContextImpl::addSchemaLocation(const XMLCh* uri, VectorOfStrings* locations, const LocationInfo *location)
{ XQThrow2(ContextException,X("XQDynamicContextImpl"), X("You cannot change the static context when using a proxying dynamic context")); }
inline VariableTypeStore* XQDynamicContextImpl::getVariableTypeStore()
{ XQThrow2(ContextException,X("XQDynamicContextImpl"), X("You cannot change the static context when using a proxying dynamic context")); return 0; }
inline void XQDynamicContextImpl::setConstructionMode(StaticContext::ConstructionMode newMode)
{ XQThrow2(ContextException,X("XQDynamicContextImpl"), X("You cannot change the static context when using a proxying dynamic context")); }
inline void XQDynamicContextImpl::setPreserveBoundarySpace(bool value)
{ XQThrow2(ContextException,X("XQDynamicContextImpl"), X("You cannot change the static context when using a proxying dynamic context")); }
inline void XQDynamicContextImpl::setDefaultFLWOROrderingMode(StaticContext::FLWOROrderingMode newMode)
{ XQThrow2(ContextException,X("XQDynamicContextImpl"), X("You cannot change the static context when using a proxying dynamic context")); }
inline void XQDynamicContextImpl::setInheritNamespaces(bool value) 
{ XQThrow2(ContextException,X("XQDynamicContextImpl"), X("You cannot change the static context when using a proxying dynamic context")); }
inline void XQDynamicContextImpl::setPreserveNamespaces(bool value) 
{ XQThrow2(ContextException,X("XQDynamicContextImpl"), X("You cannot change the static context when using a proxying dynamic context")); }
inline void XQDynamicContextImpl::setNodeSetOrdering(NodeSetOrdering newOrder)
{ XQThrow2(ContextException,X("XQDynamicContextImpl"), X("You cannot change the static context when using a proxying dynamic context")); }
inline void XQDynamicContextImpl::setModuleResolver(ModuleResolver *resolver)
{ XQThrow2(ContextException,X("XQDynamicContextImpl"), X("You cannot change the static context when using a proxying dynamic context")); }
inline void XQDynamicContextImpl::setExternalFunctionResolver(ExternalFunctionResolver *resolver)
{ XQThrow2(ContextException,X("XQDynamicContextImpl"), X("You cannot change the static context when using a proxying dynamic context")); }
inline void XQDynamicContextImpl::setRevalidationMode(DocumentCache::ValidationMode mode)
{ XQThrow2(ContextException,X("XQDynamicContextImpl"), X("You cannot change the static context when using a proxying dynamic context")); }
inline const XMLCh *XQDynamicContextImpl::allocateTempVarName(const XMLCh *prefix)
{ XQThrow2(ContextException,X("XQDynamicContextImpl"), X("You cannot change the static context when using a proxying dynamic context")); return 0; }
inline void XQDynamicContextImpl::setNamespaceBinding(const XMLCh* prefix, const XMLCh* uri)
{ XQThrow2(ContextException,X("XQDynamicContextImpl"), X("You cannot change the static context when using a proxying dynamic context")); }
inline void XQDynamicContextImpl::setDoLintWarnings(bool enabled)
{ XQThrow2(ContextException,X("XQDynamicContextImpl"), X("You cannot change the static context when using a proxying dynamic context")); }
inline void XQDynamicContextImpl::setModule(XQQuery *module)
{ XQThrow2(ContextException,X("XQDynamicContextImpl"), X("You cannot change the static context when using a proxying dynamic context")); }


inline const StaticType &XQDynamicContextImpl::getContextItemType() const { XQThrow2(ContextException,X("XQDynamicContextImpl"), X("The static context is not available from a proxying dynamic context")); }
inline const XMLCh* XQDynamicContextImpl::getDefaultFuncNS() const { XQThrow2(ContextException,X("XQDynamicContextImpl"), X("The static context is not available from a proxying dynamic context")); }
inline StaticContext::NodeSetOrdering XQDynamicContextImpl::getNodeSetOrdering() const { XQThrow2(ContextException,X("XQDynamicContextImpl"), X("The static context is not available from a proxying dynamic context")); }
inline ModuleResolver * XQDynamicContextImpl::getModuleResolver() const { XQThrow2(ContextException,X("XQDynamicContextImpl"), X("The static context is not available from a proxying dynamic context")); }
inline ExternalFunctionResolver * XQDynamicContextImpl::getExternalFunctionResolver() const { XQThrow2(ContextException,X("XQDynamicContextImpl"), X("The static context is not available from a proxying dynamic context")); }
inline ASTNode* XQDynamicContextImpl::lookUpFunction(const XMLCh* uri, const XMLCh* name, const VectorOfASTNodes& v, const LocationInfo *location) const { XQThrow2(ContextException,X("XQDynamicContextImpl"), X("The static context is not available from a proxying dynamic context")); }
inline const ExternalFunction *XQDynamicContextImpl::lookUpExternalFunction(const XMLCh *uri, const XMLCh *name, size_t numArgs) const { XQThrow2(ContextException,X("XQDynamicContextImpl"), X("The static context is not available from a proxying dynamic context")); }
inline StaticContext::FLWOROrderingMode XQDynamicContextImpl::getDefaultFLWOROrderingMode() const { XQThrow2(ContextException,X("XQDynamicContextImpl"), X("The static context is not available from a proxying dynamic context")); }
inline bool XQDynamicContextImpl::getXPath1CompatibilityMode() const { XQThrow2(ContextException,X("XQDynamicContextImpl"), X("The static context is not available from a proxying dynamic context")); }
inline bool XQDynamicContextImpl::getPreserveBoundarySpace() const { XQThrow2(ContextException,X("XQDynamicContextImpl"), X("The static context is not available from a proxying dynamic context")); }
inline DocumentCache::ValidationMode XQDynamicContextImpl::getRevalidationMode() const { XQThrow2(ContextException,X("XQDynamicContextImpl"), X("The static context is not available from a proxying dynamic context")); }
inline bool XQDynamicContextImpl::getDoLintWarnings() const { XQThrow2(ContextException,X("XQDynamicContextImpl"), X("The static context is not available from a proxying dynamic context")); }
inline XQQuery *XQDynamicContextImpl::getModule() const { XQThrow2(ContextException,X("XQDynamicContextImpl"), X("The static context is not available from a proxying dynamic context")); }

inline void XQDynamicContextImpl::setFTCase(FTCaseOption option){ XQThrow2(ContextException,X("XQDynamicContextImpl"), X("The static context is not available from a proxying dynamic context")); }
inline XQDynamicContextImpl::FTCaseOption XQDynamicContextImpl::getFTCase(){ XQThrow2(ContextException,X("XQDynamicContextImpl"), X("The static context is not available from a proxying dynamic context")); }

inline XQilla::Language XQDynamicContextImpl::getLanguage() const { return _staticContext->getLanguage(); }
inline const XMLCh* XQDynamicContextImpl::getBaseURI() const { return _staticContext->getBaseURI(); }
inline Collation* XQDynamicContextImpl::getCollation(const XMLCh* URI, const LocationInfo *location) const { return _staticContext->getCollation(URI, location); }
inline bool XQDynamicContextImpl::isTypeOrDerivedFromType(const XMLCh* uri, const XMLCh* typeName, const XMLCh* uriToCheck, const XMLCh* typeNameToCheck) const { return _staticContext->isTypeOrDerivedFromType(uri, typeName, uriToCheck, typeNameToCheck); }
inline bool XQDynamicContextImpl::getInheritNamespaces() const { return _staticContext->getInheritNamespaces(); }
inline bool XQDynamicContextImpl::getPreserveNamespaces() const { return _staticContext->getPreserveNamespaces(); }
inline StaticContext::ConstructionMode XQDynamicContextImpl::getConstructionMode() const { return _staticContext->getConstructionMode(); }
inline const UserFunctions &XQDynamicContextImpl::getTemplateRules() const { return _staticContext->getTemplateRules(); }
inline const XQUserFunction *XQDynamicContextImpl::lookUpNamedTemplate(const XMLCh *uri, const XMLCh *name) const { return _staticContext->lookUpNamedTemplate(uri, name); }

#endif

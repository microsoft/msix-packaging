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

#include <xqilla/context/impl/XQDynamicContextImpl.hpp>

#include <xqilla/context/VariableStore.hpp>
#include <xqilla/utils/XPath2NSUtils.hpp>
#include <xqilla/utils/XPath2Utils.hpp>
#include <xqilla/exceptions/ContextException.hpp>
#include <xqilla/exceptions/NamespaceLookupException.hpp>
#include <xqilla/exceptions/TypeNotFoundException.hpp>
#include <xqilla/context/impl/CodepointCollation.hpp>
#include <xqilla/context/Collation.hpp>
#include <xqilla/items/ATDurationOrDerived.hpp>
#include <xqilla/items/Timezone.hpp>
#include <xqilla/ast/XQFunction.hpp>
#include <xqilla/items/DatatypeLookup.hpp>
#include <xqilla/functions/FunctionLookup.hpp>
#include <xqilla/dom-api/impl/XQillaNSResolverImpl.hpp>
#include <xqilla/items/DatatypeFactory.hpp>
#include <xqilla/context/URIResolver.hpp>
#include <xqilla/utils/ContextUtils.hpp>
#include <xqilla/simple-api/XQillaConfiguration.hpp>

XERCES_CPP_NAMESPACE_USE;

XQDynamicContextImpl::XQDynamicContextImpl(XQillaConfiguration *conf, const StaticContext *staticContext,
                                           VarStoreImpl *defaultVarStore, MemoryManager* memMgr)
  : _conf(conf),
    _staticContext(staticContext),
    _createdWith(memMgr),
    _internalMM(memMgr),
    _nsResolver(staticContext->getNSResolver()),
    _defaultCollation(0),
    _defaultElementNS(staticContext->getDefaultElementAndTypeNS()),
    _contextItem(0),
    _contextPosition(1),
    _contextSize(1),
    _defaultVarStore(defaultVarStore ? defaultVarStore : new VarStoreImpl(&_internalMM)),
    _defaultVarStoreOwned(defaultVarStore == 0),
    _varStore(_defaultVarStore),
    _globalVarStore(_defaultVarStore),
    _regexStore(0),
    _implicitTimezone(0),
    _resolvers(XQillaAllocator<ResolverEntry>(&_internalMM)),
    _debugListener(0),
    _stackFrame(0),
    // This is created with the _createdWith memory manager,
    // since a bug in xerces means we can't use a non-thread-safe
    // memory manager - jpcs
    _docCache(staticContext->getDocumentCache()->createDerivedCache(_createdWith)),
    _messageListener(staticContext->getMessageListener()),
    _projection(staticContext->getProjection())
{
  time(&_currentTime);
  _memMgr = &_internalMM;
  _itemFactory = staticContext->getItemFactory();
  _itemFactoryOwned = false;

  setXMLEntityResolver(staticContext->getXMLEntityResolver());

  // Set up the default URIResolver
  _defaultResolver.resolver = _conf->createDefaultURIResolver(&_internalMM);
  if(_defaultResolver.resolver != 0) {
    _defaultResolver.adopt = true;
  }
}

XQDynamicContextImpl::~XQDynamicContextImpl()
{
  _contextItem = 0;
  _implicitTimezone = 0;
  if(_defaultVarStoreOwned) {
    _defaultVarStore->clear();
    delete _defaultVarStore;
  }

  if(_itemFactoryOwned) delete _itemFactory;
  delete _docCache;

  std::vector<ResolverEntry, XQillaAllocator<ResolverEntry> >::reverse_iterator end = _resolvers.rend();
  for(std::vector<ResolverEntry, XQillaAllocator<ResolverEntry> >::reverse_iterator i = _resolvers.rbegin(); i != end; ++i) {
    if(i->adopt) {
      delete i->resolver;
    }
  }
  if(_defaultResolver.adopt)
    delete _defaultResolver.resolver;
}

DynamicContext *XQDynamicContextImpl::createModuleContext(MemoryManager *memMgr) const
{
  return _staticContext->createModuleContext(memMgr);
}

DynamicContext *XQDynamicContextImpl::createModuleDynamicContext(const DynamicContext* moduleCtx, MemoryManager *memMgr) const
{
  XQDynamicContextImpl* moduleDCtx = new (memMgr) XQDynamicContextImpl(_conf, moduleCtx, _defaultVarStore, memMgr);

  // Force the context to use our memory manager
  moduleDCtx->setMemoryManager(getMemoryManager());

  // Use the variables from our context
  moduleDCtx->setGlobalVariableStore(getGlobalVariableStore());
  moduleDCtx->setVariableStore(getVariableStore());

  // Add our URIResolvers to the module context
  moduleDCtx->setDefaultURIResolver(_defaultResolver.resolver, /*adopt*/false);
  std::vector<ResolverEntry, XQillaAllocator<ResolverEntry> >::const_iterator end = _resolvers.end();
  for(std::vector<ResolverEntry, XQillaAllocator<ResolverEntry> >::const_iterator i = _resolvers.begin(); i != end; ++i) {
    moduleDCtx->registerURIResolver(i->resolver, /*adopt*/false);
  }

  // Set the XMLEntityResolver
  moduleDCtx->setXMLEntityResolver(_docCache->getXMLEntityResolver());

  // Set the MessageListener
  moduleDCtx->setMessageListener(_messageListener);

  // Set the DebugListener and StackFrame
  moduleDCtx->setDebugListener(_debugListener);
  moduleDCtx->setStackFrame(_stackFrame);

  _conf->populateDynamicContext(moduleDCtx);
  return moduleDCtx;
}

DynamicContext *XQDynamicContextImpl::createDynamicContext(MemoryManager *memMgr) const
{
  return _staticContext->createDynamicContext(memMgr);
}

DynamicContext *XQDynamicContextImpl::createDebugQueryContext(const Item::Ptr &contextItem,
                                                              size_t contextPosition,
                                                              size_t contextSize,
                                                              const VariableStore *variables,
                                                              const DOMXPathNSResolver *nsResolver,
                                                              const XMLCh *defaultElementNS,
                                                              MemoryManager *memMgr) const
{
  return _staticContext->createDebugQueryContext(contextItem, contextPosition, contextSize,
                                                 variables, nsResolver, defaultElementNS, memMgr);
}

XQillaConfiguration *XQDynamicContextImpl::getConfiguration() const
{
  return _conf;
}

void XQDynamicContextImpl::setMemoryManager(XPath2MemoryManager* memMgr)
{
  _memMgr = memMgr;
}

void XQDynamicContextImpl::clearDynamicContext()
{
  _nsResolver = _staticContext->getNSResolver();
  _contextItem = 0;
  _contextSize = 1;
  _contextPosition = 1;
  _implicitTimezone = 0;

  _defaultVarStore->clear();
  _varStore = _defaultVarStore;
  _globalVarStore = _defaultVarStore;

//   if(_defaultResolver.adopt)
//     delete _defaultResolver.resolver;
//   _defaultResolver.adopt = false;
//   _defaultResolver.resolver = 0;

//   std::vector<ResolverEntry, XQillaAllocator<ResolverEntry> >::reverse_iterator end = _resolvers.rend();
//   for(std::vector<ResolverEntry, XQillaAllocator<ResolverEntry> >::reverse_iterator i = _resolvers.rbegin(); i != end; ++i) {
//     if(i->adopt) {
//       delete i->resolver;
//     }
//   }
//   _resolvers.clear();

//   _defaultResolver.resolver = _conf->createDefaultURIResolver(&_internalMM);
//   if(_defaultResolver.resolver != 0) {
//     _defaultResolver.adopt = true;
//   }

  time(&_currentTime);
}

DocumentCache *XQDynamicContextImpl::getDocumentCache() const
{
  return _docCache;
}

void XQDynamicContextImpl::setDocumentCache(DocumentCache* docCache)
{
  _docCache=docCache;
}

void XQDynamicContextImpl::setContextItem(const Item::Ptr &item)
{
  _contextItem = item;
}

void XQDynamicContextImpl::setContextSize(size_t size)
{
  _contextSize = size;
}

void XQDynamicContextImpl::setContextPosition(size_t pos)
{
  _contextPosition = pos;
}

Item::Ptr XQDynamicContextImpl::getContextItem() const
{
  return _contextItem;
}

size_t XQDynamicContextImpl::getContextSize() const
{
  return _contextSize;
}

size_t XQDynamicContextImpl::getContextPosition() const
{
  return _contextPosition;
}

const VariableStore* XQDynamicContextImpl::getVariableStore() const
{
  return _varStore;
}

void XQDynamicContextImpl::setVariableStore(const VariableStore *store)
{
  assert(store);
  _varStore = store;
}

const VariableStore* XQDynamicContextImpl::getGlobalVariableStore() const
{
  return _globalVarStore;
}

void XQDynamicContextImpl::setGlobalVariableStore(const VariableStore *store)
{
  _globalVarStore = store;
}

void XQDynamicContextImpl::setExternalVariable(const XMLCh *namespaceURI, const XMLCh *name, const Result &value)
{
  _defaultVarStore->setVar(namespaceURI, name, value);
}

void XQDynamicContextImpl::setExternalVariable(const XMLCh *qname, const Result &value)
{
  const XMLCh *uri = getUriBoundToPrefix(XPath2NSUtils::getPrefix(qname, getMemoryManager()), 0);
  const XMLCh *name = XPath2NSUtils::getLocalName(qname);

  _defaultVarStore->setVar(uri, name, value);
}

const RegexGroupStore *XQDynamicContextImpl::getRegexGroupStore() const
{
  return _regexStore;
}

void XQDynamicContextImpl::setRegexGroupStore(const RegexGroupStore *store)
{
  _regexStore = store;
}

time_t XQDynamicContextImpl::getCurrentTime() const
{
  return _currentTime;
}

void XQDynamicContextImpl::setCurrentTime(time_t newTime)
{
  _currentTime=newTime;
}

ATDurationOrDerived::Ptr XQDynamicContextImpl::getImplicitTimezone() const {
  if(_implicitTimezone == NULLRCP) {
    // validate tzone
    Timezone tzone(ContextUtils::getTimezone());
    // c-style the const away since we are in a const method
    ((XQDynamicContextImpl*)this)->_implicitTimezone = tzone.asDayTimeDuration(this);
}

  return _implicitTimezone;
}

void XQDynamicContextImpl::setImplicitTimezone(const ATDurationOrDerived::Ptr &timezoneAsDuration) {
  // validate the timezone -- will trow if invalid
  Timezone timezone(timezoneAsDuration, this);
  this->_implicitTimezone = timezoneAsDuration;
}

XPath2MemoryManager* XQDynamicContextImpl::getMemoryManager() const
{
  return _memMgr;
}

const XMLCh* XQDynamicContextImpl::getDefaultElementAndTypeNS() const
{
  return _defaultElementNS;
}

void XQDynamicContextImpl::setDefaultElementAndTypeNS(const XMLCh* newNS) {
  _defaultElementNS = newNS;
}

void XQDynamicContextImpl::registerURIResolver(URIResolver *resolver, bool adopt)
{
  if(resolver != 0) {
    _resolvers.push_back(ResolverEntry(resolver, adopt));
  }
}

URIResolver *XQDynamicContextImpl::getDefaultURIResolver() const
{
  return _defaultResolver.resolver;
}

void XQDynamicContextImpl::setDefaultURIResolver(URIResolver *resolver, bool adopt)
{
  if(_defaultResolver.adopt) {
    delete _defaultResolver.resolver;
  }
  _defaultResolver.resolver = resolver;
  _defaultResolver.adopt = adopt;
}

Node::Ptr XQDynamicContextImpl::parseDocument(InputSource &srcToUse, const LocationInfo *location,
                                              const QueryPathNode *projection)
{
  try {
    return _docCache->parseDocument(srcToUse, this, projection);
  }
  catch(XQException &e) {
    if(e.getXQueryLine() == 0 && location)
      e.setXQueryPosition(location);
    throw;
  }
}

Sequence XQDynamicContextImpl::resolveDocument(const XMLCh* uri, const LocationInfo *location, const QueryPathNode *projection)
{
  Sequence result(getMemoryManager());

  // Check the URIResolver objects
  try {
    std::vector<ResolverEntry, XQillaAllocator<ResolverEntry> >::reverse_iterator end = _resolvers.rend();
    for(std::vector<ResolverEntry, XQillaAllocator<ResolverEntry> >::reverse_iterator i = _resolvers.rbegin(); i != end; ++i) {
      if(i->resolver->resolveDocument(result, uri, this, projection))
        return result;
    }
    if(_defaultResolver.resolver)
      _defaultResolver.resolver->resolveDocument(result, uri, this, projection);
  }
  catch(XQException &e) {
    if(e.getXQueryLine() == 0 && location)
      e.setXQueryPosition(location);
    throw;
  }

  return result;
}

Sequence XQDynamicContextImpl::resolveCollection(const XMLCh* uri, const LocationInfo *location, const QueryPathNode *projection)
{
  Sequence result(getMemoryManager());

  // Check the URIResolver objects
  try {
    std::vector<ResolverEntry, XQillaAllocator<ResolverEntry> >::reverse_iterator end = _resolvers.rend();
    for(std::vector<ResolverEntry, XQillaAllocator<ResolverEntry> >::reverse_iterator i = _resolvers.rbegin(); i != end; ++i) {
      if(i->resolver->resolveCollection(result, uri, this, projection))
        return result;
    }
    if(_defaultResolver.resolver)
      _defaultResolver.resolver->resolveCollection(result, uri, this, projection);
  }
  catch(XQException &e) {
    if(e.getXQueryLine() == 0 && location)
      e.setXQueryPosition(location);
    throw;
  }

  return result;
}

Sequence XQDynamicContextImpl::resolveDefaultCollection(const QueryPathNode *projection)
{
  Sequence result(getMemoryManager());
  std::vector<ResolverEntry, XQillaAllocator<ResolverEntry> >::reverse_iterator end = _resolvers.rend();
  for(std::vector<ResolverEntry, XQillaAllocator<ResolverEntry> >::reverse_iterator i = _resolvers.rbegin(); i != end; ++i) {
    if(i->resolver->resolveDefaultCollection(result, this, projection))
      return result;
  }
  if(_defaultResolver.resolver)
    _defaultResolver.resolver->resolveDefaultCollection(result, this, projection);
  return result;
}

bool XQDynamicContextImpl::putDocument(const Node::Ptr &document, const XMLCh *uri)
{
  std::vector<ResolverEntry, XQillaAllocator<ResolverEntry> >::reverse_iterator end = _resolvers.rend();
  for(std::vector<ResolverEntry, XQillaAllocator<ResolverEntry> >::reverse_iterator i = _resolvers.rbegin(); i != end; ++i) {
    if(i->resolver->putDocument(document, uri, this))
      return true;
  }
  if(_defaultResolver.resolver)
    return _defaultResolver.resolver->putDocument(document, uri, this);
  return false;
}

VectorOfStrings* XQDynamicContextImpl::resolveModuleURI(const XMLCh* uri) const
{
  return _staticContext->resolveModuleURI(uri);
}

SequenceBuilder *XQDynamicContextImpl::createSequenceBuilder() const
{
  return _conf->createSequenceBuilder(this);
}

ItemFactory *XQDynamicContextImpl::getItemFactory() const
{
  return _itemFactory;
}

void XQDynamicContextImpl::setItemFactory(ItemFactory *factory)
{
  _itemFactory = factory;
  _itemFactoryOwned = true;
}

UpdateFactory *XQDynamicContextImpl::createUpdateFactory() const
{
  return _conf->createUpdateFactory(const_cast<ProxyMemoryManager*>(&_internalMM));
}

void XQDynamicContextImpl::setNSResolver(const DOMXPathNSResolver* resolver) {
  _nsResolver = resolver;
}

const DOMXPathNSResolver* XQDynamicContextImpl::getNSResolver() const {
  return _nsResolver;
}

const XMLCh* XQDynamicContextImpl::getUriBoundToPrefix(const XMLCh* prefix, const LocationInfo *location) const
{
  if(prefix == 0 || *prefix == 0) return 0;

  const XMLCh* uri = _nsResolver->lookupNamespaceURI(prefix);

  if((uri == 0 || *uri == 0) && prefix != 0 && *prefix != 0) {
    const XMLCh* msg = XPath2Utils::concatStrings(X("No namespace for prefix \'"), prefix, X("\' [err:XPST0081]"), getMemoryManager());
    XQThrow3(NamespaceLookupException, X("XQDynamicContextImpl::getUriBoundToPrefix"), msg, location);
  }

  return uri;
}

const XMLCh* XQDynamicContextImpl::getPrefixBoundToUri(const XMLCh* uri) const
{
  return _nsResolver->lookupPrefix(uri);
}

void XQDynamicContextImpl::setXMLEntityResolver(XMLEntityResolver* const handler)
{
  _docCache->setXMLEntityResolver(handler);
}

XMLEntityResolver* XQDynamicContextImpl::getXMLEntityResolver() const
{
  return _docCache->getXMLEntityResolver();
}

void XQDynamicContextImpl::setDefaultCollation(const XMLCh* URI)
{
  _defaultCollation=getMemoryManager()->getPooledString(URI);
}

Collation* XQDynamicContextImpl::getDefaultCollation(const LocationInfo *location) const
{
  if(_defaultCollation == 0) {
    return _staticContext->getDefaultCollation(location);
  }
  else {
    return getCollation(_defaultCollation, location);
  }
}

void XQDynamicContextImpl::setMessageListener(MessageListener *listener)
{
  _messageListener = listener;
}

MessageListener *XQDynamicContextImpl::getMessageListener() const
{
  return _messageListener;
}

void XQDynamicContextImpl::setDebugListener(DebugListener *listener)
{
  _debugListener = listener;
}

DebugListener *XQDynamicContextImpl::getDebugListener() const
{
  return _debugListener;
}

void XQDynamicContextImpl::setStackFrame(const StackFrame *frame)
{
  _stackFrame = frame;
}

const StackFrame *XQDynamicContextImpl::getStackFrame() const
{
  return _stackFrame;
}

void XQDynamicContextImpl::testInterrupt() const
{
  _conf->testInterrupt();
}


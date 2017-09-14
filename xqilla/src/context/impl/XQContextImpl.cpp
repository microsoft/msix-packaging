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

#include <xercesc/validators/schema/SchemaSymbols.hpp>
#include <xercesc/util/XMLUri.hpp>

#include <xqilla/context/impl/XQContextImpl.hpp>
#include <xqilla/context/impl/XQDynamicContextImpl.hpp>

#include <xqilla/context/impl/CodepointCollation.hpp>
#include <xqilla/context/VariableStore.hpp>
#include <xqilla/context/VariableTypeStore.hpp>
#include <xqilla/context/URIResolver.hpp>
#include <xqilla/context/ModuleResolver.hpp>
#include <xqilla/context/ExternalFunctionResolver.hpp>
#include <xqilla/context/Collation.hpp>
#include <xqilla/ast/XQFunction.hpp>
#include <xqilla/ast/XQSequence.hpp>
#include <xqilla/ast/XQVariable.hpp>
#include <xqilla/ast/XQCastAs.hpp>
#include <xqilla/utils/XPath2NSUtils.hpp>
#include <xqilla/utils/XPath2Utils.hpp>
#include <xqilla/utils/ContextUtils.hpp>
#include <xqilla/exceptions/ContextException.hpp>
#include <xqilla/exceptions/NamespaceLookupException.hpp>
#include <xqilla/exceptions/TypeNotFoundException.hpp>
#include <xqilla/exceptions/StaticErrorException.hpp>
#include <xqilla/items/ATDurationOrDerived.hpp>
#include <xqilla/items/Timezone.hpp>
#include <xqilla/items/DatatypeLookup.hpp>
#include <xqilla/items/DatatypeFactory.hpp>
#include <xqilla/functions/FunctionLookup.hpp>
#include <xqilla/functions/XQUserFunction.hpp>
#include <xqilla/dom-api/impl/XQillaNSResolverImpl.hpp>
#include <xqilla/simple-api/XQillaConfiguration.hpp>
#include <xqilla/schema/SequenceType.hpp>

XERCES_CPP_NAMESPACE_USE;

static CodepointCollation g_codepointCollation;

XQContextImpl::XQContextImpl(XQillaConfiguration *conf, XQilla::Language language, MemoryManager* memMgr)
  : _conf(conf),
    _language(language),
    _createdWith(memMgr),
    _internalMM(memMgr),
    _contextItemType(StaticType::ITEM_TYPE, 0, 1),
    _varTypeStore(0),
    _templateNameMap(29, false, &_internalMM),
    _templates(XQillaAllocator<XQUserFunction*>(&_internalMM)),
    _functionTable(0),
    _collations(XQillaAllocator<Collation*>(&_internalMM)),
    _constructionMode(CONSTRUCTION_MODE_PRESERVE),
    _bPreserveBoundarySpace(false),
    _revalidationMode(DocumentCache::VALIDATION_LAX),
    _messageListener(0),
    _module(0),
    _varStore(&_defaultVarStore),
    _globalVarStore(&_defaultVarStore),
    _defaultVarStore(&_internalMM),
    _regexStore(0),
    _resolvers(XQillaAllocator<ResolverEntry>(&_internalMM)),
    _moduleResolver(0),
    _exFuncResolver(0),
    _exFuncs(XQillaAllocator<ExternalFunction*>(&_internalMM)),
    _debugListener(0),
    _stackFrame(0),
    _projection(true),
    _lint(false),
    _tmpVarCounter(0),
    _ftCaseOption(StaticContext::CASE_INSENSITIVE)
{
  _memMgr = &_internalMM;

  ////////////////////
  // static context //
  ////////////////////
  _xpath1Compatibility = false;    // according to Formal Semantics, § 4.1.1
  _ordering = ORDERING_ORDERED;

  _globalNSResolver = new (&_internalMM) XQillaNSResolverImpl(&_internalMM);
  _nsResolver = _globalNSResolver;

  _defaultElementNS = 0;
  // by default, the default namespace for functions is the XPath2 namespace
  _functionNS = XQFunction::XMLChFunctionURI;

  // This is created with the _createdWith memory manager,
  // since a bug in xerces means we can't use a non-thread-safe
  // memory manager - jpcs
  _docCache = conf->createDocumentCache(_createdWith);
  _itemFactory = conf->createItemFactory(_docCache, &_internalMM);
  _docCacheOwned = true;

  if(_varTypeStore==NULL)
    _varTypeStore=_internalMM.createVariableTypeStore();

  if ((language & XQilla::UPDATE) != 0) {
    _functionTable = new (&_internalMM) FunctionLookup(&_internalMM);
    _functionTable->insertUpdateFunctions(&_internalMM);
  }

  // insert the default collation
  addCollation(_internalMM.createCollation(&g_codepointCollation));
  setDefaultCollation(g_codepointCollation.getCollationName());

  _flworOrderingMode = FLWOR_ORDER_EMPTY_LEAST; // implementation-defined

  _bInheritNamespaces = true;
  _bPreserveNamespaces = true;

  _baseURI=0;

  ////////////////////////
  // Evaluation context //
  ////////////////////////

  _contextItem = NULL;

  _contextPosition = 1;
  _contextSize = 1;
  time(&_currentTime);

  // implicit timezone - lazily set in getImplicitTimezone
  _implicitTimezone = 0;

  // Set up the default URIResolver
  _defaultResolver.resolver = conf->createDefaultURIResolver(&_internalMM);
  if(_defaultResolver.resolver != 0) {
    _defaultResolver.adopt = true;
  }
}

XQContextImpl::~XQContextImpl()
{
  _varTypeStore->clear();
  _contextItem = 0;
  _implicitTimezone = 0;
  _defaultVarStore.clear();

  delete _varTypeStore;
  if(_docCacheOwned) {
    delete _itemFactory;
    delete _docCache;
  }

  std::vector<ResolverEntry, XQillaAllocator<ResolverEntry> >::reverse_iterator end = _resolvers.rend();
  for(std::vector<ResolverEntry, XQillaAllocator<ResolverEntry> >::reverse_iterator i = _resolvers.rbegin(); i != end; ++i) {
    if(i->adopt) {
      delete i->resolver;
    }
  }
  if(_defaultResolver.adopt)
    delete _defaultResolver.resolver;

  std::vector<ExternalFunction*, XQillaAllocator<ExternalFunction*> >::iterator end2 = _exFuncs.end();
  for(std::vector<ExternalFunction*, XQillaAllocator<ExternalFunction*> >::iterator j = _exFuncs.begin(); j != end2; ++j) {
    delete *j;
  }
}

DynamicContext *XQContextImpl::createModuleContext(MemoryManager *memMgr) const
{
  DynamicContext* moduleCtx = new (memMgr) XQContextImpl(_conf, _language, memMgr);

  // Force the context to use our memory manager
  moduleCtx->setMemoryManager(getMemoryManager());

  // Set the ModuleResolver
  moduleCtx->setModuleResolver(_moduleResolver);

  // Set the ExternalFunctionResolver
  moduleCtx->setExternalFunctionResolver(_exFuncResolver);

  // Set the MessageListener
  moduleCtx->setMessageListener(_messageListener);

  // Set the DebugListener
  moduleCtx->setDebugListener(_debugListener);

  // Add our collations
  for(std::vector<Collation*, XQillaAllocator<Collation*> >::const_iterator it= _collations.begin(); it!=_collations.end(); ++it)
    moduleCtx->addCollation(*it);

  // Add the external functions
  if(_functionTable) {
    _functionTable->copyExternalFunctionsTo(moduleCtx);
  }

  _conf->populateStaticContext(moduleCtx);
  return moduleCtx;
}

DynamicContext *XQContextImpl::createModuleDynamicContext(const DynamicContext* moduleCtx, MemoryManager *memMgr) const
{
  XQDynamicContextImpl* moduleDCtx = new (memMgr) XQDynamicContextImpl(_conf, moduleCtx, (VarStoreImpl*)&_defaultVarStore, memMgr);

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

DynamicContext *XQContextImpl::createDynamicContext(MemoryManager *memMgr) const
{
  DynamicContext *result = new (memMgr) XQDynamicContextImpl(_conf, this, 0, memMgr);
  _conf->populateDynamicContext(result);
  return result;
}

DynamicContext *XQContextImpl::createDebugQueryContext(const Item::Ptr &contextItem,
                                                       size_t contextPosition,
                                                       size_t contextSize,
                                                       const VariableStore *variables,
                                                       const DOMXPathNSResolver *nsResolver,
                                                       const XMLCh *defaultElementNS,
                                                       MemoryManager *memMgr) const
{
  XQContextImpl *result = new (memMgr) XQContextImpl(_conf,
                                                     (XQilla::Language)(XQilla::XQUERY | XQilla::EXTENSIONS),
                                                     memMgr);

  // Set up the static type of all the in-scope variables
  VariableTypeStore *store = result->getVariableTypeStore();
  XPath2MemoryManager *rmm = result->getMemoryManager();

  // For simplicity we'll make them all have type item()*
  StaticAnalysis *src = new (rmm) StaticAnalysis(rmm);
  src->getStaticType() = StaticType(StaticType::ITEM_TYPE, 0, StaticType::UNLIMITED);

  std::vector<std::pair<const XMLCh *, const XMLCh*> > inScopeVars;
  variables->getInScopeVariables(inScopeVars);
  std::vector<std::pair<const XMLCh *, const XMLCh*> >::iterator i = inScopeVars.begin();
  for(; i != inScopeVars.end(); ++i) {
    store->declareGlobalVar(i->first, i->second, *src, 0);
  }

  // Set up all the in-scope namespaces
  XQillaNSResolver *newresolver = new (rmm) XQillaNSResolverImpl(rmm, nsResolver);
  result->_globalNSResolver = newresolver;
  result->_nsResolver = newresolver;

  result->_defaultElementNS = defaultElementNS;
  result->_functionNS = _functionNS;

  // Set up the functions
  result->_functionTable = _functionTable;

  // Set up the schema definitions
  if(result->_docCacheOwned) {
    delete result->_itemFactory;
    delete result->_docCache;
  }

  result->_docCache = _docCache;
  result->_itemFactory = _itemFactory;
  result->_docCacheOwned = false;

  // Set up the focus and in-scope variable values
  result->setContextItem(contextItem);
  result->setContextPosition(contextPosition);
  result->setContextSize(contextSize);
  result->setVariableStore(variables);

  // TBD regex groups? - jpcs

  return result;
}

XQilla::Language XQContextImpl::getLanguage() const
{
  return _language;
}

void XQContextImpl::setMemoryManager(XPath2MemoryManager* memMgr)
{
  _memMgr = memMgr;
}

void XQContextImpl::setNamespaceBinding(const XMLCh* prefix, const XMLCh* uri)
{
  XQillaNSResolverImpl::forbiddenBindingCheck(prefix, uri, 0);
  _globalNSResolver->addNamespaceBinding(prefix,uri);
}

void XQContextImpl::setPreserveBoundarySpace(bool value)
{
  _bPreserveBoundarySpace=value;
}

bool XQContextImpl::getPreserveBoundarySpace() const
{
  return _bPreserveBoundarySpace;
}

void XQContextImpl::setInheritNamespaces(bool value)
{
    _bInheritNamespaces=value;
}

bool XQContextImpl::getInheritNamespaces() const
{
    return _bInheritNamespaces;
}

void XQContextImpl::setPreserveNamespaces(bool value)
{
    _bPreserveNamespaces=value;
}

bool XQContextImpl::getPreserveNamespaces() const
{
    return _bPreserveNamespaces;
}

void XQContextImpl::clearDynamicContext()
{
  _nsResolver = _globalNSResolver;
  _contextItem = 0;
  _contextSize = 1;
  _contextPosition = 1;
  _implicitTimezone = 0;

  _defaultVarStore.clear();
  _varStore = &_defaultVarStore;
  _globalVarStore = &_defaultVarStore;

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

const StaticType &XQContextImpl::getContextItemType() const
{
  return _contextItemType;
}

void XQContextImpl::setContextItemType(const StaticType &st)
{
  _contextItemType = st;
}

bool XQContextImpl::getXPath1CompatibilityMode() const
{
  return _xpath1Compatibility;
}

void XQContextImpl::setXPath1CompatibilityMode(bool newMode)
{
  _xpath1Compatibility=newMode;
}

const XMLCh* XQContextImpl::getDefaultFuncNS() const
{
  return _functionNS;
}

void XQContextImpl::setDefaultFuncNS(const XMLCh* newNS)
{
  _functionNS=newNS;
}

const XMLCh* XQContextImpl::getBaseURI() const
{
  return _baseURI;
}

void XQContextImpl::setBaseURI(const XMLCh* newURI)
{
  _baseURI = _internalMM.getPooledString(newURI);
}

StaticContext::ConstructionMode XQContextImpl::getConstructionMode() const
{
    return _constructionMode;
}

void XQContextImpl::setConstructionMode(ConstructionMode newMode)
{
    _constructionMode=newMode;
}

StaticContext::NodeSetOrdering XQContextImpl::getNodeSetOrdering() const
{
    return _ordering;
}

void XQContextImpl::setNodeSetOrdering(NodeSetOrdering newOrder)
{
    _ordering=newOrder;
}

StaticContext::FLWOROrderingMode XQContextImpl::getDefaultFLWOROrderingMode() const
{
    return _flworOrderingMode;
}

void XQContextImpl::setDefaultFLWOROrderingMode(StaticContext::FLWOROrderingMode newMode)
{
    _flworOrderingMode=newMode;
}

time_t XQContextImpl::getCurrentTime() const
{
  return _currentTime;
}

void XQContextImpl::setCurrentTime(time_t newTime)
{
  _currentTime=newTime;
}

ATDurationOrDerived::Ptr XQContextImpl::getImplicitTimezone() const {
  if(_implicitTimezone == NULLRCP) {
    // validate tzone
    Timezone tzone(ContextUtils::getTimezone());
    // c-style the const away since we are in a const method
    ((XQContextImpl*)this)->_implicitTimezone = tzone.asDayTimeDuration(this);
  }

  return _implicitTimezone;
}

void XQContextImpl::setImplicitTimezone(const ATDurationOrDerived::Ptr &timezoneAsDuration) {
  // validate the timezone -- will trow if invalid
  Timezone timezone(timezoneAsDuration, this);
  this->_implicitTimezone = timezoneAsDuration;
}

void XQContextImpl::setNSResolver(const DOMXPathNSResolver* resolver) {
  _nsResolver = resolver;
}

const DOMXPathNSResolver* XQContextImpl::getNSResolver() const {
  return _nsResolver;
}

const XMLCh* XQContextImpl::getUriBoundToPrefix(const XMLCh* prefix, const LocationInfo *location) const
{
  if(prefix == 0 || *prefix == 0) return 0;

  const XMLCh* uri = _nsResolver->lookupNamespaceURI(prefix);

  if((uri == 0 || *uri == 0) && prefix != 0 && *prefix != 0) {
    const XMLCh* msg = XPath2Utils::concatStrings(X("No namespace for prefix \'"), prefix, X("\' [err:XPST0081]"), getMemoryManager());
    XQThrow3(NamespaceLookupException, X("XQContextImpl::getUriBoundToPrefix"), msg, location);
  }

  return uri;
}

const XMLCh* XQContextImpl::getPrefixBoundToUri(const XMLCh* uri) const
{
  return _nsResolver->lookupPrefix(uri);
}

void XQContextImpl::setContextItem(const Item::Ptr &item)
{
  _contextItem = item;
}

void XQContextImpl::addTemplate(XQUserFunction *tp)
{
  if(tp->getName() != 0) {
    if(_templateNameMap.containsKey((void*)tp->getURINameHash())) {
      // [ERR XTSE0660] It is a static error if a stylesheet contains more than one template with
      // the same name and the same import precedence, unless it also contains a template with the
      // same name and higher import precedence.
      XMLBuffer buf;
      buf.set(X("Multiple templates have the same expanded QName {"));
      buf.append(tp->getURI());
      buf.append(X("}"));
      buf.append(tp->getName());
      buf.append(X(" [err:XTSE0660]."));
      XQThrow3(StaticErrorException,X("XQContextImpl::addNamedTemplate"), buf.getRawBuffer(), tp);
    }
    _templateNameMap.put((void*)tp->getURINameHash(), tp);
  }
  _templates.push_back(tp);
}

const XQUserFunction *XQContextImpl::lookUpNamedTemplate(const XMLCh *uri, const XMLCh *name) const
{
  XMLBuffer key;
  key.set(name);
  key.append(uri);
  return _templateNameMap.get(key.getRawBuffer());
}

const UserFunctions &XQContextImpl::getTemplateRules() const
{
  return _templates;
}

void XQContextImpl::addCustomFunction(FuncFactory *func)
{
  if(_functionTable==NULL)
    _functionTable = new (&_internalMM) FunctionLookup(&_internalMM);
  _functionTable->insertFunction(func);
}

void XQContextImpl::removeCustomFunction(FuncFactory *func)
{
  if(_functionTable != NULL)
    _functionTable->removeFunction(func);
}

void XQContextImpl::setContextSize(size_t size)
{
  _contextSize = size;
}

void XQContextImpl::setContextPosition(size_t pos)
{
  _contextPosition = pos;
}

Item::Ptr XQContextImpl::getContextItem() const
{
  return _contextItem;
}

const VariableStore* XQContextImpl::getVariableStore() const
{
  return _varStore;
}

void XQContextImpl::setVariableStore(const VariableStore *store)
{
  assert(store);
  _varStore = store;
}

const VariableStore* XQContextImpl::getGlobalVariableStore() const
{
  return _globalVarStore;
}

void XQContextImpl::setGlobalVariableStore(const VariableStore *store)
{
  _globalVarStore = store;
}

void XQContextImpl::setExternalVariable(const XMLCh *namespaceURI, const XMLCh *name, const Result &value)
{
  _defaultVarStore.setVar(namespaceURI, name, value);
}

void XQContextImpl::setExternalVariable(const XMLCh *qname, const Result &value)
{
  const XMLCh *uri = getUriBoundToPrefix(XPath2NSUtils::getPrefix(qname, getMemoryManager()), 0);
  const XMLCh *name = XPath2NSUtils::getLocalName(qname);

  _defaultVarStore.setVar(uri, name, value);
}

VariableTypeStore* XQContextImpl::getVariableTypeStore()
{
  return _varTypeStore;
}

const RegexGroupStore *XQContextImpl::getRegexGroupStore() const
{
  return _regexStore;
}

void XQContextImpl::setRegexGroupStore(const RegexGroupStore *store)
{
  _regexStore = store;
}

size_t XQContextImpl::getContextSize() const
{
  return _contextSize;
}

size_t XQContextImpl::getContextPosition() const
{
  return _contextPosition;
}

XPath2MemoryManager* XQContextImpl::getMemoryManager() const
{
  return _memMgr;
}

const XMLCh* XQContextImpl::getDefaultElementAndTypeNS() const
{
  return _defaultElementNS;
}

void XQContextImpl::setDefaultElementAndTypeNS(const XMLCh* newNS) {
  _defaultElementNS = newNS;
}

void XQContextImpl::addCollation(Collation* collation)
{
  _collations.push_back(collation);
}

void XQContextImpl::setDefaultCollation(const XMLCh* URI)
{
  _defaultCollation=getMemoryManager()->getPooledString(URI);
}

Collation* XQContextImpl::getCollation(const XMLCh* URI, const LocationInfo *location) const
{
  if(!XMLUri::isValidURI(false, URI))
  {
    const XMLCh* baseURI=getBaseURI();
    if(baseURI && *baseURI)
    {
      try
      {
        XMLUri base(baseURI, getMemoryManager());
        XMLUri full(&base, URI, getMemoryManager());
        URI = getMemoryManager()->getPooledString(full.getUriText());
      }
      catch(XMLException &e)
      {
        //if can't build, assume it's because there was an invalid base URI, so use the original URI
      }
    }
  }
  for(std::vector<Collation*, XQillaAllocator<Collation*> >::const_iterator it= _collations.begin(); it!=_collations.end(); ++it)
    if(XPath2Utils::equals((*it)->getCollationName(), URI))
      return (*it);
  const XMLCh* msg = XPath2Utils::concatStrings(X("The requested collation ('"), URI, X("') is not defined [err:FOCH0002]"), getMemoryManager());

  XQThrow3(ContextException, X("XQContextImpl::getCollation"), msg, location);
  return NULL;
}

Collation* XQContextImpl::getDefaultCollation(const LocationInfo *location) const
{
  return getCollation(_defaultCollation, location);
}

ASTNode *XQContextImpl::lookUpFunction(const XMLCh *uri, const XMLCh* name, const VectorOfASTNodes &v, const LocationInfo *location) const
{
  ASTNode* functionImpl = FunctionLookup::lookUpGlobalFunction(uri, name, v, getMemoryManager(), _functionTable);
  if(functionImpl) {
    functionImpl->setLocationInfo(location);
    return functionImpl;
  }

  if(v.size() != 1) return 0;

  // maybe it's not a function, but a datatype
  try {
    bool isPrimitive;
    _itemFactory->getPrimitiveTypeIndex(uri, name, isPrimitive);
  }
  catch(TypeNotFoundException&) {
    // ignore this exception: it means the type has not been found
    return 0;
  }

  if((XPath2Utils::equals(name, XMLUni::fgNotationString) || XPath2Utils::equals(name, AnyAtomicType::fgDT_ANYATOMICTYPE)) &&
     XPath2Utils::equals(uri, SchemaSymbols::fgURI_SCHEMAFORSCHEMA))
    return 0;

  XPath2MemoryManager *mm = getMemoryManager();

  SequenceType *seqType = new (mm) SequenceType(uri, name, SequenceType::QUESTION_MARK, mm);
  seqType->setLocationInfo(location);

  functionImpl = new (mm) XQCastAs(v[0], seqType, mm);
  functionImpl->setLocationInfo(location);
  return functionImpl;
}

void XQContextImpl::addExternalFunction(const ExternalFunction *func)
{
  if(_functionTable==NULL)
    _functionTable = new (&_internalMM) FunctionLookup(&_internalMM);
  _functionTable->insertExternalFunction(func);
}

const ExternalFunction *XQContextImpl::lookUpExternalFunction(const XMLCh *uri, const XMLCh *name,
                                                              size_t numArgs) const
{
  if(_exFuncResolver) {
    ExternalFunction *result = _exFuncResolver->resolveExternalFunction(uri, name, numArgs, this);
    if(result != 0) {
      // Store the ExternalFunction so we can delete it later
      const_cast<XQContextImpl*>(this)->_exFuncs.push_back(result);
      return result;
    }
  }

  return FunctionLookup::lookUpGlobalExternalFunction(uri, name, numArgs, _functionTable);
}

void XQContextImpl::setXMLEntityResolver(XMLEntityResolver* const handler)
{
  _docCache->setXMLEntityResolver(handler);
}

XMLEntityResolver* XQContextImpl::getXMLEntityResolver() const
{
  return _docCache->getXMLEntityResolver();
}

void XQContextImpl::registerURIResolver(URIResolver *resolver, bool adopt)
{
  if(resolver != 0) {
    _resolvers.push_back(ResolverEntry(resolver, adopt));
  }
}

URIResolver *XQContextImpl::getDefaultURIResolver() const
{
  return _defaultResolver.resolver;
}

void XQContextImpl::setDefaultURIResolver(URIResolver *resolver, bool adopt)
{
  if(_defaultResolver.adopt) {
    delete _defaultResolver.resolver;
  }
  _defaultResolver.resolver = resolver;
  _defaultResolver.adopt = adopt;
}

Node::Ptr XQContextImpl::parseDocument(InputSource &srcToUse, const LocationInfo *location,
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

Sequence XQContextImpl::resolveDocument(const XMLCh* uri, const LocationInfo *location,
                                        const QueryPathNode *projection)
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

Sequence XQContextImpl::resolveCollection(const XMLCh* uri, const LocationInfo *location, const QueryPathNode *projection)
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

Sequence XQContextImpl::resolveDefaultCollection(const QueryPathNode *projection)
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

bool XQContextImpl::putDocument(const Node::Ptr &document, const XMLCh *uri)
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

void XQContextImpl::setModuleResolver(ModuleResolver *resolver)
{
  _moduleResolver=resolver;
}

ModuleResolver * XQContextImpl::getModuleResolver() const
{
  return _moduleResolver;
}

VectorOfStrings* XQContextImpl::resolveModuleURI(const XMLCh* uri) const
{
  VectorOfStrings* vect=new (getMemoryManager()) VectorOfStrings(XQillaAllocator<const XMLCh*>(getMemoryManager()));
  if(_moduleResolver)
    _moduleResolver->resolveModuleLocation(vect, uri, this);
  return vect;
}

void XQContextImpl::setExternalFunctionResolver(ExternalFunctionResolver *resolver)
{
  _exFuncResolver=resolver;
}

ExternalFunctionResolver *XQContextImpl::getExternalFunctionResolver() const
{
  return _exFuncResolver;
}

/*
 * returns true if the type represented by uri:typename is an instance of uriToCheck:typeNameToCheck
 *
 * ie: to check
 * xs:integer instance of xs:decimal,
 * call
 * isTypeOrDerivedFromType("xs", "integer", "xs", "decimal")
 * (except of course, call with URIs, not prefixes!)
 */
bool XQContextImpl::isTypeOrDerivedFromType(const XMLCh* uri, const XMLCh* typeName, const XMLCh* uriToCheck, const XMLCh* typeNameToCheck) const
{
  return _docCache->isTypeOrDerivedFromType(uri,typeName,uriToCheck,typeNameToCheck);
}

void XQContextImpl::addSchemaLocation(const XMLCh* uri, VectorOfStrings* locations, const LocationInfo *location)
{
  _docCache->addSchemaLocation(uri, locations, this, location);
}

DocumentCache* XQContextImpl::getDocumentCache() const
{
  return _docCache;
}

void XQContextImpl::setDocumentCache(DocumentCache* docCache)
{
    _docCache = docCache;
}

SequenceBuilder *XQContextImpl::createSequenceBuilder() const
{
  return _conf->createSequenceBuilder(this);
}

ItemFactory *XQContextImpl::getItemFactory() const
{
  return _itemFactory;
}

void XQContextImpl::setItemFactory(ItemFactory *factory)
{
  _itemFactory = factory;
}

UpdateFactory *XQContextImpl::createUpdateFactory() const
{
  return _conf->createUpdateFactory(const_cast<ProxyMemoryManager*>(&_internalMM));
}

void XQContextImpl::setRevalidationMode(DocumentCache::ValidationMode mode)
{
  _revalidationMode = mode;
}

DocumentCache::ValidationMode XQContextImpl::getRevalidationMode() const
{
  return _revalidationMode;
}

void XQContextImpl::setMessageListener(MessageListener *listener)
{
  _messageListener = listener;
}

MessageListener *XQContextImpl::getMessageListener() const
{
  return _messageListener;
}

void XQContextImpl::setDebugListener(DebugListener *listener)
{
  _debugListener = listener;
}

DebugListener *XQContextImpl::getDebugListener() const
{
  return _debugListener;
}

void XQContextImpl::setStackFrame(const StackFrame *frame)
{
  _stackFrame = frame;
}

const StackFrame *XQContextImpl::getStackFrame() const
{
  return _stackFrame;
}

void XQContextImpl::testInterrupt() const
{
  _conf->testInterrupt();
}

const XMLCh *XQContextImpl::allocateTempVarName(const XMLCh *prefix)
{
  static XMLCh default_prefix[] = { 't', 'm', 'p', 0 };

  if(prefix == 0) prefix = default_prefix;

  XMLBuffer buf(20);
  buf.append('#');
  buf.append(prefix);
  XPath2Utils::numToBuf(_tmpVarCounter, buf);

  ++_tmpVarCounter;
  return getMemoryManager()->getPooledString(buf.getRawBuffer());
}


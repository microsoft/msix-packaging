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

#include <xqilla/simple-api/XQQuery.hpp>
#include <xqilla/simple-api/XQillaConfiguration.hpp>
#include <xqilla/framework/XQillaExport.hpp>
#include <xqilla/simple-api/XQilla.hpp>
#include <xqilla/context/DynamicContext.hpp>
#include <xqilla/context/VariableStore.hpp>
#include <xqilla/context/VariableTypeStore.hpp>
#include <xqilla/context/ContextHelpers.hpp>
#include <xqilla/functions/FunctionLookup.hpp>
#include <xqilla/functions/XQUserFunction.hpp>
#include <xqilla/functions/FunctionSignature.hpp>
#include <xqilla/exceptions/XQException.hpp>
#include <xqilla/exceptions/XPath2ErrorException.hpp>
#include <xqilla/exceptions/StaticErrorException.hpp>
#include <xqilla/items/Node.hpp>
#include <xqilla/ast/XQGlobalVariable.hpp>
#include <xqilla/ast/XQSequence.hpp>
#include <xqilla/ast/StaticAnalysis.hpp>
#include <xqilla/runtime/Result.hpp>
#include <xqilla/utils/XPath2Utils.hpp>
#include <xqilla/utils/XPath2NSUtils.hpp>
#include <xqilla/optimizer/ASTToXML.hpp>
#include <xqilla/events/EventHandler.hpp>
#include <xqilla/optimizer/StaticTyper.hpp>
#include <xqilla/functions/XQillaFunction.hpp>
#include <xqilla/context/MessageListener.hpp>

#include <xercesc/util/XMLUni.hpp>
#include <xercesc/util/XMLURL.hpp>
#include <xercesc/util/XMLResourceIdentifier.hpp>
#include <xercesc/util/XMLEntityResolver.hpp>
#include <xercesc/framework/URLInputSource.hpp>
#include <xercesc/framework/LocalFileInputSource.hpp>
#include <xercesc/util/Janitor.hpp>

#include <sstream>
#include <list>

using namespace std;

#if defined(XERCES_HAS_CPP_NAMESPACE)
XERCES_CPP_NAMESPACE_USE
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////

ModuleCache::ModuleCache(MemoryManager *mm)
  : byURI_(11, true, mm),
    byNamespace_(11, false, mm),
    ordered_(XQillaAllocator<XQQuery*>(mm))
{
}

void ModuleCache::put(XQQuery *module)
{
  assert(!byURI_.get(module->getFile()));

  byURI_.put((void*)module->getFile(), module);

  XQQuery *cached = byNamespace_.get(module->getModuleTargetNamespace());
  if(cached) {
    while(cached->getNext() != 0) {
      cached = cached->getNext();
    }
    cached->setNext(module);
  }
  else {
    byNamespace_.put((void*)module->getModuleTargetNamespace(), module);
  }
}

XQQuery *ModuleCache::getByURI(const XMLCh *uri) const
{
  return (XQQuery*)byURI_.get(uri);
}

XQQuery *ModuleCache::getByNamespace(const XMLCh *ns) const
{
  return (XQQuery*)byNamespace_.get(ns);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

XQQuery::XQQuery(DynamicContext *context, bool contextOwned, ModuleCache *moduleCache,
                 MemoryManager *memMgr)
  : m_memMgr(memMgr),
    m_context(context),
    m_contextOwned(contextOwned),
    m_query(NULL),
    m_bIsLibraryModule(false),
    m_szTargetNamespace(NULL),
    m_szQueryText(0),
    m_szCurrentFile(NULL),
    m_userDefFns(XQillaAllocator<XQUserFunction*>(memMgr)),
    m_delayedFunctions(XQillaAllocator<DelayedFuncFactory*>(memMgr)),
    m_userDefVars(XQillaAllocator<XQGlobalVariable*>(memMgr)),
    m_importedModules(XQillaAllocator<XQQuery*>(memMgr)),
    m_moduleCache(moduleCache ? moduleCache : new (memMgr) ModuleCache(memMgr)),
    m_moduleCacheOwned(moduleCache == 0),
    m_version3(context->getLanguage() & XQilla::VERSION3),
    m_staticTyped(BEFORE),
    m_next(0)
{
  context->setModule(this);
}

XQQuery::~XQQuery()
{
  if(m_moduleCacheOwned)
    delete m_moduleCache;
  if(m_contextOwned)
    delete m_context;
}

DynamicContext *XQQuery::createDynamicContext(MemoryManager *memMgr) const
{
  return m_context->createDynamicContext(memMgr);
}

Result XQQuery::execute(const Item::Ptr &contextItem, DynamicContext *context) const
{
  context->setContextItem(contextItem);
  return execute(context);
}

Result XQQuery::execute(const XMLCh *templateQName, DynamicContext *context) const
{
  Item::Ptr value = context->getItemFactory()->createUntypedAtomic(templateQName, context);
  context->setExternalVariable(XQillaFunction::XMLChFunctionURI, X("name"), value);
  return execute(context);
}

void XQQuery::executeProlog(DynamicContext *context) const
{
  try {
    // Execute the imported modules
    if(m_moduleCacheOwned) {
      for(ImportedModules::const_iterator modIt = m_moduleCache->ordered_.begin(); modIt != m_moduleCache->ordered_.end(); ++modIt) {
        // Derive the module's execution context from it's static context
        AutoDelete<DynamicContext> moduleCtx(context->createModuleDynamicContext((*modIt)->getStaticContext(),
                                                                                 context->getMemoryManager()));
        (*modIt)->executeProlog(moduleCtx);
      }
    }

    // define global variables
    for(GlobalVariables::const_iterator it3 = m_userDefVars.begin(); it3 != m_userDefVars.end(); ++it3)
      (*it3)->execute(context);
  }
  catch(XQException& e) {
    if(e.getXQueryLine() == 0) {
      e.setXQueryPosition(m_szCurrentFile, 1, 1);
    }
    throw e;
  }
}

void XQQuery::execute(EventHandler *events, DynamicContext* context) const
{
  executeProlog(context);

  if(m_query != NULL) {
    try {
      // execute the query body
      m_query->generateAndTailCall(events, context, true, true);
      events->endEvent();
    }
    catch(XQException& e) {
      if(e.getXQueryLine() == 0) {
        e.setXQueryPosition(m_szCurrentFile, 1, 1);
      }
      throw e;
    }
  }
}

void XQQuery::execute(EventHandler *events, const Item::Ptr &contextItem, DynamicContext *context) const
{
  context->setContextItem(contextItem);
  execute(events, context);
}

void XQQuery::execute(EventHandler *events, const XMLCh *templateQName, DynamicContext *context) const
{
  Item::Ptr value = context->getItemFactory()->createUntypedAtomic(templateQName, context);
  context->setExternalVariable(XQillaFunction::XMLChFunctionURI, X("name"), value);
  execute(events, context);
}

XQQuery *XQQuery::findModuleForFunction(const XMLCh *uri, const XMLCh *name, int numArgs)
{
  UserFunctions::iterator itFn = m_userDefFns.begin();
  for(; itFn != m_userDefFns.end(); ++itFn) {
    if(*itFn && XPath2Utils::equals(name, (*itFn)->getName()) &&
       XPath2Utils::equals(uri, (*itFn)->getURI()) &&
       (*itFn)->getMinArgs() == (size_t)numArgs) {
      return this;
    }
  }

  ImportedModules::const_iterator modIt;
  for(modIt = m_importedModules.begin(); modIt != m_importedModules.end(); ++modIt) {
    if(XPath2Utils::equals(uri, (*modIt)->getModuleTargetNamespace())) {
      XQQuery *module = *modIt;
      for(; module; module = module->getNext()) {
        itFn = module->m_userDefFns.begin();
        for(; itFn != module->m_userDefFns.end(); ++itFn) {
          if(*itFn && XPath2Utils::equals(name, (*itFn)->getName()) &&
             (*itFn)->getMinArgs() == (size_t)numArgs) {
            return module;
          }
        }
      }
    }
  }

  return 0;
}

XQQuery *XQQuery::findModuleForVariable(const XMLCh *uri, const XMLCh *name)
{
  GlobalVariables::iterator itVar = m_userDefVars.begin();
  for(; itVar != m_userDefVars.end(); ++itVar) {
    if(*itVar && XPath2Utils::equals(name, (*itVar)->getVariableLocalName()) &&
       XPath2Utils::equals(uri, (*itVar)->getVariableURI())) {
      return this;
    }
  }

  ImportedModules::const_iterator modIt;
  for(modIt = m_importedModules.begin(); modIt != m_importedModules.end(); ++modIt) {
    if(XPath2Utils::equals(uri, (*modIt)->getModuleTargetNamespace())) {
      XQQuery *module = *modIt;
      for(; module; module = module->getNext()) {
        itVar = module->m_userDefVars.begin();
        for(; itVar != module->m_userDefVars.end(); ++itVar) {
          if(*itVar && XPath2Utils::equals(name, (*itVar)->getVariableLocalName())) {
            return module;
          }
        }
      }
    }
  }

  return 0;
}

static void duplicateVariableError(const XQGlobalVariable *existing, const XQGlobalVariable *bad,
                                   MessageListener *mlistener)
{
  if(mlistener) {
    mlistener->warning(X("In the context of this variable declaration"), existing);
  }
  XMLBuffer buf;
  buf.set(X("A variable with name {"));
  buf.append(bad->getVariableURI());
  buf.append(X("}"));
  buf.append(bad->getVariableLocalName());
  buf.append(X(" conflicts with an existing global variable [err:XQST0049]."));
  XQThrow3(StaticErrorException, X("XQQuery::staticResolution"), buf.getRawBuffer(), bad);
}

void XQQuery::staticResolution()
{
  MessageListener *mlistener = m_context->getMessageListener();

  // Statically resolve all the modules
  if(m_moduleCacheOwned) {

    RefHashTableOfEnumerator<XQQuery> en(&m_moduleCache->byURI_);
    while(en.hasMoreElements()) {
      XQQuery *module = &en.nextElement();
      if(module != this)
        module->staticResolution();
    }
  }

  // Deal with the module imports
  ImportedModules::const_iterator modIt;
  GlobalVariables::iterator itVar;
  for(modIt = m_importedModules.begin(); modIt != m_importedModules.end(); ++modIt) {
    XQQuery *module = *modIt;

    // Add variables from this module in the module namespace to the map
    RefHashTableOf<XQGlobalVariable> nsVars(11, false);
    for(itVar = m_userDefVars.begin(); itVar != m_userDefVars.end(); ++itVar) {
      if(XPath2Utils::equals((*itVar)->getVariableURI(), module->getModuleTargetNamespace())) {
        XQGlobalVariable *existing = nsVars.get((*itVar)->getVariableLocalName());
        if(existing) duplicateVariableError(existing, *itVar, mlistener);
        nsVars.put((void*)(*itVar)->getVariableLocalName(), *itVar);
      }
    }

    // Import the functions from the modules with this target namespace
    for(; module; module = module->getNext()) {
      if(module == this) continue;

      const DynamicContext *moduleCtx = module->getStaticContext();

      // Move the function definitions into my context
      for(UserFunctions::iterator itFn = module->m_userDefFns.begin(); itFn != module->m_userDefFns.end(); ++itFn) {
        (*itFn)->setModuleDocumentCache(const_cast<DocumentCache*>(moduleCtx->getDocumentCache()));
        if((*itFn)->isTemplate()) {
          m_context->addTemplate(*itFn);
        }
        else if((*itFn)->getName() && (*itFn)->getSignature()->privateOption != FunctionSignature::OP_TRUE) {
          m_context->addCustomFunction(*itFn);
        }
      }

      // Move the delayed function definitions into my context
      for(DelayedFunctions::iterator itd = module->m_delayedFunctions.begin(); itd != module->m_delayedFunctions.end(); ++itd) {
        if(!(*itd)->isParsed()) {
          m_context->addCustomFunction(*itd);
        }
      }

      // Check that global variable names don't clash
      for(itVar = module->m_userDefVars.begin(); itVar != module->m_userDefVars.end(); ++itVar) {
        XQGlobalVariable *existing = nsVars.get((*itVar)->getVariableLocalName());
        if(existing) duplicateVariableError(existing, *itVar, mlistener);
        nsVars.put((void*)(*itVar)->getVariableLocalName(), *itVar);
      }
    }
  }

  // Run staticResolutionStage1 on the user defined functions,
  // which gives them the static type they were defined with
  UserFunctions::iterator i;
  for(i = m_userDefFns.begin(); i != m_userDefFns.end(); ++i) {
    (*i)->staticResolutionStage1(m_context);

    if(getIsLibraryModule() && !(*i)->isTemplate() && !XERCES_CPP_NAMESPACE::XMLString::equals((*i)->getURI(), getModuleTargetNamespace()))
      XQThrow3(StaticErrorException,X("XQQuery::staticResolution"), X("Every function in a module must be in the module namespace [err:XQST0048]."), *i);

    if((*i)->isTemplate()) {
      m_context->addTemplate(*i);
    }
    else if((*i)->getName() && !(*i)->isDelayed()) {
      m_context->addCustomFunction(*i);
    }
  }

  // Run staticResolution on the global variables
  for(itVar = m_userDefVars.begin(); itVar != m_userDefVars.end(); ++itVar) {
    (*itVar)->staticResolution(m_context);
    if(getIsLibraryModule() && !XPath2Utils::equals((*itVar)->getVariableURI(), getModuleTargetNamespace()))
      XQThrow3(StaticErrorException,X("XQQuery::staticResolution"),
               X("Every global variable in a module must be in the module namespace [err:XQST0048]."), *itVar);
  }

  // check for duplicate variable declarations
  for(itVar = m_userDefVars.begin(); itVar != m_userDefVars.end(); ++itVar) {
    for(GlobalVariables::iterator it2 = itVar+1; it2 != m_userDefVars.end(); ++it2) {
      if(XPath2Utils::equals((*itVar)->getVariableURI(), (*it2)->getVariableURI()) &&
         XPath2Utils::equals((*itVar)->getVariableLocalName(), (*it2)->getVariableLocalName())) {
        duplicateVariableError(*itVar, *it2, mlistener);
      }
    }
  }

  // Run staticResolutionStage2 on the user defined functions,
  // which statically resolves their function bodies
  for(i = m_userDefFns.begin(); i != m_userDefFns.end(); ++i) {
    (*i)->staticResolutionStage2(m_context);
  }

  // Run static resolution on the query body
  if(m_query) m_query = m_query->staticResolution(m_context);
}

bool XQQuery::staticTypingOnce(StaticTyper *styper)
{
  switch(m_staticTyped) {
  case AFTER: return false;
  case DURING: {
      XMLBuffer buf;
      buf.set(X("The graph of module imports contains a cycle for namespace '"));
      buf.append(getModuleTargetNamespace());
      buf.append(X("' [err:XQST0093]"));
      XQThrow2(StaticErrorException, X("XQQuery::staticResolution"), buf.getRawBuffer());
  }
  case BEFORE: break;
  }

  m_staticTyped = DURING;

  staticTyping(styper);

  m_staticTyped = AFTER;
  m_moduleCache->ordered_.push_back(this);

  return true;
}

void XQQuery::staticTyping(StaticTyper *styper)
{
  StaticTyper defaultTyper;
  if(styper == 0) styper = &defaultTyper;

  // Static type the imported modules (again)
  ImportedModules::const_iterator modIt;
  if(m_moduleCacheOwned) {
    for(modIt = m_moduleCache->ordered_.begin(); modIt != m_moduleCache->ordered_.end(); ++modIt) {
      (*modIt)->staticTyping(styper);
    }
  }

  // Define types for the imported variables
  VariableTypeStore* varStore = m_context->getVariableTypeStore();
  GlobalVariables::const_iterator varIt;
  for(modIt = m_importedModules.begin(); modIt != m_importedModules.end(); ++modIt) {
    XQQuery *module = *modIt;
    for(; module; module = module->getNext()) {
      for(varIt = module->m_userDefVars.begin(); varIt != module->m_userDefVars.end(); ++varIt) {
        varStore->declareGlobalVar((*varIt)->getVariableURI(), (*varIt)->getVariableLocalName(),
                                   (*varIt)->getStaticAnalysis(), *varIt);
      }
    }
  }

  // Set up a default type for the global variables
  for(varIt = m_userDefVars.begin(); varIt != m_userDefVars.end(); ++varIt) {
    (*varIt)->resetStaticTypingOnce();
    varStore->declareGlobalVar((*varIt)->getVariableURI(), (*varIt)->getVariableLocalName(),
                               (*varIt)->getStaticAnalysis(), *varIt);
  }

  UserFunctions::const_iterator i, j;
  {
    GlobalVariables globalsOrder(XQillaAllocator<XQGlobalVariable*>(m_context->getMemoryManager()));
    AutoReset<GlobalVariables*> autoReset(styper->getGlobalsOrder());
    styper->getGlobalsOrder() = &globalsOrder;

    // Run staticTyping on the global variables
    for(varIt = m_userDefVars.begin(); varIt != m_userDefVars.end(); ++varIt) {
      for(j = m_userDefFns.begin(); j != m_userDefFns.end(); ++j) {
        (*j)->resetStaticTypingOnce();
      }

      (*varIt)->staticTypingOnce(m_context, styper);
    }

    // XQuery 1.1 reorders the global variables to enable forward references
    if(m_version3)
      m_userDefVars = globalsOrder;
  }

  // Run staticTyping on the user defined functions,
  // which calculates a better type for them
  for(i = m_userDefFns.begin(); i != m_userDefFns.end(); ++i) {
    for(j = m_userDefFns.begin(); j != m_userDefFns.end(); ++j) {
      (*j)->resetStaticTypingOnce();
    }

    (*i)->staticTypingOnce(m_context, styper);
  }

  // Run staticTyping on the query body
  if(m_query) m_query = m_query->staticTyping(m_context, styper);
}

std::string XQQuery::getQueryPlan() const
{
  AutoDelete<ASTToXML> a2x(m_context->getConfiguration()->createASTToXML());
  return a2x->print(this, m_context);
}

ASTNode* XQQuery::getQueryBody() const
{
  return m_query;
}

void XQQuery::setQueryBody(ASTNode* query)
{
  m_query=query;
}

void XQQuery::addFunction(XQUserFunction* fnDef)
{
  m_userDefFns.push_back(fnDef);
}

void XQQuery::addDelayedFunction(const XMLCh *uri, const XMLCh *name, size_t numArgs,
                                 const XMLCh *functionDeclaration, bool isPrivate, int line, int column)
{
  DelayedFuncFactory *f = new (m_context->getMemoryManager())
	  DelayedFuncFactory(uri, name, numArgs, functionDeclaration, line, column, this);
  if(!isPrivate)
    m_delayedFunctions.push_back(f);
  m_context->addCustomFunction(f);
}

void XQQuery::addDelayedFunction(const XMLCh *uri, const XMLCh *name, size_t numArgs,
                                 const char *functionDeclaration, bool isPrivate, int line, int column)
{
  DelayedFuncFactory *f = new (m_context->getMemoryManager())
	  DelayedFuncFactory(uri, name, numArgs, functionDeclaration, line, column, this);
  if(!isPrivate)
    m_delayedFunctions.push_back(f);
  m_context->addCustomFunction(f);
}

void XQQuery::addVariable(XQGlobalVariable* varDef)
{
  m_userDefVars.push_back(varDef);
}

void XQQuery::setIsLibraryModule(bool bIsModule/*=true*/)
{
  m_bIsLibraryModule=bIsModule;
}

bool XQQuery::getIsLibraryModule() const
{
  return m_bIsLibraryModule;
}

void XQQuery::setModuleTargetNamespace(const XMLCh* uri)
{
  m_szTargetNamespace=uri;
}

const XMLCh* XQQuery::getModuleTargetNamespace() const
{
  return m_szTargetNamespace;
}

void XQQuery::importModule(const XMLCh* szUri, VectorOfStrings* locations, const LocationInfo *location)
{
  for(ImportedModules::iterator modIt = m_importedModules.begin();
      modIt != m_importedModules.end(); ++modIt) {
    if(XPath2Utils::equals((*modIt)->getModuleTargetNamespace(),szUri)) {
      XMLBuffer buf;
      buf.set(X("Module for namespace '"));
      buf.append(szUri);
      buf.append(X("' has already been imported [err:XQST0047]"));
      XQThrow3(StaticErrorException, X("XQQuery::ImportModule"), buf.getRawBuffer(), location);
    }
  }

  // Search in the module cache
  XQQuery *module = 0;
  if(!XPath2Utils::equals(szUri, m_szTargetNamespace)) {
    module = m_moduleCache->getByNamespace(szUri);
    if(module != 0) {
      importModule(module);
      return;
    }
  }

  if(locations == NULL)
    locations = m_context->resolveModuleURI(szUri);
  if(locations == NULL || locations->empty()) {
    XMLBuffer buf;
    buf.set(X("Cannot locate module for namespace "));
    buf.append(szUri);
    buf.append(X(" without the 'at <location>' keyword [err:XQST0059]"));
    XQThrow3(StaticErrorException,X("XQQuery::ImportModule"), buf.getRawBuffer(), location);
  }

  for(VectorOfStrings::iterator it=locations->begin();it!=locations->end();++it) {
    module = parseModule(szUri, *it, location);

    if(!module->getIsLibraryModule()) {
      XMLBuffer buf;
      buf.set(X("The module at "));
      buf.append(module->getFile());
      buf.append(X(" is not a module"));
      XQThrow3(StaticErrorException, X("XQQuery::ImportModule"), buf.getRawBuffer(), location);
    }
    if(!XERCES_CPP_NAMESPACE::XMLString::equals(szUri, module->getModuleTargetNamespace())) {
      XMLBuffer buf;
      buf.set(X("The module at "));
      buf.append(module->getFile());
      buf.append(X(" specifies a different namespace [err:XQST0059]"));
      XQThrow3(StaticErrorException, X("XQQuery::ImportModule"), buf.getRawBuffer(), location);
    }
  }

  // Search in the module cache again, to get the head of the linked list of modules
  module = m_moduleCache->getByNamespace(szUri);
  if(!module) {
    XMLBuffer buf;
    buf.set(X("Cannot locate the module for namespace \""));
    buf.append(szUri);
    buf.append(X("\" [err:XQST0059]"));
    XQThrow3(StaticErrorException,X("XQQuery::ImportModule"), buf.getRawBuffer(), location);
  }

  importModule(module);
}

XQQuery *XQQuery::parseModule(const XMLCh *ns, const XMLCh *at, const LocationInfo *location) const
{
  InputSource* srcToUse = 0;
  if(m_context->getDocumentCache()->getXMLEntityResolver()){
    XMLResourceIdentifier resourceIdentifier(XMLResourceIdentifier::UnKnown, at, ns,
                                             XMLUni::fgZeroLenString, m_context->getBaseURI());
    srcToUse = m_context->getDocumentCache()->getXMLEntityResolver()->resolveEntity(&resourceIdentifier);
  }

  if(srcToUse==0) {
    try {
      XMLURL urlTmp(m_context->getBaseURI(), at);
      if (urlTmp.isRelative()) {
        throw MalformedURLException(__FILE__, __LINE__, XMLExcepts::URL_NoProtocolPresent);
      }
      srcToUse = new URLInputSource(urlTmp);
    }
    catch(const MalformedURLException&) {
      // It's not a URL, so let's assume it's a local file name.
      const XMLCh* baseUri=m_context->getBaseURI();
      if(baseUri && baseUri[0]) {
        XMLCh* tmpBuf = XMLPlatformUtils::weavePaths(baseUri, at);
        srcToUse = new LocalFileInputSource(tmpBuf);
        XMLPlatformUtils::fgMemoryManager->deallocate(tmpBuf);
      }
      else {
        srcToUse = new LocalFileInputSource(at);
      }
    }
  }
  Janitor<InputSource> janIS(srcToUse);

  XQQuery *module = m_moduleCache->getByURI(srcToUse->getSystemId());
  if(module) return module;

  AutoDelete<DynamicContext> ctxGuard(m_context->createModuleContext());
  DynamicContext *moduleCtx = ctxGuard.get();

  moduleCtx->setBaseURI(srcToUse->getSystemId());
  moduleCtx->setXMLEntityResolver(m_context->getXMLEntityResolver());

  AutoDelete<XQQuery> moduleGuard(new (XMLPlatformUtils::fgMemoryManager)
                                  XQQuery(ctxGuard.adopt(), true, m_moduleCache, XMLPlatformUtils::fgMemoryManager));
  module = moduleGuard;
  module->setFile(srcToUse->getSystemId());
  module->setModuleTargetNamespace(ns);

  // Put the unparsed module in the cache, to resolve loops correctly
  m_moduleCache->put(moduleGuard.adopt());

  XQilla::parse(*srcToUse, moduleCtx, XQilla::NO_STATIC_RESOLUTION, XMLPlatformUtils::fgMemoryManager, module);

  return module;
}

void XQQuery::importModule(XQQuery *module)
{
  m_importedModules.push_back(module);
}

void XQQuery::setFile(const XMLCh* file)
{
	m_szCurrentFile = m_context->getMemoryManager()->getPooledString(file);
}

void XQQuery::setQueryText(const XMLCh *v)
{
  m_szQueryText = m_context->getMemoryManager()->getPooledString(v);
}

Result XQQuery::execute(DynamicContext* context) const
{
  executeProlog(context);
  if(getQueryBody() != NULL)
    return getQueryBody()->createResult(context);
  return 0;
}


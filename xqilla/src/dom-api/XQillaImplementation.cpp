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

#include "../config/xqilla_config.h"
#include "XQillaImplementation.hpp"
#include <xqilla/utils/XPath2Utils.hpp>
#include "impl/XQillaDocumentImpl.hpp"
#include "impl/XQillaBuilderImpl.hpp"
#include "impl/XQillaXMLGrammarPoolImpl.hpp"

#include "XPath2NodeSerializer.hpp"

#include <xercesc/dom/DOMDocument.hpp>
#include <xercesc/dom/DOMDocumentType.hpp>
#include <xercesc/dom/DOMImplementation.hpp>
#include <xercesc/dom/DOMException.hpp>
#if _XERCES_VERSION >= 30000
#include <xercesc/dom/impl/DOMImplementationListImpl.hpp>
#include <xercesc/dom/DOMLSSerializer.hpp>
#else
#include <xercesc/dom/DOMWriter.hpp>
#endif

#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/util/XMLStringTokenizer.hpp>
#include <xercesc/util/XMLUniDefs.hpp>

XERCES_CPP_NAMESPACE_USE;

const XMLCh XQillaImplementation::gXQilla[] =   // Points to "XPath2"
{chLatin_X, chLatin_P, chLatin_a, chLatin_t, chLatin_h, chDigit_2, chNull};
 
const XMLCh XQillaImplementation::g3_0[] =      // Points to "3.0"
        {chDigit_3, chPeriod, chDigit_0, chNull};

XQillaImplementation::XQillaImplementation()
{
    domImpl = DOMImplementation::getImplementation();
}
  
XQillaImplementation::~XQillaImplementation()
{
}

XQillaImplementation * XQillaImplementation::gDomimp = 0;

void XQillaImplementation::initialize()
{
  if(gDomimp) delete gDomimp;
  gDomimp = new XQillaImplementation();
  DOMImplementationRegistry::addSource(gDomimp);
}

void XQillaImplementation::terminate()
{
  if(gDomimp) delete gDomimp;
  gDomimp = 0;
}

XQillaImplementation* XQillaImplementation::getDOMImplementationImpl()
{
  // initialized by XQillaImplementation::initialize()
  return gDomimp;
}

bool XQillaImplementation::hasFeature(const  XMLCh * feature,  const  XMLCh * version) const
{
  if (!feature)
    return false;
    
  if (XMLString::compareIString(feature, gXQilla) == 0
      && (version == 0 || !*version || XMLString::equals(version, g3_0)))
      return true;
  
  return domImpl->hasFeature(feature, version);
}

DOMDocument* XQillaImplementation::createDocument(const XMLCh *namespaceURI,
  const XMLCh *qualifiedName, DOMDocumentType *doctype, MemoryManager* const manager)
{
    return new (manager) XQillaDocumentImpl(namespaceURI, qualifiedName, doctype, this, manager);
}


DOMDocument* XQillaImplementation::createDocument(MemoryManager* const manager)
{
  return new (manager) XQillaDocumentImpl(this, manager);
}

#if _XERCES_VERSION >= 30000
DOMLSParser* XQillaImplementation::createLSParser(const DOMImplementationLS::DOMImplementationLSMode mode,
                                                  const XMLCh* const     schemaType,
                                                  MemoryManager* const   manager,
                                                  XMLGrammarPool*  const gramPool)
{
  XMLGrammarPool *temp = 0;
  bool adoptGramPool = false;
  
  if(!gramPool) {
    temp = new (manager) XQillaXMLGrammarPoolImpl(manager);
    adoptGramPool = true;
  }
  else {
    temp = gramPool;
  }

  if(mode == DOMImplementationLS::MODE_ASYNCHRONOUS)
    throw DOMException(DOMException::NOT_SUPPORTED_ERR, 0);

  DOMLSParser *tmp = new (manager) XQillaBuilderImpl(0, manager, temp, adoptGramPool);
#if _XERCES_VERSION >= 30000
  tmp->getDomConfig()->setParameter(XMLUni::fgXercesParserUseDocumentFromImplementation, (void*)gXQilla);
  tmp->getDomConfig()->setParameter(XMLUni::fgXercesCacheGrammarFromParse, true);
#else
  tmp->setProperty(XMLUni::fgXercesParserUseDocumentFromImplementation, (void*)gXQilla);
  tmp->setFeature(XMLUni::fgXercesCacheGrammarFromParse, true);
#endif

  return tmp;
}

DOMLSSerializer* XQillaImplementation::createLSSerializer(MemoryManager* const manager)
{
  return new (manager) XPath2NodeSerializer(manager);
}

DOMLSInput* XQillaImplementation::createLSInput(MemoryManager* const manager)
{
  return domImpl->createLSInput(manager);
}

DOMLSOutput* XQillaImplementation::createLSOutput(MemoryManager* const manager)
{
  return domImpl->createLSOutput(manager);
}
#else
DOMWriter* XQillaImplementation::createDOMWriter(MemoryManager* const manager)
{
    return new (manager) XPath2NodeSerializer(manager);
}

DOMBuilder* XQillaImplementation::createDOMBuilder(const short mode,
                                                   const XMLCh* const schemaType,
                                                   MemoryManager* const manager,
                                                   XMLGrammarPool*  const gramPool)
{
  XMLGrammarPool *temp = 0;
  bool adoptGramPool = false;
  
  if(!gramPool) {
    temp = new (manager) XQillaXMLGrammarPoolImpl(manager);
    adoptGramPool = true;
  }
  else {
    temp = gramPool;
  }

  if(mode == DOMImplementationLS::MODE_ASYNCHRONOUS)
    throw DOMException(DOMException::NOT_SUPPORTED_ERR, 0);

  DOMBuilder *tmp = new (manager) XQillaBuilderImpl(0, manager, temp, adoptGramPool);
  tmp->setProperty(XMLUni::fgXercesParserUseDocumentFromImplementation, (void*)gXQilla);
  tmp->setFeature(XMLUni::fgXercesCacheGrammarFromParse, true);

  return tmp;
}

DOMInputSource* XQillaImplementation::createDOMInputSource()
{
  return domImpl->createDOMInputSource();
}
#endif



DOMImplementation* XQillaImplementation::getDOMImplementation(const XMLCh* features) const
{
   DOMImplementation* impl = XQillaImplementation::getDOMImplementationImpl();

  XMLStringTokenizer tokenizer(features);
  const XMLCh* feature = 0;

  while (feature || tokenizer.hasMoreTokens()) {

    if (!feature)
      feature = tokenizer.nextToken();
    
    const XMLCh* version = 0;
    const XMLCh* token = tokenizer.nextToken();
    
    if (token && XMLString::isDigit(token[0]))
      version = token;
    
    if (!impl->hasFeature(feature, version))
      return 0;
    
    if (!version)
      feature = token;
  }
  return impl;
}

#if _XERCES_VERSION >= 30000
DOMImplementationList *XQillaImplementation::getDOMImplementationList(const XMLCh* features) const
{
  DOMImplementationListImpl* list = new DOMImplementationListImpl;
  DOMImplementation* myImpl=getDOMImplementation(features);
  if(myImpl)
    list->add(myImpl);
  return list;
}
#endif

DOMDocumentType *XQillaImplementation::createDocumentType(const XMLCh *qualifiedName,
                                                          const XMLCh * publicId, const XMLCh *systemId)
{
    return domImpl->createDocumentType(qualifiedName, publicId, systemId); 
}

#if _XERCES_VERSION >= 30000
void* XQillaImplementation::getFeature(const XMLCh* feature, const XMLCh* version) const
{
  return domImpl->getFeature(feature, version);
}
#else
DOMImplementation* XQillaImplementation::getInterface(const XMLCh* feature)
{
  return domImpl->getInterface(feature);
}
#endif

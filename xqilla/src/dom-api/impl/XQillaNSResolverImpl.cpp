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

#include "../../config/xqilla_config.h"
#include <xqilla/dom-api/impl/XQillaNSResolverImpl.hpp>
#include <xqilla/utils/XPath2Utils.hpp>
#include <xqilla/framework/XPath2MemoryManager.hpp>
#include <xqilla/exceptions/StaticErrorException.hpp>

#include <xercesc/dom/DOMElement.hpp>
#include <xercesc/util/XMLUni.hpp>
#include <xercesc/util/XMLUniDefs.hpp>
#include <xercesc/util/XMLString.hpp>
#include <xercesc/dom/DOM.hpp>

XERCES_CPP_NAMESPACE_USE;

static const XMLCh g_nsBlocker[]={ chOpenSquare, chOpenSquare, 
                                   chCloseSquare, chCloseSquare, 
                                   chOpenCurly, chCloseCurly, 
                                   chNull };

XQillaNSResolverImpl::XQillaNSResolverImpl(XPath2MemoryManager* memMgr)
  : _namespaceBindings(6, false, memMgr),
    _resolverNode(0),
    _prevScope(0),
    _memMgr(memMgr)
{
  // "xml" is always bound to "http://www.w3.org/XML/1998/namespace"
  _namespaceBindings.put((void*)XMLUni::fgXMLString,(XMLCh*)XMLUni::fgXMLURIName);

  // "xmlns" is always bound to "http://www.w3.org/2000/xmlns/"
  _namespaceBindings.put((void*)XMLUni::fgXMLNSString,(XMLCh*)XMLUni::fgXMLNSURIName);
}


XQillaNSResolverImpl::XQillaNSResolverImpl(XPath2MemoryManager* memMgr,
                                           const DOMXPathNSResolver *prevScope)
  : _namespaceBindings(6, false, memMgr),
    _resolverNode(0),
    _prevScope(prevScope),
    _memMgr(memMgr)
{
}


XQillaNSResolverImpl::XQillaNSResolverImpl(XPath2MemoryManager* memMgr, 
                                           const DOMNode *resolverNode,
                                           const DOMXPathNSResolver *prevScope)
  : _namespaceBindings(6, false, memMgr),
    _resolverNode(resolverNode),
    _prevScope(prevScope),
    _memMgr(memMgr)
{
  if(prevScope == 0) {
    // "xml" is always bound to "http://www.w3.org/XML/1998/namespace"
    _namespaceBindings.put((void*)XMLUni::fgXMLString,(XMLCh*)XMLUni::fgXMLURIName);

    // "xmlns" is always bound to "http://www.w3.org/2000/xmlns/"
    _namespaceBindings.put((void*)XMLUni::fgXMLNSString,(XMLCh*)XMLUni::fgXMLNSURIName);
  }
}


XQillaNSResolverImpl::~XQillaNSResolverImpl()
{
}

const XMLCh* XQillaNSResolverImpl::lookupNamespaceURI(const XMLCh* prefix) const
{
  const XMLCh *uri = _namespaceBindings.get((void*)prefix);
  if(XPath2Utils::equals(uri, g_nsBlocker)) return NULL;
  else if(uri != NULL && *uri != 0) return uri;

  if(_resolverNode != 0) {
    uri = _resolverNode->lookupNamespaceURI(prefix);
    if(uri != NULL && *uri != 0) return uri;
  }

  if(_prevScope != 0)
    return _prevScope->lookupNamespaceURI(prefix);
  return 0;
}

const XMLCh* XQillaNSResolverImpl::lookupPrefix(const XMLCh* uri) const
{
  RefHashTableOfEnumerator<XMLCh> iterValue((RefHashTableOf<XMLCh>*)&_namespaceBindings);
  RefHashTableOfEnumerator<XMLCh> iterKey((RefHashTableOf<XMLCh>*)&_namespaceBindings);
  while(iterValue.hasMoreElements()) {
    XMLCh* key=(XMLCh*)iterKey.nextElementKey();
    if(XPath2Utils::equals(&iterValue.nextElement(), uri)) {
      return key;
    }
  }

  if(_resolverNode != 0) {
#if _XERCES_VERSION >= 30000
    const XMLCh *prefix = _resolverNode->lookupPrefix(uri);
#else
    const XMLCh *prefix = _resolverNode->lookupNamespacePrefix(uri, false);
#endif
    if(prefix != NULL && *prefix != 0) return prefix;
  }

  if(_prevScope != 0)
    return _prevScope->lookupPrefix(uri);
  return 0;
}

void XQillaNSResolverImpl::addNamespaceBinding(const XMLCh* prefix, const XMLCh* uri)
{
  if(uri == 0 || *uri == 0)
    uri = g_nsBlocker;
  _namespaceBindings.put((void*)_memMgr->getPooledString(prefix), (XMLCh*)_memMgr->getPooledString(uri));
}

void XQillaNSResolverImpl::release() {
  this->~XQillaNSResolverImpl();
  _memMgr->deallocate(this);
}

void XQillaNSResolverImpl::forbiddenBindingCheck(const XMLCh* prefix, const XMLCh* uri, const LocationInfo *info)
{
  if(XPath2Utils::equals(prefix,XMLUni::fgXMLNSString) || XPath2Utils::equals(prefix,XMLUni::fgXMLString))
    XQThrow3(StaticErrorException,X("XQillaNSResolverImpl::forbiddenBindingCheck"),
             X("The prefixes 'xmlns' and 'xml' cannot be used in a namespace declaration [err:XQST0070]"), info);
  if(XPath2Utils::equals(uri,XMLUni::fgXMLURIName))
    XQThrow3(StaticErrorException,X("XQillaNSResolverImpl::forbiddenBindingCheck"),
             X("The 'http://www.w3.org/XML/1998/namespace' namespace cannot be bound to any prefix [err:XQST0070]"), info);
}

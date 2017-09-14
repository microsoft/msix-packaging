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
#include "XQillaDocumentImpl.hpp"

#include "XQillaExpressionImpl.hpp"
#include <xqilla/dom-api/XQillaExpression.hpp>

#include <xqilla/dom-api/XQillaNSResolver.hpp>
#include <xqilla/dom-api/XPath2Result.hpp>
#include <xqilla/context/DynamicContext.hpp>

#include <xqilla/exceptions/FunctionException.hpp>
#include <xqilla/exceptions/ContextException.hpp>

#include <xercesc/dom/DOMException.hpp>
#include <xercesc/dom/DOMNode.hpp>
#include <xercesc/dom/DOMException.hpp>
#include <xercesc/dom/DOMDocumentType.hpp>
#include <xercesc/dom/DOMException.hpp>
#include <xercesc/dom/DOMXPathEvaluator.hpp>
#include <xercesc/dom/impl/DOMDocumentImpl.hpp>
#include <xercesc/dom/DOMXPathException.hpp>
#include <xercesc/framework/XMLGrammarPool.hpp>

#include <xqilla/utils/XPath2Utils.hpp>

XERCES_CPP_NAMESPACE_USE;

XQillaDocumentImpl::XQillaDocumentImpl(DOMImplementation* domImpl,
                                       MemoryManager* const memMgr)
#if _XERCES_VERSION >= 30000
  : DOMDocumentImpl(domImpl, memMgr),
#else
  : DOMDocumentImpl(memMgr),
#endif
    _createdWith(memMgr),
    _memMgr(memMgr),
    _xmlGrammarPool(0),
    _adoptGramPool(false)
{
}

XQillaDocumentImpl::XQillaDocumentImpl(const XMLCh* namespaceURI, const XMLCh* qualifiedName,
                                       DOMDocumentType* doctype,
                                       DOMImplementation* domImpl,
                                       MemoryManager* const memMgr)
#if _XERCES_VERSION >= 30000
  : DOMDocumentImpl(namespaceURI, qualifiedName, doctype, domImpl, memMgr),
#else
  : DOMDocumentImpl(namespaceURI, qualifiedName, doctype, memMgr),
#endif
    _createdWith(memMgr),
    _memMgr(memMgr),
    _xmlGrammarPool(0),
    _adoptGramPool(false)
{
}

XQillaDocumentImpl::~XQillaDocumentImpl()
{
  if(_adoptGramPool) delete _xmlGrammarPool;
}

// weak version, create a context within
#if _XERCES_VERSION >= 30000
DOMXPathExpression*
XQillaDocumentImpl::createExpression(const XMLCh* expression, 
                                     const DOMXPathNSResolver* resolver)
#else
const DOMXPathExpression*
XQillaDocumentImpl::createExpression(const XMLCh* expression, 
                                     const DOMXPathNSResolver* resolver)
#endif
{
  // Use placement new, because XQillaExpressionImpl inherits from XercesConfiguration,
  // which inherits from XMemory - which screws up our operator new overload
  void *mem = _createdWith->allocate(sizeof(XQillaExpressionImpl));
  try {
    new (mem) XQillaExpressionImpl(expression, _createdWith, resolver, _xmlGrammarPool);
  } catch(...) {
    _createdWith->deallocate(mem);
    throw;
  }
  return (XQillaExpressionImpl*)mem;
}


#if _XERCES_VERSION >= 30000
DOMXPathResult* XQillaDocumentImpl::evaluate(const XMLCh *expression, const DOMNode *contextNode,
                                             const DOMXPathNSResolver *resolver,
                                             DOMXPathResult::ResultType type,
                                             DOMXPathResult* reuseableResult)
#else
void* XQillaDocumentImpl::evaluate(const XMLCh* expression,
                                   DOMNode* contextNode,
                                   const DOMXPathNSResolver* resolver,
                                   unsigned short type,
                                   void* reuseableResult)
#endif
{
  return ((XQillaExpressionImpl*)createExpression(expression, resolver))->evaluateOnce(contextNode, type, reuseableResult);
}

#if _XERCES_VERSION >= 30000
DOMXPathNSResolver *XQillaDocumentImpl::createNSResolver(const DOMNode *nodeResolver)
#else
const DOMXPathNSResolver *XQillaDocumentImpl::createNSResolver(DOMNode* nodeResolver)
#endif
{
  return _memMgr.createNSResolver(const_cast<DOMNode*>(nodeResolver));
}

void XQillaDocumentImpl::setGrammarPool(XMLGrammarPool *xmlGrammarPool, bool adoptGramPool) {
    _xmlGrammarPool = xmlGrammarPool;
    _adoptGramPool = adoptGramPool;
}

XMLGrammarPool *XQillaDocumentImpl::getGrammarPool() {
  return _xmlGrammarPool;
}

DOMNode *XQillaDocumentImpl::cloneNode(bool deep) const
{
  // Note:  the cloned document node goes on the same heap we live in.
  XQillaDocumentImpl *newdoc = new (fMemoryManager) XQillaDocumentImpl(fDOMImplementation, fMemoryManager);
  newdoc->becomeClone(this, deep);

  fNode.callUserDataHandlers(DOMUserDataHandler::NODE_CLONED, this, newdoc);
  return newdoc;
}

void XQillaDocumentImpl::becomeClone(const XQillaDocumentImpl *toClone, bool deep)
{
  if(toClone->fXmlEncoding && *toClone->fXmlEncoding)
    setXmlEncoding(toClone->fXmlEncoding);
  if(toClone->fXmlVersion && *toClone->fXmlVersion)
    setXmlVersion(toClone->fXmlVersion);
  setXmlStandalone(toClone->fXmlStandalone);

  if(deep) {
    for (DOMNode *n = toClone->getFirstChild(); n != 0; n = n->getNextSibling()) {
      appendChild(importNode(n, true, true));
    }
  }
}

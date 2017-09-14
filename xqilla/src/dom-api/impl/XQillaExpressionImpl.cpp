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
#include "XQillaExpressionImpl.hpp"
#include <xqilla/simple-api/XQilla.hpp>
#include <xqilla/simple-api/XQQuery.hpp>
#include <xqilla/items/Node.hpp>
#include <xqilla/items/Item.hpp>
#include <xqilla/runtime/Sequence.hpp>
#include <xqilla/context/DynamicContext.hpp>
#include <xqilla/dom-api/XQillaExpression.hpp>
#include "XPath2ResultImpl.hpp"
#include <xqilla/ast/ASTNode.hpp>
#include <xqilla/runtime/Sequence.hpp>
#include <xqilla/context/DynamicContext.hpp>
#include <xqilla/framework/XPath2MemoryManager.hpp>

#include <xqilla/exceptions/XQException.hpp>
#include <xqilla/exceptions/XQillaException.hpp>
#include <xqilla/schema/DocumentCacheImpl.hpp>

#include <xercesc/util/XMLString.hpp>
#include <xercesc/util/XMLException.hpp>
#include <xercesc/util/RuntimeException.hpp>
#include <xercesc/framework/XMLGrammarPool.hpp>
#include <xercesc/dom/DOMNode.hpp>
#include <xercesc/dom/DOMDocument.hpp>
#include <xercesc/dom/DOMException.hpp>
#include <xercesc/util/XercesDefs.hpp>
#include <xercesc/dom/DOMXPathResult.hpp>
#include <xercesc/dom/DOMXPathNSResolver.hpp>

XERCES_CPP_NAMESPACE_USE;

XQillaExpressionImpl::XQillaExpressionImpl(const XMLCh *expression,
                                           MemoryManager* memMgr,
                                           const DOMXPathNSResolver *nsr,
                                           XMLGrammarPool *xmlGP)
  : _createdWith(memMgr),
    _xmlgr(xmlGP)
{
  try {
    _staticContext = XQilla::createContext(XQilla::XPATH2, this, _createdWith);
    if(nsr != 0) {
	    _staticContext->setNSResolver(nsr);

	    const XMLCh *defaultElementNS = _staticContext->getMemoryManager()->
		    getPooledString(nsr->lookupNamespaceURI(XMLUni::fgZeroLenString));
	    _staticContext->setDefaultElementAndTypeNS(defaultElementNS);
    }
    _compiledExpression = XQilla::parse(expression, _staticContext, NULL, 0, _createdWith);
  }
  catch(XQException &e) {
    if(XQillaException::getDebug()) {
      e.printDebug(X("Caught exception at Interface"));
    }
    throw XQillaException(e);
  }
}

XQillaExpressionImpl::~XQillaExpressionImpl() 
{
  delete _compiledExpression;
}

void XQillaExpressionImpl::release()
{
  this->~XQillaExpressionImpl();
  _createdWith->deallocate(this);
}

DocumentCache *XQillaExpressionImpl::createDocumentCache(MemoryManager *memMgr)
{
  return new(memMgr) DocumentCacheImpl(memMgr, _xmlgr);
}

#if _XERCES_VERSION >= 30000
DOMXPathResult *XQillaExpressionImpl::evaluate(const DOMNode *contextNode,
                                               DOMXPathResult::ResultType type,
                                               DOMXPathResult* result) const
{
  // TBD deal with the other result types - jpcs
  switch(type) {
  case DOMXPathResult::FIRST_RESULT_TYPE: {
    return new (_createdWith) XPath2FirstResultImpl(_compiledExpression, const_cast<DOMNode*>(contextNode),
                                                    _staticContext, _createdWith);
    break;
  }
  case DOMXPathResult::UNORDERED_NODE_ITERATOR_TYPE:
  case DOMXPathResult::ORDERED_NODE_ITERATOR_TYPE:
  case DOMXPathResult::ITERATOR_RESULT_TYPE: {
    return new (_createdWith) XPath2IteratorResultImpl(_compiledExpression, const_cast<DOMNode*>(contextNode),
                                                       _staticContext, _createdWith);
    break;
  }
  case DOMXPathResult::ANY_TYPE:
  case DOMXPathResult::NUMBER_TYPE:
  case DOMXPathResult::STRING_TYPE:
  case DOMXPathResult::BOOLEAN_TYPE:
  case DOMXPathResult::ANY_UNORDERED_NODE_TYPE:
  case DOMXPathResult::FIRST_ORDERED_NODE_TYPE:
  case DOMXPathResult::UNORDERED_NODE_SNAPSHOT_TYPE:
  case DOMXPathResult::ORDERED_NODE_SNAPSHOT_TYPE:
  case DOMXPathResult::SNAPSHOT_RESULT_TYPE: {
    return new (_createdWith) XPath2SnapshotResultImpl(_compiledExpression, const_cast<DOMNode*>(contextNode),
                                                       _staticContext, _createdWith);
    break;
  }
  }
  return 0;
}

DOMXPathResult *XQillaExpressionImpl::evaluateOnce(const DOMNode *contextNode,
                                                   DOMXPathResult::ResultType type,
                                                   DOMXPathResult* result)
{
  // TBD deal with the other result types - jpcs
  switch(type) {
  case DOMXPathResult::FIRST_RESULT_TYPE: {
    return new (_createdWith) XPath2FirstResultImpl(_compiledExpression, const_cast<DOMNode*>(contextNode),
                                                    _staticContext, _createdWith, this);
    break;
  }
  case DOMXPathResult::UNORDERED_NODE_ITERATOR_TYPE:
  case DOMXPathResult::ORDERED_NODE_ITERATOR_TYPE:
  case DOMXPathResult::ITERATOR_RESULT_TYPE: {
    return new (_createdWith) XPath2IteratorResultImpl(_compiledExpression, const_cast<DOMNode*>(contextNode),
                                                       _staticContext, _createdWith, this);
    break;
  }
  case DOMXPathResult::ANY_TYPE:
  case DOMXPathResult::NUMBER_TYPE:
  case DOMXPathResult::STRING_TYPE:
  case DOMXPathResult::BOOLEAN_TYPE:
  case DOMXPathResult::ANY_UNORDERED_NODE_TYPE:
  case DOMXPathResult::FIRST_ORDERED_NODE_TYPE:
  case DOMXPathResult::UNORDERED_NODE_SNAPSHOT_TYPE:
  case DOMXPathResult::ORDERED_NODE_SNAPSHOT_TYPE:
  case DOMXPathResult::SNAPSHOT_RESULT_TYPE: {
    return new (_createdWith) XPath2SnapshotResultImpl(_compiledExpression, const_cast<DOMNode*>(contextNode),
                                                       _staticContext, _createdWith, this);
    break;
  }
  }
  return 0;
}
#else
void* XQillaExpressionImpl::evaluate(DOMNode* contextNode,
                                     unsigned short type,
                                     void*) const
{
  switch((XPath2Result::ResultType)type) {
  case XPath2Result::FIRST_RESULT: {
    return new (_createdWith) XPath2FirstResultImpl(_compiledExpression, contextNode,
                                                    _staticContext, _createdWith);
    break;
  }
  case XPath2Result::ITERATOR_RESULT: {
    return new (_createdWith) XPath2IteratorResultImpl(_compiledExpression, contextNode,
                                                       _staticContext, _createdWith);
    break;
  }
  case XPath2Result::SNAPSHOT_RESULT: {
    return new (_createdWith) XPath2SnapshotResultImpl(_compiledExpression, contextNode,
                                                       _staticContext, _createdWith);
    break;
  }
  }
  return 0;
}

void* XQillaExpressionImpl::evaluateOnce(DOMNode* contextNode,
                                         unsigned short type,
                                         void*)
{
  switch((XPath2Result::ResultType)type) {
  case XPath2Result::FIRST_RESULT: {
    return new (_createdWith) XPath2FirstResultImpl(_compiledExpression, contextNode,
                                                    _staticContext, _createdWith, this);
    break;
  }
  case XPath2Result::ITERATOR_RESULT: {
    return new (_createdWith) XPath2IteratorResultImpl(_compiledExpression, contextNode,
                                                       _staticContext, _createdWith, this);
    break;
  }
  case XPath2Result::SNAPSHOT_RESULT: {
    return new (_createdWith) XPath2SnapshotResultImpl(_compiledExpression, contextNode,
                                                       _staticContext, _createdWith, this);
    break;
  }
  }
  return 0;
}
#endif

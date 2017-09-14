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

#ifndef __XQILLAEXPRESSIONIMPL_HPP
#define __XQILLAEXPRESSIONIMPL_HPP

#include <xqilla/framework/XQillaExport.hpp>
#include <xqilla/dom-api/XQillaExpression.hpp>
#include <xqilla/runtime/Sequence.hpp>

#include <xqilla/xerces/XercesConfiguration.hpp>

class XPath2Result;
class DynamicContext;
class XPath2MemoryManager;
class XQQuery;

XERCES_CPP_NAMESPACE_BEGIN
class XMLGrammarPool;
class DOMNode;
class DOMDocument;
class DOMException;
class DOMXPathNSResolver;
XERCES_CPP_NAMESPACE_END 

class XQILLA_API XQillaExpressionImpl : public XQillaExpression, public XercesConfiguration
{
public:

  XQillaExpressionImpl(const XMLCh *expression,
                       XERCES_CPP_NAMESPACE_QUALIFIER MemoryManager* memMgr,
                       const XERCES_CPP_NAMESPACE_QUALIFIER DOMXPathNSResolver *nsr,
                       XERCES_CPP_NAMESPACE_QUALIFIER XMLGrammarPool *xmlGP);
  virtual ~XQillaExpressionImpl();

#if _XERCES_VERSION >= 30000
  virtual XERCES_CPP_NAMESPACE_QUALIFIER DOMXPathResult*
  evaluate(const XERCES_CPP_NAMESPACE_QUALIFIER DOMNode *contextNode,
           XERCES_CPP_NAMESPACE_QUALIFIER DOMXPathResult::ResultType type,
           XERCES_CPP_NAMESPACE_QUALIFIER DOMXPathResult* result) const;

  virtual XERCES_CPP_NAMESPACE_QUALIFIER DOMXPathResult*
  evaluateOnce(const XERCES_CPP_NAMESPACE_QUALIFIER DOMNode *contextNode,
           XERCES_CPP_NAMESPACE_QUALIFIER DOMXPathResult::ResultType type,
           XERCES_CPP_NAMESPACE_QUALIFIER DOMXPathResult* result);
#else
  virtual void* evaluate(XERCES_CPP_NAMESPACE_QUALIFIER DOMNode* contextNode,
                         unsigned short type,
                         void* reuseableResult) const;
  virtual void* evaluateOnce(XERCES_CPP_NAMESPACE_QUALIFIER DOMNode* contextNode,
                             unsigned short type,
                             void* reuseableResult);
#endif

  virtual void release();

  const XQQuery *getCompiledExpression() const { return _compiledExpression; }
  void setCompiledExpression(XQQuery *expr) { _compiledExpression = expr; }

private:
  virtual DocumentCache *createDocumentCache(XERCES_CPP_NAMESPACE_QUALIFIER MemoryManager *memMgr);

  XERCES_CPP_NAMESPACE_QUALIFIER MemoryManager* _createdWith;

  XERCES_CPP_NAMESPACE_QUALIFIER XMLGrammarPool *_xmlgr;
  DynamicContext *_staticContext;
  XQQuery* _compiledExpression;
}; //XQillaExpressionImpl


#endif //__XQILLAEXPRESSIONIMPL_HPP


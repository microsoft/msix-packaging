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

#ifndef __XQILLADOCUMENTIMPL_HPP
#define __XQILLADOCUMENTIMPL_HPP

#include <xqilla/framework/XQillaExport.hpp>

#include <xqilla/framework/XPath2MemoryManagerImpl.hpp>
#include <xqilla/framework/ProxyMemoryManager.hpp>

#include <xercesc/dom/impl/DOMDocumentImpl.hpp>
#include <xercesc/dom/DOMException.hpp>
#include <xercesc/dom/DOMXPathException.hpp>
#include <xercesc/util/XMemory.hpp>


class DynamicContext;
class XPath2Result;

XERCES_CPP_NAMESPACE_BEGIN
class XMLGrammarPool;
XERCES_CPP_NAMESPACE_END  

class XQillaDocumentImpl : public XERCES_CPP_NAMESPACE_QUALIFIER DOMDocumentImpl
{
public:
  XQillaDocumentImpl(XERCES_CPP_NAMESPACE_QUALIFIER DOMImplementation* domImpl,
                     XERCES_CPP_NAMESPACE_QUALIFIER MemoryManager* const manager);
  XQillaDocumentImpl(const XMLCh* namespaceURI, const XMLCh* qualifiedName,
                     XERCES_CPP_NAMESPACE_QUALIFIER DOMDocumentType* doctype,
                     XERCES_CPP_NAMESPACE_QUALIFIER DOMImplementation* domImpl,
                     XERCES_CPP_NAMESPACE_QUALIFIER MemoryManager* const manager);
  virtual ~XQillaDocumentImpl();
  
#if _XERCES_VERSION >= 30000
  virtual XERCES_CPP_NAMESPACE_QUALIFIER DOMXPathExpression*
  createExpression(const XMLCh *expression, const XERCES_CPP_NAMESPACE_QUALIFIER DOMXPathNSResolver *resolver);

  virtual XERCES_CPP_NAMESPACE_QUALIFIER DOMXPathNSResolver*
  createNSResolver(const XERCES_CPP_NAMESPACE_QUALIFIER DOMNode *nodeResolver);

  virtual XERCES_CPP_NAMESPACE_QUALIFIER DOMXPathResult*
  evaluate(const XMLCh *expression, const XERCES_CPP_NAMESPACE_QUALIFIER DOMNode *contextNode,
           const XERCES_CPP_NAMESPACE_QUALIFIER DOMXPathNSResolver *resolver,
           XERCES_CPP_NAMESPACE_QUALIFIER DOMXPathResult::ResultType type,
           XERCES_CPP_NAMESPACE_QUALIFIER DOMXPathResult* result);
#else
  virtual const XERCES_CPP_NAMESPACE_QUALIFIER DOMXPathExpression*
  createExpression(const XMLCh *expression, const XERCES_CPP_NAMESPACE_QUALIFIER DOMXPathNSResolver *resolver);
  
  virtual const XERCES_CPP_NAMESPACE_QUALIFIER DOMXPathNSResolver*
  createNSResolver(XERCES_CPP_NAMESPACE_QUALIFIER DOMNode* nodeResolver);
  
  virtual void* evaluate(const XMLCh* expression,
                         XERCES_CPP_NAMESPACE_QUALIFIER DOMNode* contextNode,
                         const XERCES_CPP_NAMESPACE_QUALIFIER DOMXPathNSResolver* resolver,
                         unsigned short type,
                         void* reuseableResult);
#endif
  
  virtual XERCES_CPP_NAMESPACE_QUALIFIER DOMNode *cloneNode(bool deep) const;

  virtual void setGrammarPool(XERCES_CPP_NAMESPACE_QUALIFIER XMLGrammarPool *xmlGrammarPool, bool adoptGramPool);
  virtual XERCES_CPP_NAMESPACE_QUALIFIER XMLGrammarPool *getGrammarPool();

protected:
  void becomeClone(const XQillaDocumentImpl *toClone, bool deep);

  XERCES_CPP_NAMESPACE_QUALIFIER MemoryManager *_createdWith;
  /// Used for creating objects returned from DOMXPathEvaluator methods
  ProxyMemoryManager _memMgr;

  XERCES_CPP_NAMESPACE_QUALIFIER XMLGrammarPool *_xmlGrammarPool;
  bool _adoptGramPool;

};


#endif

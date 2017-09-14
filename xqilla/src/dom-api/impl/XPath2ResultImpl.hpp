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

#ifndef __XPATH2RESULTIMPL_HPP
#define __XPATH2RESULTIMPL_HPP

#include <xqilla/framework/XQillaExport.hpp>
#include <xqilla/dom-api/XPath2Result.hpp>

#include <xqilla/runtime/Sequence.hpp>
#include <xqilla/runtime/Result.hpp>

#include <xercesc/dom/impl/DOMDocumentImpl.hpp>
#include <xercesc/dom/DOMXPathResult.hpp>

class DynamicContext;
class XQQuery;
class XQillaExpressionImpl;

class XQILLA_API XPath2ResultImpl : public
#if _XERCES_VERSION >= 30000
  XERCES_CPP_NAMESPACE_QUALIFIER DOMXPathResult,
#else
  XPath2Result,
#endif
  XERCES_CPP_NAMESPACE_QUALIFIER DOMTypeInfo
{
public:
  XPath2ResultImpl(XERCES_CPP_NAMESPACE_QUALIFIER DOMNode* contextNode,
                   DynamicContext *staticContext,
                   XERCES_CPP_NAMESPACE_QUALIFIER MemoryManager* memMgr,
                   XQillaExpressionImpl *exprImpl = 0);

  virtual ~XPath2ResultImpl();

  virtual bool isNode() const;
  virtual const XERCES_CPP_NAMESPACE_QUALIFIER DOMTypeInfo *getTypeInfo() const;


  virtual int getIntegerValue() const;
  virtual double getNumberValue() const;
  virtual const XMLCh* getStringValue() const;
  virtual bool getBooleanValue() const;
  virtual XERCES_CPP_NAMESPACE_QUALIFIER DOMNode* getNodeValue() const;

  /// DOMTypeInfo methods
#if _XERCES_VERSION >= 30000
    virtual const XMLCh* getTypeName() const;
    virtual const XMLCh* getTypeNamespace() const;
    virtual bool isDerivedFrom(const XMLCh* typeNamespaceArg,
                               const XMLCh* typeNameArg,
                               DerivationMethods derivationMethod) const;
#else
  virtual const XMLCh* getName() const;
  virtual const XMLCh* getNamespace() const;
#endif

protected:
  XERCES_CPP_NAMESPACE_QUALIFIER MemoryManager* _createdWith;

  DynamicContext* _context;
  Item::Ptr _currentItem;

  XQillaExpressionImpl *_exprToDelete;
};

class XQILLA_API XPath2FirstResultImpl : public XPath2ResultImpl
{
public:

  XPath2FirstResultImpl(const XQQuery *expression,
                        XERCES_CPP_NAMESPACE_QUALIFIER DOMNode* contextNode,
                        DynamicContext *staticContext,
                        XERCES_CPP_NAMESPACE_QUALIFIER MemoryManager* memMgr,
                        XQillaExpressionImpl *exprImpl = 0);

  virtual ~XPath2FirstResultImpl();
  virtual void release();

  virtual ResultType getResultType() const;

  virtual bool getInvalidIteratorState() const;
  virtual bool iterateNext();
#if _XERCES_VERSION >= 30000
  virtual XMLSize_t getSnapshotLength() const;
  virtual bool snapshotItem(XMLSize_t index);
#else
  virtual size_t getSnapshotLength() const;
  virtual bool snapshotItem(size_t index);
#endif  
};

class XQILLA_API XPath2SnapshotResultImpl : public XPath2ResultImpl
{
public:

  XPath2SnapshotResultImpl(const XQQuery *expression,
                           XERCES_CPP_NAMESPACE_QUALIFIER DOMNode* contextNode,
                           DynamicContext *staticContext,
                           XERCES_CPP_NAMESPACE_QUALIFIER MemoryManager* memMgr,
                           XQillaExpressionImpl *exprImpl = 0);

  virtual ~XPath2SnapshotResultImpl();
  virtual void release();

  virtual ResultType getResultType() const;

  virtual bool getInvalidIteratorState() const;
  virtual bool iterateNext();


#if _XERCES_VERSION >= 30000
  virtual XMLSize_t getSnapshotLength() const;
  virtual bool snapshotItem(XMLSize_t index);
#else 
  virtual size_t getSnapshotLength() const;
  virtual bool snapshotItem(size_t);
#endif

private:
  Sequence *_sequence;
};

class XQILLA_API XPath2IteratorResultImpl : public XPath2ResultImpl
{
public:
  XPath2IteratorResultImpl(const XQQuery *expression,
                           XERCES_CPP_NAMESPACE_QUALIFIER DOMNode* contextNode,
                           DynamicContext *staticContext,
                           XERCES_CPP_NAMESPACE_QUALIFIER MemoryManager* memMgr,
                           XQillaExpressionImpl *exprImpl = 0);

  virtual ~XPath2IteratorResultImpl();
  virtual void release();

  virtual ResultType getResultType() const;

  virtual bool getInvalidIteratorState() const;
  virtual bool iterateNext();

#if _XERCES_VERSION >= 30000
  virtual XMLSize_t getSnapshotLength() const;
  virtual bool snapshotItem(XMLSize_t index);
#else 
  virtual size_t getSnapshotLength() const;
  virtual bool snapshotItem(size_t); 
#endif

private:
  Result _results;

  const XERCES_CPP_NAMESPACE_QUALIFIER DOMDocumentImpl* _documentRoot;
  int _changes;
};

#endif

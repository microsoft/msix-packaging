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
#include "XPath2ResultImpl.hpp"
#include "XQillaExpressionImpl.hpp"
#include <string>

#include <xqilla/items/Node.hpp>
#include <xqilla/items/AnyAtomicType.hpp>
#include <xqilla/items/ATDecimalOrDerived.hpp>
#include <xqilla/items/ATDoubleOrDerived.hpp>
#include <xqilla/items/ATBooleanOrDerived.hpp>
#include <xqilla/exceptions/XQillaException.hpp>
#include <xqilla/exceptions/XPath2TypeCastException.hpp>
#include <xqilla/context/DynamicContext.hpp>
#include <xqilla/runtime/Result.hpp>
#include <xqilla/simple-api/XQQuery.hpp>
#include <xqilla/utils/UTF8Str.hpp>
#include <xqilla/utils/XPath2Utils.hpp>
#include <xqilla/functions/FunctionNumber.hpp>
#include <xqilla/functions/FunctionString.hpp>

#include <xqilla/exceptions/FunctionException.hpp>
#include <xqilla/items/DatatypeFactory.hpp>
#include <xqilla/context/ItemFactory.hpp>
#include <xqilla/xerces/XercesConfiguration.hpp>

#include <xercesc/dom/DOMException.hpp>
#include <xercesc/dom/DOMXPathException.hpp>
#include <xercesc/dom/DOMElement.hpp>
#include <xercesc/dom/DOMAttr.hpp>
#include <xercesc/dom/DOMNode.hpp>
#include <xercesc/validators/schema/SchemaSymbols.hpp>
#include <xercesc/dom/impl/DOMDocumentImpl.hpp>
#include <xercesc/util/XMLDOMMsg.hpp>

using namespace std;
XERCES_CPP_NAMESPACE_USE;

string typeName(XPath2Result::ResultType type) {
    switch(type) {
      case XPath2Result::FIRST_RESULT: return "FIRST_RESULT";
      case XPath2Result::ITERATOR_RESULT: return "ITERATOR_RESULT";
      case XPath2Result::SNAPSHOT_RESULT: return "SNAPSHOT_RESULT";
    }
    return "";
}

string errorMessage(XPath2Result::ResultType requestedType,
                         XPath2Result::ResultType resultType) {
  return typeName(requestedType) + " was requested from a XPath2Result of type " + typeName(resultType);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

XPath2ResultImpl::XPath2ResultImpl(DOMNode* contextNode,
                                   DynamicContext *staticContext,
                                   MemoryManager* memMgr,
                                   XQillaExpressionImpl *exprImpl)
  : _createdWith(memMgr),
    _context(exprImpl != 0 ? staticContext : staticContext->createDynamicContext(_createdWith)),
    _currentItem(0),
    _exprToDelete(exprImpl)
{
  //Check for illegal contextNode types
  if(contextNode != 0) {
    //More illegal types here?
    switch (contextNode->getNodeType()) {
    case DOMNode::ENTITY_REFERENCE_NODE:
      throw XQillaException(DOMException::NOT_SUPPORTED_ERR, X("Context node of illegal type."));
      break;
    default: break;
    }
    _context->setContextItem(((XercesConfiguration*)_context->getConfiguration())->
	    createNode(contextNode, _context));
  }
}

XPath2ResultImpl::~XPath2ResultImpl() { 
  _currentItem = 0;
  if(_exprToDelete) _exprToDelete->release();
  else delete _context;
}

const DOMTypeInfo *XPath2ResultImpl::getTypeInfo() const
{
  if(_currentItem.isNull()) return 0;
  return this;
}

bool XPath2ResultImpl::isNode() const {
  return !_currentItem.isNull() && _currentItem->isNode();
}

/// DOMTypeInfo methods
#if _XERCES_VERSION >= 30000
const XMLCh* XPath2ResultImpl::getTypeName() const
{
   if(_currentItem.isNull()) return 0;
   return _currentItem->getTypeName();
}

const XMLCh* XPath2ResultImpl::getTypeNamespace() const
{
   if(_currentItem.isNull()) return 0;
   return _currentItem->getTypeURI();
}

bool XPath2ResultImpl::isDerivedFrom(const XMLCh* typeNamespaceArg,
                               const XMLCh* typeNameArg,
                               DerivationMethods derivationMethod) const
{
  // TBD implement this - jpcs
  return false;
}
#else
const XMLCh* XPath2ResultImpl::getName() const
{
   if(_currentItem.isNull()) return 0;
   return _currentItem->getTypeName();
}

const XMLCh* XPath2ResultImpl::getNamespace() const
{
   if(_currentItem.isNull()) return 0;
   return _currentItem->getTypeURI();
}
#endif

int XPath2ResultImpl::getIntegerValue() const
{
  if(_currentItem.isNull()) {
    throw XQillaException(DOMException::INVALID_STATE_ERR, X("There is no current result in the result")); 
  }

  return FunctionNumber::number(_currentItem, _context, 0)->asInt();
}

double XPath2ResultImpl::getNumberValue() const
{
  if(_currentItem.isNull()) {
    throw XQillaException(DOMException::INVALID_STATE_ERR, X("There is no current result in the result"));
  }

  return FunctionNumber::number(_currentItem, _context, 0)->asDouble();
}

const XMLCh* XPath2ResultImpl::getStringValue() const
{
  if(_currentItem.isNull()) {
    throw XQillaException(DOMException::INVALID_STATE_ERR, X("There is no current result in the result"));
  }

  return FunctionString::string(_currentItem, _context);
}

bool XPath2ResultImpl::getBooleanValue() const
{
  if(_currentItem.isNull()) {
    throw XQillaException(DOMException::INVALID_STATE_ERR, X("There is no current result in the result"));
  }

  if(!_currentItem->isAtomicValue()) {
    throw XQillaException(DOMXPathException::TYPE_ERR, X("Cannot convert result to a boolean"));
  }

  AnyAtomicType::Ptr atom = (const AnyAtomicType::Ptr)_currentItem;
  Item::Ptr boolean;
  try {
    boolean = atom->castAs(AnyAtomicType::BOOLEAN, _context);
  } catch (XPath2TypeCastException &e) {
    throw XQillaException(DOMXPathException::TYPE_ERR, X("Cannot convert result to a boolean"));
  }
  return ((const ATBooleanOrDerived*)boolean.get())->isTrue();
}

DOMNode* XPath2ResultImpl::getNodeValue() const
{
  if(_currentItem.isNull()) {
    throw XQillaException(DOMException::INVALID_STATE_ERR, X("There is no current result in the result"));
  }

  if(!_currentItem->isNode()) {
    throw XQillaException(DOMXPathException::TYPE_ERR, X("The requested result is not a node"));
  }

  DOMNode *node = (DOMNode*)_currentItem->getInterface(XercesConfiguration::gXerces);
  if(node == 0) {
    // Should never happen
    throw XQillaException(DOMXPathException::TYPE_ERR, X("The requested result not a XQilla implementation node"));
  }

  return node;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

XPath2FirstResultImpl::XPath2FirstResultImpl(const XQQuery *expression,
                                                   DOMNode* contextNode,
                                                   DynamicContext *staticContext,
                                                   MemoryManager* memMgr,
                                                   XQillaExpressionImpl *exprImpl)
  : XPath2ResultImpl(contextNode, staticContext, memMgr, exprImpl)
{
  try {
    _currentItem = expression->execute(_context)->next(_context);
  }
  catch(const XQException &e) {
    if(XQillaException::getDebug()) {
      e.printDebug( X("Caught exception at Interface") );
    }    
    throw XQillaException(e);
  }
  catch(const XQillaException &) {
    // rethrow it
    throw;
  }
  catch(DOMException &e) {
    throw XQillaException(DOMXPathException::INVALID_EXPRESSION_ERR,
	    X("XQillaExpressionImpl::evaluateToSequence(): DOMException!"));
  }
  catch(XMLException &e) {
    throw XQillaException(DOMXPathException::INVALID_EXPRESSION_ERR, e.getMessage());
  }
  catch (...) {
    throw XQillaException(DOMXPathException::INVALID_EXPRESSION_ERR,
	    X("XQillaExpressionImpl::evaluateToSequence(): Unknown exception caught."));
  }
}

XPath2FirstResultImpl::~XPath2FirstResultImpl() { 
}

void XPath2FirstResultImpl::release()
{
  this->~XPath2FirstResultImpl();
  _createdWith->deallocate(this);
}

#if _XERCES_VERSION >= 30000
DOMXPathResult::ResultType XPath2FirstResultImpl::getResultType() const
{
  return DOMXPathResult::FIRST_RESULT_TYPE;
}
#else
XPath2Result::ResultType XPath2FirstResultImpl::getResultType() const
{
  return XPath2Result::FIRST_RESULT;
}
#endif

bool XPath2FirstResultImpl::getInvalidIteratorState() const {
  return false;
}

#if _XERCES_VERSION >= 30000
XMLSize_t XPath2FirstResultImpl::getSnapshotLength() const {
  string error = errorMessage(XPath2Result::SNAPSHOT_RESULT, XPath2Result::FIRST_RESULT);
  throw XQillaException(DOMXPathException::TYPE_ERR, X(error.c_str()));
}
#else
size_t XPath2FirstResultImpl::getSnapshotLength() const {
  string error = errorMessage(XPath2Result::SNAPSHOT_RESULT, XPath2Result::FIRST_RESULT);
  throw XQillaException(DOMXPathException::TYPE_ERR, X(error.c_str()));
}
#endif

bool XPath2FirstResultImpl::iterateNext() {
  string error = errorMessage(XPath2Result::ITERATOR_RESULT, XPath2Result::FIRST_RESULT);
  throw XQillaException(DOMXPathException::TYPE_ERR, X(error.c_str()));
}

#if _XERCES_VERSION >= 30000
bool XPath2FirstResultImpl::snapshotItem(XMLSize_t index) {
  string error = errorMessage(XPath2Result::SNAPSHOT_RESULT, XPath2Result::FIRST_RESULT);
  throw XQillaException(DOMXPathException::TYPE_ERR, X(error.c_str()));
}
#else
bool XPath2FirstResultImpl::snapshotItem(size_t index) {
  string error = errorMessage(XPath2Result::SNAPSHOT_RESULT, XPath2Result::FIRST_RESULT);
  throw XQillaException(DOMXPathException::TYPE_ERR, X(error.c_str()));
}
#endif
////////////////////////////////////////////////////////////////////////////////////////////////////

XPath2SnapshotResultImpl::XPath2SnapshotResultImpl(const XQQuery *expression,
                                                   DOMNode* contextNode,
                                                   DynamicContext *staticContext,
                                                   MemoryManager* memMgr,
                                                   XQillaExpressionImpl *exprImpl)
  : XPath2ResultImpl(contextNode, staticContext, memMgr, exprImpl),
    _sequence(0)
{
  try {
    Sequence seq = expression->execute(_context)->toSequence(_context);
    _sequence = new (_createdWith) Sequence(seq, _createdWith);
  }
  catch(const XQException &e) {
    if(XQillaException::getDebug()) {
      e.printDebug( X("Caught exception at Interface") );
    }    
    throw XQillaException(e);
  }
  catch(const XQillaException &) {
    // rethrow it
    throw;
  }
  catch(DOMException &e) {
    throw XQillaException(DOMXPathException::INVALID_EXPRESSION_ERR,
	    X("XQillaExpressionImpl::evaluateToSequence(): DOMException!"));
  }
  catch(XMLException &e) {
    throw XQillaException(DOMXPathException::INVALID_EXPRESSION_ERR, e.getMessage());
  }
  catch (...) {
    throw XQillaException(DOMXPathException::INVALID_EXPRESSION_ERR,
	    X("XQillaExpressionImpl::evaluateToSequence(): Unknown exception caught."));
  }
}

XPath2SnapshotResultImpl::~XPath2SnapshotResultImpl() { 
  // the destructor will try to access the context, that XPath2ResultImpl is about to delete
  if(_sequence) delete _sequence;
}

void XPath2SnapshotResultImpl::release()
{
  this->~XPath2SnapshotResultImpl();
  _createdWith->deallocate(this);
}

#if _XERCES_VERSION >= 30000
DOMXPathResult::ResultType XPath2SnapshotResultImpl::getResultType() const {
  return DOMXPathResult::SNAPSHOT_RESULT_TYPE;
}
#else
XPath2Result::ResultType XPath2SnapshotResultImpl::getResultType() const {
  return XPath2Result::SNAPSHOT_RESULT;
}
#endif

bool XPath2SnapshotResultImpl::getInvalidIteratorState() const {
  return false;
}

#if _XERCES_VERSION >= 30000
XMLSize_t XPath2SnapshotResultImpl::getSnapshotLength() const {
  return _sequence->getLength();
}
#else
size_t XPath2SnapshotResultImpl::getSnapshotLength() const {
  return _sequence->getLength();
}
#endif

bool XPath2SnapshotResultImpl::iterateNext() {
  string error = errorMessage(XPath2Result::ITERATOR_RESULT, XPath2Result::SNAPSHOT_RESULT);
  throw XQillaException(DOMXPathException::TYPE_ERR, X(error.c_str()));
}

#if _XERCES_VERSION >= 30000
bool XPath2SnapshotResultImpl::snapshotItem(XMLSize_t index) {
  //Reached end of set, return false
  if(index >= _sequence->getLength()) {
    _currentItem = 0;
    return false;
  }

  // this method is const, need to cast that away
  _currentItem = _sequence->item(index);
  return true;
}
#else
bool XPath2SnapshotResultImpl::snapshotItem(size_t index) {
  //Reached end of set, return false
  if(index >= _sequence->getLength()) {
    _currentItem = 0;
    return false;
  }

  // this method is const, need to cast that away
  _currentItem = _sequence->item(index);
  return true;
}
#endif
////////////////////////////////////////////////////////////////////////////////////////////////////

XPath2IteratorResultImpl::XPath2IteratorResultImpl(const XQQuery *expression,
                                                   DOMNode* contextNode,
                                                   DynamicContext *staticContext,
                                                   MemoryManager* memMgr,
                                                   XQillaExpressionImpl *exprImpl)
  : XPath2ResultImpl(contextNode, staticContext, memMgr, exprImpl),
    _results(0),
    _documentRoot(0),
    _changes(0)
{
  if(contextNode != 0) {
    if(contextNode->getNodeType() == DOMNode::DOCUMENT_NODE) {
      _documentRoot = (DOMDocumentImpl*)contextNode;
    }
    else {
      _documentRoot = (DOMDocumentImpl*)contextNode->getOwnerDocument();
    }
    _changes =_documentRoot->changes();
  }

  try {
    _results = expression->execute(_context);
  }
  catch(const XQException &e) {
    if(XQillaException::getDebug()) {
      e.printDebug( X("Caught exception at Interface") );
    }    
    throw XQillaException(e);
  }
  catch(const XQillaException &) {
    // rethrow it
    throw;
  }
  catch(DOMException &e) {
    throw XQillaException(DOMXPathException::INVALID_EXPRESSION_ERR,
	    X("XQillaExpressionImpl::evaluateToSequence(): DOMException!"));
  }
  catch(XMLException &e) {
    throw XQillaException(DOMXPathException::INVALID_EXPRESSION_ERR, e.getMessage());
  }
  catch (...) {
    throw XQillaException(DOMXPathException::INVALID_EXPRESSION_ERR,
	    X("XQillaExpressionImpl::evaluateToSequence(): Unknown exception caught."));
  }
}

XPath2IteratorResultImpl::~XPath2IteratorResultImpl()
{
  // the destructor will try to access the context, that XPath2ResultImpl is about to delete
  if(!_results.isNull()) _results = 0;
}

void XPath2IteratorResultImpl::release()
{
  this->~XPath2IteratorResultImpl();
  _createdWith->deallocate(this);
}

#if _XERCES_VERSION >= 30000
DOMXPathResult::ResultType XPath2IteratorResultImpl::getResultType() const
{
  return DOMXPathResult::ITERATOR_RESULT_TYPE;
}
#else
XPath2Result::ResultType XPath2IteratorResultImpl::getResultType() const
{
  return XPath2Result::ITERATOR_RESULT;
}
#endif

bool XPath2IteratorResultImpl::getInvalidIteratorState() const
{
  return _documentRoot != 0 && _documentRoot->changes() != _changes;
}

bool XPath2IteratorResultImpl::iterateNext()
{
  // check for document changes
  if(getInvalidIteratorState()) {
    throw XQillaException(DOMException::INVALID_STATE_ERR, X("Document has changed"));
  }

  try {
    _currentItem = _results->next(_context);
  }
  catch(const XQException &e) {
    if(XQillaException::getDebug()) {
      e.printDebug( X("Caught exception at Interface") );
    }    
    throw XQillaException(e);
  }

  if(_currentItem.isNull()) {
    _results = 0;
    return false;
  }
  
  return true;
}

#if _XERCES_VERSION >= 30000
XMLSize_t XPath2IteratorResultImpl::getSnapshotLength() const
{
  string error = errorMessage(XPath2Result::SNAPSHOT_RESULT, XPath2Result::ITERATOR_RESULT);
  throw XQillaException(DOMXPathException::TYPE_ERR, X(error.c_str()));
}

bool XPath2IteratorResultImpl::snapshotItem(XMLSize_t index)
{
  string error = errorMessage(XPath2Result::SNAPSHOT_RESULT, XPath2Result::ITERATOR_RESULT);
  throw XQillaException(DOMXPathException::TYPE_ERR, X(error.c_str()));
}
#else
size_t XPath2IteratorResultImpl::getSnapshotLength() const
{
  string error = errorMessage(XPath2Result::SNAPSHOT_RESULT, XPath2Result::ITERATOR_RESULT);
  throw XQillaException(DOMXPathException::TYPE_ERR, X(error.c_str()));
}

bool XPath2IteratorResultImpl::snapshotItem(size_t index)
{
  string error = errorMessage(XPath2Result::SNAPSHOT_RESULT, XPath2Result::ITERATOR_RESULT);
  throw XQillaException(DOMXPathException::TYPE_ERR, X(error.c_str()));
}
#endif


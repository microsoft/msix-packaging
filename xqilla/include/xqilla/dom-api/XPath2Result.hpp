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

#ifndef __XPATH2RESULT_HPP
#define __XPATH2RESULT_HPP

#include <xqilla/framework/XQillaExport.hpp>

#include <xercesc/dom/DOMNode.hpp>
#include <xercesc/dom/DOMTypeInfo.hpp>
#include <xercesc/dom/DOMException.hpp>
#include <xercesc/dom/DOMXPathException.hpp>

/**
 * The XPathResult2 interface represents the result of the evaluation of an XPath
 * 2.0 expression within the context of a particular node. Since evaluation of an
 * XPath 2.0 expression can result in various result types, this object makes it
 * possible to discover and manipulate the type and value of the result. 
 * @since DOM Level 3
 */
class XQILLA_API XPath2Result
{
public:
  /** @name Destructor */
  //@{
  /**
   * Destructor
   */
  virtual ~XPath2Result() {};
  //@}

  /** @name Public Constants */
  //@{
  /**
   * <p>FIRST_RESULT
   * <br>The result is a sequence as defined by XPath 2.0 and will be accessed
   * as a single current value or there will be no current value if the sequence
   * is empty. Document modification does not invalidate the value, but may mean
   * that the result no longer corresponds to the current document. This is a
   * convenience that permits optimization since the implementation can stop once
   * the first item in the resulting sequence has been found. If there is more
   * than one item in the actual result, the single item returned might not be
   * the first in document order.
   * <p>ITERATOR_RESULT
   * <br>The result is a sequence as defined by XPath 2.0 that will be accessed
   * iteratively. Document modification invalidates the iteration.
   * <p>SNAPSHOT_RESULT
   * <br>The result is a sequence as defined by XPath 2.0 that will be accessed
   * as a snapshot list of values. Document modification does not invalidate the
   * snapshot but may mean that reevaluation would not yield the same snapshot
   * and any items in the snapshot may have been altered, moved, or removed from
   * the document.
   */
  enum ResultType {
    FIRST_RESULT    = 100,
    ITERATOR_RESULT = 101,
    SNAPSHOT_RESULT = 102
  };
  //@}

  /** @name Functions introduced in DOM Level 3 */
  //@{

  /**
   * Returns the result type of this result
   * @return ResultType 
   * A code representing the type of this result, as defined by the type constants.
   */
  virtual ResultType getResultType() const = 0;

  /**
   * Returns true if the result has a current result and the value is a node.
   * @return isNode of type boolean, readonly
   */
  virtual bool isNode() const = 0;

  /**
   * Returns the DOM type info of the current result node or value.
   * @return typeInfo of type TypeInfo, readonly
   */
  virtual const XERCES_CPP_NAMESPACE_QUALIFIER DOMTypeInfo *getTypeInfo() const = 0;

  /**
   * Conversion of the current result to double. If the native double type of the
   * DOM binding does not directly support the exact IEEE 754 result of the XPath
   * expression, then it is up to the definition of the binding to specify how the
   * XPath number is converted to the native binding number.
   * @return asDouble of type double, readonly
   * @exception XPathException
   * TYPE_ERR: raised if current result cannot be properly converted to double.
   * @exception DOMException
   * INVALID_STATE_ERR: There is no current result in the result.
   */
  virtual double getNumberValue() const = 0;

  /**
   * Conversion of the current result to int.
   * @return asInt of type int, readonly
   * @exception XPathException
   * TYPE_ERR: raised if current result cannot be properly converted to int.
   * @exception DOMException
   * INVALID_STATE_ERR: There is no current result in the result.
   */
  virtual int getIntegerValue() const = 0;

  /**
   * Conversion of the current result to string.
   * @return asString of type DOMString, readonly
   * @exception XPathException
   * TYPE_ERR: raised if current result cannot be properly converted to string.
   * @exception DOMException
   * INVALID_STATE_ERR: There is no current result in the result.
   */
  virtual const XMLCh* getStringValue() const = 0;

  /**
   * Conversion of the current result to boolean.
   * @return asBoolean of type boolean, readonly
   * @exception XPathException
   * TYPE_ERR: raised if  cannot be properly converted to boolean.
   * @exception DOMException
   * INVALID_STATE_ERR: There is no current result in the result.
   */
  virtual bool getBooleanValue() const = 0;

  /**
   * Retrieve the current node value.
   * @return asNode of type Node, readonly
   * @exception XPathException
   * TYPE_ERR: raised if current result is not a node.
   * @exception DOMException
   * INVALID_STATE_ERR: There is no current result in the result.
   */
  virtual XERCES_CPP_NAMESPACE_QUALIFIER DOMNode* getNodeValue() const = 0;

  /**
   * Signifies that the iterator has become invalid.
   * @return invalidIteratorState of type boolean, readonly
   */
  virtual bool getInvalidIteratorState() const = 0;

  /**
   * The number of items in the result snapshot. Valid values for snapshotItem
   * indices are 0 to snapshotLength-1 inclusive.
   * @return snapshotLength of type size_t, readonly
   * @exception XPathException
   * TYPE_ERR: raised if resultType is not SNAPSHOT_RESULT.
   */
  virtual size_t getSnapshotLength() const = 0;

  /**
   * Iterates and returns true if the current result is the next item from the
   * sequence or false if there are no more items.
   * @return boolean True if the current result is the next item from the sequence
   * or false if there are no more items.
   * @exception XPathException
   * TYPE_ERR: raised if resultType is not ITERATOR_RESULT.
   * @exception DOMException
   * INVALID_STATE_ERR: The document has been mutated since the result was returned.
   */
  virtual bool iterateNext() = 0;

  /**
   * Sets the current result to the indexth item in the snapshot collection. If
   * index is greater than or equal to the number of items in the list, this method
   * returns false. Unlike the iterator result, the snapshot does not become
   * invalid, but may not correspond to the current document if it is mutated.
   * @param index of type size_t - Index into the snapshot collection.
   * @return boolean True if the current result is the next item from the sequence
   * or false if there are no more items.
   * @exception XPathException
   * TYPE_ERR: raised if resultType is not SNAPSHOT_RESULT.
   */
  virtual bool snapshotItem(size_t) = 0;
  //@}

  /** @name Non-standard Extension */
  //@{
  /**
   * Called to indicate that this object (and its associated children) is no longer in use
   * and that the implementation may relinquish any resources associated with it and
   * its associated children.
   *
   * Access to a released object will lead to unexpected result.
   */
  virtual void release() = 0;
  //@}

protected:
  /** @name Hidden constructors */
  //@{    
  XPath2Result() {};
  //@}
private:
  /** @name Unimplemented constructors and operators */
  //@{
  XPath2Result(const XPath2Result &);
  XPath2Result &operator=(const XPath2Result &);
  //@}
};

#endif

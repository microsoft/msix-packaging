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

#ifndef __XQILLAEXPRESSION_HPP
#define __XQILLAEXPRESSION_HPP

#include <xqilla/framework/XQillaExport.hpp>

#include <xqilla/dom-api/XPath2Result.hpp>
#include <xqilla/runtime/Sequence.hpp>
#include <xercesc/dom/DOMException.hpp>
#include <xercesc/dom/DOMNode.hpp>
#include <xercesc/dom/DOMXPathExpression.hpp>
#include <xercesc/dom/DOMXPathException.hpp>


class DynamicContext;

/**
 * The <code>XQillaExpression</code> interface extends the <code>DOMXPathExpression</code> interface, 
 * providing the ability to release the object. The evaluate method will always return an object of type
 * <code>XPath2Result</code>
 * @since DOM Level 3
 */
class XQILLA_API XQillaExpression: public XERCES_CPP_NAMESPACE_QUALIFIER DOMXPathExpression
{
public:
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
};

#endif


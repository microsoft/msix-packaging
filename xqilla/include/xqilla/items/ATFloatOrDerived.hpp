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

#ifndef _ATFLOATORDERIVED_HPP
#define _ATFLOATORDERIVED_HPP

#include <xercesc/util/XercesDefs.hpp>
#include <xqilla/framework/XQillaExport.hpp>
#include <xqilla/items/Numeric.hpp>

class XQILLA_API ATFloatOrDerived : public Numeric
{
public:
  typedef RefCountPointer<const ATFloatOrDerived> Ptr;

  /* Get the name of the primitive type (basic type) of this type
   * (ie "decimal" for xs:decimal) */
  virtual const XMLCh* getPrimitiveTypeName() const = 0;

  /* Get the namespace URI for this type */
  virtual const XMLCh* getTypeURI() const = 0;

  /* Get the name of this type  (ie "integer" for xs:integer) */
  virtual const XMLCh* getTypeName() const = 0;

  /* returns the XMLCh* (canonical) representation of this type */
  virtual const XMLCh* asString(const DynamicContext* context) const = 0;

  /* Promote this to the given type, if possible */
  virtual Numeric::Ptr promoteTypeIfApplicable(AnyAtomicType::AtomicObjectType typeIndex,
                                               const DynamicContext* context) const = 0;

  /** Returns a Numeric object which is the sum of this and other */
  virtual Numeric::Ptr add(const Numeric::Ptr &other, const DynamicContext* context) const = 0;

  /** Returns a Numeric object which is the difference of this and
   * other */
  virtual Numeric::Ptr subtract(const Numeric::Ptr &other, const DynamicContext* context) const = 0;

  /** Returns a Numeric object which is the product of this and other */
  virtual Numeric::Ptr multiply(const Numeric::Ptr &other, const DynamicContext* context) const = 0;

  /** Returns a Numeric object which is the quotient of this and other */
  virtual Numeric::Ptr divide(const Numeric::Ptr &other, const DynamicContext* context) const = 0;

  /** Returns the arithmetic product of its operands as a Numeric */
  virtual Numeric::Ptr mod(const Numeric::Ptr &other, const DynamicContext* context) const = 0;

  /** Returns the floor of this Numeric */
  virtual Numeric::Ptr floor(const DynamicContext* context) const = 0;

  /** Returns the ceiling of this Numeric */
  virtual Numeric::Ptr ceiling(const DynamicContext* context) const = 0;

  /** Rounds this Numeric */
  virtual Numeric::Ptr round(const DynamicContext* context) const = 0;

  /** Rounds this Numeric to the given precision, and rounds a half to even */
  virtual Numeric::Ptr roundHalfToEven(const Numeric::Ptr &precision, const DynamicContext* context) const = 0;

  /** Returns the Additive inverse of this Numeric */
  virtual Numeric::Ptr invert(const DynamicContext* context) const = 0;
  
  /** Returns the absolute value of this Numeric */
  virtual Numeric::Ptr abs(const DynamicContext* context) const = 0;

  /** Returns the square root of this Numeric */
  virtual Numeric::Ptr sqrt(const DynamicContext* context) const = 0;

  /** Returns the sinus of this Numeric */
  virtual Numeric::Ptr sin(const DynamicContext* context) const = 0;

  /** Returns the cosinus of this Numeric */
  virtual Numeric::Ptr cos(const DynamicContext* context) const = 0;

  /** Returns the tan of this Numeric */
  virtual Numeric::Ptr tan(const DynamicContext* context) const = 0;

  /** Returns the asin of this Numeric */
  virtual Numeric::Ptr asin(const DynamicContext* context) const = 0;

  /** Returns the acos of this Numeric */
  virtual Numeric::Ptr acos(const DynamicContext* context) const = 0;

  /** Returns the atan of this Numeric */
  virtual Numeric::Ptr atan(const DynamicContext* context) const = 0;

  /** Returns the atan of this Numeric */
  virtual Numeric::Ptr log(const DynamicContext* context) const = 0;

  /** Returns the exp of this Numeric */
  virtual Numeric::Ptr exp(const DynamicContext* context) const = 0;

  /* Is this xs:double not a number */
  virtual bool isNaN() const = 0;

  /* Is this xs:double infinite? */
  virtual bool isInfinite() const = 0;

  /** Does this Numeric have value 0? */
  virtual bool isZero() const = 0;

  /** Is this Numeric positive? */
  virtual bool isPositive() const = 0;

  /** Is this Numeric negative? */
  virtual bool isNegative() const = 0;  

  virtual AnyAtomicType::AtomicObjectType getPrimitiveTypeIndex() const = 0;

  virtual const MAPM &asMAPM() const = 0;

  virtual State getState() const = 0;
};

#endif //  _ATFLOATORDERIVED_HPP

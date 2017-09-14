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

#ifndef __NUMERIC_HPP
#define __NUMERIC_HPP

#include <xercesc/util/XercesDefs.hpp>
#include <xqilla/framework/XQillaExport.hpp>
#include <xqilla/items/AnyAtomicType.hpp>

class DynamicContext;
class StaticContext;
class MAPM;

class XQILLA_API Numeric : public AnyAtomicType
{
public:
  typedef RefCountPointer<const Numeric> Ptr;

  /* is this type numeric?  Return true */
  virtual bool isNumericValue() const { return true; };

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
  
  /* returns true if the two Numeric values are equal 
   * false otherwise */
  virtual bool equals(const AnyAtomicType::Ptr &target, const DynamicContext* context) const;

  /** Returns true if this is less than other, false otherwise */
  virtual bool lessThan(const Numeric::Ptr &other, const DynamicContext* context) const;

  /** Returns true if this is greater than other, false otherwise */
  virtual bool greaterThan(const Numeric::Ptr &other, const DynamicContext* context) const;

  /** Returns less than 0 if this is less that other,
      0 if they are the same, and greater than 0 otherwise */
  virtual int compare(const Numeric::Ptr &other, const DynamicContext *context) const;

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
 
  /** Returns the squart root of this Numeric */
  virtual Numeric::Ptr sqrt(const DynamicContext* context) const = 0;

  /** Returns the sin of this Numeric */
  virtual Numeric::Ptr sin(const DynamicContext* context) const = 0;

  /** Returns the cos root of this Numeric */
  virtual Numeric::Ptr cos(const DynamicContext* context) const = 0;

  /** Returns the tan of this Numeric */
  virtual Numeric::Ptr tan(const DynamicContext* context) const = 0;

  /** Returns the asin of this Numeric */
  virtual Numeric::Ptr asin(const DynamicContext* context) const = 0;

  /** Returns the acos of this Numeric */
  virtual Numeric::Ptr acos(const DynamicContext* context) const = 0;

  /** Returns the atan of this Numeric */
  virtual Numeric::Ptr atan(const DynamicContext* context) const = 0;

  /** Returns the ln of this Numeric */
  virtual Numeric::Ptr log(const DynamicContext* context) const = 0;

  /** Returns the exp of this Numeric */
  virtual Numeric::Ptr exp(const DynamicContext* context) const = 0;

  /** Returns a Numeric object which is this raised to the power of other */
  virtual Numeric::Ptr power(const Numeric::Ptr &other, const DynamicContext* context) const = 0;


  /** Does this Numeric have value 0? */
  virtual bool isZero() const = 0;

  /** Is this Numeric positive? */
  virtual bool isPositive() const = 0;

  /** Is this Numeric negative? */
  virtual bool isNegative() const = 0;

  /* Is this floating point value not a number? */
  virtual bool isNaN() const = 0;

  /* Is this floating point value infinite? */
  virtual bool isInfinite() const = 0;

  /* Is this floating point value infinite? */
  virtual bool isInteger() const;

  virtual AnyAtomicType::AtomicObjectType getPrimitiveTypeIndex() const = 0;

  virtual const MAPM &asMAPM() const = 0;
  virtual double asDouble() const;
  virtual int asInt() const;

  enum State {
    NEG_INF = 0,
    NEG_NUM = 1,
    NUM = 2,
    INF = 3,
    NaN = 4
  };

  virtual State getState() const = 0;

  static const XMLCh NaN_string[];
  static const XMLCh NAN_string[];
  static const XMLCh INF_string[];
  static const XMLCh NegINF_string[];
  static const XMLCh NegZero_string[];
  static const XMLCh PosZero_string[];

  static void checkFloatLimits(Numeric::State &state, MAPM &value);
  static void checkDoubleLimits(Numeric::State &state, MAPM &value);

  static const XMLCh *asDecimalString(const MAPM &value, int significantDigits, const StaticContext* context);
  static const XMLCh *asDoubleString(State state, const MAPM &value, int significantDigits, const StaticContext* context);

protected:
  virtual AnyAtomicType::Ptr castAsInternal(AtomicObjectType targetIndex, const XMLCh* targetURI,
                                            const XMLCh* targetType, const DynamicContext* context) const;

  const XMLCh *asDecimalString(int significantDigits, const StaticContext* context) const;
  const XMLCh *asDoubleString(int significantDigits, const StaticContext* context) const;
  
};

#endif //  __NUMERIC_HPP

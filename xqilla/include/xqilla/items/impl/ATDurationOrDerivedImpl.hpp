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

#ifndef _ATDURATIONORDERIVEDIMPL_HPP
#define _ATDURATIONORDERIVEDIMPL_HPP

#include <xqilla/framework/XQillaExport.hpp>
#include <xqilla/items/ATDurationOrDerived.hpp>
#include <xqilla/mapm/m_apm.h>
#include <xercesc/util/XercesDefs.hpp>

class AnyAtomicType;
class ATDecimalOrDerived;
class XPath2MemoryManager;
class MAPM;

class XQILLA_API ATDurationOrDerivedImpl : public ATDurationOrDerived 
{

public:

  /* constructor */
  ATDurationOrDerivedImpl(const XMLCh* typeURI, const XMLCh* typeName, const XMLCh* value, const DynamicContext* context);
  /* constructor */
  ATDurationOrDerivedImpl(const XMLCh* typeURI, const XMLCh* typeName, const MAPM &months, const MAPM &seconds,
                          const DynamicContext* context);

  virtual void *getInterface(const XMLCh *name) const;

  /* Get the name of the primitive type (basic type) of this type 
   * (ie "decimal" for xs:decimal) */
  virtual const XMLCh* getPrimitiveTypeName() const;

  /* Get the namespace URI for this type */
  virtual const XMLCh* getTypeURI() const;

  /* Get the name of this type  (ie "integer" for xs:integer) */
  virtual const XMLCh* getTypeName() const;

  /* returns the XMLCh* (canonical) representation of this type */
  virtual const XMLCh* asString(const DynamicContext* context) const;

  /* returns true if this duration is an instance of a xdt:dayTimeDuration */
  virtual bool isDayTimeDuration() const;

  /* returns true if this duration is an instance of a xdt:yearMonthDuration */
  virtual bool isYearMonthDuration() const;
  
  /* returns true if the two objects have the same duration
   * false otherwise */
  virtual bool equals(const AnyAtomicType::Ptr &target, const DynamicContext* context) const;

  /** Returns less than 0 if this is less that other,
      0 if they are the same, and greater than 0 otherwise */
  virtual int compare(const ATDurationOrDerived::Ptr &other, const DynamicContext *context) const;

  /** Divide this duration by a number -- only available for xdt:dayTimeDuration
   *  and xdt:yearMonthDuration */
  virtual ATDurationOrDerived::Ptr divide(const Numeric::Ptr &divisor, const DynamicContext* context) const;

  /** Divide this duration by a duration -- only available for xdt:dayTimeDuration
   *  and xdt:yearMonthDuration */
  virtual ATDecimalOrDerived::Ptr divide(const ATDurationOrDerived::Ptr &divisor, const DynamicContext* context) const;

  /** Multiply this duration by a number -- only available for xdt:dayTimeDuration
   *  and xdt:yearMonthDuration */
  virtual ATDurationOrDerived::Ptr multiply(const Numeric::Ptr &multiplier, const DynamicContext* context) const;

  /** Add a duration to this duration -- only available for xdt:dayTimeDuration 
   *  and xdt:yearMonthDuration */
  virtual ATDurationOrDerived::Ptr add(const ATDurationOrDerived::Ptr &other, const DynamicContext* context) const;

  /** Subtract a duration from this duration -- only available for xdt:dayTimeDuration
   *  and xdt:yearMonthDuration */
  virtual ATDurationOrDerived::Ptr subtract(const ATDurationOrDerived::Ptr &other, const DynamicContext* context) const;

  /** Returns the year portion of this duration */
  virtual ATDecimalOrDerived::Ptr getYears(const DynamicContext* context) const;

  /** Returns the month portion of this duration */
  virtual ATDecimalOrDerived::Ptr getMonths(const DynamicContext* context) const;

  /** Returns the days portion of this duration */
  virtual ATDecimalOrDerived::Ptr getDays(const DynamicContext* context) const;

  /** Returns the hours portion of this duration */
  virtual ATDecimalOrDerived::Ptr getHours(const DynamicContext* context) const;

  /** Returns the minutes portion of this duration */
  virtual ATDecimalOrDerived::Ptr getMinutes(const DynamicContext* context) const;

  /** Returns the seconds portion of this duration */
  virtual ATDecimalOrDerived::Ptr getSeconds(const DynamicContext* context) const;

  /* return this duration in forms of seconds */
  virtual ATDecimalOrDerived::Ptr asSeconds(const DynamicContext* context) const;

  /* return this duration in forms of months */
  virtual ATDecimalOrDerived::Ptr asMonths(const DynamicContext* context) const;

  /** Returns true if this Duration is negative, false otherwise */
  virtual bool isNegative() const;

  /* Get the primitive index associated with this type */
  static AnyAtomicType::AtomicObjectType getTypeIndex(); 

  /* Get the primitive type name */
  static const XMLCh* getPrimitiveName();
  
  virtual AnyAtomicType::AtomicObjectType getPrimitiveTypeIndex() const;

  static void parseDuration(const XMLCh *const s, MAPM &months, MAPM &seconds);

protected:
  
  /* If possible, cast this type to the target type */
  virtual AnyAtomicType::Ptr castAsInternal(AtomicObjectType targetIndex, const XMLCh* targetURI, const XMLCh* targetType, const DynamicContext* context) const;

private:
  /*The values that make up this duration */
  bool _isPositive;
  MAPM _months;
  MAPM _seconds;

  AtomicObjectType _durationType;
  
  /* the name of this type */
  const XMLCh* _typeName;

  /* the uri of this type */
  const XMLCh* _typeURI;

  void init(const DynamicContext* context);
  void setDuration(const XMLCh* const value);
 
};

#endif // _ATDURATIONORDERIVEDIMPL_HPP

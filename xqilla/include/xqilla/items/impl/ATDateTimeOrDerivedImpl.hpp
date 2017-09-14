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

#ifndef _ATDATETIMEORDERIVEDIMPL_HPP
#define _ATDATETIMEORDERIVEDIMPL_HPP

#include <xercesc/util/XMLString.hpp>
#include <xqilla/items/ATDateTimeOrDerived.hpp>
#include <xqilla/mapm/m_apm.h>

#include <xqilla/framework/XQillaExport.hpp>

class Timezone;
class ATDecimalOrDerived;
class ATDurationOrDerived;
class XPath2MemoryManager;
class DynamicContext;
class StaticContext;

class XQILLA_API ATDateTimeOrDerivedImpl : public ATDateTimeOrDerived 
{

public:

  /* constructor */
  ATDateTimeOrDerivedImpl(const XMLCh* typeURI, const XMLCh* typeName, const XMLCh* value, const DynamicContext* context = 0);

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

  /* returns true if the two objects represent the same date,
   * false otherwise */
  virtual bool equals(const AnyAtomicType::Ptr &target, const DynamicContext* context) const;

  /** Returns less than 0 if this is less that other,
      0 if they are the same, and greater than 0 otherwise */
  virtual int compare(const ATDateTimeOrDerived::Ptr &other, const DynamicContext *context) const;

  /** 
   * Returns an integer representing the year component  of this object
   */
  virtual ATDecimalOrDerived::Ptr getYears(const DynamicContext *context) const;

  /** 
   * Returns an integer representing the month component  of this object
   */
  virtual ATDecimalOrDerived::Ptr getMonths(const DynamicContext *context) const;

  /** 
   * Returns an integer representing the day component  of this object
   */
  virtual ATDecimalOrDerived::Ptr getDays(const DynamicContext *context) const;

  /** 
   * Returns an integer representing the hour component of this object
   */
  virtual ATDecimalOrDerived::Ptr getHours(const DynamicContext *context) const;

  /** 
   * Returns an integer representing the minute component of this object
   */
  virtual ATDecimalOrDerived::Ptr getMinutes(const DynamicContext *context) const;

  /** 
   * Returns an decimal representing the second component of this object
   */
  virtual ATDecimalOrDerived::Ptr getSeconds(const DynamicContext *context) const;

  /**
   * Returns the timezone associated with this object, or
   * null, if the timezone is not set
   */
  virtual const Timezone::Ptr &getTimezone() const;
  
  /**
   * Returns true if the timezone is defined for this object, false otherwise.
   */
  virtual bool hasTimezone() const;

  /**
   * Setter for timezone.  Overrides the current timezone. (Not to be 
   * confused with addTimezone().
   */
  virtual ATDateTimeOrDerived::Ptr setTimezone(const Timezone::Ptr &timezone, const DynamicContext* context) const;

  /**
   * Returns an ATDateTimeOrDerived with a timezone added to it
   */
  virtual ATDateTimeOrDerived::Ptr addTimezone(const ATDurationOrDerived::Ptr &timezone, const DynamicContext* context) const;
  
  /**
   * Returns a date with the given yearMonthDuration added to it
   */
  virtual ATDateTimeOrDerived::Ptr addYearMonthDuration(const ATDurationOrDerived::Ptr &yearMonth,  const DynamicContext* context) const;

  /**
   * Returns a date with the given dayTimeDuration added to it
   */
  virtual ATDateTimeOrDerived::Ptr addDayTimeDuration(const ATDurationOrDerived::Ptr &dayTime, const DynamicContext* context) const;
  
  /**
   * Returns a date with the given yearMonthDuration subtracted from it
   */
  virtual ATDateTimeOrDerived::Ptr subtractYearMonthDuration(const ATDurationOrDerived::Ptr &yearMonth, const DynamicContext* context) const;

  /**
   * Returns a date with the given dayTimeDuration subtracted from it
   */
  virtual ATDateTimeOrDerived::Ptr subtractDayTimeDuration(const ATDurationOrDerived::Ptr &dayTime, const DynamicContext* context) const;

  /**
   * Returns a dayTimeDuration corresponding to the difference between this
   * and the given ATDateTimeOrDerived*
   */
  virtual ATDurationOrDerived::Ptr subtractDateTimeAsDayTimeDuration(const ATDateTimeOrDerived::Ptr &date, const DynamicContext* context) const;

  /**
   * Returns a dayTimeDuration corresponding to the difference between this
   * and the given ATDateTimeOrDerived*
   */
  virtual ATDurationOrDerived::Ptr subtractDateTimeAsYearMonthDuration(const ATDateTimeOrDerived::Ptr &date, const DynamicContext* context) const;

  /* Get the primitive index associated with this type */
  static AnyAtomicType::AtomicObjectType getTypeIndex(); 
  
  /* Get the primitive type name */
  static const XMLCh* getPrimitiveName();
  
  /* Get the primitive index associated with this type */
  virtual AnyAtomicType::AtomicObjectType getPrimitiveTypeIndex() const;

  static MAPM addYearMonthDuration(const MAPM &seconds, const MAPM &monthsToAdd);
	
  static MAPM parseDateTime(const XMLCh* const dateTime, const MAPM &implicitTimezone);

protected:
  
  /* If possible, cast this type to the target type */
  virtual AnyAtomicType::Ptr castAsInternal(AtomicObjectType targetIndex, const XMLCh* targetURI, const XMLCh* targetType, const DynamicContext* context) const;

private:
  // private constructor for use with clone()
  ATDateTimeOrDerivedImpl(const XMLCh* typeURI, const XMLCh* typeName, const MAPM &seconds, 
                          const Timezone::Ptr &timezone, bool hasTimezone);

  void setDateTime(const XMLCh* const date);

  ATDateTimeOrDerived::Ptr addDayTimeDuration(const MAPM &seconds, const DynamicContext* context) const;

  ATDateTimeOrDerived::Ptr addYearMonthDuration(const MAPM &months, const DynamicContext* context) const;

  /*The value of this dateTime, in seconds since 0001-01-01T00:00:00,
    always normalized if a timezone is present*/
  MAPM seconds_;
  
  /* the timezone associated with this ATDateTimeOrDerived */
  Timezone::Ptr timezone_;
  
  /* true if the timezone is defined */
  bool _hasTimezone;

  /* the name of this type */
  const XMLCh* _typeName;

  /* the uri of this type */
  const XMLCh* _typeURI;
 
};

#endif // _ATDATETIMEORDERIVEDIMPL_HPP

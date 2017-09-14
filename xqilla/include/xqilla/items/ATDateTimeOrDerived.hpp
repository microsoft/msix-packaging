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

#ifndef _ATDATETIMEORDERIVED_HPP
#define _ATDATETIMEORDERIVED_HPP

#include <xercesc/util/XMLString.hpp>
#include <xqilla/items/DateOrTimeType.hpp>

#include <xqilla/framework/XQillaExport.hpp>
#include <xqilla/items/ATDecimalOrDerived.hpp>
#include <xqilla/items/ATDurationOrDerived.hpp>
#include <xqilla/items/Timezone.hpp>

class XQILLA_API ATDateTimeOrDerived : public DateOrTimeType
{
public:
  typedef RefCountPointer<const ATDateTimeOrDerived> Ptr;

  /* Get the name of the primitive type (basic type) of this type
   * (ie "decimal" for xs:decimal) */
  virtual const XMLCh* getPrimitiveTypeName() const = 0;

  /* Get the namespace URI for this type */
  virtual const XMLCh* getTypeURI() const = 0;

  /* Get the name of this type  (ie "integer" for xs:integer) */
  virtual const XMLCh* getTypeName() const = 0;

  /* returns the XMLCh* (lexical) representation of this type */
  virtual const XMLCh* asString(const DynamicContext* context) const = 0;

  /* returns true if the two objects represent the same date,
   * false otherwise */
  virtual bool equals(const AnyAtomicType::Ptr &target, const DynamicContext* context) const = 0;

  /** Returns less than 0 if this is less that other,
      0 if they are the same, and greater than 0 otherwise */
  virtual int compare(const ATDateTimeOrDerived::Ptr &other, const DynamicContext *context) const = 0;
 
  /** 
   * Returns an integer representing the year component  of this object
   */
  virtual ATDecimalOrDerived::Ptr getYears(const DynamicContext *context) const = 0;

  /** 
   * Returns an integer representing the month component  of this object
   */
  virtual ATDecimalOrDerived::Ptr getMonths(const DynamicContext *context) const = 0;

  /** 
   * Returns an integer representing the day component  of this object
   */
  virtual ATDecimalOrDerived::Ptr getDays(const DynamicContext *context) const = 0;

  /** 
   * Returns an integer representing the hour component of this object
   */
  virtual ATDecimalOrDerived::Ptr getHours(const DynamicContext *context) const = 0;

  /** 
   * Returns an integer representing the minute component of this object
   */
  virtual ATDecimalOrDerived::Ptr getMinutes(const DynamicContext *context) const = 0;

  /** 
   * Returns a decimal representing the second component of this object
   */
  virtual ATDecimalOrDerived::Ptr getSeconds(const DynamicContext *context) const = 0;

  /** 
   * Returns a timezone object representing the timezone component of this object
   */
  virtual const Timezone::Ptr &getTimezone() const = 0;

  /**
   * Returns true if the timezone is defined for this object, false otherwise.
   */
  virtual bool hasTimezone() const = 0;

  /**
   * Setter for timezone.  Overrides the current timezone. (Not to be 
   * confused with addTimezone().
   */
  virtual ATDateTimeOrDerived::Ptr setTimezone(const Timezone::Ptr &timezone, const DynamicContext* context) const = 0;

  /**
   * Returns an ATDateTimeOrDerived with a timezone added to it
   */
  virtual ATDateTimeOrDerived::Ptr addTimezone(const ATDurationOrDerived::Ptr &timezone, const DynamicContext* context) const = 0;

  /**
   * Returns a date with the given yearMonthDuration added to it
   */
  virtual ATDateTimeOrDerived::Ptr addYearMonthDuration(const ATDurationOrDerived::Ptr &yearMonth,  const DynamicContext* context) const = 0;

  /**
   * Returns a date with the given dayTimeDuration added to it
   */
  virtual ATDateTimeOrDerived::Ptr addDayTimeDuration(const ATDurationOrDerived::Ptr &dayTime,  const DynamicContext* context) const = 0;
  
  /**
   * Returns a date with the given yearMonthDuration subtracted from it
   */
  virtual ATDateTimeOrDerived::Ptr subtractYearMonthDuration(const ATDurationOrDerived::Ptr &yearMonth,  const DynamicContext* context) const = 0;

  /**
   * Returns a date with the given dayTimeDuration subtracted from it
   */
  virtual ATDateTimeOrDerived::Ptr subtractDayTimeDuration(const ATDurationOrDerived::Ptr &dayTime,  const DynamicContext* context) const = 0;

  /**
   * Returns a dayTimeDuration corresponding to the difference between this
   * and the given ATDateTimeOrDerived*
   */
  virtual ATDurationOrDerived::Ptr subtractDateTimeAsDayTimeDuration(const ATDateTimeOrDerived::Ptr &date,  const DynamicContext* context) const = 0;

    /**
   * Returns a dayTimeDuration corresponding to the difference between this
   * and the given ATDateTimeOrDerived*
   */
  virtual ATDurationOrDerived::Ptr subtractDateTimeAsYearMonthDuration(const ATDateTimeOrDerived::Ptr &date, const DynamicContext* context) const = 0;


  virtual AnyAtomicType::AtomicObjectType getPrimitiveTypeIndex() const = 0;
};

#endif //  _ATDATETIMEORDERIVED_HPP

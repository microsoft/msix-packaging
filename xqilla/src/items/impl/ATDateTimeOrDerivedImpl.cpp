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
#include <xqilla/items/impl/ATDateTimeOrDerivedImpl.hpp>
#include <xercesc/util/XMLUniDefs.hpp>
#include <xercesc/validators/schema/SchemaSymbols.hpp>
#include <xercesc/framework/XMLBuffer.hpp>
#include <xqilla/mapm/m_apm.h>
#include <xqilla/framework/XPath2MemoryManager.hpp>
#include <xqilla/items/DatatypeFactory.hpp>
#include <xqilla/exceptions/IllegalArgumentException.hpp>
#include <xqilla/exceptions/XPath2TypeCastException.hpp>
#include <xqilla/items/ATDateOrDerived.hpp>
#include <xqilla/items/ATDecimalOrDerived.hpp>
#include <xqilla/items/ATDurationOrDerived.hpp>
#include <xqilla/items/ATGDayOrDerived.hpp>
#include <xqilla/items/ATGMonthDayOrDerived.hpp>
#include <xqilla/items/ATGMonthOrDerived.hpp>
#include <xqilla/items/ATGYearMonthOrDerived.hpp>
#include <xqilla/items/ATGYearOrDerived.hpp>
#include <xqilla/items/ATTimeOrDerived.hpp>
#include <xqilla/items/AnyAtomicType.hpp>
#include <xqilla/items/Timezone.hpp>
#include <xqilla/items/impl/ATDecimalOrDerivedImpl.hpp>
#include <xqilla/context/ItemFactory.hpp>
#include <xqilla/utils/XPath2Utils.hpp>

#include <limits.h>   // for INT_MIN and INT_MAX
#include <stdlib.h>   // for atoi
#include <assert.h>

#include "../../utils/DateUtils.hpp"

#if defined(XERCES_HAS_CPP_NAMESPACE)
XERCES_CPP_NAMESPACE_USE
#endif

ATDateTimeOrDerivedImpl::
ATDateTimeOrDerivedImpl(const XMLCh* typeURI, const XMLCh* typeName, const XMLCh* value, const DynamicContext* context): 
    _typeName(typeName),
    _typeURI(typeURI)
{ 
  setDateTime(value);
}

// private constructor for internal use
ATDateTimeOrDerivedImpl::ATDateTimeOrDerivedImpl(const XMLCh* typeURI, const XMLCh* typeName, const MAPM &seconds,
                                                 const Timezone::Ptr &timezone, bool hasTimezone)
  : seconds_(seconds),
    timezone_(timezone),
    _hasTimezone(hasTimezone),
    _typeName(typeName),
    _typeURI(typeURI)
{
}

void *ATDateTimeOrDerivedImpl::getInterface(const XMLCh *name) const
{
  if(name == Item::gXQilla) {
    return (void*)this;
  }
  return 0;
}

/* Get the name of the primitive type (basic type) of this type
 * (ie "decimal" for xs:decimal) */
const XMLCh* ATDateTimeOrDerivedImpl::getPrimitiveTypeName() const {
  return this->getPrimitiveName();
}

const XMLCh* ATDateTimeOrDerivedImpl::getPrimitiveName()  {
  return SchemaSymbols::fgDT_DATETIME;
}

/* Get the name of this type  (ie "integer" for xs:integer) */
const XMLCh* ATDateTimeOrDerivedImpl::getTypeName() const {
  return _typeName;
}

/* Get the namespace URI for this type */
const XMLCh* ATDateTimeOrDerivedImpl::getTypeURI() const {
  return _typeURI; 
}

AnyAtomicType::AtomicObjectType ATDateTimeOrDerivedImpl::getTypeIndex() {
  return AnyAtomicType::DATE_TIME;
} 

static inline MAPM hourFromSeconds(const MAPM &value)
{
  return DateUtils::modulo(value, DateUtils::g_secondsPerDay).integer_divide(DateUtils::g_secondsPerHour);
}

static inline MAPM minuteFromSeconds(const MAPM &value)
{
  return DateUtils::modulo(value, DateUtils::g_secondsPerHour).integer_divide(DateUtils::g_secondsPerMinute);
}

static inline MAPM secondFromSeconds(const MAPM &value)
{
  return DateUtils::modulo(value, DateUtils::g_secondsPerMinute);
}

static inline void dateFromSeconds(const MAPM &value, MAPM &year, MAPM &month, MAPM &day)
{
  DateUtils::convertAbsolute2DMY((value/DateUtils::g_secondsPerDay).floor(), day, month, year);
}

static inline void decomposeSeconds(const MAPM &value, MAPM &year, MAPM &month, MAPM &day,
                                    MAPM &hour, MAPM &minute, MAPM &second)
{
  hour = hourFromSeconds(value);
  minute = minuteFromSeconds(value);
  second = secondFromSeconds(value);
  dateFromSeconds(value, year, month, day);
}

static inline MAPM composeSeconds(MAPM &YY, MAPM &MM, MAPM &DD,
                                  MAPM &hh, MAPM &mm, MAPM &ss)
{
  return DateUtils::convertDMY2Absolute(DD, MM, YY) * DateUtils::g_secondsPerDay +
    hh * DateUtils::g_secondsPerHour +
    mm * DateUtils::g_secondsPerMinute +
    ss;
}

static inline MAPM tzLocalize(bool hasTimezone, const MAPM &value, const Timezone::Ptr &timezone)
{
  if(!hasTimezone) return value;
  return value + timezone->asSeconds();
}

static inline MAPM tzNormalize(bool hasTimezone, const MAPM &value, const DynamicContext *context)
{
  if(hasTimezone) return value;
  return value - context->getImplicitTimezone()->asSeconds(context)->asMAPM();
}

/* If possible, cast this type to the target type */
AnyAtomicType::Ptr ATDateTimeOrDerivedImpl::castAsInternal(AtomicObjectType targetIndex, const XMLCh* targetURI, const XMLCh* targetType, const DynamicContext* context) const {
  XMLBuffer buf(1023, context->getMemoryManager());
  
  switch (targetIndex) {
    case DATE: {
      MAPM year, month, day;
      dateFromSeconds(tzLocalize(_hasTimezone, seconds_, timezone_), year, month, day);

      DateUtils::formatNumber(year, 4, buf);
      buf.append(chDash);
      DateUtils::formatNumber(month, 2, buf);
      buf.append(chDash);
      DateUtils::formatNumber(day, 2, buf);
      // Add timezone if exists
      if (_hasTimezone) {
        buf.append(timezone_->asString(context));
      }
      return context->getItemFactory()->createDateOrDerived(targetURI, targetType, buf.getRawBuffer(), context);
    }
    case G_DAY: {
      MAPM year, month, day;
      dateFromSeconds(tzLocalize(_hasTimezone, seconds_, timezone_), year, month, day);

      buf.append(chDash);
      buf.append(chDash);
      buf.append(chDash);
      DateUtils::formatNumber(day, 2, buf);
      if (_hasTimezone) {
        buf.append(timezone_->asString(context));
      }
      return context->getItemFactory()->createGDayOrDerived(targetURI, targetType, buf.getRawBuffer(), context);
    }
    case G_MONTH_DAY: {
      MAPM year, month, day;
      dateFromSeconds(tzLocalize(_hasTimezone, seconds_, timezone_), year, month, day);

      buf.append(chDash);
      buf.append(chDash);
      DateUtils::formatNumber(month, 2, buf);
      buf.append(chDash);
      DateUtils::formatNumber(day, 2, buf);
      if (_hasTimezone) {
        buf.append(timezone_->asString(context));
      }
      return context->getItemFactory()->createGMonthDayOrDerived(targetURI, targetType, buf.getRawBuffer(), context);
    } 
    case G_MONTH: {
      MAPM year, month, day;
      dateFromSeconds(tzLocalize(_hasTimezone, seconds_, timezone_), year, month, day);

      buf.append(chDash);
      buf.append(chDash);
      DateUtils::formatNumber(month, 2, buf);
      if (_hasTimezone) {
        buf.append(timezone_->asString(context));
      }
      return context->getItemFactory()->createGMonthOrDerived(targetURI, targetType, buf.getRawBuffer(), context);
    } 
    case TIME: {
      MAPM value = tzLocalize(_hasTimezone, seconds_, timezone_);

      MAPM hour = hourFromSeconds(value);
      MAPM minute = minuteFromSeconds(value);
      MAPM second = secondFromSeconds(value);

      DateUtils::formatNumber(hour, 2, buf);
      buf.append(chColon);
      DateUtils::formatNumber(minute, 2, buf);
      buf.append(chColon);
      if(second < 10)
        buf.append(chDigit_0);
      buf.append(Numeric::asDecimalString(second, ATDecimalOrDerivedImpl::g_nSignificantDigits, context));
      if (_hasTimezone) {
        buf.append(timezone_->asString(context));
      }
      return context->getItemFactory()->createTimeOrDerived(targetURI, targetType, buf.getRawBuffer(), context);
    } 
    case G_YEAR_MONTH: {
      MAPM year, month, day;
      dateFromSeconds(tzLocalize(_hasTimezone, seconds_, timezone_), year, month, day);

      DateUtils::formatNumber(year, 4, buf);
      buf.append(chDash);
      DateUtils::formatNumber(month, 2, buf);
      if (_hasTimezone) {
        buf.append(timezone_->asString(context));
      }
      return context->getItemFactory()->createGYearMonthOrDerived(targetURI, targetType, buf.getRawBuffer(), context);
    } 
    case G_YEAR: {
      MAPM year, month, day;
      dateFromSeconds(tzLocalize(_hasTimezone, seconds_, timezone_), year, month, day);

      DateUtils::formatNumber(year, 4, buf);
      if (_hasTimezone) {
        buf.append(timezone_->asString(context));
      }
      return context->getItemFactory()->createGYearOrDerived(targetURI, targetType, buf.getRawBuffer(), context);
    } 
    case ANY_SIMPLE_TYPE:
    case UNTYPED_ATOMIC:
      //anySimpleType and untypedAtomic follow the same casting rules as string.
    case STRING: {
      return context->getItemFactory()->createDerivedFromAtomicType(targetIndex, targetURI, targetType, asString(context), context);
    }
		default: {
      return AnyAtomicType::castAsInternal(targetIndex, targetURI, targetType, context);
    }
  }  
}

/* returns the XMLCh* (canonical) representation of this type */
const XMLCh* ATDateTimeOrDerivedImpl::asString(const DynamicContext* context) const {
  XMLBuffer buffer(1023, context->getMemoryManager());

  MAPM year, month, day, hour, minute, second;
  decomposeSeconds(tzLocalize(_hasTimezone, seconds_, timezone_), year, month, day, hour, minute, second);

  DateUtils::formatNumber(year, 4, buffer);
  buffer.append(chDash);
  DateUtils::formatNumber(month, 2, buffer);
  buffer.append(chDash);
  DateUtils::formatNumber(day, 2, buffer);
  buffer.append(chLatin_T);
  DateUtils::formatNumber(hour, 2, buffer);
  buffer.append(chColon);
  DateUtils::formatNumber(minute, 2, buffer);
  buffer.append(chColon);
  if(second < 10)
    buffer.append(chDigit_0);
  buffer.append(Numeric::asDecimalString(second, ATDecimalOrDerivedImpl::g_nSignificantDigits, context));
  // Add timezone if exists 
  if(_hasTimezone) {
    buffer.append(timezone_->asString(context));
  }
  return context->getMemoryManager()->getPooledString(buffer.getRawBuffer());
}

/* returns true if the two objects represent the same date,
 * false otherwise */
bool ATDateTimeOrDerivedImpl::equals(const AnyAtomicType::Ptr &target, const DynamicContext* context) const {
  if(this->getPrimitiveTypeIndex() != target->getPrimitiveTypeIndex()) {
    XQThrow2(::IllegalArgumentException,X("ATDateTimeOrDerivedImpl::equals"),
            X("Equality operator for given types not supported [err:XPTY0004]"));
  }
  return compare((const ATDateTimeOrDerivedImpl *)target.get(), context) == 0;
}

int ATDateTimeOrDerivedImpl::compare(const ATDateTimeOrDerived::Ptr &target, const DynamicContext* context) const
{
  const ATDateTimeOrDerivedImpl *other = (const ATDateTimeOrDerivedImpl *)target.get();

  return tzNormalize(_hasTimezone, seconds_, context).compare(tzNormalize(other->_hasTimezone, other->seconds_, context));
}

/** 
 * Returns an integer representing the year component of this object
 */
ATDecimalOrDerived::Ptr ATDateTimeOrDerivedImpl::getYears(const DynamicContext *context) const {
  MAPM year, month, day;
  dateFromSeconds(tzLocalize(_hasTimezone, seconds_, timezone_), year, month, day);

  return context->getItemFactory()->createInteger(year, context);
}

/** 
 * Returns an integer representing the month component of this object
 */
ATDecimalOrDerived::Ptr ATDateTimeOrDerivedImpl::getMonths(const DynamicContext *context) const {
  MAPM year, month, day;
  dateFromSeconds(tzLocalize(_hasTimezone, seconds_, timezone_), year, month, day);

  return context->getItemFactory()->createNonNegativeInteger(month, context);
}

/** 
 * Returns an integer representing the day component of this object
 */
ATDecimalOrDerived::Ptr ATDateTimeOrDerivedImpl::getDays(const DynamicContext *context) const {
  MAPM year, month, day;
  dateFromSeconds(tzLocalize(_hasTimezone, seconds_, timezone_), year, month, day);

  return context->getItemFactory()->createNonNegativeInteger(day, context);
}

/** 
 * Returns an integer representing the hour component of this object
 */
ATDecimalOrDerived::Ptr ATDateTimeOrDerivedImpl::getHours(const DynamicContext *context) const {
  return context->getItemFactory()->
    createNonNegativeInteger(hourFromSeconds(tzLocalize(_hasTimezone, seconds_, timezone_)), context);
}

/** 
 * Returns an integer representing the minute component of this object
 */
ATDecimalOrDerived::Ptr ATDateTimeOrDerivedImpl::getMinutes(const DynamicContext *context) const {
  return context->getItemFactory()->
    createNonNegativeInteger(minuteFromSeconds(tzLocalize(_hasTimezone, seconds_, timezone_)), context);
}

/** 
 * Returns an decimal representing the second component of this object
 */
ATDecimalOrDerived::Ptr ATDateTimeOrDerivedImpl::getSeconds(const DynamicContext *context) const {
  return context->getItemFactory()->
    createDecimal(secondFromSeconds(tzLocalize(_hasTimezone, seconds_, timezone_)), context);
}

/**
 * Returns the timezone associated with this object, or
 * null, if the timezone is not set
 */
const Timezone::Ptr &ATDateTimeOrDerivedImpl::getTimezone() const {
  return timezone_;
}
  
/**
 * Returns true if the timezone is defined for this object, false otherwise.
 */
bool ATDateTimeOrDerivedImpl::hasTimezone() const {
  return _hasTimezone;
}

/**
 * Setter for timezone.  Overrides the current timezone. (Not to be 
 * confused with addTimezone().  If passed null, timezone is removed (unset)
 */
ATDateTimeOrDerived::Ptr ATDateTimeOrDerivedImpl::setTimezone(const Timezone::Ptr &timezone,
                                                              const DynamicContext* context) const
{
  MAPM result = seconds_;
  if(_hasTimezone) result += timezone_->asSeconds();
  if(timezone != NULLRCP) result -= timezone->asSeconds();

  return new ATDateTimeOrDerivedImpl(_typeURI, _typeName, result, timezone, timezone != NULLRCP);
}

/**
 * Returns an ATDateTimeOrDerived with a timezone added to it
 */
ATDateTimeOrDerived::Ptr ATDateTimeOrDerivedImpl::addTimezone(const ATDurationOrDerived::Ptr &timezone,
	const DynamicContext* context) const
{
  if(!_hasTimezone) return setTimezone(new Timezone(timezone, context), context);
  return new ATDateTimeOrDerivedImpl(_typeURI, _typeName, seconds_, new Timezone(timezone, context), true);
}

/**
 * Returns a date with the given yearMonthDuration added to it
 */
ATDateTimeOrDerived::Ptr ATDateTimeOrDerivedImpl::addYearMonthDuration(const ATDurationOrDerived::Ptr &yearMonth,
                                                                       const DynamicContext* context) const {
  return addYearMonthDuration(yearMonth->asMonths(context)->asMAPM(), context);
}

/**
 * Returns a date with the given yearMonthDuration added to it
 */
ATDateTimeOrDerived::Ptr ATDateTimeOrDerivedImpl::addYearMonthDuration(const MAPM &monthsToAdd,
                                                                       const DynamicContext* context) const {
  return new ATDateTimeOrDerivedImpl(_typeURI, _typeName, addYearMonthDuration(seconds_, monthsToAdd),
                                     timezone_, _hasTimezone);
}

MAPM ATDateTimeOrDerivedImpl::addYearMonthDuration(const MAPM &seconds, const MAPM &monthsToAdd)
{
  MAPM year, month, day, hour, minute, second;
  decomposeSeconds(seconds, year, month, day, hour, minute, second);

  MAPM totalMonths = month + monthsToAdd - MM_One;
  
  MAPM MM = DateUtils::modulo(totalMonths, 12) + MM_One;
  MAPM carry = (totalMonths / 12).floor();
  MAPM finalYears = carry + year;
  
  assert(year.sign() != 0);  // We should never have _YY = 0000

  MAPM YY;
  // Fix year 0000 problem
  if(finalYears.sign() <= 0 && year.sign() >= 0) {
    YY = finalYears - MM_One;
  }
  else if(finalYears.sign() >= 0 && year.sign() < 0) {
    YY = finalYears + MM_One;
  } else {
    YY = finalYears;
  }

  int maxDay = DateUtils::maximumDayInMonthFor(YY, MM);
  if(day > maxDay)
      day = maxDay;

  return composeSeconds(YY, MM, day, hour, minute, second);
}

/**
 * Returns a date with the given dayTimeDuration added to it
 */
ATDateTimeOrDerived::Ptr ATDateTimeOrDerivedImpl::addDayTimeDuration(const ATDurationOrDerived::Ptr &dayTime, const DynamicContext* context) const {
  return addDayTimeDuration(dayTime->asSeconds(context)->asMAPM(), context);
}
  
/**
 * Returns a date with the given yearMonthDuration subtracted from it
 */
ATDateTimeOrDerived::Ptr ATDateTimeOrDerivedImpl::subtractYearMonthDuration(const ATDurationOrDerived::Ptr &yearMonth,
                                                                            const DynamicContext* context) const {
  return addYearMonthDuration(yearMonth->asMonths(context)->asMAPM().neg(), context);
}

/**
 * Returns a date with the given dayTimeDuration subtracted from it
 */
ATDateTimeOrDerived::Ptr ATDateTimeOrDerivedImpl::subtractDayTimeDuration(const ATDurationOrDerived::Ptr &dayTime,
                                                                          const DynamicContext* context) const {
  return addDayTimeDuration(dayTime->asSeconds(context)->asMAPM().neg(), context);
}

ATDateTimeOrDerived::Ptr ATDateTimeOrDerivedImpl::addDayTimeDuration(const MAPM &secondsToAdd,
                                                                     const DynamicContext* context) const {

  return new ATDateTimeOrDerivedImpl(_typeURI, _typeName, seconds_ + secondsToAdd,
                                     timezone_, _hasTimezone);
}

/**
 * Returns a dayTimeDuration corresponding to the difference between this
 * and the given ATDateTimeOrDerived*
 */
ATDurationOrDerived::Ptr ATDateTimeOrDerivedImpl::subtractDateTimeAsDayTimeDuration(const ATDateTimeOrDerived::Ptr &date,
                                                                                    const DynamicContext* context) const {
  const ATDateTimeOrDerivedImpl *other = (const ATDateTimeOrDerivedImpl *)date.get();

  MAPM secDiff = tzNormalize(_hasTimezone, seconds_, context) -
    tzNormalize(other->_hasTimezone, other->seconds_, context);

  return context->getItemFactory()->createDayTimeDuration(secDiff, context);
}

/**
 * Returns a dayTimeDuration corresponding to the difference between this
 * and the given ATDateTimeOrDerived*
 */
ATDurationOrDerived::Ptr ATDateTimeOrDerivedImpl::subtractDateTimeAsYearMonthDuration(const ATDateTimeOrDerived::Ptr &date,
                                                                                      const DynamicContext* context) const {

  const ATDateTimeOrDerivedImpl *other = (const ATDateTimeOrDerivedImpl *)date.get();

  // Call subtractDateTimeAsDayTimeDuration
  const ATDurationOrDerived::Ptr dayTimeDiff = subtractDateTimeAsDayTimeDuration(date, context);

  // put it into yearMonthDuration form
  MAPM days = dayTimeDiff->getDays(context)->asMAPM();
  
  MAPM months;
  // Get number of months
  MAPM YY, MM, DD;
  dateFromSeconds(tzNormalize(other->_hasTimezone, other->seconds_, context), YY, MM, DD);

  int curYear = DateUtils::asInt(YY);
  int curMonth = DateUtils::asInt(MM);
  int curDays = DateUtils::asInt(DD) - 1;
  while (true) {
    int currentDaysInMonth = DateUtils::maximumDayInMonthFor(curYear, curMonth);
    int daysToNextMonth = currentDaysInMonth - curDays;
    if (days > daysToNextMonth) {
      days = days - daysToNextMonth;
      ++months;

      ++curMonth;
      if(curMonth > 12) {
        ++curYear;
        curMonth = 1;
        curDays = 0;
      }
    }
    else
      break;
  }

  return context->getItemFactory()->createYearMonthDuration(months, context);
}

AnyAtomicType::AtomicObjectType ATDateTimeOrDerivedImpl::getPrimitiveTypeIndex() const {
  return this->getTypeIndex();
}

void ATDateTimeOrDerivedImpl::setDateTime(const XMLCh* const dateTime) {
  unsigned int length = XPath2Utils::uintStrlen(dateTime);

  if(dateTime == 0) {
    XQThrow2(XPath2TypeCastException,X("ATDateTimeOrDerivedImpl::setDateTime"), X("Invalid representation of dateTime [err:FORG0001]"));
  }
  
  // State variables etc.
  bool gotDot = false;
  bool gotDigit = false;

  unsigned int pos = 0;
  long int tmpnum = 0;
  MAPM decplace = 1;
  MAPM tmpdec = 0;
  bool negative = false;

  unsigned numDigit = 0;

  // defaulting values
  MAPM YY = 0;
  MAPM MM = 0;
  MAPM DD = 0;
  MAPM hh = 0;
  MAPM mm = 0;
  MAPM ss = 0;
  _hasTimezone = false;

  bool zonepos = true;
  int zonehh = 0;
  int zonemm = 0;

  int state = 0 ; // 0 = year / 1 = month / 2 = day / 3 = hour 
                   // 4 = minutes / 5 = sec / 6 = timezonehour / 7 = timezonemin
  XMLCh tmpChar;
  
  bool wrongformat = false;

  if ( length > 0 && dateTime[0] == L'-'  ) {
                negative = true;
    pos = 1;
  }else{
    pos = 0;
  } 
    
  while ( ! wrongformat && pos < length) {
    tmpChar = dateTime[pos];
    pos++;
    switch(tmpChar) {
      case L'.': {
        if (! gotDot && gotDigit && state == 5 && numDigit == 2) {
          gotDot = true;
          ss = tmpnum;
          gotDigit = false;
          tmpnum = 0;
          break;
        }
        wrongformat = true;                    
        break;
    }
      case 0x0030:
      case 0x0031:
      case 0x0032:
      case 0x0033:
      case 0x0034:
      case 0x0035:
      case 0x0036:
      case 0x0037:
      case 0x0038:
      case 0x0039: {
        if ( gotDot && state == 5) {
          decplace *= 10;                
        } else  {           
          numDigit ++;
        }                        
        tmpnum *= 10;
        tmpnum +=  static_cast<int>(tmpChar - 0x0030);
        gotDigit = true;
        break;
      }
    case L'T' : {
      if ( state == 2 && gotDigit && !gotDot && numDigit == 2) {
        state ++;
        DD = tmpnum;
        tmpnum = 0;
        gotDigit = false;
        numDigit = 0;
      } else {
        wrongformat = true;
      }
      break;
    }
    case L'-' : {
      if ( gotDigit ) {
        if (state == 0 && numDigit >= 4 ) { 
          YY = tmpnum;
          if (negative) {
            YY = YY * -1;
          }
          tmpnum = 0;
          gotDigit = false;
          numDigit = 0;
        } else if (state == 1 && numDigit == 2) {
          MM = tmpnum;    
          tmpnum = 0;
          gotDigit = false;
          numDigit = 0;
        } else if ( state == 5 && numDigit == 2) {          
          tmpdec =  tmpnum / decplace;      
          ss += tmpdec;          
          gotDigit = false;  
          _hasTimezone = true;
          zonepos = false;
          tmpnum = 0;
          numDigit = 0;
        } else {
          wrongformat = true;
        }
        state ++;
      }
      break;      
    }
    case L'+' : {
      if ( gotDigit && state == 5 && numDigit == 2) {
        tmpdec =  tmpnum / decplace;      
        ss += tmpdec;  
        state = 6; 
        gotDigit = false;      
        _hasTimezone = true;
        zonepos = true;
        tmpnum = 0;
        numDigit = 0;
      } else {
        wrongformat = true;
      }
      break;
    }
    case L':' : {
      if (gotDigit ) {
        if (state == 3 && numDigit == 2) {
          hh = tmpnum;  
          tmpnum = 0;
          gotDigit = false;
          numDigit = 0;
        } else if ( state == 4 && numDigit == 2) {
          mm = tmpnum;
          tmpnum = 0;
          gotDigit = false;
          numDigit = 0;
        } else if ( state == 6 && numDigit == 2) {
          zonehh = tmpnum;
          tmpnum = 0;
          gotDigit = false;
          numDigit = 0;
        }
        state ++;
      }else {
        wrongformat = true;
      }
      break;
    }
    case L'Z' : {
      if (gotDigit && state == 5 && numDigit == 2) {
        tmpdec =  tmpnum / decplace;      
        ss += tmpdec;  
        state = 8; // final state
        _hasTimezone = true;
        gotDigit = false;
        tmpnum = 0;
        numDigit = 0;
      } else {        
        wrongformat = true;
      }
      break;
    }
    default:
      wrongformat = true;
    }  
  }

  if (gotDigit) {
    if ( gotDigit && state == 7 && numDigit == 2) {
      zonemm = tmpnum;
    }else if ( gotDigit && state == 5 && numDigit == 2) {
      tmpdec =  tmpnum / decplace;      
      ss += tmpdec;      
    }else {
      wrongformat = true;
    }
  } 
  
  // check time format
  if(MM > 12 || YY == 0 || DD > DateUtils::maximumDayInMonthFor(YY, MM) || hh > 24 || mm > 59 || ss >= 60 || zonehh > 24 || zonemm > 59 ) 
    {
    wrongformat = true;
    }

  if (wrongformat) 
  {
    XQThrow2(XPath2TypeCastException,X("XSDateTimeImpl::setDateTime"), X("Invalid representation of dateTime [err:FORG0001]"));
  }

  timezone_ = new Timezone(Timezone::convert(zonepos, zonehh, zonemm));

  seconds_ = composeSeconds(YY, MM, DD, hh, mm, ss);

  if(_hasTimezone) {
    // If we have a timezone, then seconds_ needs to be normalized
    seconds_ -= timezone_->asSeconds();
  }
}

MAPM ATDateTimeOrDerivedImpl::parseDateTime(const XMLCh* const dateTime, const MAPM &implicitTimezone)
{
  ATDateTimeOrDerivedImpl dt(0, 0, dateTime);
  if(!dt._hasTimezone)
    return dt.seconds_ - implicitTimezone;
  return dt.seconds_;
}

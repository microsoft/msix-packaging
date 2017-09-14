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
#include <xqilla/items/impl/ATDateOrDerivedImpl.hpp>
#include <xercesc/util/XMLUniDefs.hpp>
#include <xercesc/validators/schema/SchemaSymbols.hpp>
#include <xercesc/framework/XMLBuffer.hpp>
#include <xqilla/mapm/m_apm.h>
#include <xqilla/framework/XPath2MemoryManager.hpp>
#include <xqilla/items/DatatypeFactory.hpp>
#include <xqilla/exceptions/IllegalArgumentException.hpp>
#include <xqilla/exceptions/XPath2TypeCastException.hpp>
#include <xqilla/items/impl/ATDateTimeOrDerivedImpl.hpp>
#include <xqilla/items/ATDecimalOrDerived.hpp>
#include <xqilla/items/ATDurationOrDerived.hpp>
#include <xqilla/items/ATGDayOrDerived.hpp>
#include <xqilla/items/ATGMonthDayOrDerived.hpp>
#include <xqilla/items/ATGMonthOrDerived.hpp>
#include <xqilla/items/ATGYearMonthOrDerived.hpp>
#include <xqilla/items/ATGYearOrDerived.hpp>
#include <xqilla/items/Timezone.hpp>
#include <xqilla/context/ItemFactory.hpp>
#include <xqilla/utils/XPath2Utils.hpp>

#include <limits.h>   // for INT_MIN and INT_MAX
#include <stdlib.h>   // for atoi
#include <assert.h>

#include "../../utils/DateUtils.hpp"

#if defined(XERCES_HAS_CPP_NAMESPACE)
XERCES_CPP_NAMESPACE_USE
#endif

ATDateOrDerivedImpl::
ATDateOrDerivedImpl(const XMLCh* typeURI, const XMLCh* typeName, const XMLCh* value, const DynamicContext* context)
  : _typeName(typeName),
    _typeURI(typeURI)
{    
  setDate(value);
}

// private constructor for internal use()
ATDateOrDerivedImpl::ATDateOrDerivedImpl(const XMLCh* typeURI, const XMLCh* typeName, const MAPM &seconds,
                                         const Timezone::Ptr &timezone, bool hasTimezone)
  : seconds_(seconds),
    timezone_(timezone),
    _hasTimezone(hasTimezone),
    _typeName(typeName),
    _typeURI(typeURI)
{
}

void *ATDateOrDerivedImpl::getInterface(const XMLCh *name) const
{
  if(name == Item::gXQilla) {
    return (void*)this;
  }
  return 0;
}

/* Get the name of the primitive type (basic type) of this type
 * (ie "decimal" for xs:decimal) */
const XMLCh* ATDateOrDerivedImpl::getPrimitiveTypeName() const {
  return this->getPrimitiveName();
}

const XMLCh* ATDateOrDerivedImpl::getPrimitiveName()  {
  return SchemaSymbols::fgDT_DATE;
}

/* Get the name of this type  (ie "integer" for xs:integer) */
const XMLCh* ATDateOrDerivedImpl::getTypeName() const {
  return _typeName;
}

/* Get the namespace URI for this type */
const XMLCh* ATDateOrDerivedImpl::getTypeURI() const {
  return _typeURI; 
}

AnyAtomicType::AtomicObjectType ATDateOrDerivedImpl::getTypeIndex() {
  return AnyAtomicType::DATE;
} 

static inline void decomposeSeconds(const MAPM &value, MAPM &year, MAPM &month, MAPM &day)
{
  DateUtils::convertAbsolute2DMY((value/DateUtils::g_secondsPerDay).floor(), day, month, year);
}

static inline MAPM composeSeconds(MAPM &YY, MAPM &MM, MAPM &DD)
{
  return DateUtils::convertDMY2Absolute(DD, MM, YY) * DateUtils::g_secondsPerDay;
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
AnyAtomicType::Ptr ATDateOrDerivedImpl::castAsInternal(AtomicObjectType targetIndex, const XMLCh* targetURI,
                                                       const XMLCh* targetType, const DynamicContext* context) const
{
  XMLBuffer buf(1023, context->getMemoryManager());
  
  const XMLCh doubleZero[] = { chDigit_0, chDigit_0, chNull };

  switch (targetIndex) {
    case DATE_TIME: {
      MAPM year, month, day;
      decomposeSeconds(tzLocalize(_hasTimezone, seconds_, timezone_), year, month, day);

      DateUtils::formatNumber(year, 4, buf);
      buf.append(chDash);
      DateUtils::formatNumber(month, 2, buf);
      buf.append(chDash);
      DateUtils::formatNumber(day, 2, buf);
      buf.append(chLatin_T);
      buf.append(doubleZero);
      buf.append(chColon);
      buf.append(doubleZero);
      buf.append(chColon);
      buf.append(doubleZero);
      
      // Add timezone if exists
      if (_hasTimezone) {
        buf.append(timezone_->asString(context));
      }
      return context->getItemFactory()->createDateTimeOrDerived(targetURI, targetType, buf.getRawBuffer(), context);
    }
    case G_DAY: {
      MAPM year, month, day;
      decomposeSeconds(tzLocalize(_hasTimezone, seconds_, timezone_), year, month, day);

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
      decomposeSeconds(tzLocalize(_hasTimezone, seconds_, timezone_), year, month, day);

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
      decomposeSeconds(tzLocalize(_hasTimezone, seconds_, timezone_), year, month, day);

      buf.append(chDash);
      buf.append(chDash);
      DateUtils::formatNumber(month, 2, buf);
      if (_hasTimezone) {
        buf.append(timezone_->asString(context));
      }
      return context->getItemFactory()->createGMonthOrDerived(targetURI, targetType, buf.getRawBuffer(), context);
    } 
    case G_YEAR_MONTH: {
      MAPM year, month, day;
      decomposeSeconds(tzLocalize(_hasTimezone, seconds_, timezone_), year, month, day);

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
      decomposeSeconds(tzLocalize(_hasTimezone, seconds_, timezone_), year, month, day);

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
const XMLCh* ATDateOrDerivedImpl::asString(const DynamicContext* context) const
{
  XMLBuffer buffer(1023, context->getMemoryManager());

  MAPM year, month, day;
  decomposeSeconds(tzLocalize(_hasTimezone, seconds_, timezone_), year, month, day);

  DateUtils::formatNumber(year, 4, buffer);
  buffer.append(chDash);
  DateUtils::formatNumber(month, 2, buffer);
  buffer.append(chDash);
  DateUtils::formatNumber(day, 2, buffer);
  // Add timezone if exists
  if (_hasTimezone) {
    buffer.append(timezone_->asString(context));
  }
  return context->getMemoryManager()->getPooledString(buffer.getRawBuffer());
}

/* returns true if the two objects represent the same date,
 * false otherwise */
bool ATDateOrDerivedImpl::equals(const AnyAtomicType::Ptr &target, const DynamicContext* context) const {
  if(getPrimitiveTypeIndex() != target->getPrimitiveTypeIndex()) {
    XQThrow2(::IllegalArgumentException,X("ATDateOrDerivedImpl::equals"),
            X("Equality operator for given types not supported [err:XPTY0004]"));
  }
  return compare((const ATDateOrDerivedImpl *)target.get(), context) == 0;
}

int ATDateOrDerivedImpl::compare(const ATDateOrDerived::Ptr &target, const DynamicContext *context) const
{
  const ATDateOrDerivedImpl *other = (const ATDateOrDerivedImpl *)target.get();
  return tzNormalize(_hasTimezone, seconds_, context).compare(tzNormalize(other->_hasTimezone, other->seconds_, context));
}

/** 
 * Returns an integer representing the year component  of this object
 */
ATDecimalOrDerived::Ptr ATDateOrDerivedImpl::getYears(const DynamicContext *context) const {
  MAPM year, month, day;
  decomposeSeconds(tzLocalize(_hasTimezone, seconds_, timezone_), year, month, day);

  return context->getItemFactory()->createInteger(year, context);
}

/** 
 * Returns an integer representing the month component  of this object
 */
ATDecimalOrDerived::Ptr ATDateOrDerivedImpl::getMonths(const DynamicContext *context) const {
  MAPM year, month, day;
  decomposeSeconds(tzLocalize(_hasTimezone, seconds_, timezone_), year, month, day);

  return context->getItemFactory()->createNonNegativeInteger(month, context);
}

/** 
 * Returns an integer representing the day component  of this object
 */
ATDecimalOrDerived::Ptr ATDateOrDerivedImpl::getDays(const DynamicContext *context) const {
  MAPM year, month, day;
  decomposeSeconds(tzLocalize(_hasTimezone, seconds_, timezone_), year, month, day);

  return context->getItemFactory()->createNonNegativeInteger(day, context);
}

/**
 * Returns the timezone associated with this object, or
 * null, if the timezone is not set
 */
const Timezone::Ptr &ATDateOrDerivedImpl::getTimezone() const {
  return timezone_;
}
  

/**
 * Returns true if the timezone is defined for this object, false otherwise.
 */
bool ATDateOrDerivedImpl::hasTimezone() const {
  return _hasTimezone;
}

/**mrg@decisionsoft.com
 * Setter for timezone.  Overrides the current timezone. (Not to be 
 * confused with addTimezone(). If passed NULL, timezone is removed (unset)
 */
ATDateOrDerived::Ptr ATDateOrDerivedImpl::setTimezone(const Timezone::Ptr &timezone, const DynamicContext* context) const
{
  MAPM result = seconds_;
  if(_hasTimezone) result += timezone_->asSeconds();
  if(timezone != NULLRCP) result -= timezone->asSeconds();

  return new ATDateOrDerivedImpl(_typeURI, _typeName, result, timezone, timezone != NULLRCP);
}

/**
 * Returns an ATDateOrDerived with a timezone added to it
 */
ATDateOrDerived::Ptr ATDateOrDerivedImpl::addTimezone(const ATDurationOrDerived::Ptr &timezone,
                                                      const DynamicContext* context) const
{
  Timezone::Ptr tz = new Timezone(timezone, context);

  if(!_hasTimezone) return setTimezone(tz, context);

  // Keep the time components as 00:00:00
  MAPM result = seconds_ + tz->asSeconds();
  result = (result / DateUtils::g_secondsPerDay).floor() * DateUtils::g_secondsPerDay;
  result -= tz->asSeconds();

  return new ATDateOrDerivedImpl(_typeURI, _typeName, result, tz, true);
}

/**
 * Returns a date with the given yearMonthDuration added to it
 */
ATDateOrDerived::Ptr ATDateOrDerivedImpl::addYearMonthDuration(const ATDurationOrDerived::Ptr &yearMonth,
                                                               const DynamicContext* context) const
{
  MAPM result = ATDateTimeOrDerivedImpl::addYearMonthDuration(seconds_, yearMonth->asMonths(context)->asMAPM());
  return new ATDateOrDerivedImpl(_typeURI, _typeName, result, timezone_, _hasTimezone);
}

/**
 * Returns a date with the given dayTimeDuration added to it
 */
ATDateOrDerived::Ptr ATDateOrDerivedImpl::addDayTimeDuration(const ATDurationOrDerived::Ptr &dayTime,
                                                             const DynamicContext* context) const
{
  MAPM result = seconds_ + dayTime->asSeconds(context)->asMAPM();
  result = (result / DateUtils::g_secondsPerDay).floor() * DateUtils::g_secondsPerDay;
  return new ATDateOrDerivedImpl(_typeURI, _typeName, result, timezone_, _hasTimezone);
}
  
/**
 * Returns a date with the given yearMonthDuration subtracted from it
 */
ATDateOrDerived::Ptr ATDateOrDerivedImpl::subtractYearMonthDuration(const ATDurationOrDerived::Ptr &yearMonth,
                                                                    const DynamicContext* context) const
{
  MAPM result = ATDateTimeOrDerivedImpl::addYearMonthDuration(seconds_, yearMonth->asMonths(context)->asMAPM().neg());
  return new ATDateOrDerivedImpl(_typeURI, _typeName, result, timezone_, _hasTimezone);
}

/**
 * Returns a date with the given dayTimeDuration subtracted from it
 */
ATDateOrDerived::Ptr ATDateOrDerivedImpl::subtractDayTimeDuration(const ATDurationOrDerived::Ptr &dayTime,
                                                                  const DynamicContext* context) const
{
  MAPM result = seconds_ - dayTime->asSeconds(context)->asMAPM();
  result = (result / DateUtils::g_secondsPerDay).floor() * DateUtils::g_secondsPerDay;
  return new ATDateOrDerivedImpl(_typeURI, _typeName, result, timezone_, _hasTimezone);
}

/**
 * Returns a dayTimeDuration corresponding to the difference between this
 * and the given ATDateOrDerived*
 */
ATDurationOrDerived::Ptr ATDateOrDerivedImpl::subtractDate(const ATDateOrDerived::Ptr &date,
                                                           const DynamicContext* context) const
{
  const ATDateOrDerivedImpl *other = (const ATDateOrDerivedImpl *)date.get();

  MAPM secDiff = tzNormalize(_hasTimezone, seconds_, context) -
    tzNormalize(other->_hasTimezone, other->seconds_, context);

  return context->getItemFactory()->createDayTimeDuration(secDiff, context);
}


AnyAtomicType::AtomicObjectType ATDateOrDerivedImpl::getPrimitiveTypeIndex() const {
  return this->getTypeIndex();
}

void ATDateOrDerivedImpl::setDate(const XMLCh* const date) {
  unsigned int length = XPath2Utils::uintStrlen(date);
 
  if(date == 0) {
      XQThrow2(XPath2TypeCastException,X("ATDateOrDerived::setDate"), X("Invalid representation of date [err:FORG0001]"));
  }
  
  // State variables etc.
  bool gotDigit = false;

  unsigned int pos = 0;
  long int tmpnum = 0;
  unsigned int numDigit = 0;
  bool negative = false;

  // defaulting values
  MAPM YY = 1;
  MAPM MM = 0;
  MAPM DD = 0;
  _hasTimezone = false;
  bool zonepos = false;
  int zonehh = 0;
  int zonemm = 0;

  int state = 0 ; // 0 = year / 1 = month / 2 = day 
                  // 6 = timezonehour / 7 = timezonemin
  XMLCh tmpChar;
  
  bool wrongformat = false;

  if ( length > 0 && date[0] == L'-'  ) {
    negative = true;
    pos = 1;
  }else{
    pos = 0;
  } 
    
  while ( ! wrongformat && pos < length) {
    tmpChar = date[pos];
    pos++;
    switch(tmpChar) {
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
        numDigit ++;
        tmpnum *= 10;
        tmpnum +=  static_cast<int>(tmpChar - 0x0030);
        gotDigit = true;
        break;
      }
    case L'-' : {
      if ( gotDigit ) {
        if (state == 0 && numDigit >= 4) { 
          YY = tmpnum;
          if (negative) {
            YY = YY * -1;
          }
          tmpnum = 0;
          gotDigit = false;
          numDigit = 0 ;
        } else if (state == 1 && numDigit == 2) {
          MM = tmpnum;    
          tmpnum = 0;
          gotDigit = false;
          numDigit = 0 ;
        } else if ( state == 2 && numDigit == 2) {          
          DD += tmpnum;          
          gotDigit = false;  
          zonepos = false;
          _hasTimezone = true;
          tmpnum = 0;
          state = 5;
          numDigit = 0 ;
        } else {
          wrongformat = true;
        }
        state ++;
      }
      break;      
    }
    case L':' : {
      if (gotDigit && state == 6 && numDigit == 2) {
        zonehh = tmpnum;
        tmpnum = 0;
        gotDigit = false;
        state ++;
        numDigit = 0 ;
      }else {
        wrongformat = true;
      }
      break;
    }
    case L'+' : {
      if ( gotDigit && state == 2 && numDigit == 2) {
        DD += tmpnum;  
        state = 6; 
        gotDigit = false;      
        zonepos = true;
        _hasTimezone = true;
        tmpnum = 0;
        numDigit = 0 ;
      } else {
        wrongformat = true;
      }
      break;
    }
    case L'Z' : {
      if (gotDigit && state == 2 && numDigit == 2) {
        DD += tmpnum;  
        state = 8; // final state
        _hasTimezone = true;
        gotDigit = false;
        tmpnum = 0;
        numDigit = 0 ;
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
      _hasTimezone = true;
    }else if ( gotDigit && state == 2 && numDigit == 2) {
      DD += tmpnum;      
    }else {
      wrongformat = true;
    }
  } 

  // Verify date
  if ( MM > 12 || YY == 0 || zonehh > 24 || zonemm > 59 ) 
    wrongformat = true;
  else if(DD > DateUtils::maximumDayInMonthFor(YY, MM))
    wrongformat = true;

  if ( wrongformat) 
  {
    XQThrow2(XPath2TypeCastException,X("ATDateOrDerivedImpl::setDate"), X("Invalid representation of date [err:FORG0001]"));
  }

  timezone_ = new Timezone(Timezone::convert(zonepos, zonehh, zonemm));

  seconds_ = composeSeconds(YY, MM, DD);

  if(_hasTimezone) {
    // If we have a timezone, then seconds_ needs to be normalized
    seconds_ -= timezone_->asSeconds();
  }
}

MAPM ATDateOrDerivedImpl::parseDate(const XMLCh* const date, const MAPM &implicitTimezone)
{
  ATDateOrDerivedImpl dt(0, 0, date);
  if(!dt._hasTimezone)
    return dt.seconds_ - implicitTimezone;
  return dt.seconds_;
}


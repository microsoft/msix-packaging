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
#include <xqilla/items/impl/ATTimeOrDerivedImpl.hpp>
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

ATTimeOrDerivedImpl::
ATTimeOrDerivedImpl(const XMLCh* typeURI, const XMLCh* typeName, const XMLCh* value, const DynamicContext* context)
  : ATTimeOrDerived(),
    _typeName(typeName),
    _typeURI(typeURI)
{
  setTime(value);
}


// private constructor for internal use
ATTimeOrDerivedImpl::ATTimeOrDerivedImpl(const XMLCh* typeURI, const XMLCh* typeName, const MAPM &seconds,
                                         const Timezone::Ptr &timezone, bool hasTimezone)
  : seconds_(seconds),
    timezone_(timezone),
    _hasTimezone(hasTimezone),
    _typeName(typeName),
    _typeURI(typeURI)
{
}

void *ATTimeOrDerivedImpl::getInterface(const XMLCh *name) const
{
  if(name == Item::gXQilla) {
    return (void*)this;
  }
  return 0;
}

/* Get the name of the primitive type (basic type) of this type
 * (ie "decimal" for xs:decimal) */
const XMLCh* ATTimeOrDerivedImpl::getPrimitiveTypeName() const {
  return this->getPrimitiveName();
}

const XMLCh* ATTimeOrDerivedImpl::getPrimitiveName()  {
  return SchemaSymbols::fgDT_TIME;
}

/* Get the name of this type  (ie "integer" for xs:integer) */
const XMLCh* ATTimeOrDerivedImpl::getTypeName() const {
  return _typeName;
}

/* Get the namespace URI for this type */
const XMLCh* ATTimeOrDerivedImpl::getTypeURI() const {
  return _typeURI; 
}

AnyAtomicType::AtomicObjectType ATTimeOrDerivedImpl::getTypeIndex() {
  return AnyAtomicType::TIME;
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

static inline void decomposeSeconds(const MAPM &value, MAPM &hour, MAPM &minute, MAPM &second)
{
  hour = hourFromSeconds(value);
  minute = minuteFromSeconds(value);
  second = secondFromSeconds(value);
}

static inline MAPM composeSeconds(MAPM &hh, MAPM &mm, MAPM &ss)
{
  return hh * DateUtils::g_secondsPerHour +
    mm * DateUtils::g_secondsPerMinute +
    ss;
}

/* If possible, cast this type to the target type */
AnyAtomicType::Ptr ATTimeOrDerivedImpl::castAsInternal(AtomicObjectType targetIndex, const XMLCh* targetURI,
                                                       const XMLCh* targetType, const DynamicContext* context) const
{
  XMLBuffer buf(1023, context->getMemoryManager());

  switch (targetIndex) {
    case DATE_TIME: {
      ATDateOrDerived::Ptr currentDate = DateUtils::getCurrentDate(context);
      Timezone::Ptr nulltz;
      currentDate = currentDate->setTimezone(nulltz, context);

      buf.append(currentDate->asString(context));
      buf.append(chLatin_T);
      buf.append(this->asString(context));

      return context->getItemFactory()->createDateTimeOrDerived(targetURI, targetType, buf.getRawBuffer(), context);
    }
    case ANY_SIMPLE_TYPE:
    case UNTYPED_ATOMIC:
      //anySimpleType and untypedAtomic follow the same casting rules as string.
    case STRING: {
      return context->getItemFactory()->createDerivedFromAtomicType(targetIndex, targetURI, targetType, this->asString(context), context);
		}
    default: return AnyAtomicType::castAsInternal(targetIndex, targetURI, targetType, context);
  }
}

/* returns the XMLCh* (canonical) representation of this type */
const XMLCh* ATTimeOrDerivedImpl::asString(const DynamicContext* context) const
{
  XMLBuffer buffer(1023, context->getMemoryManager());

  MAPM hour, minute, second;
  decomposeSeconds(seconds_, hour, minute, second);

  DateUtils::formatNumber(hour, 2, buffer);
  buffer.append(chColon);
  DateUtils::formatNumber(minute, 2, buffer);
  buffer.append(chColon);
  if(second < 10)
    buffer.append(chDigit_0);
  buffer.append(Numeric::asDecimalString(second, ATDecimalOrDerivedImpl::g_nSignificantDigits, context));
  // Add timezone if exists
  if (_hasTimezone) {
    buffer.append(timezone_->asString(context));
  }
  return context->getMemoryManager()->getPooledString(buffer.getRawBuffer());
}

static inline MAPM referenceDateTime(const MAPM &seconds, bool hasTimezone, const Timezone::Ptr &timezone)
{
  MAPM result = DateUtils::convertDMY2Absolute(31, 12, 1972) * DateUtils::g_secondsPerDay;
  result += seconds;

  if(hasTimezone) {
    result -= timezone->asSeconds();
  }

  return result;
}

MAPM ATTimeOrDerivedImpl::buildReferenceDateTime(const DynamicContext *context) const
{
  MAPM result = referenceDateTime(seconds_, _hasTimezone, timezone_);

  if(!_hasTimezone)
    result -= context->getImplicitTimezone()->asSeconds(context)->asMAPM();

  return result;
}

/**
 * Returns true if and only if the value of $arg1 converted to an xs:dateTime using the date components from the reference 
 * xs:dateTime is equal to the value of $arg2 converted to an xs:dateTime using the date components from the same reference 
 * xs:dateTime. Returns false otherwise.
 * 
 * The two xs:dateTime values are compared using op:dateTime-equal.
 */
bool ATTimeOrDerivedImpl::equals(const AnyAtomicType::Ptr &target, const DynamicContext* context) const {
  if(this->getPrimitiveTypeIndex() != target->getPrimitiveTypeIndex()) {
    XQThrow2(::IllegalArgumentException,X("ATTimeOrDerivedImpl::equals"), X("Equality operator for given types not supported [err:XPTY0004]"));
  }
  return compare((const ATTimeOrDerived*)target.get(), context) == 0;
}

int ATTimeOrDerivedImpl::compare(const ATTimeOrDerived::Ptr &target, const DynamicContext* context) const
{
  const ATTimeOrDerivedImpl *other = (const ATTimeOrDerivedImpl *)target.get();
  return buildReferenceDateTime(context).compare(other->buildReferenceDateTime(context));
}

/** 
 * Returns an integer representing the hour component of this object
 */
ATDecimalOrDerived::Ptr ATTimeOrDerivedImpl::getHours(const DynamicContext *context) const {
  return context->getItemFactory()->createNonNegativeInteger(hourFromSeconds(seconds_), context);
}

/** 
 * Returns an integer representing the minute component of this object
 */
ATDecimalOrDerived::Ptr ATTimeOrDerivedImpl::getMinutes(const DynamicContext *context) const {
  return context->getItemFactory()->createNonNegativeInteger(minuteFromSeconds(seconds_), context);
}

/** 
 * Returns an decimal representing the second component of this object
 */
ATDecimalOrDerived::Ptr ATTimeOrDerivedImpl::getSeconds(const DynamicContext *context) const {
  return context->getItemFactory()->createDecimal(secondFromSeconds(seconds_), context);
}

/**
 * Returns true if the timezone is defined for this object, false otherwise.
 */
bool ATTimeOrDerivedImpl::hasTimezone() const {
  return _hasTimezone;
}

/**
 * Returns the timezone associated with this object, or
 * null, if the timezone is not set
 */
const Timezone::Ptr &ATTimeOrDerivedImpl::getTimezone() const {
  return timezone_;
}

/**
 * Setter for timezone.  Overrides the current timezone. (Not to be 
 * confused with addTimezone(). If passed NULL, timezone is removed (unset)
 */
ATTimeOrDerived::Ptr ATTimeOrDerivedImpl::setTimezone(const Timezone::Ptr &timezone, const DynamicContext* context) const
{
  return new ATTimeOrDerivedImpl(_typeURI, _typeName, seconds_, timezone, timezone != NULLRCP);
}

/**
 * Returns an ATTimeOrDerived with a timezone added to it
 */
ATTimeOrDerived::Ptr ATTimeOrDerivedImpl::addTimezone(const ATDurationOrDerived::Ptr &timezone,
                                                      const DynamicContext* context) const
{
  Timezone::Ptr tz = new Timezone(timezone, context);  

  // If this time does not have a timezone, add the given timezone
  if(!_hasTimezone) return setTimezone(tz, context);

  MAPM result = seconds_;
  result -= timezone_->asSeconds();
  result += tz->asSeconds();

  return new ATTimeOrDerivedImpl(_typeURI, _typeName, result, tz, true);
}

/**
 * Returns a time with the given dayTimeDuration added to it
 */
ATTimeOrDerived::Ptr ATTimeOrDerivedImpl::addDayTimeDuration(const ATDurationOrDerived::Ptr &dayTime, const DynamicContext* context) const {
  return addDayTimeDuration(dayTime->asSeconds(context)->asMAPM(), context);
}
  
/**
 * Returns a time with the given dayTimeDuration subtracted from it
 */
ATTimeOrDerived::Ptr ATTimeOrDerivedImpl::subtractDayTimeDuration(const ATDurationOrDerived::Ptr &dayTime, const DynamicContext* context) const {
  return addDayTimeDuration(dayTime->asSeconds(context)->asMAPM().neg(), context);
}

ATTimeOrDerived::Ptr ATTimeOrDerivedImpl::addDayTimeDuration(const MAPM &secondsToAdd, const DynamicContext* context) const
{
  MAPM result = DateUtils::modulo(seconds_ + secondsToAdd, DateUtils::g_secondsPerDay);
  return new ATTimeOrDerivedImpl(_typeURI, _typeName, result, timezone_, _hasTimezone);
}

/**
 * Returns the xdt:dayTimeDuration that corresponds to the difference between the value of $arg1 converted to an 
 * xs:dateTime using the date components from the reference xs:dateTime and the value of $arg2 converted to an xs:dateTime 
 * using the date components from the same reference xs:dateTime
 */
ATDurationOrDerived::Ptr ATTimeOrDerivedImpl::subtractTime(const ATTimeOrDerived::Ptr &time,
                                                           const DynamicContext* context) const
{  
  const ATTimeOrDerivedImpl *other = (const ATTimeOrDerivedImpl *)time.get();

  MAPM secDiff = buildReferenceDateTime(context) - other->buildReferenceDateTime(context);

  return context->getItemFactory()->createDayTimeDuration(secDiff, context);
}

AnyAtomicType::AtomicObjectType ATTimeOrDerivedImpl::getPrimitiveTypeIndex() const {
  return this->getTypeIndex();
}

void ATTimeOrDerivedImpl::setTime(const XMLCh* const time) {
	unsigned int length = XPath2Utils::uintStrlen(time);
 
	if(time == NULL) {
	  XQThrow2(XPath2TypeCastException,X("XSTimeImpl::setTime"), X("Invalid representation of time [err:FORG0001]"));
	}
	
	// State variables etc.
	bool gotDot = false;
	bool gotDigit = false;

	unsigned int pos = 0;
	long int tmpnum = 0;
	double decplace = 1;
	double tmpdec = 0;

	unsigned int numDigit = 0;

	// defaulting values
	MAPM hh = 0;
	MAPM mm = 0;
	MAPM ss = 0;
	_hasTimezone = false;
	bool zonepos = false;
	int zonehh = 0;
	int zonemm = 0;

	int state = 3 ;  // 3 = hour
	                 // 4 = minutes / 5 = sec / 6 = timezonehour / 7 = timezonemin
	XMLCh tmpChar;
	
	bool wrongformat = false;
	pos = 0;

	while ( ! wrongformat && pos < length) {
		tmpChar = time[pos];
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
				} else {
					numDigit ++;
				}
				tmpnum *= 10;
				tmpnum +=  static_cast<int>(tmpChar - 0x0030);
				gotDigit = true;
				
				break;
			}
		case L'-' : {
			if ( gotDigit && state == 5 && numDigit == 2) {					
					tmpdec =  tmpnum / decplace;			
					ss += tmpdec;					
					gotDigit = false;	
					_hasTimezone = true;
					zonepos = false;
					_hasTimezone = true;
					tmpnum = 0;
					numDigit = 0;
			}else {
				
				wrongformat = true;
			}
			state ++;		
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
					numDigit = 0;
					gotDigit = false;
				} else if ( state == 4 && numDigit == 2) {
					mm = tmpnum;
					tmpnum = 0;
					numDigit = 0;
					gotDigit = false;
				} else if ( state == 6 && numDigit == 2) {
					zonehh = tmpnum;
					_hasTimezone = true;
					tmpnum = 0;
					numDigit = 0;
					gotDigit = false;
				} else {
					wrongformat = true;
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
			_hasTimezone = true;
		}else if ( gotDigit && state == 5 && numDigit == 2) {
			tmpdec =  tmpnum / decplace;			
			ss += tmpdec;			
		}else {
			wrongformat = true;
		}
	} 

    if ( hh > 24 || mm > 59 || ss >= 60 || zonehh > 24 || zonemm > 59 )
        wrongformat = true;
    if(hh==24 && mm==0 && ss==0)
        hh=0;

	if ( wrongformat) {
		XQThrow2(XPath2TypeCastException,X("XSTimeImpl::setTime"), X("Invalid representation of time [err:FORG0001]"));
	}
	
  timezone_ = new Timezone(Timezone::convert(zonepos, zonehh,zonemm));

  seconds_ = composeSeconds(hh, mm, ss);
}

MAPM ATTimeOrDerivedImpl::parseTime(const XMLCh* const time, const MAPM &implicitTimezone)
{
  ATTimeOrDerivedImpl dt(0, 0, time);

  MAPM result = referenceDateTime(dt.seconds_, dt._hasTimezone, dt.timezone_);

  if(!dt._hasTimezone)
    result -= implicitTimezone;

  return result;
}

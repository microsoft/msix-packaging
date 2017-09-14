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
#include <xqilla/items/impl/ATDurationOrDerivedImpl.hpp>
#include <xqilla/items/impl/ATDecimalOrDerivedImpl.hpp>
#include <xqilla/items/ATDecimalOrDerived.hpp>
#include <xqilla/items/ATDoubleOrDerived.hpp>
#include <xqilla/exceptions/XPath2TypeCastException.hpp>
#include <xqilla/exceptions/IllegalArgumentException.hpp>
#include <xqilla/framework/XPath2MemoryManager.hpp>
#include <xqilla/context/DynamicContext.hpp>
#include <xercesc/util/XMLUniDefs.hpp>
#include <xercesc/util/XMLString.hpp>
#include <xercesc/framework/XMLBuffer.hpp>
#include <xercesc/validators/schema/SchemaSymbols.hpp>
#include <xqilla/mapm/m_apm.h>
#include "../../utils/DateUtils.hpp"
#include <xqilla/utils/XPath2Utils.hpp>
#include <xqilla/context/ItemFactory.hpp>

#if defined(XERCES_HAS_CPP_NAMESPACE)
XERCES_CPP_NAMESPACE_USE
#endif

ATDurationOrDerivedImpl::
ATDurationOrDerivedImpl(const XMLCh* typeURI, const XMLCh* typeName, const XMLCh* value, const DynamicContext* context)
  :_isPositive(true),
    _typeName(typeName),
   _typeURI(typeURI)
{
  setDuration(value);
  init(context);
}

ATDurationOrDerivedImpl::
ATDurationOrDerivedImpl(const XMLCh* typeURI, const XMLCh* typeName, const MAPM &months, const MAPM &seconds,
                        const DynamicContext* context)
  : _isPositive(true),
    _months(months),
    _seconds(seconds),
    _typeName(typeName),
    _typeURI(typeURI)
{
  init(context);
}

void ATDurationOrDerivedImpl::init(const DynamicContext* context)
{
  if(_months.sign() < 0) {
    _isPositive = false;
    _months = _months.neg();
  }
  if(_seconds.sign() != 0) {
    if(abs(_seconds.exponent()) > (int)ATDecimalOrDerivedImpl::g_nSignificantDigits) {
      // Call it zero if the size is too small to display
      _seconds = 0;
    }
    else if(_seconds.sign() < 0) {
      _isPositive = false;
      _seconds = _seconds.neg();
    }
  }

  if(this->isInstanceOfType (SchemaSymbols::fgURI_SCHEMAFORSCHEMA,
                             ATDurationOrDerived::fgDT_DAYTIMEDURATION, context)) {
    _durationType = DAY_TIME_DURATION;
  } else if (this->isInstanceOfType (SchemaSymbols::fgURI_SCHEMAFORSCHEMA,
                                     ATDurationOrDerived::fgDT_YEARMONTHDURATION, context)) {
    _durationType = YEAR_MONTH_DURATION;
  } else {
    _durationType = DURATION;
  }
}

void *ATDurationOrDerivedImpl::getInterface(const XMLCh *name) const
{
  if(name == Item::gXQilla) {
    return (void*)this;
  }
  return 0;
}

/* Get the name of the primitive type (basic type) of this type 
 * (ie "decimal" for xs:decimal) */
const XMLCh* ATDurationOrDerivedImpl::getPrimitiveTypeName() const
{
  switch(_durationType) {
  case DAY_TIME_DURATION:
    return fgDT_DAYTIMEDURATION;
  case YEAR_MONTH_DURATION:
    return fgDT_YEARMONTHDURATION;
  default: break;
  }

  return this->getPrimitiveName();
}

const XMLCh* ATDurationOrDerivedImpl::getPrimitiveName()  {
  return SchemaSymbols::fgDT_DURATION;
}

/* Get the name of this type  (ie "integer" for xs:integer) */
const XMLCh* ATDurationOrDerivedImpl::getTypeName() const {
  return _typeName;
}

/* Get the namespace URI for this type */
const XMLCh* ATDurationOrDerivedImpl::getTypeURI() const {
  return _typeURI; 
}

AnyAtomicType::AtomicObjectType ATDurationOrDerivedImpl::getTypeIndex() {
  return AnyAtomicType::DURATION;
}

/* If possible, cast this type to the target type */
AnyAtomicType::Ptr ATDurationOrDerivedImpl::castAsInternal(AtomicObjectType targetIndex, const XMLCh* targetURI,
                                                           const XMLCh* targetType, const DynamicContext* context) const {
  XMLBuffer buf(1023, context->getMemoryManager());
  // checking if what we're casting to by using isTypeOrDerivedFrom is expensive so we will 
  // determine what we are casting to and then what type we actually are.

  if(targetIndex == AnyAtomicType::YEAR_MONTH_DURATION) {
    //we're casting to a yearMonthDuration
 
    if (_durationType == DAY_TIME_DURATION) {
      // If ST is xdt:dayTimeDuration and TT is xdt:yearMonthDuration, the cast is permitted and returns a 
      // xdt:yearMonthDuration with value 0 months.
      buf.append(chLatin_P);
      buf.append(chDigit_0);
      buf.append(chLatin_M);         
      return context->getItemFactory()->createDerivedFromAtomicType(targetIndex, targetURI, targetType,
                                                                    buf.getRawBuffer(), context);
    } else if (_durationType == YEAR_MONTH_DURATION) {
      return context->getItemFactory()->createDerivedFromAtomicType(targetIndex, targetURI, targetType,
                                                                    this->asString(context), context);
    } else {
      //else we're a duration and we must remove the day and time components
      if(_months.sign() == 0) {
        buf.append(chLatin_P);
        buf.append(chDigit_0);
        buf.append(chLatin_M);         
      } else {
        if(!_isPositive) {
          buf.append(chDash); 
        }   
        buf.append(chLatin_P);

        MAPM yrs = _months.integer_divide(12);
        MAPM mths = DateUtils::modulo(_months, 12);
        if(yrs.sign() != 0) {
          buf.append(Numeric::asDecimalString(yrs, ATDecimalOrDerivedImpl::g_nSignificantDigits, context));
          buf.append(chLatin_Y);
        }
        if(mths.sign() != 0) {
          buf.append(Numeric::asDecimalString(mths, ATDecimalOrDerivedImpl::g_nSignificantDigits, context));
          buf.append(chLatin_M);
        }
      }
      return context->getItemFactory()->createDerivedFromAtomicType(targetIndex, targetURI, targetType,
                                                                    buf.getRawBuffer(), context);
    }

  } else if(targetIndex == AnyAtomicType::DAY_TIME_DURATION) {
    //we're casting to a dayTimeDuration

    if (_durationType == YEAR_MONTH_DURATION) {
      // If ST is xdt:yearMonthDuration and TT is xdt:dayTimeDuration, the cast is permitted and returns a 
      // xdt:dayTimeDuration with value 0.0 seconds.
      buf.append(chLatin_P);
      buf.append(chLatin_T);
      buf.append(chDigit_0);
      buf.append(chLatin_M);         
      return context->getItemFactory()->createDerivedFromAtomicType(targetIndex, targetURI, targetType,
                                                                    buf.getRawBuffer(), context);
    } else if (_durationType == DAY_TIME_DURATION) {
      return context->getItemFactory()->createDerivedFromAtomicType(targetIndex, targetURI, targetType,
                                                                    this->asString(context), context);
    } else {
      //else we're a duration and we must remove the year and month components
      if (_seconds.sign() == 0) {
        buf.append(chLatin_P);
        buf.append(chLatin_T);
        buf.append(chDigit_0);
        buf.append(chLatin_S);
      } else {
        if(!_isPositive) {
          buf.append(chDash); 
        }   
        buf.append(chLatin_P);   

        MAPM days = _seconds.integer_divide(DateUtils::g_secondsPerDay);
        MAPM hrs = DateUtils::modulo(_seconds, DateUtils::g_secondsPerDay).integer_divide(DateUtils::g_secondsPerHour);
        MAPM mnts = DateUtils::modulo(_seconds, DateUtils::g_secondsPerHour).integer_divide(DateUtils::g_secondsPerMinute);
        MAPM secs = DateUtils::modulo(_seconds, DateUtils::g_secondsPerMinute);
        if(days.sign() != 0) {
          buf.append(Numeric::asDecimalString(days, ATDecimalOrDerivedImpl::g_nSignificantDigits, context));
          buf.append(chLatin_D);
        }
        
        // mandatory center 'T', if the time is not zero
        if(hrs.sign() != 0 || mnts.sign() != 0 || secs.sign() != 0) {
          buf.append(chLatin_T);
        
          if(hrs.sign() != 0) {
            buf.append(Numeric::asDecimalString(hrs, ATDecimalOrDerivedImpl::g_nSignificantDigits, context));
            buf.append(chLatin_H);
          } 
          if(mnts.sign() != 0) {
            buf.append(Numeric::asDecimalString(mnts, ATDecimalOrDerivedImpl::g_nSignificantDigits, context));
            buf.append(chLatin_M);
          } 
          if(secs.sign() != 0) {
            buf.append(Numeric::asDecimalString(secs, ATDecimalOrDerivedImpl::g_nSignificantDigits, context));
            buf.append(chLatin_S);
          } 
        }
      }
      return context->getItemFactory()->createDerivedFromAtomicType(targetIndex, targetURI, targetType,
                                                                    buf.getRawBuffer(), context);
    }

  } else {
    //else we're casting to a duration or some other type and the base castAs will handle it.
    return AnyAtomicType::castAsInternal(targetIndex, targetURI, targetType, context);
  }

  return 0;
}

/* returns the XMLCh* (canonical) representation of this type */
const XMLCh* ATDurationOrDerivedImpl::asString(const DynamicContext* context) const {
  XMLBuffer buffer(1023, context->getMemoryManager());
  
  // if the value of this duration is zero, return 'PT0S' or 'P0M'
  if(_months.sign() == 0 && _seconds.sign() == 0) {
    if(_durationType == YEAR_MONTH_DURATION) {
      buffer.append(chLatin_P);
      buffer.append(chDigit_0);
      buffer.append(chLatin_M);
    } else {
      buffer.append(chLatin_P);
      buffer.append(chLatin_T);
      buffer.append(chDigit_0);
      buffer.append(chLatin_S);
    }    
  }
  else {
    if ( !_isPositive ) {
      buffer.append(chDash);
    }
  
    // madatory leading 'P'
    buffer.append(chLatin_P);

    if(_durationType != DAY_TIME_DURATION) {
      MAPM yrs = _months.integer_divide(12);
      MAPM mths = DateUtils::modulo(_months, 12);
      if(yrs.sign() != 0) {
        buffer.append(Numeric::asDecimalString(yrs, ATDecimalOrDerivedImpl::g_nSignificantDigits, context));
        buffer.append(chLatin_Y);
      }
      if(mths.sign() != 0) {
        buffer.append(Numeric::asDecimalString(mths, ATDecimalOrDerivedImpl::g_nSignificantDigits, context));
        buffer.append(chLatin_M);
      }
    }
    
    // append the day and time information if this is not a yearMonthDuration
    if(_durationType != YEAR_MONTH_DURATION) {
      MAPM days = _seconds.integer_divide(DateUtils::g_secondsPerDay);
      MAPM hrs = DateUtils::modulo(_seconds, DateUtils::g_secondsPerDay).integer_divide(DateUtils::g_secondsPerHour);
      MAPM mnts = DateUtils::modulo(_seconds, DateUtils::g_secondsPerHour).integer_divide(DateUtils::g_secondsPerMinute);
      MAPM secs = DateUtils::modulo(_seconds, DateUtils::g_secondsPerMinute);
      if(days.sign() != 0) {
        buffer.append(Numeric::asDecimalString(days, ATDecimalOrDerivedImpl::g_nSignificantDigits, context));
        buffer.append(chLatin_D);
      }
        
      // mandatory center 'T', if the time is not zero
      if(hrs.sign() != 0 || mnts.sign() != 0 || secs.sign() != 0) {
        buffer.append(chLatin_T);
        
        if(hrs.sign() != 0) {
          buffer.append(Numeric::asDecimalString(hrs, ATDecimalOrDerivedImpl::g_nSignificantDigits, context));
          buffer.append(chLatin_H);
        } 
        if(mnts.sign() != 0) {
          buffer.append(Numeric::asDecimalString(mnts, ATDecimalOrDerivedImpl::g_nSignificantDigits, context));
          buffer.append(chLatin_M);
        } 
        if(secs.sign() != 0) {
          buffer.append(Numeric::asDecimalString(secs, ATDecimalOrDerivedImpl::g_nSignificantDigits, context));
          buffer.append(chLatin_S);
        } 
      }
    }
  }
  return context->getMemoryManager()->getPooledString(buffer.getRawBuffer());
}

/* returns true if this duration is an instance of a xdt:dayTimeDuration */
bool ATDurationOrDerivedImpl::isDayTimeDuration() const {
  return _durationType == DAY_TIME_DURATION;
}

/* returns true if this duration is an instance of a xdt:yearMonthDuration */
bool ATDurationOrDerivedImpl::isYearMonthDuration() const {
  return _durationType == YEAR_MONTH_DURATION;
}

/* returns true if the two objects have the same boolean value
 * false otherwise */
bool ATDurationOrDerivedImpl::equals(const AnyAtomicType::Ptr &target, const DynamicContext* context) const
{
  switch(target->getPrimitiveTypeIndex()) {
  case DAY_TIME_DURATION:
  case YEAR_MONTH_DURATION:
  case DURATION: {
    return compare((const ATDurationOrDerived*)target.get(), context) == 0;
  }
  default:
    XQThrow2(::IllegalArgumentException,X("ATDurationOrDerivedImpl::equals"),
            X("Equality operator for given types not supported [err:XPTY0004]"));
  }
  return false;
}

int ATDurationOrDerivedImpl::compare(const ATDurationOrDerived::Ptr &other, const DynamicContext* context) const
{
  const ATDurationOrDerivedImpl* otherImpl = (const ATDurationOrDerivedImpl*)other.get();

  int cmp = _isPositive - otherImpl->_isPositive;
  if(cmp != 0) return cmp;

  cmp = _months.compare(otherImpl->_months) * (_isPositive ? 1 : -1);
  if(cmp != 0) return cmp;

  return _seconds.compare(otherImpl->_seconds) * (_isPositive ? 1 : -1);
}

static inline ATDurationOrDerived::Ptr newDayTimeDuration(const Numeric::Ptr &valueSeconds,
                                                          const DynamicContext* context)
{
  if(valueSeconds->getState() != Numeric::NUM &&
     valueSeconds->getState() != Numeric::NEG_NUM)
    XQThrow2(::IllegalArgumentException,X("newDayTimeDuration"),
            X("Overflow in duration operation [err:FODT0002]"));

  return context->getItemFactory()->createDayTimeDuration(valueSeconds->asMAPM(), context);
}

static inline ATDurationOrDerived::Ptr newYearMonthDuration(const Numeric::Ptr &valueMonth,
                                                            const DynamicContext* context)
{
  if(valueMonth->getState() != Numeric::NUM &&
     valueMonth->getState() != Numeric::NEG_NUM)
    XQThrow2(::IllegalArgumentException,X("newYearMonthDuration"),
            X("Overflow in duration operation [err:FODT0002]"));

  return context->getItemFactory()->createYearMonthDuration(valueMonth->asMAPM(), context);
}

/** Divide this duration by a number -- only available for xdt:dayTimeDuration
 *  and xdt:yearMonthDuration */
ATDurationOrDerived::Ptr ATDurationOrDerivedImpl::divide(const Numeric::Ptr &divisor,
                                                         const DynamicContext* context) const {
  if(divisor->isNaN())
      XQThrow2(::IllegalArgumentException,X("ATDurationOrDerivedImpl::divide"),
              X("Cannot divide a duration by NaN [err:FOCA0005]."));

  if(divisor->isZero())
    XQThrow2(::IllegalArgumentException,X("ATDurationOrDerivedImpl::divide"),
            X("Overflow in duration operation [err:FODT0002]"));

  if(_durationType == DAY_TIME_DURATION) {
    return newDayTimeDuration(asSeconds(context)->divide(divisor, context), context);
  }
  else if(_durationType == YEAR_MONTH_DURATION) { 
    return newYearMonthDuration(asMonths(context)->divide(divisor, context)->round(context), context);
  }
  else {
    // if we are trying to compare anything else -- error //
    XQThrow2(::IllegalArgumentException,X("ATDurationOrDerivedImpl::divide"),
            X("divide operator for given types not supported"));
  }
}

/** Divide this duration by a duration -- only available for xdt:dayTimeDuration
*  and xdt:yearMonthDuration */
ATDecimalOrDerived::Ptr ATDurationOrDerivedImpl::divide(const ATDurationOrDerived::Ptr &divisor,
                                                        const DynamicContext* context) const {
  if(_durationType == DAY_TIME_DURATION) {
    return this->asSeconds(context)->divide(divisor->asSeconds(context), context);
  }
  else if(_durationType == YEAR_MONTH_DURATION) { 
    return this->asMonths(context)->divide(divisor->asMonths(context), context);
  }
  else {
    // if we are trying to compare anything else -- error //
    XQThrow2(::IllegalArgumentException,X("ATDurationOrDerivedImpl::divide"),
            X("divide operator for given types not supported"));
  }
}

AnyAtomicType::AtomicObjectType ATDurationOrDerivedImpl::getPrimitiveTypeIndex() const {
  return _durationType;
}

/** Multiply this duration by a number -- only available for xdt:dayTimeDuration
 *  and xdt:yearMonthDuration */
ATDurationOrDerived::Ptr ATDurationOrDerivedImpl::multiply(const Numeric::Ptr &multiplier, const DynamicContext* context) const {
  if(multiplier->isNaN())
      XQThrow2(::IllegalArgumentException,X("ATDurationOrDerivedImpl::multiply"),
              X("Cannot multiply a duration by NaN [err:FOCA0005]."));

  if(_durationType == DAY_TIME_DURATION) {
    return newDayTimeDuration(asSeconds(context)->multiply(multiplier, context), context);
  }
  else if(_durationType == YEAR_MONTH_DURATION) { 
    return newYearMonthDuration(asMonths(context)->multiply(multiplier, context)->round(context), context);
  }
  else {
    // if we are trying to compare anything else -- error //
    XQThrow2(::IllegalArgumentException,X("ATDurationOrDerivedImpl::multiply"),
            X("multiply operator for given types not supported"));
  }

}

/** Add a duration to this duration -- only available for xdt:dayTimeDuration 
 *  and xdt:yearMonthDuration */
ATDurationOrDerived::Ptr ATDurationOrDerivedImpl::add(const ATDurationOrDerived::Ptr &other, const DynamicContext* context) const {
  if(this->isDayTimeDuration() && ((const ATDurationOrDerived*)other)->isDayTimeDuration()) {
    return newDayTimeDuration(asSeconds(context)->add(other->asSeconds(context), context), context);
  }
  else if(this->isYearMonthDuration() && ((const ATDurationOrDerived*)other)->isYearMonthDuration() ) {
    return newYearMonthDuration(asMonths(context)->add(other->asMonths(context), context), context);
  }
  else {
    XQThrow2(::IllegalArgumentException, X("ATDurationOrDerivedImpl::add"),
            X("add operation not supported for given types"));
  }
}

/** Subtract a duration from this duration -- only available for xdt:dayTimeDuration
 *  and xdt:yearMonthDuration */
ATDurationOrDerived::Ptr ATDurationOrDerivedImpl::subtract(const ATDurationOrDerived::Ptr &other, const DynamicContext* context) const {
  if(this->isDayTimeDuration() && ((const ATDurationOrDerived*)other)->isDayTimeDuration()) {
    return newDayTimeDuration(asSeconds(context)->subtract(other->asSeconds(context), context), context);        
  } else if(this->isYearMonthDuration() && ((const ATDurationOrDerived*)other)->isYearMonthDuration() ) {
    return newYearMonthDuration(asMonths(context)->subtract(other->asMonths(context), context), context);
  } else {
    XQThrow2(::IllegalArgumentException, X("ATDurationOrDerivedImpl::subtract"),
            X("subtract operation not supported for given types"));
  }
}

ATDecimalOrDerived::Ptr ATDurationOrDerivedImpl::getYears(const DynamicContext* context) const
{
  MAPM result = _months.integer_divide(12);
  if(!_isPositive) result = result.neg();
  return context->getItemFactory()->createInteger(result, context);
}

ATDecimalOrDerived::Ptr ATDurationOrDerivedImpl::getMonths(const DynamicContext* context) const
{
  MAPM result = DateUtils::modulo(_months, 12);
  if(!_isPositive) result = result.neg();
  return context->getItemFactory()->createInteger(result, context);
}

ATDecimalOrDerived::Ptr ATDurationOrDerivedImpl::getDays(const DynamicContext* context) const
{
  MAPM result = _seconds.integer_divide(DateUtils::g_secondsPerDay);
  if(!_isPositive) result = result.neg();
  return context->getItemFactory()->createInteger(result, context);
}

ATDecimalOrDerived::Ptr ATDurationOrDerivedImpl::getHours(const DynamicContext* context) const
{
  MAPM result = DateUtils::modulo(_seconds, DateUtils::g_secondsPerDay).integer_divide(DateUtils::g_secondsPerHour);
  if(!_isPositive) result = result.neg();
  return context->getItemFactory()->createInteger(result, context);
}

ATDecimalOrDerived::Ptr ATDurationOrDerivedImpl::getMinutes(const DynamicContext* context) const
{
  MAPM result = DateUtils::modulo(_seconds, DateUtils::g_secondsPerHour).integer_divide(DateUtils::g_secondsPerMinute);
  if(!_isPositive) result = result.neg();
  return context->getItemFactory()->createInteger(result, context);
}

ATDecimalOrDerived::Ptr ATDurationOrDerivedImpl::getSeconds(const DynamicContext* context) const
{
  MAPM result = DateUtils::modulo(_seconds, DateUtils::g_secondsPerMinute);
  if(!_isPositive) result = result.neg();
  return context->getItemFactory()->createDecimal(result, context);
}

bool ATDurationOrDerivedImpl::isNegative() const {
  return !_isPositive;
}

/* return this duration in forms of seconds -- only for dayTimeDuration */
ATDecimalOrDerived::Ptr ATDurationOrDerivedImpl::asSeconds(const DynamicContext* context) const
{
  MAPM result = _seconds;
  if(!_isPositive) result = result.neg();
  return context->getItemFactory()->createDecimal(result, context);
}

ATDecimalOrDerived::Ptr ATDurationOrDerivedImpl::asMonths(const DynamicContext* context) const
{
  MAPM result = _months;
  if(!_isPositive) result = result.neg();
  return context->getItemFactory()->createDecimal(result, context);
}

void ATDurationOrDerivedImpl::setDuration(const XMLCh* const s)
{
  parseDuration(s, _months, _seconds);
}

void ATDurationOrDerivedImpl::parseDuration(const XMLCh *const s, MAPM &months, MAPM &seconds)
{
  unsigned int length = XPath2Utils::uintStrlen(s);
 
  if(s == 0) {
    XQThrow2(XPath2TypeCastException,X("XSDurationImpl::setDuration"), X("Invalid representation of duration [err:FORG0001]"));
  }
  
  // State variables etc.
  bool gotDot = false;
  bool gotDigit = false;
  bool stop = false;
  bool Texist = false;
  unsigned int pos = 0;
  long int tmpnum = 0;
  double decplace = 1;
  double tmpdec = 0;

  // defaulting values
  bool isPositive = true;
  MAPM year = 0;
  MAPM month = 0;
  MAPM day = 0;
  MAPM hour = 0;
  MAPM minute = 0;
  MAPM sec = 0;

  int state = 0 ; // 0 = year / 1 = month / 2 = day / 3 = hour / 4 = minutes / 5 = sec
  XMLCh tmpChar;
  
  bool wrongformat = false;

  // check initial 'negative' sign and the P character

  if ( length > 1 && s[0] == L'-' && s[1] == L'P' ) {
    isPositive = false;
    pos = 2;
  } else if (  length > 1 && s[0] == L'P' ) {
    isPositive = true;
    pos = 1;
  } else {
    wrongformat = true;
  }

  
  while ( ! wrongformat && !stop && pos < length) {
    tmpChar = s[pos];
    pos++;
    switch(tmpChar) {

      // a dot, only will occur when parsing the second
      case L'.': {
        if (! gotDot && gotDigit) {
          gotDot = true;
          sec = tmpnum;
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
        if ( gotDot ) {
          decplace *= 10;          
        } 
        tmpnum *= 10;
        tmpnum +=  static_cast<int>(tmpChar - 0x0030);
        gotDigit = true;
        
        break;
      }    
      case L'Y' : {
        if ( state == 0 && gotDigit && !gotDot ) {
          year = tmpnum;
          state = 1;
          tmpnum = 0;                  
          gotDigit = false;
        } else {    
          
          wrongformat = true;
        }
        break;
      }
      case L'M' : {
        if ( gotDigit) {
          if ( state < 4 && Texist && !gotDot) {
            minute = tmpnum;
            state = 4;
            gotDigit = false;
            tmpnum = 0;                    
            break;
          } else if ( state < 2 && ! Texist && !gotDot) {
            month = tmpnum;
            state = 1;
            gotDigit = false;
            tmpnum = 0;                    
            break;
          }
        }
        
        wrongformat = true;        
        break;
      }
    case L'D' : {
        if ( state < 2 && gotDigit && !gotDot) {
          day = tmpnum;
          state = 2;
          gotDigit = false;
          tmpnum = 0;
        } else {          
          
          wrongformat = true;
        }
        break;
      
    }
    case L'T' : {
      if ( state < 3 && !gotDigit && !gotDot) {
        Texist = true;
      } else {
        
        wrongformat = true;
      }
      break;
    }
    case L'H' : {
      if ( state < 3 && gotDigit && Texist && !gotDot) {
        hour = tmpnum;
        state = 3;
        gotDigit = false;
        tmpnum = 0;
      } else {    
        
        wrongformat = true;
      }
      break;
    }
    case L'S' : {
      if ( state < 5 && gotDigit && Texist) {
        tmpdec =  tmpnum / decplace;        
        sec += tmpdec;        
        state = 5;
        gotDigit = false;
        tmpnum = 0;
      } else {    
        
        wrongformat = true;
      }      
      break;
    }
    default:
         wrongformat = true;
    }  
  }

  // check duration format
  if ( wrongformat || (Texist && state < 3) || gotDigit) {
    XQThrow2(XPath2TypeCastException,X("ATDurationOrDerivedImpl::setDuration"), X("Invalid representation of duration [err:FORG0001]"));
  }

  months = year * 12 + month;

  seconds = day * DateUtils::g_secondsPerDay +
    hour * DateUtils::g_secondsPerHour +
    minute * DateUtils::g_secondsPerMinute +
    sec;

  if(!isPositive) {
    months = months.neg();
    seconds = seconds.neg();
  }
}

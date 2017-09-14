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
#include <xqilla/items/impl/ATGDayOrDerivedImpl.hpp>
#include <xqilla/exceptions/IllegalArgumentException.hpp>
#include <xqilla/exceptions/XPath2TypeCastException.hpp>
#include <xercesc/util/XMLUni.hpp>
#include <xercesc/util/XMLUniDefs.hpp>
#include <xercesc/validators/schema/SchemaSymbols.hpp>
#include <xercesc/framework/XMLBuffer.hpp>
#include <xqilla/utils/XStr.hpp>
#include <xqilla/framework/XPath2MemoryManager.hpp>
#include <xqilla/mapm/m_apm.h>
#include <xqilla/items/Timezone.hpp>
#include "../../utils/DateUtils.hpp"
#include <xqilla/items/ATDecimalOrDerived.hpp>
#include <xqilla/items/DatatypeFactory.hpp>
#include <xqilla/context/ItemFactory.hpp>
#include <xqilla/utils/XPath2Utils.hpp>

#include <xercesc/util/XMLString.hpp>

#if defined(XERCES_HAS_CPP_NAMESPACE)
XERCES_CPP_NAMESPACE_USE
#endif

ATGDayOrDerivedImpl::
ATGDayOrDerivedImpl(const XMLCh* typeURI, const XMLCh* typeName, const XMLCh* value, const DynamicContext* context): 
    ATGDayOrDerived(),
    _typeName(typeName),
    _typeURI(typeURI) { 
    
     setGDay(value);
}

void *ATGDayOrDerivedImpl::getInterface(const XMLCh *name) const
{
  if(name == Item::gXQilla) {
    return (void*)this;
  }
  return 0;
}

/* Get the name of the primitive type (basic type) of this type
 * (ie "decimal" for xs:decimal) */
const XMLCh* ATGDayOrDerivedImpl::getPrimitiveTypeName() const {
  return this->getPrimitiveName();
}

const XMLCh* ATGDayOrDerivedImpl::getPrimitiveName()  {
  return SchemaSymbols::fgDT_DAY;
}

/* Get the name of this type  (ie "integer" for xs:integer) */
const XMLCh* ATGDayOrDerivedImpl::getTypeName() const {
  return _typeName;
}

/* Get the namespace URI for this type */
const XMLCh* ATGDayOrDerivedImpl::getTypeURI() const {
  return _typeURI; 
}

AnyAtomicType::AtomicObjectType ATGDayOrDerivedImpl::getTypeIndex() {
  return AnyAtomicType::G_DAY;
} 

/* returns the XMLCh* (canonical) representation of this type */
const XMLCh* ATGDayOrDerivedImpl::asString(const DynamicContext* context) const {
  XMLBuffer buffer(1023, context->getMemoryManager());
  buffer.append(chDash);
  buffer.append(chDash);
  buffer.append(chDash);
  DateUtils::formatNumber(_gDay, 2, buffer);
  if(_hasTimezone) {
    buffer.append(timezone_->asString(context));
  }
  return context->getMemoryManager()->getPooledString(buffer.getRawBuffer());
}

static inline MAPM referenceDateTime(const MAPM &DD, bool hasTimezone, const Timezone::Ptr &timezone)
{
  MAPM result = DateUtils::convertDMY2Absolute(DD, 12, 1972) * DateUtils::g_secondsPerDay;

  if(hasTimezone) {
    result -= timezone->asSeconds();
  }

  return result;
  }

MAPM ATGDayOrDerivedImpl::buildReferenceDateTime(const DynamicContext *context) const
{
  MAPM result = referenceDateTime(_gDay, _hasTimezone, timezone_);

  if(!_hasTimezone)
    result -= context->getImplicitTimezone()->asSeconds(context)->asMAPM();

  return result;
}

/* Returns true if and only if the xs:dateTimes representing the starting instants of equivalent occurrences of $arg1 and $arg2 
 * compare equal. The starting instants of equivalent occurrences of $arg1 and $arg2 are calculated by adding the missing 
 * components of $arg1 and $arg2 from an xs:dateTime template such as 1972-12-xxT00:00:00. Returns false otherwise.
 */
bool ATGDayOrDerivedImpl::equals(const AnyAtomicType::Ptr &target, const DynamicContext* context) const {
  if(this->getPrimitiveTypeIndex() != target->getPrimitiveTypeIndex()) {
        XQThrow2(::IllegalArgumentException,X("ATGDayOrDerivedImpl::equals"), X("Equality operator for given types not supported [err:XPTY0004]"));
  }
  return compare((const ATGDayOrDerived *)target.get(), context) == 0;
}

int ATGDayOrDerivedImpl::compare(const ATGDayOrDerived::Ptr &other, const DynamicContext *context) const
{
  return buildReferenceDateTime(context).compare(((const ATGDayOrDerivedImpl *)other.get())->buildReferenceDateTime(context));
}

/** Returns true if a timezone is defined for this.  False otherwise.*/
bool ATGDayOrDerivedImpl::hasTimezone() const {
  return _hasTimezone;
}

/** Sets the timezone to the given timezone.*/
ATGDayOrDerived::Ptr ATGDayOrDerivedImpl::setTimezone(const Timezone::Ptr &timezone, const DynamicContext* context) const {
  XMLBuffer buffer(1023, context->getMemoryManager());

  buffer.append(chDash);
  buffer.append(chDash);
  buffer.append(chDash);
  DateUtils::formatNumber(_gDay, 2, buffer);
  if(timezone != NULLRCP) 
    buffer.append(timezone->asString(context));
  return context->getItemFactory()->createGDayOrDerived(this->getTypeURI(), this->getTypeName(), buffer.getRawBuffer(), context);
}


AnyAtomicType::AtomicObjectType ATGDayOrDerivedImpl::getPrimitiveTypeIndex() const {
  return this->getTypeIndex();
}

/* parse the gDay */
void ATGDayOrDerivedImpl::setGDay(const XMLCh* const value) {
 
  if(value == NULL) {
      XQThrow2(XPath2TypeCastException,X("ATGDayOrDerivedImpl::setGDay"),
               X("Invalid representation of gDay [err:FORG0001]"));
  }
  unsigned int length = XPath2Utils::uintStrlen(value);
	
  // State variables etc.
  bool gotDigit = false;
  unsigned int pos = 0;
  long int tmpnum = 0;

  // defaulting values
  MAPM DD = 0;
  _hasTimezone = false;
  bool zonepos = false;
  int zonehh = 0;
  int zonemm = 0;

  int state = 0 ; // 0 = year / 1 = month / 2 = day / 3 = hour 
  // 4 = minutes / 5 = sec / 6 = timezonehour / 7 = timezonemin
  XMLCh tmpChar;

  bool wrongformat = false;

  if ( length < 5 || value[0] != L'-' || value[1] != L'-' || value[2] != L'-') {
    wrongformat = true;
  }else{
    pos = 3;
    state = 1;
  } 

  while ( ! wrongformat && pos < length) {
    tmpChar = value[pos];
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
      case 0x0039:  {
                      tmpnum *= 10;
                      tmpnum +=  static_cast<int>(tmpChar - 0x0030);
                      gotDigit = true;
                      break;
                    }
      case L':' : {
                    if (gotDigit &&  state == 6 ) {
                      zonehh = tmpnum;
                      tmpnum = 0;
                      gotDigit = false;				
                      state ++;
                    }else {
                      wrongformat = true;
                    }
                    break;
                  }		
      case L'-' : {
                    if ( gotDigit && state == 1 ) {
                      DD = tmpnum;		
                      state = 6;
                      gotDigit = false;			
                      _hasTimezone = true;
                      zonepos = false;
                      tmpnum = 0;
                    } else {
                      wrongformat = true;
                    }			
                    break;			
                  }
      case L'+' : {
                    if ( gotDigit && state == 1 ) {
                      DD = tmpnum;
                      state = 6; 
                      gotDigit = false;			
                      _hasTimezone = true;
                      zonepos = true;
                      tmpnum = 0;
                    } else {
                      wrongformat = true;
                    }
                    break;
                  }
      case L'Z' : {
                    if (gotDigit && state == 1 ) {
                      DD = tmpnum;
                      state = 8; // final state
                      _hasTimezone = true;
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

  if (gotDigit) {
    if ( gotDigit && state == 7 ) {
      zonemm = tmpnum;
    }else if ( gotDigit && state == 1 ) {
      DD = tmpnum;
    }else {
      wrongformat = true;
    }
  } 

  // check time format
  if ( DD > 31  || zonehh > 24 || zonemm > 60 ) {
    wrongformat = true;
  }

  if ( wrongformat) {
    XQThrow2(XPath2TypeCastException,X("ATGDayOrDerivedImpl::setGDay"),
             X("Invalid representation of gDay [err:FORG0001]"));
  }

  timezone_ = new Timezone(Timezone::convert(zonepos, zonehh, zonemm));
  _gDay = DD;
  
}

MAPM ATGDayOrDerivedImpl::parseGDay(const XMLCh* const value, const MAPM &implicitTimezone)
{
  ATGDayOrDerivedImpl dt(0, 0, value);

  MAPM result = referenceDateTime(dt._gDay, dt._hasTimezone, dt.timezone_);

  if(!dt._hasTimezone)
    result -= implicitTimezone;

  return result;
}

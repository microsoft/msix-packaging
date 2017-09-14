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
#include <xqilla/items/impl/ATGYearOrDerivedImpl.hpp>
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
#include <xqilla/context/ItemFactory.hpp>
#include <xqilla/items/DatatypeFactory.hpp>
#include <xqilla/utils/XPath2Utils.hpp>

#include <xercesc/util/XMLString.hpp>

#if defined(XERCES_HAS_CPP_NAMESPACE)
XERCES_CPP_NAMESPACE_USE
#endif

ATGYearOrDerivedImpl::
ATGYearOrDerivedImpl(const XMLCh* typeURI, const XMLCh* typeName, const XMLCh* value, const DynamicContext* context): 
    ATGYearOrDerived(),
    _typeName(typeName),
    _typeURI(typeURI) { 
    
     setGYear(value);
}

void *ATGYearOrDerivedImpl::getInterface(const XMLCh *name) const
{
  if(name == Item::gXQilla) {
    return (void*)this;
  }
  return 0;
}

/* Get the name of the primitive type (basic type) of this type
 * (ie "decimal" for xs:decimal) */
const XMLCh* ATGYearOrDerivedImpl::getPrimitiveTypeName() const {
  return getPrimitiveName();
}

const XMLCh* ATGYearOrDerivedImpl::getPrimitiveName() {
  return SchemaSymbols::fgDT_YEAR;
}

/* Get the name of this type  (ie "integer" for xs:integer) */
const XMLCh* ATGYearOrDerivedImpl::getTypeName() const {
  return _typeName;
}

/* Get the namespace URI for this type */
const XMLCh* ATGYearOrDerivedImpl::getTypeURI() const {
  return _typeURI; 
}

AnyAtomicType::AtomicObjectType ATGYearOrDerivedImpl::getTypeIndex() {
  return AnyAtomicType::G_YEAR;
} 

/* returns the XMLCh* (canonical) representation of this type */
const XMLCh* ATGYearOrDerivedImpl::asString(const DynamicContext* context) const {
  XMLBuffer buffer(1023, context->getMemoryManager());

  DateUtils::formatNumber(_YY, 4, buffer);
  if(_hasTimezone) {
    buffer.append(timezone_->asString(context));
  }
  return context->getMemoryManager()->getPooledString(buffer.getRawBuffer());
}

static inline MAPM referenceDateTime(const MAPM &YY, bool hasTimezone, const Timezone::Ptr &timezone)
{
  MAPM result = DateUtils::convertDMY2Absolute(1, 1, YY) * DateUtils::g_secondsPerDay;

  if(hasTimezone) {
    result -= timezone->asSeconds();
  }

  return result;
}

MAPM ATGYearOrDerivedImpl::buildReferenceDateTime(const DynamicContext *context) const
{
  MAPM result = referenceDateTime(_YY, _hasTimezone, timezone_);

  if(!_hasTimezone)
    result -= context->getImplicitTimezone()->asSeconds(context)->asMAPM();

  return result;
}

/* returns true if the two objects
   * false otherwise */
bool ATGYearOrDerivedImpl::equals(const AnyAtomicType::Ptr &target, const DynamicContext* context) const {
  if(this->getPrimitiveTypeIndex() != target->getPrimitiveTypeIndex()) {
    XQThrow2(::IllegalArgumentException,X("ATGYearOrDerivedImpl::equals"),
	    X("Equality operator for given types not supported [err:XPTY0004]"));
  }
  return compare((const ATGYearOrDerived *)target.get(), context) == 0;
}

int ATGYearOrDerivedImpl::compare(const ATGYearOrDerived::Ptr &other, const DynamicContext *context) const
{
  return buildReferenceDateTime(context).compare(((const ATGYearOrDerivedImpl *)other.get())->buildReferenceDateTime(context));
}

/** Returns true if a timezone is defined for this.  False otherwise.*/
bool ATGYearOrDerivedImpl::hasTimezone() const {
  return _hasTimezone;
}

/** Sets the timezone to the given timezone.*/
ATGYearOrDerived::Ptr ATGYearOrDerivedImpl::setTimezone(const Timezone::Ptr &timezone, const DynamicContext* context) const {
  XMLBuffer buffer(1023, context->getMemoryManager());

  DateUtils::formatNumber(_YY, 4, buffer);
  if(timezone != NULLRCP) 
    buffer.append(timezone->asString(context));
  return context->getItemFactory()->createGYearOrDerived(this->getTypeURI(), this->getTypeName(), buffer.getRawBuffer(), context);
}


AnyAtomicType::AtomicObjectType ATGYearOrDerivedImpl::getPrimitiveTypeIndex() const {
  return this->getTypeIndex();
}

/* parse the gYear */
void ATGYearOrDerivedImpl::setGYear(const XMLCh* const value) {
 
  unsigned int length = XPath2Utils::uintStrlen(value);
  if(value == NULL) {
    XQThrow2(XPath2TypeCastException,X("ATGYearOrDerivedImpl::setGYear"), X("Invalid representation of gYear [err:FORG0001]"));
  }
	
	// State variables etc.
	bool gotDigit = false;

	unsigned int pos = 0;
	long int tmpnum = 0;
	unsigned int numDigit = 0;
	bool negative = false;

	// defaulting values
	MAPM YY = 0;
  _hasTimezone = false;
	bool zonepos = false;
	int zonehh = 0;
	int zonemm = 0;

	int state = 0 ; // 0 = year / 1 = month / 2 = day / 3 = hour 
	                 // 4 = minutes / 5 = sec / 6 = timezonehour / 7 = timezonemin
	XMLCh tmpChar;
	
	bool wrongformat = false;

	if ( length > 0 && value[0] == L'-'  ) {
	  negative = true;
		pos = 1;
	}else{
		pos = 0;
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
	  	case 0x0039: {
				numDigit ++;
				tmpnum *= 10;
				tmpnum +=  static_cast<int>(tmpChar - 0x0030);
				gotDigit = true;
				
				break;
			}
		case L'-' : {
			if ( gotDigit && state == 0 && numDigit >= 4) { 
					YY = tmpnum;
					tmpnum = 0;
					gotDigit = false;			
					_hasTimezone = true;
					zonepos = false;
					state = 6;
					numDigit = 0;
			} else {
				wrongformat = true;
			}
			break;			
		}
    case L'+' : {
			if ( gotDigit && state == 0 && numDigit >= 4) {
				YY = tmpnum;
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
			if (gotDigit && state == 6 && numDigit == 2 ) {
					zonehh = tmpnum;
					tmpnum = 0;
					gotDigit = false;
					numDigit = 0;
					state = 7;
			}else {
				wrongformat = true;
			}
			break;
		}
		case L'Z' : {
			if (gotDigit && state == 0 && numDigit >= 4) {
				YY = tmpnum;
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
		}else if ( gotDigit && state == 0 && numDigit >= 4 ) {
				YY += tmpnum;			
		}else {
			wrongformat = true;
		}
	} 

	if (negative) {
	  YY = YY * -1;
	}

	if (YY == 0) {
	  wrongformat = true;
	}
	
	// check time format

	if (wrongformat) {
	  XQThrow2(XPath2TypeCastException,X("ATGYearOrDerivedImpl::setGYear"), X("Invalid representation of gYear [err:FORG0001]"));
	}

  timezone_ = new Timezone(Timezone::convert(zonepos, zonehh, zonemm));

  _YY = YY;
}

MAPM ATGYearOrDerivedImpl::parseGYear(const XMLCh* const value, const MAPM &implicitTimezone)
{
  ATGYearOrDerivedImpl dt(0, 0, value);

  MAPM result = referenceDateTime(dt._YY, dt._hasTimezone, dt.timezone_);

  if(!dt._hasTimezone)
    result -= implicitTimezone;

  return result;
}

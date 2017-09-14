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
#include <xqilla/items/impl/ATDecimalOrDerivedImpl.hpp>
#include <xqilla/utils/XPath2Utils.hpp>
#include <xqilla/items/ATBooleanOrDerived.hpp>
#include <xqilla/context/DynamicContext.hpp>
#include <xqilla/exceptions/IllegalArgumentException.hpp>
#include <xqilla/exceptions/XPath2ErrorException.hpp>
#include <xqilla/exceptions/XPath2TypeCastException.hpp>
#include <xercesc/util/XMLUni.hpp>
#include <xercesc/util/XMLUniDefs.hpp>
#include <xercesc/util/XMLString.hpp>
#include <xercesc/framework/XMLBuffer.hpp>
#include <xercesc/validators/schema/SchemaSymbols.hpp>
#include <xqilla/framework/XPath2MemoryManager.hpp>
#include <xqilla/context/ItemFactory.hpp>

#if defined(XERCES_HAS_CPP_NAMESPACE)
XERCES_CPP_NAMESPACE_USE
#endif

unsigned int ATDecimalOrDerivedImpl::g_nSignificantDigits=50;

ATDecimalOrDerivedImpl::
ATDecimalOrDerivedImpl(const XMLCh* typeURI, const XMLCh* typeName, const XMLCh* value, const StaticContext* context): 
    ATDecimalOrDerived(),
    _typeName(typeName),
    _typeURI(typeURI) { 
    
  setDecimal(value);
  if(this->isInstanceOfType (SchemaSymbols::fgURI_SCHEMAFORSCHEMA,
                             SchemaSymbols::fgDT_INTEGER, context)) {
    _isInteger = true;
  } else {
    _isInteger = false;
  }
}

ATDecimalOrDerivedImpl::
ATDecimalOrDerivedImpl(const XMLCh* typeURI, const XMLCh* typeName, const MAPM value, const StaticContext* context): 
    ATDecimalOrDerived(),
    _typeName(typeName),
    _typeURI(typeURI) { 
    
  _decimal = value;
  if(this->isInstanceOfType (SchemaSymbols::fgURI_SCHEMAFORSCHEMA,
                             SchemaSymbols::fgDT_INTEGER, context)) {
    _isInteger = true;
  } else {
    _isInteger = false;
  }
}

ATDecimalOrDerivedImpl::
ATDecimalOrDerivedImpl(int value):
    ATDecimalOrDerived(),
    _typeName(SchemaSymbols::fgDT_INTEGER),
    _typeURI(SchemaSymbols::fgURI_SCHEMAFORSCHEMA) { 

  _decimal = value;
  _isInteger = true;
}

void *ATDecimalOrDerivedImpl::getInterface(const XMLCh *name) const
{
  if(name == Item::gXQilla) {
    return (void*)this;
  }
  return 0;
}

/* Get the name of the primitive type (basic type) of this type
 * (ie "decimal" for xs:decimal) */
const XMLCh* ATDecimalOrDerivedImpl::getPrimitiveTypeName() const {
  return getPrimitiveName();
}

const XMLCh* ATDecimalOrDerivedImpl::getPrimitiveName() {
  return SchemaSymbols::fgDT_DECIMAL;
}

/* Get the name of this type  (ie "integer" for xs:integer) */
const XMLCh* ATDecimalOrDerivedImpl::getTypeName() const {
  return _typeName;
}

/* Get the namespace URI for this type */
const XMLCh* ATDecimalOrDerivedImpl::getTypeURI() const {
  return _typeURI; 
}

AnyAtomicType::AtomicObjectType ATDecimalOrDerivedImpl::getTypeIndex() {
  return AnyAtomicType::DECIMAL;
} 

/* returns the XMLCh* (canonical) representation of this type */
const XMLCh* ATDecimalOrDerivedImpl::asString(const DynamicContext* context) const
{
  return asDecimalString(g_nSignificantDigits, context);
}

/* Promote this to the given type, if possible */
Numeric::Ptr ATDecimalOrDerivedImpl::promoteTypeIfApplicable(AnyAtomicType::AtomicObjectType typeIndex,
                                                             const DynamicContext* context) const
{
  switch(typeIndex) {
  case AnyAtomicType::DECIMAL:
    return this;
  case AnyAtomicType::FLOAT:
  case AnyAtomicType::DOUBLE:
    return (const Numeric::Ptr)castAs(typeIndex, context);
  default:
    break;
  }
  return 0;
}

/** Returns a Numeric object which is the sum of this and other */
Numeric::Ptr ATDecimalOrDerivedImpl::add(const Numeric::Ptr &other, const DynamicContext* context) const {
  if(this->isOfType(other->getTypeURI(), other->getTypeName(), context)) {
    // if both are of the same type exactly, we can perform addition
    ATDecimalOrDerivedImpl* otherImpl = (ATDecimalOrDerivedImpl*)(const Numeric*)other;

    // if integer, return xs:integer, otherwise xs:decimal
    if(_isInteger) {
      return context->getItemFactory()->createInteger(_decimal + otherImpl->_decimal, context);
    }
    return context->getItemFactory()->createDecimal(_decimal + otherImpl->_decimal, context);
  } else if(this->getPrimitiveTypeIndex() != other->getPrimitiveTypeIndex()) {
    // if other is not a decimal, then we need to promote this to a float or double
    return ((const Numeric::Ptr )this->castAs(other->getPrimitiveTypeIndex(), context))->add(other, context);
  } else if (this->isInstanceOfType(other->getTypeURI(), other->getTypeName(), context)) {
    // here we know we have two decimals, and this is 'lower' in the hierarchy than other
    // so cast this to other's type
    return ((const Numeric::Ptr )this->castAs(AnyAtomicType::DECIMAL, other->getTypeURI(), other->getTypeName(),
                                              context))->add(other, context);
  } else if (other->isInstanceOfType(this->getTypeURI(), this->getTypeName(), context)) {
    // here we have two decimals, and this is 'higher' in the hierarchy than other
    // so cast other to this' type
    return this->add((const Numeric::Ptr )other->castAs(AnyAtomicType::DECIMAL, this->getTypeURI(), this->getTypeName(),
                                                        context), context);
  } else {
    // we have two separate branches.  if either is instance of integer, cast it to integer, otherwise, cast to decimal
    // revisit: this is not the prettiest way to do it.  You would want to go up the tree one by one instead of
    // jumping to integer and decimal
    ATDecimalOrDerived::Ptr first;
    ATDecimalOrDerived::Ptr second;
    if(this->_isInteger) {
      first = (const ATDecimalOrDerived::Ptr )this->castAs(AnyAtomicType::DECIMAL, SchemaSymbols::fgURI_SCHEMAFORSCHEMA,
                           SchemaSymbols::fgDT_INTEGER, context);
    } else {
      first = (const ATDecimalOrDerived::Ptr )this->castAs(AnyAtomicType::DECIMAL, context);
    }

    if(((ATDecimalOrDerivedImpl*)(const Numeric*)other)->_isInteger) {
      second = (const ATDecimalOrDerived::Ptr )other->castAs(AnyAtomicType::DECIMAL, SchemaSymbols::fgURI_SCHEMAFORSCHEMA,
                            SchemaSymbols::fgDT_INTEGER, context);
    } else {
      second = (const ATDecimalOrDerived::Ptr )other->castAs(AnyAtomicType::DECIMAL, context);
    }
    return first->add(second, context);
  }
      
      
}

/** Returns a Numeric object which is the difference of this and
   * other */
Numeric::Ptr ATDecimalOrDerivedImpl::subtract(const Numeric::Ptr &other, const DynamicContext* context) const {
    if(this->isOfType(other->getTypeURI(), other->getTypeName(), context)) {
    // if both are of the same type exactly, we can perform subtraction
    ATDecimalOrDerivedImpl* otherImpl = (ATDecimalOrDerivedImpl*)(const Numeric*)other;
    // if integer, return xs:integer, otherwise xs:decimal    
    if(_isInteger) {
      return context->getItemFactory()->createInteger(_decimal - otherImpl->_decimal, context);
    }
    return context->getItemFactory()->createDecimal(_decimal - otherImpl->_decimal, context);

  } else if(this->getPrimitiveTypeIndex() != other->getPrimitiveTypeIndex()) {
    // if other is not a decimal, then we need to promote this to a float or double
    return ((const Numeric::Ptr )this->castAs(other->getPrimitiveTypeIndex(), context))->subtract(other, context);
  } else if (this->isInstanceOfType(other->getTypeURI(), other->getTypeName(), context)) {
    // here we know we have two decimals, and this is 'lower' in the hierarchy than other
    // so cast this to other's type
    return ((const Numeric::Ptr )this->castAs(AnyAtomicType::DECIMAL, other->getTypeURI(), other->getTypeName(),
                                              context))->subtract(other, context);
  } else if (other->isInstanceOfType(this->getTypeURI(), this->getTypeName(), context)) {
    // here we have two decimals, and this is 'higher' in the hierarchy than other
    // so cast other to this' type
    return this->subtract((const Numeric::Ptr )other->castAs(AnyAtomicType::DECIMAL, getTypeURI(), getTypeName(),
                                                             context), context);
  } else {
    // we have two separate branches.  if either is instance of integer, cast it to integer, otherwise, cast to decimal
    // revisit: this is not the prettiest way to do it.  You would want to go up the tree one by one instead of
    // jumping to integer and decimal
    ATDecimalOrDerived::Ptr first; 
    ATDecimalOrDerived::Ptr second; 
    if(this->_isInteger) {
      first = (const ATDecimalOrDerived::Ptr )this->castAs(AnyAtomicType::DECIMAL, SchemaSymbols::fgURI_SCHEMAFORSCHEMA,
                           SchemaSymbols::fgDT_INTEGER, context);
    } else {
      first = (const ATDecimalOrDerived::Ptr )this->castAs(AnyAtomicType::DECIMAL, context);
    }

    if(((ATDecimalOrDerivedImpl*)(const Numeric*)other)->_isInteger) {
      second = (const ATDecimalOrDerived::Ptr )other->castAs(AnyAtomicType::DECIMAL, SchemaSymbols::fgURI_SCHEMAFORSCHEMA,
                            SchemaSymbols::fgDT_INTEGER, context);
    } else {
      second = (const ATDecimalOrDerived::Ptr )other->castAs(AnyAtomicType::DECIMAL, context);
    }
    return first->subtract(second, context);
  }

}

/** Returns a Numeric object which is the product of this and other */
Numeric::Ptr ATDecimalOrDerivedImpl::multiply(const Numeric::Ptr &other, const DynamicContext* context) const {
    if(this->isOfType(other->getTypeURI(), other->getTypeName(), context)) {
    // if both are of the same type exactly, we can perform multiplication
    ATDecimalOrDerivedImpl* otherImpl = (ATDecimalOrDerivedImpl*)(const Numeric*)other;
    // if integer, return xs:integer, otherwise xs:decimal    
    if(_isInteger) {
      return context->getItemFactory()->createInteger(_decimal * otherImpl->_decimal, context);
    }
    return context->getItemFactory()->createDecimal(_decimal * otherImpl->_decimal, context);

  } else if(this->getPrimitiveTypeIndex() != other->getPrimitiveTypeIndex()) {
    // if other is not a decimal, then we need to promote this to a float or double
    return ((const Numeric::Ptr )this->castAs(other->getPrimitiveTypeIndex(), context))->multiply(other, context);
  } else if (this->isInstanceOfType(other->getTypeURI(), other->getTypeName(), context)) {
    // here we know we have two decimals, and this is 'lower' in the hierarchy than other
    // so cast this to other's type
    return ((const Numeric::Ptr )this->castAs(AnyAtomicType::DECIMAL, other->getTypeURI(), other->getTypeName(), context))->multiply(other, context);
  } else if (other->isInstanceOfType(this->getTypeURI(), this->getTypeName(), context)) {
    // here we have two decimals, and this is 'higher' in the hierarchy than other
    // so cast other to this' type
    return this->multiply((const Numeric::Ptr )other->castAs(AnyAtomicType::DECIMAL, this->getTypeURI(), this->getTypeName(), context), context);
  } else {
    // we have two separate branches.  if either is instance of integer, cast it to integer, otherwise, cast to decimal
    // revisit: this is not the prettiest way to do it.  You would want to go up the tree one by one instead of
    // jumping to integer and decimal
    ATDecimalOrDerived::Ptr first;
    ATDecimalOrDerived::Ptr second; 
    if(this->_isInteger) {
      first = (const ATDecimalOrDerived::Ptr )this->castAs(AnyAtomicType::DECIMAL, SchemaSymbols::fgURI_SCHEMAFORSCHEMA,
                           SchemaSymbols::fgDT_INTEGER, context);
    } else {
      first = (const ATDecimalOrDerived::Ptr )this->castAs(AnyAtomicType::DECIMAL, context);
    }
    
    if(((ATDecimalOrDerivedImpl*)(const Numeric*)other)->_isInteger) {
      second = (const ATDecimalOrDerived::Ptr )other->castAs(AnyAtomicType::DECIMAL, SchemaSymbols::fgURI_SCHEMAFORSCHEMA,
                            SchemaSymbols::fgDT_INTEGER, context);
    } else {
      second = (const ATDecimalOrDerived::Ptr )other->castAs(AnyAtomicType::DECIMAL, context);
    }
    return first->multiply(second, context);
  }

}

/** Returns a Numeric object which is the quotient of this and other */
Numeric::Ptr ATDecimalOrDerivedImpl::divide(const Numeric::Ptr &other, const DynamicContext* context) const {
  if(this->isOfType(other->getTypeURI(), other->getTypeName(), context)) {
    // if both are of the same type exactly, we can perform division
    ATDecimalOrDerivedImpl* otherImpl = (ATDecimalOrDerivedImpl*)(const Numeric*)other;
  
    if(otherImpl->_decimal.sign() == 0) {
      XQThrow2(XPath2ErrorException, X("ATDecimalOrDerivedImpl::divide"), X("Division by zero [err:FOAR0001]"));
    }
  
    // return a xs:decimal, regardless of the actual types of the operands
    return context->getItemFactory()->createDecimal(_decimal / otherImpl->_decimal, context);
  } else if(this->getPrimitiveTypeIndex() != other->getPrimitiveTypeIndex()) {
    // if other is not a decimal, then we need to promote this to a float or double
    return ((const Numeric::Ptr )this->castAs(other->getPrimitiveTypeIndex(), context))->divide(other, context);
  } else if (this->isInstanceOfType(other->getTypeURI(), other->getTypeName(), context)) {
    // here we know we have two decimals, and this is 'lower' in the hierarchy than other
    // so cast this to other's type
    return ((const Numeric::Ptr )this->castAs(AnyAtomicType::DECIMAL, other->getTypeURI(), other->getTypeName(), context))->divide(other, context);
  } else if (other->isInstanceOfType(this->getTypeURI(), this->getTypeName(), context)) {
    // here we have two decimals, and this is 'higher' in the hierarchy than other
    // so cast other to this' type
    return this->divide((const Numeric::Ptr )other->castAs(AnyAtomicType::DECIMAL, this->getTypeURI(), this->getTypeName(), context), context);
  } else {
    // we have two separate branches.  if either is instance of integer, cast it to integer, otherwise, cast to decimal
    // revisit: this is not the prettiest way to do it.  You would want to go up the tree one by one instead of
    // jumping to integer and decimal
    ATDecimalOrDerived::Ptr first;
    ATDecimalOrDerived::Ptr second; 
    if(this->_isInteger) {
      first = (const ATDecimalOrDerived::Ptr )this->castAs(AnyAtomicType::DECIMAL, SchemaSymbols::fgURI_SCHEMAFORSCHEMA,
                           SchemaSymbols::fgDT_INTEGER, context);
    } else {
      first = (const ATDecimalOrDerived::Ptr )this->castAs(AnyAtomicType::DECIMAL, context);
    }
    
    if(((ATDecimalOrDerivedImpl*)(const Numeric*)other)->_isInteger) {
      second = (const ATDecimalOrDerived::Ptr )other->castAs(AnyAtomicType::DECIMAL, SchemaSymbols::fgURI_SCHEMAFORSCHEMA,
                            SchemaSymbols::fgDT_INTEGER, context);
    } else {
      second = (const ATDecimalOrDerived::Ptr )other->castAs(AnyAtomicType::DECIMAL, context);
    }
    return first->divide(second, context);
  }

}

/** Returns the arithmetic product of its operands as a Numeric */
Numeric::Ptr ATDecimalOrDerivedImpl::mod(const Numeric::Ptr &other, const DynamicContext* context) const {
  if(this->isOfType(other->getTypeURI(), other->getTypeName(), context)) {
    // if both are of the same type exactly, we can perform the modulo
    const ATDecimalOrDerivedImpl* otherImpl = (ATDecimalOrDerivedImpl*)(const Numeric*)other;
  
    if(otherImpl->isZero()) {
      XQThrow2(::IllegalArgumentException, X("ATDecimalOrDerivedImpl::mod"), X("Division by zero [err:FOAR0001]"));
    }
  
    MAPM result = _decimal;
    MAPM r;
    r = result.integer_divide(otherImpl->_decimal);
    result -= r * otherImpl->_decimal;
    // if integer, return xs:integer, otherwise xs:decimal    
    if(_isInteger) {
      return context->getItemFactory()->createInteger(result, context);
    }
    return context->getItemFactory()->createDecimal(result, context);
  } else if(this->getPrimitiveTypeIndex() != other->getPrimitiveTypeIndex()) {
    // if other is not a decimal, then we need to promote this to a float or double
    return ((const Numeric::Ptr )this->castAs(other->getPrimitiveTypeIndex(), context))->mod(other, context);
  } else if (this->isInstanceOfType(other->getTypeURI(), other->getTypeName(), context)) {
    // here we know we have two decimals, and this is 'lower' in the hierarchy than other
    // so cast this to other's type
    return ((const Numeric::Ptr )this->castAs(AnyAtomicType::DECIMAL, other->getTypeURI(), other->getTypeName(), context))->mod(other, context);
  } else if (other->isInstanceOfType(this->getTypeURI(), this->getTypeName(), context)) {
    // here we have two decimals, and this is 'higher' in the hierarchy than other
    // so cast other to this' type
    return this->mod((const Numeric::Ptr )other->castAs(AnyAtomicType::DECIMAL, this->getTypeURI(), this->getTypeName(), context), context);
  } else {
    // we have two separate branches.  if either is instance of integer, cast it to integer, otherwise, cast to decimal
    // revisit: this is not the prettiest way to do it.  You would want to go up the tree one by one instead of
    // jumping to integer and decimal
    ATDecimalOrDerived::Ptr first;
    ATDecimalOrDerived::Ptr second; 
    if(this->_isInteger) {
      first = (const ATDecimalOrDerived::Ptr )this->castAs(AnyAtomicType::DECIMAL, SchemaSymbols::fgURI_SCHEMAFORSCHEMA,
                           SchemaSymbols::fgDT_INTEGER, context);
    } else {
      first = (const ATDecimalOrDerived::Ptr )this->castAs(AnyAtomicType::DECIMAL, context);
    }
    
    if(((ATDecimalOrDerivedImpl*)(const Numeric*)other)->_isInteger) {
      second = (const ATDecimalOrDerived::Ptr )other->castAs(AnyAtomicType::DECIMAL, SchemaSymbols::fgURI_SCHEMAFORSCHEMA,
                            SchemaSymbols::fgDT_INTEGER, context);
    } else {
      second = (const ATDecimalOrDerived::Ptr )other->castAs(AnyAtomicType::DECIMAL, context);
    }
    return first->mod(second, context);
  }

}

Numeric::Ptr ATDecimalOrDerivedImpl::power(const Numeric::Ptr &other, const DynamicContext* context) const
{
  switch(other->getPrimitiveTypeIndex()) {
  case DECIMAL: {
    ATDecimalOrDerivedImpl *otherImpl = (ATDecimalOrDerivedImpl*)other.get();
    return context->getItemFactory()->createDecimal(_decimal.pow(otherImpl->_decimal), context);
  }
  case FLOAT:
    return context->getItemFactory()->createFloat(_decimal, context)->power(other, context);
  case DOUBLE:
    return context->getItemFactory()->createDouble(_decimal, context)->power(other, context);
  default: assert(false); return 0; // Shouldn't happen
  }
}

/** Returns the floor of this Numeric */
Numeric::Ptr ATDecimalOrDerivedImpl::floor(const DynamicContext* context) const {
  // if integer, return xs:integer, otherwise xs:decimal    
  if(_isInteger) {
    return (const Numeric::Ptr )this->castAs(AnyAtomicType::DECIMAL, SchemaSymbols::fgURI_SCHEMAFORSCHEMA,
                        SchemaSymbols::fgDT_INTEGER, context);
  }
  return context->getItemFactory()->createDecimal(_decimal.floor(), context);

}

/** Returns the ceiling of this Numeric */
Numeric::Ptr ATDecimalOrDerivedImpl::ceiling(const DynamicContext* context) const {
  // if integer, return xs:integer, otherwise xs:decimal    
  if(_isInteger) {
    return (const Numeric::Ptr )this->castAs(AnyAtomicType::DECIMAL, SchemaSymbols::fgURI_SCHEMAFORSCHEMA,
                        SchemaSymbols::fgDT_INTEGER, context);
  }
  return context->getItemFactory()->createDecimal(_decimal.ceil(), context);

}

/** Rounds this Numeric */
Numeric::Ptr ATDecimalOrDerivedImpl::round(const DynamicContext* context) const {
  // if integer, return xs:integer, otherwise xs:decimal    
  if(_isInteger) {
    return (const Numeric::Ptr )this->castAs(AnyAtomicType::DECIMAL, SchemaSymbols::fgURI_SCHEMAFORSCHEMA,
                        SchemaSymbols::fgDT_INTEGER, context);
  }
  MAPM value = _decimal + 0.5;
  return context->getItemFactory()->createDecimal(value.floor(), context);
}

/** Rounds this Numeric to the given precision, and rounds a half to even */
Numeric::Ptr ATDecimalOrDerivedImpl::roundHalfToEven(const Numeric::Ptr &precision, const DynamicContext* context) const {
  ATDecimalOrDerived::Ptr decimal_precision = (const Numeric::Ptr)precision->castAs(this->getPrimitiveTypeIndex(), context);
  MAPM exp = MAPM(10).pow(((ATDecimalOrDerivedImpl*)(const ATDecimalOrDerived*)decimal_precision)->_decimal);
  MAPM value = _decimal * exp;
  bool halfVal = false;

  // check if we're rounding on a half value 
  if((value-0.5) == (value.floor())) {
    halfVal = true;
  }
  value = _decimal * exp + 0.5;
  value = value.floor();

  // if halfVal make sure what we return has the least significant digit even
  if (halfVal) {
    if(value.is_odd()) {
      value = value - 1;
    }
  }
  value = value / exp;  
  // if integer, return xs:integer, otherwise xs:decimal    
  if(_isInteger) {
    return context->getItemFactory()->createInteger(value, context);
  }
  return context->getItemFactory()->createDecimal(value, context);
}

/** Returns the Additive inverse of this Numeric */
Numeric::Ptr ATDecimalOrDerivedImpl::invert(const DynamicContext* context) const {
  if(_isInteger) {
    return context->getItemFactory()->createInteger(_decimal.neg(), context);
  }
  return context->getItemFactory()->createDecimal(_decimal.neg(), context);
}

/** Returns the absolute value of this Numeric */
Numeric::Ptr ATDecimalOrDerivedImpl::abs(const DynamicContext* context) const {
  return context->getItemFactory()->createDecimal(_decimal.abs(), context);
}

/** Returns the square root of this Numeric */
Numeric::Ptr ATDecimalOrDerivedImpl::sqrt(const DynamicContext* context) const {
  if (_decimal < 0) return context->getItemFactory()->createDecimal(MAPM(0), context);
  return context->getItemFactory()->createDecimal(_decimal.sqrt(), context);
}

/** Returns the sinus of this Numeric */
Numeric::Ptr ATDecimalOrDerivedImpl::sin(const DynamicContext* context) const {
  return context->getItemFactory()->createDecimal(_decimal.sin(), context);
}

/** Returns the cosinus of this Numeric */
Numeric::Ptr ATDecimalOrDerivedImpl::cos(const DynamicContext* context) const {
  return context->getItemFactory()->createDecimal(_decimal.cos(), context);
}

Numeric::Ptr ATDecimalOrDerivedImpl::tan(const DynamicContext* context) const {
  return context->getItemFactory()->createDecimal(_decimal.tan(), context);
}

Numeric::Ptr ATDecimalOrDerivedImpl::asin(const DynamicContext* context) const {
  if (_decimal.abs() > 1) return context->getItemFactory()->createDecimal(MAPM(0), context);
  return context->getItemFactory()->createDecimal(_decimal.asin(), context);
}

Numeric::Ptr ATDecimalOrDerivedImpl::acos(const DynamicContext* context) const {
  if (_decimal.abs() > 1) return context->getItemFactory()->createDecimal(MAPM(0), context);
  return context->getItemFactory()->createDecimal(_decimal.acos(), context);
}

Numeric::Ptr ATDecimalOrDerivedImpl::atan(const DynamicContext* context) const {
  return context->getItemFactory()->createDecimal(_decimal.atan(), context);
}

Numeric::Ptr ATDecimalOrDerivedImpl::log(const DynamicContext* context) const {
  if (_decimal <= 0) return context->getItemFactory()->createDecimal(MAPM(0), context);
  return context->getItemFactory()->createDecimal(_decimal.log(), context);
}

Numeric::Ptr ATDecimalOrDerivedImpl::exp(const DynamicContext* context) const {
  return context->getItemFactory()->createDecimal(_decimal.exp(), context);
}

/** Does this Numeric have value 0? */
bool ATDecimalOrDerivedImpl::isZero() const {
  return _decimal.sign() == 0;
}

/** Is this Numeric negative? */
bool ATDecimalOrDerivedImpl::isNegative() const {
  return _decimal.sign() < 0;
}

/** Is this Numeric positive? */
bool ATDecimalOrDerivedImpl::isPositive() const {
  return _decimal.sign() > 0;
}

/** Treat this decimal (must be integer) as a codepoint **/
XMLInt32 ATDecimalOrDerivedImpl::treatAsCodepoint(const DynamicContext* context) const {
  if(_isInteger) {
    int integer = (int)_decimal.toDouble();
    XMLInt32 ch = (XMLInt32)integer;
    if(integer<=0 || (int)ch != integer) { // negative or lost some info
      XQThrow2(XPath2ErrorException, X("ATDecimalOrDerivedImpl::treatAsCodepoint"), X("Codepoint not legal [err:FOCH0001]."));
    }
    return ch;
  } else {
    XQThrow2(XPath2ErrorException, X("ATDecimalOrDerivedImpl::treatAsCodepoint"), X("Only integers can be treated as codepoints."));
  }
}

AnyAtomicType::AtomicObjectType ATDecimalOrDerivedImpl::getPrimitiveTypeIndex() const {
  return this->getTypeIndex();
}

void ATDecimalOrDerivedImpl::setDecimal(const XMLCh* const value)
{
  _decimal = parseDecimal(value);
}

MAPM ATDecimalOrDerivedImpl::parseDecimal(const XMLCh* const value)
{
  if(value == NULL) {
    XQThrow2(XPath2TypeCastException,X("ATDecimalOrDerivedImpl::setDecimal"), X("Invalid representation of decimal [err:FORG0001]"));
  }
  
  unsigned int length = XPath2Utils::uintStrlen(value) + 1;

  AutoDeleteArray<char> buffer(new char[length]);

  bool gotPoint = false;
  bool gotSign = false;
  bool gotDigit = false;
  bool stop = false;
  bool munchWS = true;

  const XMLCh *src = value;
  char *dest = buffer;
  XMLCh tmpChar;
  while(!stop && *src != 0) {
    tmpChar = *src++;
    
    switch(tmpChar) {/*{{{*/

    case L'+': {
      *dest++ = '+';
      if(gotSign) {
        stop = true;
      } else {
        gotSign = true;
      }
      break;
    }
             
    case L'-': {
      *dest++ = '-';
      if(gotSign) {
        stop = true;
      } else {
        gotSign = true;
      }
      break;
    }
             
      //This is '.'
    case 0x002e: {
      *dest++ = '.';
      if(gotPoint) {
        stop = true;
      } else {
        gotPoint = true;
      }
      break;
    }
             
      /* All the numerals defined by XML standard */
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
      gotDigit = true;
      *dest++ = (char)(tmpChar - 0x0030) + '0';
      break;
    }
             
      // whitespace at start or end of string...
    case 0x0020:
    case 0x0009:
    case 0x000d:
    case 0x000a: {
      bool endOfWS = false;
      while(!endOfWS && *src != 0) {
        tmpChar = *src++;
        switch(tmpChar) {
        case 0x0020:
        case 0x0009:
        case 0x000d:
        case 0x000a: {
          break;
        }
        default: {
          endOfWS = true;
          --src;
          if(munchWS) {
            //end of leading whitespace
            munchWS = false;
          } else {
            //trailing whitespace is followed by other characters - so return NaN.
            stop = true;
          }
        }
        }//switch
      }//while
      break;
    }
             
    default:
      stop = true;
      break;
             
    }//switch
    /*}}}*/
    
  }//while

  if(!gotDigit || stop) {
    XQThrow2(XPath2TypeCastException,X("ATDecimalOrDerivedImpl::setDecimal"), X("Invalid representation of decimal [err:FORG0001]"));
  }

  *dest++ = 0; // Null terminate  
  return (char*)buffer;
}

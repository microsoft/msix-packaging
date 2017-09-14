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
#include <xqilla/items/impl/ATDoubleOrDerivedImpl.hpp>
#include <xqilla/items/ATDecimalOrDerived.hpp>
#include <xqilla/items/ATBooleanOrDerived.hpp>
#include <xqilla/context/DynamicContext.hpp>
#include <xqilla/exceptions/IllegalArgumentException.hpp>
#include <xercesc/util/XMLUniDefs.hpp>
#include <xercesc/validators/schema/SchemaSymbols.hpp>
#include <xqilla/framework/XPath2MemoryManager.hpp>
#include <xqilla/utils/XPath2Utils.hpp>
#include <xqilla/context/ItemFactory.hpp>

#include <assert.h>

#if defined(XERCES_HAS_CPP_NAMESPACE)
XERCES_CPP_NAMESPACE_USE
#endif

int ATDoubleOrDerivedImpl::g_nSignificantDigits=25;

ATDoubleOrDerivedImpl::
ATDoubleOrDerivedImpl(const XMLCh* typeURI, const XMLCh* typeName, const XMLCh* value, const StaticContext* context)
  : _typeName(typeName),
    _typeURI(typeURI)
{
  setDouble(value);
  checkDoubleLimits(_state, _double);
}

ATDoubleOrDerivedImpl::
ATDoubleOrDerivedImpl(const XMLCh* typeURI, const XMLCh* typeName, const MAPM value, const StaticContext* context): 
    ATDoubleOrDerived(),
    _typeName(typeName),
    _typeURI(typeURI) { 
    
  _double = value;
  _state = NUM;
  if(value.sign() < 0) 
    _state = NEG_NUM;
  checkDoubleLimits(_state, _double);
}

void *ATDoubleOrDerivedImpl::getInterface(const XMLCh *name) const
{
  if(name == Item::gXQilla) {
    return (void*)this;
  }
  return 0;
}

/* Get the name of the primitive type (basic type) of this type
 * (ie "decimal" for xs:decimal) */
const XMLCh* ATDoubleOrDerivedImpl::getPrimitiveTypeName() const {
  return this->getPrimitiveName();
}

const XMLCh* ATDoubleOrDerivedImpl::getPrimitiveName()  {
  return SchemaSymbols::fgDT_DOUBLE;
}

/* Get the name of this type  (ie "integer" for xs:integer) */
const XMLCh* ATDoubleOrDerivedImpl::getTypeName() const {
  return _typeName;
}

/* Get the namespace URI for this type */
const XMLCh* ATDoubleOrDerivedImpl::getTypeURI() const {
  return _typeURI; 
}

AnyAtomicType::AtomicObjectType ATDoubleOrDerivedImpl::getTypeIndex() {
  return AnyAtomicType::DOUBLE;
} 

/* returns the XMLCh* (canonical) representation of this type */
const XMLCh* ATDoubleOrDerivedImpl::asString(const DynamicContext* context) const
{
  return asDoubleString(g_nSignificantDigits, context);
}

/* Promote this to the given type, if possible */
Numeric::Ptr ATDoubleOrDerivedImpl::promoteTypeIfApplicable(AnyAtomicType::AtomicObjectType typeIndex,
                                                            const DynamicContext* context) const
{
  switch(typeIndex) {
  case AnyAtomicType::DOUBLE:
    return this;
  default:
    break;
  }
  return 0;
}

/** Returns a Numeric object which is the sum of this and other */
Numeric::Ptr ATDoubleOrDerivedImpl::add(const Numeric::Ptr &other, const DynamicContext* context) const {
  if(AnyAtomicType::DOUBLE == other->getPrimitiveTypeIndex()) {
    // if both are of the same type exactly, we can perform addition
    ATDoubleOrDerivedImpl* otherImpl = (ATDoubleOrDerivedImpl*)(const Numeric*)other;
    if(otherImpl->_state == NaN) return notANumber(context);

    switch (_state) {
      case NaN: return notANumber(context);
      case INF: {
        switch(otherImpl->_state) {
          case NaN: return notANumber(context);       // case taken care of above
          case NEG_NUM:
          case NUM: return infinity(context);         // INF + NUM = INF
          case INF: return infinity(context);         // INF + INF = INF
          case NEG_INF: return notANumber(context);   // INF + (-INF) = NaN
          default: assert(false); return 0; // should never get here
        }
      }
      case NEG_INF: {
        switch(otherImpl->_state) {
          case NaN: return notANumber(context);          //case taken care of above
          case NEG_NUM:
          case NUM: return negInfinity(context);         // -INF + NUM = -INF
          case INF: return notANumber(context);          // -INF + INF = NaN
          case NEG_INF: return negInfinity(context);     // -INF + (-INF) = -INF
          default: assert(false); return 0; // should never get here
        }
      }                
      case NEG_NUM:
      case NUM: {
        switch(otherImpl->_state) {
          case NaN: return notANumber(context); // case taken care of above
          case INF: return infinity(context);
          case NEG_INF: return negInfinity(context);
          case NEG_NUM:
          case NUM: 
              {
                // Handle positive and negative zero
                if(_double.sign()==0 && otherImpl->_double!=0)
                  return other;
                else if(_double.sign()!=0 && otherImpl->_double==0)
                  return this;
                else if(_double.sign()==0 && otherImpl->_double==0)
                {
                  if(_state==otherImpl->_state)
                    // sum of two zero of the same sign -> result is equal to any of the two items
                    return this;
                  else
                    // sum of two zero of different sign -> result is equal to +0
                    return newDouble(0, context);
                }
                return newDouble(_double + otherImpl->_double, context);
              }
          default: assert(false); return 0; // should never get here 
        }
      }
      default: assert(false); return 0; // should never get here 
    } 
    
  } else {
    // if other is not a double, then we need to promote it to a double
    return this->add((const Numeric::Ptr )other->castAs(AnyAtomicType::DOUBLE, context), context);
  } 
}

/** Returns a Numeric object which is the difference of this and
   * other */
Numeric::Ptr ATDoubleOrDerivedImpl::subtract(const Numeric::Ptr &other, const DynamicContext* context) const {
  if(AnyAtomicType::DOUBLE == other->getPrimitiveTypeIndex()) {
    // if both are of the same type exactly, we can perform subtraction
    ATDoubleOrDerivedImpl* otherImpl = (ATDoubleOrDerivedImpl*)(const Numeric*)other;
    if(otherImpl->_state == NaN) return notANumber(context);

    switch (_state) {
      case NaN: return notANumber(context);
      case INF: {
        switch(otherImpl->_state) {
          case NaN: return notANumber(context);   // case taken care of above
          case NEG_NUM:
          case NUM: return infinity(context);     // INF - NUM = INF
          case INF: return notANumber(context);   // INF - INF = NaN
          case NEG_INF: return infinity(context); // INF - (-INF) = INF
          default: assert(false); return 0; // should never get here
        }
      }
      case NEG_INF: {
        switch(otherImpl->_state) {
          case NaN: return notANumber(context);          //case taken care of above
          case NEG_NUM:
          case NUM: return negInfinity(context);         // -INF - NUM = -INF
          case INF: return negInfinity(context);         // -INF - INF = -INF
          case NEG_INF: return notANumber(context);      // -INF - (-INF) = NaN
          default: assert(false); return 0; // should never get here
        }
      }                
      case NEG_NUM:
      case NUM: {
        switch(otherImpl->_state) {
          case NaN: return notANumber(context);          // case taken care of above
          case INF: return negInfinity(context);         // NUM - INF = -INF
          case NEG_INF: return infinity(context);        // NUM - (-INF) = INF
          case NEG_NUM:
          case NUM: return newDouble(_double - otherImpl->_double, context);
          default: assert(false); return 0;  // should never get here
        }
      }
      default: assert(false); return 0;  // should never get here
    } 
    
  } else {
    // if other is not a double, then we need to promote it to a double
    return this->subtract((const Numeric::Ptr )other->castAs(AnyAtomicType::DOUBLE, context), context);
  } 
}

/** Returns a Numeric object which is the product of this and other */
Numeric::Ptr ATDoubleOrDerivedImpl::multiply(const Numeric::Ptr &other, const DynamicContext* context) const {
  if(AnyAtomicType::DOUBLE == other->getPrimitiveTypeIndex()) {
    // if both are of the same type, we can perform multiplication
    ATDoubleOrDerivedImpl* otherImpl = (ATDoubleOrDerivedImpl*)(const Numeric*)other;
    if(otherImpl->_state == NaN) return notANumber(context);

    switch (_state) {
      case NaN: return notANumber(context);
      case INF: {
        switch(otherImpl->_state) {
          case NaN: return notANumber(context);      // case taken care of above
          case NEG_NUM:
          case NUM: return other->isPositive() ? infinity(context) : negInfinity(context);        // INF * NUM = +/-INF
          case INF: return infinity(context);        // INF * INF = INF
          case NEG_INF: return negInfinity(context); // INF * (-INF) = -INF
          default: assert(false); return 0; // should never get here
        }
      }
      case NEG_INF: {
        switch(otherImpl->_state) {
          case NaN: return notANumber(context);          //case taken care of above
          case NEG_NUM:
          case NUM: return other->isPositive() ? negInfinity(context) : infinity(context);         // -INF * NUM = +/-INF
          case INF: return negInfinity(context);         // -INF * INF = -INF
          case NEG_INF: return infinity(context);        // -INF * (-INF) = INF
          default: assert(false); return 0; // should never get here
        }
      }                
      case NEG_NUM:
      case NUM: {
        switch(otherImpl->_state) {
          case NaN: return notANumber(context);          // case taken care of above
          case INF: return this->isPositive() ? infinity(context) : negInfinity(context);            // NUM * INF = +/-INF
          case NEG_INF: return this->isPositive() ? negInfinity(context) : infinity(context);        // NUM * (-INF) = +/-INF
          case NEG_NUM:
          case NUM: 
            if(other->isZero() || this->isZero()) {
              if((this->isNegative() && other->isPositive()) ||
                 (this->isPositive() && other->isNegative())) {
                return negZero(context);                // 0 / (-NUM) or (-0) / NUM = -0
              } else {
                return newDouble(0, context);           // 0 / NUM or (-0) / (-NUM) = 0
              }
            }
            return newDouble(_double * otherImpl->_double, context);
          default: assert(false); return 0;  // should never get here
        }
      }
      default: assert(false); return 0;  // should never get here
    } 
    
  } else {
    // if other is not a double, then we need to promote it to a double
    return this->multiply((const Numeric::Ptr )other->castAs(AnyAtomicType::DOUBLE, context), context);
  } 
}

/** Returns a Numeric object which is the quotient of this and other */
Numeric::Ptr ATDoubleOrDerivedImpl::divide(const Numeric::Ptr &other, const DynamicContext* context) const {
  if(AnyAtomicType::DOUBLE == other->getPrimitiveTypeIndex()) {
    // if both are of the same type, we can perform division
    ATDoubleOrDerivedImpl* otherImpl = (ATDoubleOrDerivedImpl*)(const Numeric*)other;
    if(otherImpl->_state == NaN) return notANumber(context);
        switch (_state) {
      case NaN: return notANumber(context);
      case INF: {
        switch(otherImpl->_state) {
          case NaN: return notANumber(context);      // case taken care of above
          case NEG_NUM:
          case NUM: return other->isPositive() ? infinity(context) : negInfinity(context);        // INF / NUM = +/-INF
          case INF: return notANumber(context);      // INF / INF = NaN
          case NEG_INF: return notANumber(context);  // INF / (-INF) = NaN
          default: assert(false); return 0; // should never get here
        } // switch
      }// case
      case NEG_INF: {
        switch(otherImpl->_state) {
          case NaN: return notANumber(context);          //case taken care of above
          case NEG_NUM:
          case NUM: return other->isPositive() ? negInfinity(context) : infinity(context);         // -INF / NUM = -INF
          case INF: return notANumber(context);          // -INF / INF = NaN
          case NEG_INF: return notANumber(context);      // -INF / (-INF) = NaN
          default: assert(false); return 0; // should never get here
        } // switch
      } // case    
      case NEG_NUM:
      case NUM: {
        switch(otherImpl->_state) {
          case NaN: return notANumber(context);          // case taken care of above
          case INF: {                                    // NUM / INF = +/-0
            if(this->isNegative()) {
              return negZero(context);
            } else {
              return newDouble(0, context);
            }
          }// case
          case NEG_INF: {                               // NUM / -INF = +/-0
            if(this->isPositive()) {
              return negZero(context);
            } else {
              return newDouble(0, context);
            }
          }// case
          case NEG_NUM:
          case NUM: {
            if(other->isZero()) {
              if(this->isZero()) return notANumber(context);
              if((this->isNegative() && other->isPositive()) ||
                 (this->isPositive() && other->isNegative())) {
                return negInfinity(context);            // NUM / (-0) or (-NUM) / 0 = -INF
              } else {
                return infinity(context);               // NUM / 0 or (-NUM) / (-0) = INF
              }
            }
            else if(this->isZero())
            {
              if((this->isNegative() && other->isPositive()) ||
                 (this->isPositive() && other->isNegative())) {
                return negZero(context);                // 0 / (-NUM) or (-0) / NUM = -0
              } else {
                return newDouble(0, context);           // 0 / NUM or (-0) / (-NUM) = 0
              }
            }
            return newDouble(_double / otherImpl->_double, context);
          }// case
          default: assert(false); return 0;  // should never get here
        }// switch
      }// case
      default: assert(false); return 0;  // should never get here
    }// switch

  } else {
    // if other is not a double, then we need to promote it to a double
    return this->divide((const Numeric::Ptr )other->castAs(AnyAtomicType::DOUBLE, context), context);
  } 
}

/** Returns the mod of its operands as a Numeric */
Numeric::Ptr ATDoubleOrDerivedImpl::mod(const Numeric::Ptr &other, const DynamicContext* context) const {
  if(AnyAtomicType::DOUBLE == other->getPrimitiveTypeIndex()) {
    // if both are of the same type, we can perform mod
    ATDoubleOrDerivedImpl* otherImpl = (ATDoubleOrDerivedImpl*)(const Numeric*)other;
    if(this->isNaN() || otherImpl->isNaN() || this->isInfinite() || otherImpl->isZero()) {
      return notANumber(context);
    
    } else if(otherImpl->isInfinite() || this->isZero()) {
      return (const Numeric::Ptr )this->castAs(AnyAtomicType::DOUBLE, context);
    
    } else {
      MAPM result = _double;
      MAPM r;
      r = result.integer_divide(otherImpl->_double);
      result -= r * otherImpl->_double;
      if (result == 0 && isNegative())
        return negZero(context);
      return newDouble(result, context);
    }
  } else {
    // if other is not a double, then we need to promote it to a double
    return this->mod((const Numeric::Ptr )other->castAs(AnyAtomicType::DOUBLE, context), context);
  } 
}

Numeric::Ptr ATDoubleOrDerivedImpl::power(const Numeric::Ptr &other, const DynamicContext* context) const
{
  switch(other->getPrimitiveTypeIndex()) {
  case DECIMAL:
  case FLOAT:
    return power((Numeric*)other->castAs(DOUBLE, context).get(), context);
  case DOUBLE: {
    ATDoubleOrDerivedImpl *otherImpl = (ATDoubleOrDerivedImpl*)other.get();

    switch(_state) {
    case NaN: return this;
    case INF: {
      switch(otherImpl->_state) {
      case NaN: return other;
      case NEG_NUM:
      case NUM:
      case INF:
      case NEG_INF: return this;
      default: assert(false); return 0; // should never get here
      }
    }
    case NEG_INF: {
      switch(otherImpl->_state) {
      case NaN: return other;
      case NEG_NUM:
      case NUM:
      case INF:
      case NEG_INF: return this;
      default: assert(false); return 0; // should never get here
      }
    }                
    case NEG_NUM:
    case NUM: {
      switch(otherImpl->_state) {
      case NaN: return other;
      case INF: return other;
      case NEG_INF: return infinity(context);
      case NEG_NUM:
      case NUM: 
        return newDouble(_double.pow(otherImpl->_double), context);
      default: assert(false); return 0;  // should never get here
      }
    }
    default: assert(false); return 0;  // should never get here
    } 
  }
  default: assert(false); return 0; // Shouldn't happen
  }
}

/** Returns the floor of this Numeric */
Numeric::Ptr ATDoubleOrDerivedImpl::floor(const DynamicContext* context) const {
  switch (_state) {
    case NaN: return notANumber(context);
    case INF: return infinity(context);
    case NEG_INF: return negInfinity(context);
    case NEG_NUM:
    case NUM: { 
      if (isZero() && isNegative())
        return this;
      return newDouble(_double.floor(), context); 
    }      
    default: { assert(false); return 0; // should never get here 
    }
  }
}

/** Returns the ceiling of this Numeric */
Numeric::Ptr ATDoubleOrDerivedImpl::ceiling(const DynamicContext* context) const {
  switch (_state) {
    case NaN: return notANumber(context);
    case INF: return infinity(context);
    case NEG_INF: return negInfinity(context);
    case NEG_NUM:
    case NUM: { 
      if (isNegative() && _double >= -0.5) {
        return negZero(context);
      }
      return newDouble(_double.ceil(), context); 
    }
    default: { assert(false); return 0; // should never get here 
    }
  }
}

/** Rounds this Numeric */
Numeric::Ptr ATDoubleOrDerivedImpl::round(const DynamicContext* context) const {
  switch (_state) {
    case NaN: return notANumber(context);
    case INF: return infinity(context);
    case NEG_INF: return negInfinity(context);
    case NEG_NUM:
    case NUM: { 
      if (isNegative() &&_double >= -0.5) {
        return negZero(context);
      }      
      MAPM value = _double + 0.5; 
      return newDouble(value.floor(), context); 
    }
  default: {assert(false); return 0; // should never get here
    }
  }
}

/** Rounds this Numeric to the given precision, and rounds a half to even */
Numeric::Ptr ATDoubleOrDerivedImpl::roundHalfToEven(const Numeric::Ptr &precision, const DynamicContext* context) const {
  switch (_state) {
    case NaN: return notANumber(context);
    case INF: return infinity(context);
    case NEG_INF: return negInfinity(context);
    case NEG_NUM:
    case NUM: break;
    default: { assert(false); return 0;  // should never get here
    }
  }

  if (isZero() && isNegative())
    return this;
  
  ATDoubleOrDerived::Ptr double_precision = (const Numeric::Ptr)precision->castAs(AnyAtomicType::DOUBLE, context);
  MAPM exp = MAPM(10).pow(((ATDoubleOrDerivedImpl*)(const ATDoubleOrDerived*)double_precision)->_double);
  MAPM value = _double * exp;
  bool halfVal = false;

  // check if we're rounding on a half value 
  if((value-0.5) == (value.floor())) {
    halfVal = true;
  }
  value = _double * exp + 0.5;
  value = value.floor();

  // if halfVal make sure what we return has the least significant digit even
  if (halfVal) {
    if(value.is_odd()) {
      value = value - 1;
    }
  }
  value = value / exp;

  // the spec doesn't actually say to do this, but djf believes this is the correct way to handle rounding of -ve values which will result in 0.0E0
  // if (value == 0 && isNegative())
    // return negZero(context);
  return newDouble(value, context);
}

/** Returns the Additive inverse of this Numeric */
Numeric::Ptr ATDoubleOrDerivedImpl::invert(const DynamicContext* context) const {
  switch (_state) {
    case NaN: return this;
    case INF: return negInfinity(context);
    case NEG_INF: return infinity(context);
    case NEG_NUM:
    case NUM: 
        if(this->isZero())
        {
          if(this->isNegative())
            return newDouble(0, context);
          else
            return negZero(context);
        }
        return newDouble(_double.neg(), context);
    default: assert(false); return 0;  // should never get here
  }
}

/** Returns the absolute value of this Numeric */
Numeric::Ptr ATDoubleOrDerivedImpl::abs(const DynamicContext* context) const {
  switch (_state) {
    case NaN: return this;
    case INF: return infinity(context);
    case NEG_INF: return infinity(context);
    case NEG_NUM:
    case NUM: return newDouble(_double.abs(), context);
    default: assert(false); return 0;  // should never get here
  }
}

/** Returns the square root of this Numeric */
Numeric::Ptr ATDoubleOrDerivedImpl::sqrt(const DynamicContext* context) const {
  switch (_state) {
    case NaN: return this;
    case INF: return infinity(context);
    case NEG_INF: return notANumber(context);
    case NEG_NUM: return notANumber(context);
    case NUM: return newDouble(_double.sqrt(), context);
    default: assert(false); return 0;  // should never get here
  }
}

Numeric::Ptr ATDoubleOrDerivedImpl::sin(const DynamicContext* context) const {
  switch (_state) {
    case NaN: return this;
    case INF: return notANumber(context);
    case NEG_INF: return notANumber(context);
    case NEG_NUM:
    case NUM: return newDouble(_double.sin(), context);
    default: assert(false); return 0;  // should never get here
  }
}

Numeric::Ptr ATDoubleOrDerivedImpl::cos(const DynamicContext* context) const {
  switch (_state) {
    case NaN: return this;
    case INF: return notANumber(context);
    case NEG_INF: return notANumber(context);;
    case NEG_NUM:
    case NUM: return newDouble(_double.cos(), context);
    default: assert(false); return 0;  // should never get here
  }
}

Numeric::Ptr ATDoubleOrDerivedImpl::tan(const DynamicContext* context) const {
  switch (_state) {
    case NaN: return this;
    case INF: return notANumber(context);
    case NEG_INF: return notANumber(context);
    case NEG_NUM:
    case NUM: return newDouble(_double.tan(), context);
    default: assert(false); return 0;  // should never get here
  }
}

Numeric::Ptr ATDoubleOrDerivedImpl::asin(const DynamicContext* context) const {
  switch (_state) {
    case NaN: return this;
    case INF: return notANumber(context);
    case NEG_INF: return notANumber(context);
    case NEG_NUM:
    case NUM: 
               if(_double.abs() > 1)return notANumber(context);
               return newDouble(_double.acos(), context);
    default: assert(false); return 0;  // should never get here
  }
}

Numeric::Ptr ATDoubleOrDerivedImpl::acos(const DynamicContext* context) const {
  switch (_state) {
    case NaN: return this;
    case INF: return notANumber(context);
    case NEG_INF: return notANumber(context);
    case NEG_NUM:
    case NUM: 
               if(_double.abs() > 1)return notANumber(context);
               return newDouble(_double.acos(), context);
    default: assert(false); return 0;  // should never get here
  }
}

Numeric::Ptr ATDoubleOrDerivedImpl::atan(const DynamicContext* context) const {
  switch (_state) {
    case NaN: return this;
    case INF:     return newDouble((MM_HALF_PI), context);
    case NEG_INF: return newDouble(MAPM(MM_HALF_PI).neg() , context);
    case NEG_NUM: 
    case NUM: return newDouble(_double.atan(), context);
    default: assert(false); return 0;  // should never get here
  }
}

Numeric::Ptr ATDoubleOrDerivedImpl::log(const DynamicContext* context) const {
  switch (_state) {
    case NaN: return this;
    case INF: return infinity(context);
    case NEG_INF: return notANumber(context);
    case NEG_NUM: return notANumber(context);
    case NUM: if(_double == 0) return notANumber(context);
                  return newDouble(_double.log(), context);
    default: assert(false); return 0;  // should never get here
  }
}

Numeric::Ptr ATDoubleOrDerivedImpl::exp(const DynamicContext* context) const {
  switch (_state) {
    case NaN: return this;
    case INF: return infinity(context);
    case NEG_INF: return newDouble(0, context);
    case NEG_NUM:
    case NUM: return newDouble(_double.exp(), context);
    default: assert(false); return 0;  // should never get here
  }
}


/** Does this Numeric have value 0? */
bool ATDoubleOrDerivedImpl::isZero() const {
  switch (_state) {
    case NaN: 
    case INF: 
    case NEG_INF: return false;
    default: /*NUM*/{
      return _double.sign() == 0;
    }
  }
}

/** Is this Numeric negative? */
bool ATDoubleOrDerivedImpl::isNegative() const {
  switch (_state) {
  case NaN:
  case INF:
  case NUM: return false;
  case NEG_NUM:
  case NEG_INF: return true;
  }
  assert(false);
  return false; // should never get here
}

/** Is this Numeric positive? */
bool ATDoubleOrDerivedImpl::isPositive() const {
  switch (_state) {
  case INF:
  case NUM: return true;
  case NaN:
  case NEG_INF:
  case NEG_NUM: return false;
  }
  assert(false);
  return false; // should never get here
}

/* Is this xs:double not a number */
bool ATDoubleOrDerivedImpl::isNaN() const {
  return _state == NaN;
}


/* Is this xs:double infinite? */
bool ATDoubleOrDerivedImpl::isInfinite() const {
  return _state == INF || _state == NEG_INF;
}


AnyAtomicType::AtomicObjectType ATDoubleOrDerivedImpl::getPrimitiveTypeIndex() const {
  return this->getTypeIndex();
}

/* returns a new infinity xs:double*/
ATDoubleOrDerived::Ptr ATDoubleOrDerivedImpl::infinity(const DynamicContext* context) const {
  return context->getItemFactory()->createDouble(Numeric::INF_string, context);
}

/* returns a new negative infinity xs:double*/
ATDoubleOrDerived::Ptr ATDoubleOrDerivedImpl::negInfinity(const DynamicContext* context) const {
  return context->getItemFactory()->createDouble(Numeric::NegINF_string, context);
}
  
/* returns a NaN xs:double*/
ATDoubleOrDerived::Ptr ATDoubleOrDerivedImpl::notANumber(const DynamicContext* context) const {
  return context->getItemFactory()->createDouble(Numeric::NaN_string, context);
}

/* returns a -0 ATDoubleOrDerived*/
ATDoubleOrDerived::Ptr ATDoubleOrDerivedImpl::negZero(const DynamicContext* context) const {
  return context->getItemFactory()->createDouble(Numeric::NegZero_string, context);
}

/*returns a ATDoubleOrDerived of value value*/
ATDoubleOrDerived::Ptr ATDoubleOrDerivedImpl::newDouble(MAPM value, const DynamicContext* context) const {
  return context->getItemFactory()->createDouble(value, context);
}

static MAPM parse(const XMLCh* const value, Numeric::State &state)
{
  if(value == NULL) {
    // Not a Number
    state = Numeric::NaN;
    return 0;
  }
  
  unsigned int length = XPath2Utils::uintStrlen(value) + 1;

  AutoDeleteArray<char> buffer(new char[length]);

  bool gotBase = false;
  bool gotPoint = false;
  bool gotSign = false;
  bool gotDigit = false;
  bool stop = false;
  bool munchWS = true;
  bool isNegative = false;

  const XMLCh *src = value;
  char *dest = buffer;
  XMLCh tmpChar;
  while(!stop && *src != 0) {
    tmpChar = *src++;
    
    switch(tmpChar) {

    case L'+': {
      *dest++ = '+';
      if(gotSign || gotDigit) {
        stop = true;
      } else {
        gotSign = true;
      }
      break;
    }
             
    case L'-': {
      *dest++ = '-';
      if(gotSign || gotDigit) {
        stop = true;
      } else {
        gotSign = true;
        if(!gotBase) isNegative = true;
      }
      break;
    }
             
    case 0x0045:
    case 0x0065: {
      *dest++ = 'e';
      if(!gotDigit || gotBase) {
        stop = true;
      } else {
        gotPoint = false;
        gotSign = false;        
        gotBase = true;
        gotDigit = false;
      }
      break;
    }
             
      //This is '.'
    case 0x002e: {
      *dest++ = '.';
      if(gotPoint || gotBase) {
        stop = true;
      } else {
        gotPoint = true;
      }
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
        }
      }
      break;
    }
             
    default:
      stop = true;
      break;
             
    }
  }

  if(!gotDigit || stop) {
    if(XPath2Utils::equals(value, Numeric::NegINF_string)) {
      state = Numeric::NEG_INF;
    }
    else if (XPath2Utils::equals(value, Numeric::INF_string)) {
      state = Numeric::INF;
    }
    else {
      state = Numeric::NaN;
    }
    return 0;
  }

  *dest++ = 0; // Null terminate  
  if(isNegative) state = Numeric::NEG_NUM;
  else state = Numeric::NUM;

  return (char*)buffer;
}

void ATDoubleOrDerivedImpl::setDouble(const XMLCh* const value)
{
  _double = parse(value, _state);
}

MAPM ATDoubleOrDerivedImpl::parseDouble(const XMLCh* const value, State &state)
{
  MAPM result = parse(value, state);
  Numeric::checkDoubleLimits(state, result);
  return result;
}

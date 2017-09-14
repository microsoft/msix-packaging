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
#include <xqilla/items/impl/ATFloatOrDerivedImpl.hpp>
#include <xqilla/items/ATDecimalOrDerived.hpp>
#include <xqilla/items/ATBooleanOrDerived.hpp>
#include <xqilla/context/DynamicContext.hpp>
#include <xqilla/exceptions/IllegalArgumentException.hpp>
#include <xercesc/util/XMLUniDefs.hpp>
#include <xercesc/util/XMLString.hpp>
#include <xercesc/validators/schema/SchemaSymbols.hpp>
#include <xqilla/framework/XPath2MemoryManager.hpp>
#include <xqilla/utils/XPath2Utils.hpp>
#include <xqilla/context/ItemFactory.hpp>
#include <assert.h>

#if defined(XERCES_HAS_CPP_NAMESPACE)
XERCES_CPP_NAMESPACE_USE
#endif

int ATFloatOrDerivedImpl::g_nSignificantDigits=25;

ATFloatOrDerivedImpl::
ATFloatOrDerivedImpl(const XMLCh* typeURI, const XMLCh* typeName, const XMLCh* value, const StaticContext* context): 
    ATFloatOrDerived(),
    _typeName(typeName),
    _typeURI(typeURI) { 
    
  setFloat(value);
  // if state is NaN, it could be because it should be INF or -INF
  if(_state == NaN) {
    if(XPath2Utils::equals(value, Numeric::NegINF_string)) {
      _state = NEG_INF;
    } else if (XPath2Utils::equals(value, Numeric::INF_string)) {
      _state = INF;
    }
  }
  checkFloatLimits(_state, _float);
}

ATFloatOrDerivedImpl::
ATFloatOrDerivedImpl(const XMLCh* typeURI, const XMLCh* typeName, const MAPM value, const StaticContext* context): 
    ATFloatOrDerived(),
    _typeName(typeName),
    _typeURI(typeURI) { 
    
  _float = value;
  _state = NUM;
  if (value.sign() < 0) 
    _state = NEG_NUM;
  checkFloatLimits(_state, _float);
}

void *ATFloatOrDerivedImpl::getInterface(const XMLCh *name) const
{
  if(name == Item::gXQilla) {
    return (void*)this;
  }
  return 0;
}

/* Get the name of the primitive type (basic type) of this type
 * (ie "decimal" for xs:decimal) */
const XMLCh* ATFloatOrDerivedImpl::getPrimitiveTypeName() const {
  return this->getPrimitiveName();
}

const XMLCh* ATFloatOrDerivedImpl::getPrimitiveName()  {
  return SchemaSymbols::fgDT_FLOAT;
}

/* Get the name of this type  (ie "integer" for xs:integer) */
const XMLCh* ATFloatOrDerivedImpl::getTypeName() const {
  return _typeName;
}

/* Get the namespace URI for this type */
const XMLCh* ATFloatOrDerivedImpl::getTypeURI() const {
  return _typeURI; 
}

AnyAtomicType::AtomicObjectType ATFloatOrDerivedImpl::getTypeIndex() {
  return AnyAtomicType::FLOAT;
} 

/* returns the XMLCh* (canonical) representation of this type */
const XMLCh* ATFloatOrDerivedImpl::asString(const DynamicContext* context) const
{
  return asDoubleString(g_nSignificantDigits, context);
}

/* Promote this to the given type, if possible */
Numeric::Ptr ATFloatOrDerivedImpl::promoteTypeIfApplicable(AnyAtomicType::AtomicObjectType typeIndex,
                                                           const DynamicContext* context) const
{
  switch(typeIndex) {
  case AnyAtomicType::FLOAT:
    return this;
  case AnyAtomicType::DOUBLE:
    return (const Numeric::Ptr)castAs(typeIndex, context);
  default:
    break;
  }
  return 0;
}

/** Returns a Numeric object which is the sum of this and other */
Numeric::Ptr ATFloatOrDerivedImpl::add(const Numeric::Ptr &other, const DynamicContext* context) const {
  if(other->getPrimitiveTypeIndex() == AnyAtomicType::DECIMAL) {
    // if other is a decimal, promote it to xs:float
    return this->add((const Numeric::Ptr )other->castAs(this->getPrimitiveTypeIndex(), context), context);
  } else if (other->getPrimitiveTypeIndex() == AnyAtomicType::DOUBLE) {
    // if other is a double, promote this to xs:double
    return ((const Numeric::Ptr )this->castAs(other->getPrimitiveTypeIndex(), context))->add(other, context);
  } else if (other->getPrimitiveTypeIndex() == AnyAtomicType::FLOAT) {
    // same primitive type, can make comparison
    ATFloatOrDerivedImpl* otherImpl = (ATFloatOrDerivedImpl*)(const Numeric*)other;
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
                if(_float.sign()==0 && otherImpl->_float!=0)
                  return other;
                else if(_float.sign()!=0 && otherImpl->_float==0)
                  return this;
                else if(_float.sign()==0 && otherImpl->_float==0)
                {
                  if(_state==otherImpl->_state)
                    // sum of two zero of the same sign -> result is equal to any of the two items
                    return this;
                  else
                    // sum of two zero of different sign -> result is equal to +0
                    return newFloat(0, context);
                }
                return newFloat(_float + otherImpl->_float, context);
              }
          default: assert(false); return 0; // should never get here 
        }
      }
      default: assert(false); return 0; // should never get here 
    } 
  } else {
    assert(false); // should never get here, numeric types are xs:decimal, xs:float, xs:integer and xs:double
    return 0;
  }
}

/** Returns a Numeric object which is the difference of this and
   * other */
Numeric::Ptr ATFloatOrDerivedImpl::subtract(const Numeric::Ptr &other, const DynamicContext* context) const {
  if(other->getPrimitiveTypeIndex() == AnyAtomicType::DECIMAL) {
    // if other is a decimal, promote it to xs:float
    return this->subtract((const Numeric::Ptr )other->castAs(this->getPrimitiveTypeIndex(), context), context);
  } else if (other->getPrimitiveTypeIndex() == AnyAtomicType::DOUBLE) {
    // if other is a double, promote this to xs:double
    return ((const Numeric::Ptr )this->castAs(other->getPrimitiveTypeIndex(), context))->subtract(other, context);
  } else if (other->getPrimitiveTypeIndex() == AnyAtomicType::FLOAT) {
    // same primitive type, can make comparison
    ATFloatOrDerivedImpl* otherImpl = (ATFloatOrDerivedImpl*)(const Numeric*)other;
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
          case NUM: return newFloat(_float - otherImpl->_float, context);
          default: assert(false); return 0;  // should never get here
        }
      }
      default: assert(false); return 0;  // should never get here
    } 
  } else {
    assert(false); // should never get here, numeric types are xs:decimal, xs:float, xs:integer and xs:double
    return 0;
  }
}

/** Returns a Numeric object which is the product of this and other */
Numeric::Ptr ATFloatOrDerivedImpl::multiply(const Numeric::Ptr &other, const DynamicContext* context) const {
  if(other->getPrimitiveTypeIndex() == AnyAtomicType::DECIMAL) {
    // if other is a decimal, promote it to xs:float
    return this->multiply((const Numeric::Ptr )other->castAs(this->getPrimitiveTypeIndex(), context), context);
  } else if (other->getPrimitiveTypeIndex() == AnyAtomicType::DOUBLE) {
    // if other is a double, promote this to xs:double
    return ((const Numeric::Ptr )this->castAs(other->getPrimitiveTypeIndex(), context))->multiply(other, context);
  } else if (other->getPrimitiveTypeIndex() == AnyAtomicType::FLOAT) {
    // same primitive type, can make comparison
    ATFloatOrDerivedImpl* otherImpl = (ATFloatOrDerivedImpl*)(const Numeric*)other;
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
                return newFloat(0, context);           // 0 / NUM or (-0) / (-NUM) = 0
              }
            }
            return newFloat(_float * otherImpl->_float, context);
          default: assert(false); return 0;  // should never get here
        }
      }
      default: assert(false); return 0;  // should never get here
    } 
  } else {
    assert(false); // should never get here, numeric types are xs:decimal, xs:float, xs:integer and xs:double
    return 0;
  }
}

/** Returns a Numeric object which is the quotient of this and other */
Numeric::Ptr ATFloatOrDerivedImpl::divide(const Numeric::Ptr &other, const DynamicContext* context) const {
    if(other->getPrimitiveTypeIndex() == AnyAtomicType::DECIMAL) {
    // if other is a decimal, promote it to xs:float
    return this->divide((const Numeric::Ptr )other->castAs(this->getPrimitiveTypeIndex(), context), context);
  } else if (other->getPrimitiveTypeIndex() == AnyAtomicType::DOUBLE) {
    // if other is a double, promote this to xs:double
    return ((const Numeric::Ptr )this->castAs(other->getPrimitiveTypeIndex(), context))->divide(other, context);
  } else if (other->getPrimitiveTypeIndex() == AnyAtomicType::FLOAT) {
    // same primitive type, can make comparison
    ATFloatOrDerivedImpl* otherImpl = (ATFloatOrDerivedImpl*)(const Numeric*)other;
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
              return newFloat(0, context);
            }
          }// case
          case NEG_INF: {                               // NUM / -INF = +/-0
            if(this->isPositive()) {
              return negZero(context);
            } else {
              return newFloat(0, context);
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
                return newFloat(0, context);            // 0 / NUM or (-0) / (-NUM) = 0
              }
            }
            return newFloat(_float / otherImpl->_float, context);
          }// case
          default: assert(false); return 0;  // should never get here
        }// switch
      }// case
      default: assert(false); return 0;  // should never get here
    }// switch
  } else {
    assert(false); // should never get here, numeric types are xs:decimal, xs:float, xs:integer and xs:double
    return 0;
  }
}

/** Returns the mod of its operands as a Numeric */
Numeric::Ptr ATFloatOrDerivedImpl::mod(const Numeric::Ptr &other, const DynamicContext* context) const {
  if(other->getPrimitiveTypeIndex() == AnyAtomicType::DECIMAL) {
    // if other is a decimal, promote it to xs:float
    return this->mod((const Numeric::Ptr )other->castAs(this->getPrimitiveTypeIndex(), context), context);
  } else if (other->getPrimitiveTypeIndex() == AnyAtomicType::DOUBLE) {
    // if other is a double, promote this to xs:double
    return ((const Numeric::Ptr )this->castAs(other->getPrimitiveTypeIndex(), context))->mod(other, context);
  } else if (other->getPrimitiveTypeIndex() == AnyAtomicType::FLOAT) {
    // same primitive type, can make comparison
    const ATFloatOrDerivedImpl* otherImpl = (ATFloatOrDerivedImpl*)(const Numeric*)other;
    if(this->isNaN() || otherImpl->isNaN() || this->isInfinite() || otherImpl->isZero()) {
      return notANumber(context);    
    } else if(otherImpl->isInfinite() || this->isZero()) {
      return (const Numeric::Ptr )this->castAs(AnyAtomicType::FLOAT, context);
    } else {
      MAPM result = _float;
      MAPM r;
      r = result.integer_divide(otherImpl->_float);
      result -= r * otherImpl->_float;
      if (result == 0 && isNegative())
        return negZero(context);
      return newFloat(result, context);
    }
  } else {
    assert(false); // should never get here, numeric types are xs:decimal, xs:float, xs:integer and xs:double
    return 0;
  }
}

Numeric::Ptr ATFloatOrDerivedImpl::power(const Numeric::Ptr &other, const DynamicContext* context) const
{
  switch(other->getPrimitiveTypeIndex()) {
  case DECIMAL:
    // if other is a decimal, promote it to xs:float
    return power(context->getItemFactory()->createFloat(other->asMAPM(), context), context);
  case FLOAT: {
    ATFloatOrDerivedImpl *otherImpl = (ATFloatOrDerivedImpl*)other.get();

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
        return newFloat(_float.pow(otherImpl->_float), context);
      default: assert(false); return 0;  // should never get here
      }
    }
    default: assert(false); return 0;  // should never get here
    } 
  }
  case DOUBLE:
    return ((Numeric*)this->castAs(DOUBLE, context).get())->power(other, context);
  default: assert(false); return 0; // Shouldn't happen
  }
}

/** Returns the floor of this Numeric */
Numeric::Ptr ATFloatOrDerivedImpl::floor(const DynamicContext* context) const {
  switch (_state) {
    case NaN: return notANumber(context);
    case INF: return infinity(context);
    case NEG_INF: return negInfinity(context);
    case NEG_NUM:
    case NUM: { 
      if (isZero() && isNegative())
        return negZero(context);
      return newFloat(_float.floor(), context); 
    }
    default: { assert(false); return 0; // should never get here 
    }
  }
}

/** Returns the ceiling of this Numeric */
Numeric::Ptr ATFloatOrDerivedImpl::ceiling(const DynamicContext* context) const {
  switch (_state) {
    case NaN: return notANumber(context);
    case INF: return infinity(context);
    case NEG_INF: return negInfinity(context);
    case NEG_NUM:
    case NUM: { 
      if (isNegative() && _float >= -0.5) {
        return negZero(context);
      }
      return newFloat(_float.ceil(), context);
    }
    default: { assert(false); return 0; // should never get here 
    }
  }
}

/** Rounds this Numeric */
Numeric::Ptr ATFloatOrDerivedImpl::round(const DynamicContext* context) const {
  switch (_state) {
    case NaN: return notANumber(context);
    case INF: return infinity(context);
    case NEG_INF: return negInfinity(context);
    case NEG_NUM:
    case NUM: { 
     if (isNegative() &&_float >= -0.5) {
        return negZero(context);
      }      
      MAPM value = _float + 0.5; 
      return newFloat(value.floor(), context);
    }
    default: { assert(false); return 0; // should never get here
    }
  } 
}

/** Rounds this Numeric to the given precision, and rounds a half to even */
Numeric::Ptr ATFloatOrDerivedImpl::roundHalfToEven(const Numeric::Ptr &precision, const DynamicContext* context) const {
  switch (_state) {
    case NaN: return notANumber(context);
    case INF: return infinity(context);
    case NEG_INF: return negInfinity(context);
    case NEG_NUM:
    case NUM: break;
    default: { assert(false); return 0; // should never get here
    }
  }

  if (isZero() && isNegative())
    return this;

  ATFloatOrDerived::Ptr float_precision = (const Numeric::Ptr)precision->castAs(this->getPrimitiveTypeIndex(), context);
  MAPM exp = MAPM(10).pow(((ATFloatOrDerivedImpl*)(const ATFloatOrDerived*)float_precision)->_float);
  MAPM value = _float * exp;
  bool halfVal = false;

  // check if we're rounding on a half value 
  if((value-0.5) == (value.floor())) {
    halfVal = true;
  }
  value = _float * exp + 0.5;
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
  return newFloat(value, context);
}

/** Returns the Additive inverse of this Numeric */
Numeric::Ptr ATFloatOrDerivedImpl::invert(const DynamicContext* context) const {
  switch (_state) {
    case NaN: return this;
    case INF: return negInfinity(context);
    case NEG_INF: return infinity(context);
    case NEG_NUM:
    case NUM: 
        if(this->isZero())
        {
          if(this->isNegative())
            return newFloat(0, context);
          else
            return negZero(context);
        }
        return newFloat(_float.neg(), context);
    default: assert(false); return 0;  // should never get here
  }
}

Numeric::Ptr ATFloatOrDerivedImpl::abs(const DynamicContext* context) const {
  switch (_state) {
    case NaN: return this;
    case INF: return infinity(context);
    case NEG_INF: return infinity(context);
    case NEG_NUM:
    case NUM: return newFloat(_float.abs(), context);
    default: assert(false); return 0;  // should never get here
  }
}

/** Returns the square root of this Numeric */
Numeric::Ptr ATFloatOrDerivedImpl::sqrt(const DynamicContext* context) const {
  switch (_state) {
    case NaN: return this;
    case INF: return infinity(context);
    case NEG_INF: return notANumber(context);
    case NEG_NUM: return notANumber(context);
    case NUM: return newFloat(_float.sqrt(), context);
    default: assert(false); return 0;  // should never get here
  }
}

/** Returns the sinus of this Numeric */
Numeric::Ptr ATFloatOrDerivedImpl::sin(const DynamicContext* context) const {
  switch (_state) {
    case NaN: return this;
    case INF: return notANumber(context);;
    case NEG_INF: return notANumber(context);;
    case NEG_NUM:
    case NUM: return newFloat(_float.sin(), context);
    default: assert(false); return 0;  // should never get here
  }
}

Numeric::Ptr ATFloatOrDerivedImpl::cos(const DynamicContext* context) const {
  switch (_state) {
    case NaN: return this;
    case INF: return notANumber(context);
    case NEG_INF: return notANumber(context);;
    case NEG_NUM:
    case NUM: return newFloat(_float.cos(), context);
    default: assert(false); return 0;  // should never get here
  }
}

Numeric::Ptr ATFloatOrDerivedImpl::tan(const DynamicContext* context) const {
  switch (_state) {
    case NaN: return this;
    case INF: return notANumber(context);
    case NEG_INF: return notANumber(context);
    case NEG_NUM:
    case NUM: return newFloat(_float.tan(), context);
    default: assert(false); return 0;  // should never get here
  }
}

Numeric::Ptr ATFloatOrDerivedImpl::asin(const DynamicContext* context) const {
  switch (_state) {
    case NaN: return this;
    case INF: return notANumber(context);
    case NEG_INF: return notANumber(context);
    case NEG_NUM:
    case NUM: 
               if(_float.abs() > 1)return notANumber(context);
               return newFloat(_float.asin(), context);
    default: assert(false); return 0;  // should never get here
  }
}

Numeric::Ptr ATFloatOrDerivedImpl::acos(const DynamicContext* context) const {
  switch (_state) {
    case NaN: return this;
    case INF: return notANumber(context);
    case NEG_INF: return notANumber(context);
    case NEG_NUM:
    case NUM: 
               if(_float.abs() > 1)return notANumber(context);
               return newFloat(_float.acos(), context);
    default: assert(false); return 0;  // should never get here
  }
}

Numeric::Ptr ATFloatOrDerivedImpl::atan(const DynamicContext* context) const {
  switch (_state) {
    case NaN: return this;
    case INF:     return newFloat((MM_HALF_PI), context);
    case NEG_INF: return newFloat(MAPM(MM_HALF_PI).neg(), context);
    case NEG_NUM:
    case NUM: return newFloat(_float.atan(), context);
    default: assert(false); return 0;  // should never get here
  }
}

Numeric::Ptr ATFloatOrDerivedImpl::log(const DynamicContext* context) const {
  switch (_state) {
    case NaN: return this;
    case INF: return infinity(context);
    case NEG_INF: return notANumber(context);
    case NEG_NUM: return notANumber(context);
    case NUM: 
              if(_float == 0)return notANumber(context);
              return newFloat(_float.log(), context);
    default: assert(false); return 0;  // should never get here
  }
}

Numeric::Ptr ATFloatOrDerivedImpl::exp(const DynamicContext* context) const {
  switch (_state) {
    case NaN: return this;
    case INF: return infinity(context);
    case NEG_INF: return newFloat(0, context);
    case NEG_NUM:
    case NUM: return newFloat(_float.exp(), context);
    default: assert(false); return 0;  // should never get here
  }
}

/** Does this Numeric have value 0? */
bool ATFloatOrDerivedImpl::isZero() const {
  switch (_state) {
    case NaN: 
    case INF: 
    case NEG_INF: return false;
    case NEG_NUM:
    case NUM: return _float.sign() == 0;
    default: { assert(false); return false; // should never get here
    }
  }
}

/** Is this Numeric positive? */
bool ATFloatOrDerivedImpl::isPositive() const {
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

/** Is this Numeric negative? */
bool ATFloatOrDerivedImpl::isNegative() const {
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

/* Is this xs:float not a number */
bool ATFloatOrDerivedImpl::isNaN() const {
  return _state == NaN;
}


/* Is this xs:float infinite? */
bool ATFloatOrDerivedImpl::isInfinite() const {
  return _state == INF || _state == NEG_INF;
}


AnyAtomicType::AtomicObjectType ATFloatOrDerivedImpl::getPrimitiveTypeIndex() const {
  return this->getTypeIndex();
}

/* returns a new infinity xs:float*/
ATFloatOrDerived::Ptr ATFloatOrDerivedImpl::infinity(const DynamicContext* context) const {
  return context->getItemFactory()->createFloat(Numeric::INF_string, context);
}

/* returns a new negative infinity xs:float*/
ATFloatOrDerived::Ptr ATFloatOrDerivedImpl::negInfinity(const DynamicContext* context) const {
  return context->getItemFactory()->createFloat(Numeric::NegINF_string, context);
}
  
/* returns a NaN xs:float*/
ATFloatOrDerived::Ptr ATFloatOrDerivedImpl::notANumber(const DynamicContext* context) const {
  return context->getItemFactory()->createFloat(Numeric::NaN_string, context);
}

/* returns a -0 ATFloatOrDerived*/
ATFloatOrDerived::Ptr ATFloatOrDerivedImpl::negZero(const DynamicContext* context) const {
  return context->getItemFactory()->createFloat(Numeric::NegZero_string, context);
}

/*returns a ATFloatOrDerived of value value*/
ATFloatOrDerived::Ptr ATFloatOrDerivedImpl::newFloat(MAPM value, const DynamicContext* context) const {
  return context->getItemFactory()->createFloat(value, context);
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
 

void ATFloatOrDerivedImpl::setFloat(const XMLCh* const value)
{
  _float = parse(value, _state);
}

MAPM ATFloatOrDerivedImpl::parseFloat(const XMLCh* const value, State &state)
{
  MAPM result = parse(value, state);
  Numeric::checkFloatLimits(state, result);
  return result;
}

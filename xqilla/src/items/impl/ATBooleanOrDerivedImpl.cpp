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
#include <xqilla/items/impl/ATBooleanOrDerivedImpl.hpp>
#include <xercesc/util/XMLUni.hpp>
#include <xercesc/util/XMLUniDefs.hpp>
#include <xercesc/validators/schema/SchemaSymbols.hpp>
#include <xqilla/utils/XPath2Utils.hpp>
#include <xqilla/framework/XPath2MemoryManager.hpp>
#include <xqilla/exceptions/IllegalArgumentException.hpp>
#include <xqilla/exceptions/XPath2TypeCastException.hpp>
#include <xqilla/context/DynamicContext.hpp>
#include <xqilla/utils/XStr.hpp>
#include <xqilla/context/ItemFactory.hpp>

#include <xqilla/items/ATDoubleOrDerived.hpp>
#include <xqilla/items/ATFloatOrDerived.hpp>
#include <xqilla/items/ATDecimalOrDerived.hpp>

ATBooleanOrDerivedImpl::
ATBooleanOrDerivedImpl(const XMLCh* typeURI, const XMLCh* typeName, const XMLCh* value, const StaticContext* context): 
    _typeName(typeName),
    _typeURI(typeURI) { 
    
  // canonical values
  static const XMLCh zero[] = {XERCES_CPP_NAMESPACE_QUALIFIER chDigit_0, XERCES_CPP_NAMESPACE_QUALIFIER chNull };
  static const XMLCh one[] = {XERCES_CPP_NAMESPACE_QUALIFIER chDigit_1, XERCES_CPP_NAMESPACE_QUALIFIER chNull };
  
  if( XPath2Utils::equals(value, zero) || XPath2Utils::equals(value, XERCES_CPP_NAMESPACE_QUALIFIER SchemaSymbols::fgATTVAL_FALSE)) {
    _value = false;
  } else if(XPath2Utils::equals(value, one) || XPath2Utils::equals(value, XERCES_CPP_NAMESPACE_QUALIFIER SchemaSymbols::fgATTVAL_TRUE)) {
    _value = true;
  } else {
    XQThrow2(XPath2TypeCastException,X("ATBooleanOrDerivedImpl::ATBooleanOrDerivedImpl"), X("Invalid representation of boolean [err:FORG0001]"));
  }  
}

ATBooleanOrDerivedImpl::
ATBooleanOrDerivedImpl(const XMLCh* typeURI, const XMLCh* typeName, bool value, const StaticContext* context): 
    _value(value),
    _typeName(typeName),
    _typeURI(typeURI)
{ }
    
void *ATBooleanOrDerivedImpl::getInterface(const XMLCh *name) const
{
  if(name == Item::gXQilla) {
    return (void*)this;
  }
  return 0;
}

/* Get the name of the primitive type (basic type) of this type 
 * (ie "decimal" for xs:decimal) */
const XMLCh* ATBooleanOrDerivedImpl::getPrimitiveTypeName() const {
  return this->getPrimitiveName();
}

const XMLCh* ATBooleanOrDerivedImpl::getPrimitiveName()  {
  return XERCES_CPP_NAMESPACE_QUALIFIER SchemaSymbols::fgDT_BOOLEAN;
}

/* Get the name of this type  (ie "integer" for xs:integer) */
const XMLCh* ATBooleanOrDerivedImpl::getTypeName() const {
  return _typeName;
}

/* Get the namespace URI for this type */
const XMLCh* ATBooleanOrDerivedImpl::getTypeURI() const {
  return _typeURI; 
}

AnyAtomicType::AtomicObjectType ATBooleanOrDerivedImpl::getTypeIndex() {
  return AnyAtomicType::BOOLEAN;
} 

/* If possible, cast this type to the target type */
AnyAtomicType::Ptr ATBooleanOrDerivedImpl::castAsInternal(AtomicObjectType targetIndex, const XMLCh* targetURI, const XMLCh* targetType, const DynamicContext* context) const {
  // deviation from spec -- they say to convert to 1.0 and 0.0, but we don't see the point,
  // and it doesn't make casting to integer a special case (crioux)
  const XMLCh one[] = { XERCES_CPP_NAMESPACE_QUALIFIER chDigit_1, 
                        XERCES_CPP_NAMESPACE_QUALIFIER chNull };
  const XMLCh zero[] = { XERCES_CPP_NAMESPACE_QUALIFIER chDigit_0,
                         XERCES_CPP_NAMESPACE_QUALIFIER chNull };   
  const XMLCh* chValue = _value ? one : zero;

  switch(targetIndex) {
    case FLOAT: {
      return (const AnyAtomicType::Ptr)context->getItemFactory()->createFloatOrDerived(targetURI, targetType, chValue, context);
    }
    case DECIMAL:  {
      return (const AnyAtomicType::Ptr)context->getItemFactory()->createDecimalOrDerived(targetURI, targetType, chValue, context);
    }
    case DOUBLE: {
      return (const AnyAtomicType::Ptr)context->getItemFactory()->createDoubleOrDerived(targetURI, targetType, chValue, context);
    }
    default: {
      return AnyAtomicType::castAsInternal(targetIndex, targetURI, targetType, context);
    }
  }
  
}

/* returns the XMLCh* (canonical) representation of this type */
const XMLCh* ATBooleanOrDerivedImpl::asString(const DynamicContext* context) const {
  return asString();
}

const XMLCh* ATBooleanOrDerivedImpl::asString() const
{
  if(_value) {
    return XERCES_CPP_NAMESPACE_QUALIFIER SchemaSymbols::fgATTVAL_TRUE;
  } else {
    return XERCES_CPP_NAMESPACE_QUALIFIER SchemaSymbols::fgATTVAL_FALSE;
  }
}

/* returns true if the two objects have the same boolean value
 * false otherwise */
bool ATBooleanOrDerivedImpl::equals(const AnyAtomicType::Ptr &target, const DynamicContext* context) const
{
  if(this->getPrimitiveTypeIndex() != target->getPrimitiveTypeIndex()) {
    XQThrow2(IllegalArgumentException,X("ATBooleanOrDerivedImpl::equals"),
            X("Equality operator for given types not supported [err:XPTY0004]"));
  }

  return compare((const ATBooleanOrDerived*)target.get(), context) == 0;
}

int ATBooleanOrDerivedImpl::compare(const ATBooleanOrDerived::Ptr &other, const DynamicContext *context) const
{
  return (int)_value - (int)other->isTrue();
}

/* returns true if boolean value evaluates to true
 * false otherwise */
bool ATBooleanOrDerivedImpl::isTrue() const {
  return _value;
}

/* returns false if boolean value evaluates to true
 * true otherwise */
bool ATBooleanOrDerivedImpl::isFalse() const {
  return !isTrue();
}

AnyAtomicType::AtomicObjectType ATBooleanOrDerivedImpl::getPrimitiveTypeIndex() const {
  return getTypeIndex();
}

const BoolResult BoolResult::True(true);
const BoolResult BoolResult::False(false);
const BoolResult BoolResult::Null;

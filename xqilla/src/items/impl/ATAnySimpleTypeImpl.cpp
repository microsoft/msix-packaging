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
#include "ATAnySimpleTypeImpl.hpp"
#include <xqilla/items/ATDoubleOrDerived.hpp>
#include <xqilla/items/ATFloatOrDerived.hpp>
#include <xqilla/exceptions/IllegalArgumentException.hpp>
#include <xqilla/exceptions/XPath2TypeCastException.hpp>
#include <xercesc/util/XMLUni.hpp>
#include <xercesc/validators/schema/SchemaSymbols.hpp>
#include <xqilla/items/DatatypeFactory.hpp>
#include <xqilla/utils/XStr.hpp>
#include <xqilla/framework/XPath2MemoryManager.hpp>
#include <xqilla/utils/XPath2Utils.hpp>
#include <xqilla/context/DynamicContext.hpp>
#include <xercesc/util/XMLString.hpp>
#include <xqilla/context/ItemFactory.hpp>

// We ignore the type URI and typeName passed in, since they are always xdt:anySimpleType
// (you cannot derived from it)
ATAnySimpleTypeImpl::
ATAnySimpleTypeImpl(const XMLCh*, const XMLCh*, const XMLCh* value, const StaticContext* context)
{    
  _value = context->getMemoryManager()->getPooledString(value);
}

void *ATAnySimpleTypeImpl::getInterface(const XMLCh *name) const
{
  if(name == Item::gXQilla) {
    return (void*)this;
  }
  return 0;
}

/* Get the name of the primitive type (basic type) of this type
 * (ie "decimal" for xs:decimal) */
const XMLCh* ATAnySimpleTypeImpl::getPrimitiveTypeName() const {
  return this->getPrimitiveName();
}

const XMLCh* ATAnySimpleTypeImpl::getPrimitiveName()  {
  return XERCES_CPP_NAMESPACE_QUALIFIER SchemaSymbols::fgDT_ANYSIMPLETYPE;
}

/* Get the name of this type  (ie "integer" for xs:integer)-- 
 * same as the primitive type name. AnySimpleType cannot be 
 * extended */
const XMLCh* ATAnySimpleTypeImpl::getTypeName() const {
  return getPrimitiveTypeName();
}

/* Get the namespace URI for this type -- same as the primitive uri
 * AnySimpleType cannot be extended */
const XMLCh* ATAnySimpleTypeImpl::getTypeURI() const {
  return getPrimitiveTypeURI(); 
}

AnyAtomicType::AtomicObjectType ATAnySimpleTypeImpl::getTypeIndex() {
  return AnyAtomicType::ANY_SIMPLE_TYPE;
} 

AnyAtomicType::Ptr ATAnySimpleTypeImpl::castAsInternal(AtomicObjectType targetIndex, const XMLCh* targetURI, const XMLCh* targetType, const DynamicContext* context) const {
  switch (targetIndex) {
    case DOUBLE: { 
      const XMLCh* upCase = XPath2Utils::toUpper(_value, context->getMemoryManager());
      return context->getItemFactory()->createDoubleOrDerived(targetURI, targetType, upCase, context);
    } 
    case FLOAT: {
      const XMLCh* upCase = XPath2Utils::toUpper(_value, context->getMemoryManager());
      return context->getItemFactory()->createFloatOrDerived(targetURI, targetType, upCase, context);
    } 
    default: return AnyAtomicType::castAsInternal(targetIndex, targetURI, targetType, context);
  }
}

/* returns the XMLCh* (canonical) representation of this type */
const XMLCh* ATAnySimpleTypeImpl::asString(const DynamicContext* context) const {
  return _value;
}

/* returns true if the two objects' URI are equal (string comparison)
   * false otherwise */
bool ATAnySimpleTypeImpl::equals(const AnyAtomicType::Ptr &target, const DynamicContext* context) const {
  if(this->getPrimitiveTypeIndex() != target->getPrimitiveTypeIndex()) {
    XQThrow2(IllegalArgumentException,X("ATAnySimpleTypeImpl::equals"), X("Equality operator for given types not supported [err:XPTY0004]"));
  }
  return XPath2Utils::equals(target->asString(context), _value);  
}

AnyAtomicType::AtomicObjectType ATAnySimpleTypeImpl::getPrimitiveTypeIndex() const {
  return this->getTypeIndex();
}

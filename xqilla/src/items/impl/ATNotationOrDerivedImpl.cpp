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
#include "ATNotationOrDerivedImpl.hpp"
#include <xqilla/exceptions/IllegalArgumentException.hpp>
#include <xqilla/exceptions/NamespaceLookupException.hpp>
#include <xqilla/exceptions/StaticErrorException.hpp>
#include <xqilla/utils/XPath2Utils.hpp>
#include <xqilla/utils/XPath2NSUtils.hpp>
#include <xqilla/framework/XPath2MemoryManager.hpp>
#include <xqilla/context/DynamicContext.hpp>

#if defined(XERCES_HAS_CPP_NAMESPACE)
XERCES_CPP_NAMESPACE_USE
#endif

ATNotationOrDerivedImpl::
ATNotationOrDerivedImpl(const XMLCh* typeURI, const XMLCh* typeName, const XMLCh* value, const StaticContext* context): 
    ATNotationOrDerived(),
    _typeName(typeName),
    _typeURI(typeURI) { 
    
  _notation = context->getMemoryManager()->getPooledString(value);
      
  const XMLCh* prefix = XPath2NSUtils::getPrefix(_notation, context->getMemoryManager());
  const XMLCh* uri;
  if(prefix==0 || *prefix==0)
    uri = 0;
  else 
  {
    try
    {
      uri = context->getUriBoundToPrefix(prefix, 0);
    }
    catch(NamespaceLookupException&)
    {
      XQThrow2(StaticErrorException, X("ATNotationOrDerivedImpl::ATNotationOrDerivedImpl"),X("No namespace for prefix [err:XPST0081]"));
    }
  }

  // _uri will be null if there is no default NS
  _uri = context->getMemoryManager()->getPooledString(uri);
  _name = context->getMemoryManager()->getPooledString(XPath2NSUtils::getLocalName(_notation));
}

void *ATNotationOrDerivedImpl::getInterface(const XMLCh *name) const
{
  if(name == Item::gXQilla) {
    return (void*)this;
  }
  return 0;
}

/* Get the name of the primitive type (basic type) of this type
 * (ie "decimal" for xs:decimal) */
const XMLCh* ATNotationOrDerivedImpl::getPrimitiveTypeName() const {
  return this->getPrimitiveName();
}

const XMLCh* ATNotationOrDerivedImpl::getPrimitiveName()  {
  return XMLUni::fgNotationString;
}

/* Get the name of this type  (ie "integer" for xs:integer) */
const XMLCh* ATNotationOrDerivedImpl::getTypeName() const {
  return _typeName;
}

/* Get the namespace URI for this type */
const XMLCh* ATNotationOrDerivedImpl::getTypeURI() const {
  return _typeURI; 
}

AnyAtomicType::AtomicObjectType ATNotationOrDerivedImpl::getTypeIndex() {
  return AnyAtomicType::NOTATION;
} 

/* returns the XMLCh* (canonical) representation of this type */
const XMLCh* ATNotationOrDerivedImpl::asString(const DynamicContext* context) const {
  return _notation;
}

/* returns true if the two objects are equal (string comparison)
   * false otherwise */
bool ATNotationOrDerivedImpl::equals(const AnyAtomicType::Ptr &target, const DynamicContext* context) const {
  if(this->getPrimitiveTypeIndex() != target->getPrimitiveTypeIndex()) {
    XQThrow2(::IllegalArgumentException,X("ATNotationOrDerivedImpl::equals"),
	    X("Equality operator for given types not supported [err:XPTY0004]"));
  }

  return compare((const ATNotationOrDerived *)target.get(), context) == 0;
}

int ATNotationOrDerivedImpl::compare(const ATNotationOrDerivedImpl::Ptr &other, const DynamicContext *context) const
{
  const ATNotationOrDerivedImpl *otherImpl = (const ATNotationOrDerivedImpl *)other.get();

  int cmp = XPath2Utils::compare(_name, otherImpl->_name);
  if(cmp != 0) return cmp;

  return XPath2Utils::compare(_uri, otherImpl->_uri);
}

AnyAtomicType::AtomicObjectType ATNotationOrDerivedImpl::getPrimitiveTypeIndex() const {
  return this->getTypeIndex();
}

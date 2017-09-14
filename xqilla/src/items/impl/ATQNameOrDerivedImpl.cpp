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
#include <xqilla/items/impl/ATQNameOrDerivedImpl.hpp>
#include <xqilla/items/ATStringOrDerived.hpp>
#include <xqilla/utils/XPath2Utils.hpp>
#include <xqilla/utils/XPath2NSUtils.hpp>
#include <xqilla/runtime/Sequence.hpp>
#include <xqilla/exceptions/IllegalArgumentException.hpp>
#include <xqilla/exceptions/NamespaceLookupException.hpp>
#include <xqilla/exceptions/XPath2TypeCastException.hpp>
#include <xqilla/exceptions/StaticErrorException.hpp>
#include <xercesc/util/XMLUni.hpp>
#include <xercesc/util/XMLUniDefs.hpp>
#include <xercesc/framework/XMLBuffer.hpp>
#include <xercesc/validators/schema/SchemaSymbols.hpp>
#include <xqilla/utils/XStr.hpp>
#include <xqilla/framework/XPath2MemoryManager.hpp>
#include <xqilla/context/DynamicContext.hpp>
#include <xqilla/context/ItemFactory.hpp>

#include <xercesc/util/XMLString.hpp>

#if defined(XERCES_HAS_CPP_NAMESPACE)
XERCES_CPP_NAMESPACE_USE
#endif

ATQNameOrDerivedImpl::
ATQNameOrDerivedImpl(const XMLCh* typeURI, const XMLCh* typeName, const XMLCh* value, const StaticContext* context)
  : ATQNameOrDerived(),
    _typeName(typeName),
    _typeURI(typeURI)
{
  const XMLCh* prefix = XPath2NSUtils::getPrefix(value, context->getMemoryManager());
  const XMLCh* uri;
  if(XPath2Utils::equals(prefix, XMLUni::fgZeroLenString)) {
    // If $qname has no prefix
    uri = context->getDefaultElementAndTypeNS();
  }
  // If $qname has a prefix
  else {
    try {
      uri = context->getUriBoundToPrefix(prefix, 0);
    }
    catch(NamespaceLookupException&) {
      XQThrow2(StaticErrorException, X("ATQNameOrDerivedImpl::ATQNameOrDerivedImpl"),
              X("No namespace for prefix [err:FONS0004]"));
    }
  }

  // _uri will be null if there is no default NS
  _uri = context->getMemoryManager()->getPooledString(uri);
  _prefix = context->getMemoryManager()->getPooledString(prefix);
  _name = context->getMemoryManager()->getPooledString(XPath2NSUtils::getLocalName(value));

}

ATQNameOrDerivedImpl::
ATQNameOrDerivedImpl(const XMLCh* typeURI, const XMLCh* typeName, const XMLCh* uri, const XMLCh* prefix,
                     const XMLCh* name, const StaticContext* context)
  : ATQNameOrDerived(),
    _typeName(typeName),
    _typeURI(typeURI)
{
  // if uri is NULL, so will _uri
  _uri = context->getMemoryManager()->getPooledString(uri);
  _prefix = context->getMemoryManager()->getPooledString(prefix);
  _name = context->getMemoryManager()->getPooledString(name);

}


void *ATQNameOrDerivedImpl::getInterface(const XMLCh *name) const
{
  if(name == Item::gXQilla) {
    return (void*)this;
  }
  return 0;
}

/* Get the name of the primitive type (basic type) of this type
 * (ie "decimal" for xs:decimal) */
const XMLCh* ATQNameOrDerivedImpl::getPrimitiveTypeName() const {
  return this->getPrimitiveName();
}

/* Get the name of this type  (ie "integer" for xs:integer) */
const XMLCh* ATQNameOrDerivedImpl::getTypeName() const {
  return _typeName;
}

/* Get the namespace URI for this type */
const XMLCh* ATQNameOrDerivedImpl::getTypeURI() const {
  return _typeURI; 
}

/* returns the URI */
const XMLCh* ATQNameOrDerivedImpl::getURI(void) const {
  return _uri;
}

/* returns the prefix */
const XMLCh* ATQNameOrDerivedImpl::getPrefix(void) const {
  return _prefix;
}

/* returns the name */
const XMLCh* ATQNameOrDerivedImpl::getName(void) const {
  return _name;
}


AnyAtomicType::AtomicObjectType ATQNameOrDerivedImpl::getTypeIndex() {
  return AnyAtomicType::QNAME;
} 

/* If possible, cast this type to the target type */
AnyAtomicType::Ptr ATQNameOrDerivedImpl::castAsInternal(AtomicObjectType targetIndex, const XMLCh* targetURI,
                                                        const XMLCh* targetType, const DynamicContext* context) const
{
  switch (targetIndex) {
    case ANY_SIMPLE_TYPE:
    case UNTYPED_ATOMIC:
      //anySimpleType and untypedAtomic follow the same casting rules as string.
    case STRING: {
      return context->getItemFactory()->createDerivedFromAtomicType(targetIndex, targetURI, targetType,
                                                                    this->asLexicalString(context), context);
    } 
    case QNAME: {
      if(targetType == 0) {
        targetURI = SchemaSymbols::fgURI_SCHEMAFORSCHEMA;
        targetType = SchemaSymbols::fgDT_QNAME;
      }
      return context->getItemFactory()->createQNameOrDerived(targetURI, targetType, _uri, _prefix, _name, context);
    }
    default: return AnyAtomicType::castAsInternal(targetIndex, targetURI, targetType, context);
  }
}

/* returns the XMLCh* (canonical) representation of this type */
const XMLCh* ATQNameOrDerivedImpl::asString(const DynamicContext* context) const {
  XMLBuffer buffer(1023, context->getMemoryManager());
  if(_prefix != 0 && *_prefix != 0) {
    buffer.set(_prefix);
	  buffer.append(chColon);
  }
  buffer.append(_name);

  return context->getMemoryManager()->getPooledString(buffer.getRawBuffer());
}

/* returns the XMLCh* (lexical => prefix:localname) representation of this type */
const XMLCh* ATQNameOrDerivedImpl::asLexicalString(const DynamicContext* context) const {
  XMLBuffer buffer(1023, context->getMemoryManager());
  const XMLCh* prefix;
  if(this->_uri == 0 || XPath2Utils::equals(_uri, XMLUni::fgZeroLenString)) {
    if(context->getDefaultElementAndTypeNS() != 0) {
      XQThrow2(NamespaceLookupException,X("ATQNameOrDerivedImpl::asLexicalString"),
              X("Default namespace is defined: casting a QName that has no namespace is ambiguous"));
    }
    buffer.set(_name);
  } 
  else {
    // if we have a prefix, use it, otherwise try looking at the in-scope namespaces
    // (unless the namespace is the default one)
    if(XPath2Utils::equals(_prefix,XMLUni::fgZeroLenString) && 
       !XPath2Utils::equals(_uri, context->getDefaultElementAndTypeNS()))
        prefix = context->getPrefixBoundToUri(this->_uri);
    else
        prefix = _prefix;
    if (prefix == 0) {
      XMLBuffer msg(1023, context->getMemoryManager());
      msg.set(X("No prefix defined for namespace '"));
      msg.append(_uri);
      msg.append(X("'"));
      XQThrow2(NamespaceLookupException,X("ATQNameOrDerivedImpl::asLexicalString"), msg.getRawBuffer());
    }
    buffer.set(prefix);
    buffer.append(chColon);
    buffer.append(_name);    
  }
  return context->getMemoryManager()->getPooledString(buffer.getRawBuffer());
}

/* returns true if the two objects' URI are equal (string comparison)
 * false otherwise */
bool ATQNameOrDerivedImpl::equals(const AnyAtomicType::Ptr &target, const DynamicContext* context) const
{
  if(this->getPrimitiveTypeIndex() != target->getPrimitiveTypeIndex()) {
    XQThrow2(::IllegalArgumentException,X("ATQNameOrDerivedImpl::equals"),
            X("Equality operator for given types not supported [err:XPTY0004]"));
  }

  return compare((const ATQNameOrDerived *)target.get(), context) == 0;
}

int ATQNameOrDerivedImpl::compare(const ATQNameOrDerived::Ptr &other, const DynamicContext *context) const
{
  const ATQNameOrDerivedImpl *otherImpl = (const ATQNameOrDerivedImpl *)other.get();

  int cmp = XPath2Utils::compare(_name, otherImpl->_name);
  if(cmp != 0) return cmp;

  return XPath2Utils::compare(_uri, otherImpl->_uri);
}

const XMLCh* ATQNameOrDerivedImpl::getPrimitiveName()  {
  return SchemaSymbols::fgDT_QNAME;
}

AnyAtomicType::AtomicObjectType ATQNameOrDerivedImpl::getPrimitiveTypeIndex() const {
    return this->getTypeIndex();
}

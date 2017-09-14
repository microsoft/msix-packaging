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
#include "ATBase64BinaryOrDerivedImpl.hpp"
#include "ATHexBinaryOrDerivedImpl.hpp"
#include <xqilla/utils/XPath2Utils.hpp>
#include <xqilla/exceptions/IllegalArgumentException.hpp>
#include <xercesc/util/XMLUni.hpp>
#include <xercesc/util/Base64.hpp>
#include <xercesc/util/XMLString.hpp>
#include <xercesc/util/XMLChar.hpp>
#include <xercesc/util/Janitor.hpp>
#include <xercesc/validators/schema/SchemaSymbols.hpp>
#include <xqilla/framework/XPath2MemoryManager.hpp>
#include <xqilla/context/DynamicContext.hpp>
#include <xqilla/context/ItemFactory.hpp>

#if defined(XERCES_HAS_CPP_NAMESPACE)
XERCES_CPP_NAMESPACE_USE
#endif

#if _XERCES_VERSION >= 30000
typedef XMLSize_t stringLen_t;
#else
typedef unsigned int stringLen_t;
#endif

ATBase64BinaryOrDerivedImpl::
ATBase64BinaryOrDerivedImpl(const XMLCh* typeURI, const XMLCh* typeName, const XMLCh* value, const StaticContext* context): 
    _typeName(typeName),
    _typeURI(typeURI){ 
    
  // check if it's a valid base64 sequence, and then make it canonical by stripping whitespace
  MemoryManager* mm = context->getMemoryManager();
  stringLen_t srcLen = XMLString::stringLen(value);
  XMLByte *dataInByte = (XMLByte*) mm->allocate((srcLen+1) * sizeof(XMLByte));
  ArrayJanitor<XMLByte> janFill(dataInByte, mm);
  stringLen_t i;
  for (i = 0; i < srcLen; i++)
    dataInByte[i] = (XMLByte)value[i];

  dataInByte[srcLen] = 0;

  stringLen_t length=0, outLength=0;
  AutoDeallocate<XMLByte> decodedBinary(Base64::decode(dataInByte, &length, mm, Base64::Conf_Schema), mm);
  AutoDeallocate<XMLByte> encodedBase64(Base64::encode(decodedBinary, length, &outLength, mm), mm);
  _base64Data = (XMLCh*) mm->allocate((outLength+1) * sizeof(XMLCh));
  XMLCh* cursor=_base64Data;
  for (i = 0; i < outLength; i++)
    if(!XMLChar1_0::isWhitespace(encodedBase64[i]))
      *cursor++ = encodedBase64[i];
  *cursor = 0;
}

void *ATBase64BinaryOrDerivedImpl::getInterface(const XMLCh *name) const
{
  if(name == Item::gXQilla) {
    return (void*)this;
  }
  return 0;
}

/* Get the name of the primitive type (basic type) of this type (ie "decimal" for xs:decimal) */
const XMLCh* ATBase64BinaryOrDerivedImpl::getPrimitiveTypeName() const {
  return this->getPrimitiveName();
}

const XMLCh* ATBase64BinaryOrDerivedImpl::getPrimitiveName()  {
  return SchemaSymbols::fgDT_BASE64BINARY;
}

/* Get the name of this type  (ie "integer" for xs:integer) */
const XMLCh* ATBase64BinaryOrDerivedImpl::getTypeName() const {
  return _typeName;
}

/* Get the namespace URI for this type */
const XMLCh* ATBase64BinaryOrDerivedImpl::getTypeURI() const {
  return _typeURI; 
}

AnyAtomicType::AtomicObjectType ATBase64BinaryOrDerivedImpl::getTypeIndex() {
  return AnyAtomicType::BASE_64_BINARY;
} 

/* If possible, cast this type to the target type */
AnyAtomicType::Ptr ATBase64BinaryOrDerivedImpl::castAsInternal(AtomicObjectType targetIndex, const XMLCh* targetURI, const XMLCh* targetType, const DynamicContext* context) const
{
  static const XMLCh hexDigits[]={ chDigit_0, chDigit_1,
                                   chDigit_2, chDigit_3,
                                   chDigit_4, chDigit_5,
                                   chDigit_6, chDigit_7,
                                   chDigit_8, chDigit_9,
                                   chLatin_A, chLatin_B,
                                   chLatin_C, chLatin_D,
                                   chLatin_E, chLatin_F };

  switch(targetIndex) {
    case HEX_BINARY: {
      stringLen_t srcLen = XMLString::stringLen(_base64Data);
      XMLByte *dataInByte = (XMLByte*) context->getMemoryManager()->allocate((srcLen+1) * sizeof(XMLByte));
      ArrayJanitor<XMLByte> janFill(dataInByte, context->getMemoryManager());

      stringLen_t i;
      for (i = 0; i < srcLen; i++)
        dataInByte[i] = (XMLByte)_base64Data[i];

      dataInByte[srcLen] = 0;

      stringLen_t length=0;
      AutoDeallocate<XMLByte> decodedBinary(Base64::decode(dataInByte, &length, context->getMemoryManager(),
                                                           Base64::Conf_Schema), context->getMemoryManager());

      XMLBuffer buf(length*2, context->getMemoryManager());
      for(i=0;i<length;i++)
      {
          buf.append(hexDigits[decodedBinary[i]/16]);
          buf.append(hexDigits[decodedBinary[i]%16]);
      }
      if(targetType == 0) {
        targetURI = SchemaSymbols::fgURI_SCHEMAFORSCHEMA;
        targetType = SchemaSymbols::fgDT_HEXBINARY;
      }
      return new ATHexBinaryOrDerivedImpl(targetURI, targetType, buf.getRawBuffer(), context);
    }
    default: {
      return AnyAtomicType::castAsInternal(targetIndex, targetURI, targetType, context);
    }
  }
}

/* returns the XMLCh* (canonical) representation of this type */
const XMLCh* ATBase64BinaryOrDerivedImpl::asString(const DynamicContext* context) const {
  return _base64Data;
}

bool ATBase64BinaryOrDerivedImpl::equals(const AnyAtomicType::Ptr &target, const DynamicContext* context) const {
  if(this->getPrimitiveTypeIndex() != target->getPrimitiveTypeIndex()) {
    XQThrow2(::IllegalArgumentException,X("ATBase64BinaryOrDerivedImpl::equals"),
             X("Equality operator for given types not supported [err:XPTY0004]"));
  }
  return compare((const ATBase64BinaryOrDerived *)target.get(), context) == 0;
}

int ATBase64BinaryOrDerivedImpl::compare(const ATBase64BinaryOrDerived::Ptr &other, const DynamicContext *context) const
{
  return XPath2Utils::compare(_base64Data, ((const ATBase64BinaryOrDerivedImpl *)other.get())->_base64Data);
}

AnyAtomicType::AtomicObjectType ATBase64BinaryOrDerivedImpl::getPrimitiveTypeIndex() const {
  return this->getTypeIndex();
}

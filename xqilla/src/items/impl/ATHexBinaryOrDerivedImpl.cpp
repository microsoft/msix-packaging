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
#include "ATHexBinaryOrDerivedImpl.hpp"
#include "ATBase64BinaryOrDerivedImpl.hpp"
#include <xqilla/utils/XPath2Utils.hpp>
#include <xqilla/exceptions/IllegalArgumentException.hpp>
#include <xercesc/util/XMLUni.hpp>
#include <xercesc/util/HexBin.hpp>
#include <xercesc/util/Base64.hpp>
#include <xercesc/validators/schema/SchemaSymbols.hpp>
#include <xqilla/utils/XStr.hpp>
#include <xqilla/framework/XPath2MemoryManager.hpp>
#include <xercesc/util/XMLString.hpp>
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

ATHexBinaryOrDerivedImpl::
ATHexBinaryOrDerivedImpl(const XMLCh* typeURI, const XMLCh* typeName, const XMLCh* value, const StaticContext* context): 
    ATHexBinaryOrDerived(),
    _typeName(typeName),
    _typeURI(typeURI) { 
    
  XMLCh* tempValue=XMLString::replicate(value, context->getMemoryManager());
  XMLString::upperCase(tempValue);
  _hexBinaryData = context->getMemoryManager()->getPooledString(tempValue);
#if _XERCES_VERSION >= 30000
  XMLString::release(&tempValue, context->getMemoryManager());
#else
  XMLString::release((void**)&tempValue, context->getMemoryManager());
#endif
}

void *ATHexBinaryOrDerivedImpl::getInterface(const XMLCh *name) const
{
  if(name == Item::gXQilla) {
    return (void*)this;
  }
  return 0;
}

/* Get the name of the primitive type (basic type) of this type
 * (ie "decimal" for xs:decimal) */
const XMLCh* ATHexBinaryOrDerivedImpl::getPrimitiveTypeName() const {
  return this->getPrimitiveName();
}

const XMLCh* ATHexBinaryOrDerivedImpl::getPrimitiveName()  {
  return SchemaSymbols::fgDT_HEXBINARY;
}

/* Get the name of this type  (ie "integer" for xs:integer) */
const XMLCh* ATHexBinaryOrDerivedImpl::getTypeName() const {
  return _typeName;
}

/* Get the namespace URI for this type */
const XMLCh* ATHexBinaryOrDerivedImpl::getTypeURI() const {
  return _typeURI; 
}

AnyAtomicType::AtomicObjectType ATHexBinaryOrDerivedImpl::getTypeIndex() {
  return AnyAtomicType::HEX_BINARY;
} 

/* If possible, cast this type to the target type */
AnyAtomicType::Ptr ATHexBinaryOrDerivedImpl::castAsInternal(AtomicObjectType targetIndex, const XMLCh* targetURI, const XMLCh* targetType, const DynamicContext* context) const
{
  switch(targetIndex) {
    case BASE_64_BINARY: {
      XMLByte* binData=HexBin::decodeToXMLByte(_hexBinaryData, context->getMemoryManager());
      stringLen_t length=0;
      XMLByte* base64Data=Base64::encode(binData, 
                                         XMLString::stringLen(_hexBinaryData)/2, 
                                         &length, 
                                         context->getMemoryManager()); 
      XMLCh* uniBase64=(XMLCh*)context->getMemoryManager()->allocate((length+1)*sizeof(XMLCh));
      stringLen_t i;
      for(i=0;i<length;i++)
          uniBase64[i]=(XMLCh)base64Data[i];
      uniBase64[i]=0;
      // replace #xA with #x20, collapse multiple spaces
      XMLString::collapseWS(uniBase64, context->getMemoryManager());
      if(targetType == 0) {
        targetURI = SchemaSymbols::fgURI_SCHEMAFORSCHEMA;
        targetType = SchemaSymbols::fgDT_BASE64BINARY;
      }
      ATBase64BinaryOrDerivedImpl* retVal=new ATBase64BinaryOrDerivedImpl(targetURI, targetType, uniBase64, context);
      context->getMemoryManager()->deallocate(uniBase64);
      context->getMemoryManager()->deallocate(binData);
      context->getMemoryManager()->deallocate(base64Data);
      return retVal;
    }
    default: {
      return AnyAtomicType::castAsInternal(targetIndex, targetURI, targetType, context);
    }
  }
}

/* returns the XMLCh* (canonical) representation of this type */
const XMLCh* ATHexBinaryOrDerivedImpl::asString(const DynamicContext* context) const {
  return _hexBinaryData;
}

/* returns true if the two objects' URI are equal (string comparison)
   * false otherwise */
bool ATHexBinaryOrDerivedImpl::equals(const AnyAtomicType::Ptr &target, const DynamicContext* context) const {
  if(this->getPrimitiveTypeIndex() != target->getPrimitiveTypeIndex()) {
    XQThrow2(::IllegalArgumentException,X("ATHexBinaryOrDerivedImpl::equals"),
            X("Equality operator for given types not supported [err:XPTY0004]"));
  }
  return compare((const ATHexBinaryOrDerived *)target.get(), context) == 0;
}

int ATHexBinaryOrDerivedImpl::compare(const ATHexBinaryOrDerived::Ptr &other, const DynamicContext *context) const
{
  return XPath2Utils::compare(_hexBinaryData, ((const ATHexBinaryOrDerivedImpl *)other.get())->_hexBinaryData);
}

AnyAtomicType::AtomicObjectType ATHexBinaryOrDerivedImpl::getPrimitiveTypeIndex() const {
  return this->getTypeIndex();
}

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

#include "../config/xqilla_config.h"
#include <assert.h>

#include <xqilla/items/DatatypeFactory.hpp>
#include <xqilla/context/DynamicContext.hpp>
#include "../exceptions/InvalidLexicalSpaceException.hpp"
#include <xqilla/items/ATDurationOrDerived.hpp>
#include <xqilla/items/ATUntypedAtomic.hpp>
#include <xqilla/utils/XPath2Utils.hpp>
#include <xqilla/utils/XPath2NSUtils.hpp>
#include <xqilla/exceptions/TypeNotFoundException.hpp>

#include <xercesc/util/XMLString.hpp>
#include <xercesc/validators/schema/SchemaSymbols.hpp>

#if defined(XERCES_HAS_CPP_NAMESPACE)
XERCES_CPP_NAMESPACE_USE
#endif

DatatypeFactory::DatatypeFactory(const DocumentCache* dc, DatatypeValidator *baseValidator)
  : fBaseValidator(baseValidator),
    fDocumentCache(dc)
{
}

DatatypeFactory::~DatatypeFactory()
{
}

AnyAtomicType::Ptr DatatypeFactory::createInstance(const XMLCh* value,
                                                   const DynamicContext* context) const
{
  if(fBaseValidator->getWSFacet()==DatatypeValidator::COLLAPSE && !XMLString::isWSCollapsed(value))
  {
      XMLCh* tempValue=XMLString::replicate(value, context->getMemoryManager());
      XMLString::collapseWS(tempValue, context->getMemoryManager());
      value=tempValue;
  }
  if(fBaseValidator->getWSFacet()==DatatypeValidator::REPLACE && !XMLString::isWSReplaced(value))
  {
      XMLCh* tempValue=XMLString::replicate(value, context->getMemoryManager());
      XMLString::replaceWS(tempValue, context->getMemoryManager());
      value=tempValue;
  }
  
  try {
    fBaseValidator->validate(value, 0, context->getMemoryManager());
  } catch (XMLException &e) {
    XQThrow2(InvalidLexicalSpaceException, X("DatatypeFactory::createInstance"), e.getMessage());
  }

  return createInstanceNoCheck(getPrimitiveTypeURI(), getPrimitiveTypeName(), value, context);
}

AnyAtomicType::Ptr DatatypeFactory::createInstance(const XMLCh* typeURI,
                                                   const XMLCh* typeName,
                                                   const XMLCh* value,
                                                   const DynamicContext* context) const
{
  if(typeName == 0)
    return createInstance(value, context);

  DatatypeValidator* validator =
    const_cast<DatatypeValidator*>(fDocumentCache->getDatatypeValidator(typeURI, typeName));

  if(!validator) {
    XMLBuffer buf(1023, context->getMemoryManager());
    buf.append(X("Type "));
    buf.append(typeURI);
    buf.append(chColon);
    buf.append(typeName);
    buf.append(X(" not found"));
    XQThrow2(TypeNotFoundException, X("DatatypeFactoryTemplate::createInstance"), buf.getRawBuffer());
  }

  if(validator->getWSFacet()==DatatypeValidator::COLLAPSE && !XMLString::isWSCollapsed(value))
  {
      XMLCh* tempValue=XMLString::replicate(value, context->getMemoryManager());
      XMLString::collapseWS(tempValue, context->getMemoryManager());
      value=tempValue;
  }
  if(validator->getWSFacet()==DatatypeValidator::REPLACE && !XMLString::isWSReplaced(value))
  {
      XMLCh* tempValue=XMLString::replicate(value, context->getMemoryManager());
      XMLString::replaceWS(tempValue, context->getMemoryManager());
      value=tempValue;
  }
  
  try {
    const XMLCh* valueToValidate=value;
    if(validator->getType()==DatatypeValidator::NOTATION)
    {
        const XMLCh* localPart = XPath2NSUtils::getLocalName(value);
        const XMLCh* prefix = XPath2NSUtils::getPrefix(value, context->getMemoryManager());
        const XMLCh* uriStr = (prefix && *prefix) ? context->getUriBoundToPrefix(prefix, 0) : XMLUni::fgZeroLenString;
        XMLCh szColon[]={ chColon, chNull };
        valueToValidate=XPath2Utils::concatStrings(uriStr, szColon, localPart, context->getMemoryManager());
    }
    validator->validate(valueToValidate, 0, context->getMemoryManager());
  } catch (XMLException &e) {
    XMLBuffer buf(1023, context->getMemoryManager());
    buf.append(e.getMessage());
    buf.append(X(" [err:FORG0001]"));
    XQThrow2(InvalidLexicalSpaceException, X("DatatypeFactory::createInstance"), buf.getRawBuffer());
  }

  return createInstanceNoCheck(typeURI, typeName, value, context);
}

bool DatatypeFactory::checkInstance(const XMLCh* value,
                                    MemoryManager *mm) const {
  try {
    fBaseValidator->validate(value, 0, mm);
  } catch (XMLException &e) {
    return false;
  }

  return true;
}

bool DatatypeFactory::checkInstance(const XMLCh* typeURI,
                                    const XMLCh* typeName,
                                    const XMLCh* value,
                                    MemoryManager *mm) const {
      
  DatatypeValidator* validator =
    const_cast<DatatypeValidator*>(fDocumentCache->getDatatypeValidator(typeURI, typeName));

  if(!validator) {
    XMLBuffer buf(1023, mm);
    buf.append(X("Type "));
    buf.append(typeURI);
    buf.append(chColon);
    buf.append(typeName);
    buf.append(X(" not found"));
    XQThrow2(TypeNotFoundException, X("DatatypeFactoryTemplate::createInstance"), buf.getRawBuffer());
  }

  try {
    validator->validate(value, 0, mm);
  } catch (XMLException &e) {
    return false;
  }

  return true;
}

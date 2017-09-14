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

/*
  Factory template class for Datatypes
*/

#ifndef _DatatypeFactoryTemplate_HPP
#define _DatatypeFactoryTemplate_HPP

#include <xqilla/framework/XQillaExport.hpp>
#include <xqilla/items/DatatypeFactory.hpp>
#include <xqilla/items/ATDurationOrDerived.hpp>
#include <xqilla/context/DynamicContext.hpp>
#include "impl/ATStringOrDerivedImpl.hpp"

class AnyAtomicType;

///used to facilitate the creation of datatypes
template<class TYPE>
class DatatypeFactoryTemplate : public DatatypeFactory
{
public:
  DatatypeFactoryTemplate(const DocumentCache* dc)
    : DatatypeFactory(dc, const_cast<XERCES_CPP_NAMESPACE_QUALIFIER DatatypeValidator*>
                      (dc->getDatatypeValidator(getPrimitiveTypeURI(), getPrimitiveTypeName()))) {}

  AnyAtomicType::Ptr createInstanceNoCheck(const XMLCh* typeURI,
                                           const XMLCh* typeName,
                                           const XMLCh* value,
                                           const DynamicContext* context) const {
    return new TYPE(typeURI, typeName, value, context);
  }

  AnyAtomicType::AtomicObjectType getPrimitiveTypeIndex() const
  {
    return TYPE::getTypeIndex();
  }

  virtual const XMLCh* getPrimitiveTypeURI() const {
    return XERCES_CPP_NAMESPACE_QUALIFIER SchemaSymbols::fgURI_SCHEMAFORSCHEMA;
  }

  const XMLCh* getPrimitiveTypeName() const {
    return TYPE::getPrimitiveName();
  }

protected:
  DatatypeFactoryTemplate(const DocumentCache* dc, XERCES_CPP_NAMESPACE_QUALIFIER DatatypeValidator *val)
    : DatatypeFactory(dc, val) {}
};

template<class TYPE>
class StringDatatypeFactory : public DatatypeFactoryTemplate<TYPE>
{
public:
  StringDatatypeFactory(const DocumentCache* dc)
    : DatatypeFactoryTemplate<TYPE>(dc) {}

  AnyAtomicType::Ptr createInstance(const XMLCh* value,
                                    const DynamicContext* context) const
  {
    return this->createInstanceNoCheck(DatatypeFactoryTemplate<TYPE>::getPrimitiveTypeURI(),
                                       DatatypeFactoryTemplate<TYPE>::getPrimitiveTypeName(), value, context);
  }

  bool checkInstance(const XMLCh* value,
                     const StaticContext* context) const
  {
    return true;
  }
};

template<class TYPE>
class DayTimeDurationDatatypeFactory : public DatatypeFactoryTemplate<TYPE>
{
public:
  DayTimeDurationDatatypeFactory(const DocumentCache* dc)
    : DatatypeFactoryTemplate<TYPE>(dc, const_cast<XERCES_CPP_NAMESPACE_QUALIFIER DatatypeValidator*>
                                    (dc->getDatatypeValidator(DatatypeFactoryTemplate<TYPE>::getPrimitiveTypeURI(),
                                                              getPrimitiveTypeName()))) {}

  AnyAtomicType::AtomicObjectType getPrimitiveTypeIndex() const
  {
    return AnyAtomicType::DAY_TIME_DURATION;
  }

  const XMLCh* getPrimitiveTypeName() const {
    return ATDurationOrDerived::fgDT_DAYTIMEDURATION;
  }
};

template<class TYPE>
class YearMonthDurationDatatypeFactory : public DatatypeFactoryTemplate<TYPE>
{
public:
  YearMonthDurationDatatypeFactory(const DocumentCache* dc)
    : DatatypeFactoryTemplate<TYPE>(dc, const_cast<XERCES_CPP_NAMESPACE_QUALIFIER DatatypeValidator*>
                                    (dc->getDatatypeValidator(DatatypeFactoryTemplate<TYPE>::getPrimitiveTypeURI(),
                                                              getPrimitiveTypeName()))) {}

  AnyAtomicType::AtomicObjectType getPrimitiveTypeIndex() const
  {
    return AnyAtomicType::YEAR_MONTH_DURATION;
  }

  const XMLCh* getPrimitiveTypeName() const {
    return ATDurationOrDerived::fgDT_YEARMONTHDURATION;
  }
};

#endif // _DatatypeFactoryTemplate_HPP

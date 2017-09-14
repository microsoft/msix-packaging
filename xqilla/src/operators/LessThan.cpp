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
#include <xqilla/operators/LessThan.hpp>
#include <xercesc/util/XMLUni.hpp>
#include <xqilla/items/ATBooleanOrDerived.hpp>
#include <xqilla/items/Numeric.hpp>
#include <xqilla/items/ATDoubleOrDerived.hpp>
#include <xqilla/items/ATDecimalOrDerived.hpp>
#include <xqilla/items/Node.hpp>
#include <xqilla/items/ATDateOrDerived.hpp>
#include <xqilla/items/ATDateTimeOrDerived.hpp>
#include <xqilla/items/ATTimeOrDerived.hpp>
#include <xqilla/items/ATStringOrDerived.hpp>
#include <xqilla/items/ATDurationOrDerived.hpp>
#include <xqilla/context/DynamicContext.hpp>
#include <xqilla/exceptions/FunctionException.hpp>
#include <xqilla/exceptions/XPath2ErrorException.hpp>
#include <xqilla/items/DatatypeFactory.hpp>
#include <xqilla/context/Collation.hpp>
#include <xqilla/context/impl/CodepointCollation.hpp>

/*static*/ const XMLCh LessThan::name[]={ XERCES_CPP_NAMESPACE_QUALIFIER chLatin_L, XERCES_CPP_NAMESPACE_QUALIFIER chLatin_T, XERCES_CPP_NAMESPACE_QUALIFIER chNull };

LessThan::LessThan(const VectorOfASTNodes &args, XPath2MemoryManager* memMgr)
  : ComparisonOperator(name, args, memMgr)
{
}

/*static*/ bool LessThan::less_than(const AnyAtomicType::Ptr &atom1, const AnyAtomicType::Ptr &atom2, Collation* collation, DynamicContext* context, const LocationInfo *info)
{
  try {
    // take care of Numeric types first
    if(atom1->isNumericValue()) {
      if(atom2->isNumericValue()) {
        return ((Numeric*)atom1.get())->lessThan((Numeric*)atom2.get(), context);
      } else {
        XQThrow2(XPath2ErrorException,X("LessThan::less_than"), X("An attempt to compare a numeric type to a non numeric type has occurred [err:XPTY0004]"));
      }
    }

    switch(atom1->getPrimitiveTypeIndex()) {
    case AnyAtomicType::BOOLEAN:
    {
      // op:boolean-less-than(A, B)
      if(atom2->getPrimitiveTypeIndex() != AnyAtomicType::BOOLEAN) 
        XQThrow2(XPath2ErrorException,X("LessThan::less_than"), X("An attempt to compare a boolean type to a non boolean type has occurred [err:XPTY0004]"));
      return ((const ATBooleanOrDerived*)atom1.get())->compare((const ATBooleanOrDerived*)atom2.get(), context) < 0;
    }
    case AnyAtomicType::STRING:
    case AnyAtomicType::ANY_URI:
    {
      // op:numeric-less-than(fn:compare(A, B), 0)
      if(atom2->getPrimitiveTypeIndex() != AnyAtomicType::STRING &&
         atom2->getPrimitiveTypeIndex() != AnyAtomicType::ANY_URI)
        XQThrow2(XPath2ErrorException,X("LessThan::less_than"), X("An attempt to compare a string type to a non string type has occurred [err:XPTY0004]"));
      // if the function returns -1, then atom1 is less
      if(!collation) collation = context->getDefaultCollation(info);
      return collation->compare(atom1->asString(context),atom2->asString(context))<0;
    }
    case AnyAtomicType::DATE:
    {
      // op:date-less-than(A, B)
      if(atom2->getPrimitiveTypeIndex() != AnyAtomicType::DATE)
        XQThrow2(XPath2ErrorException,X("LessThan::less_than"), X("An attempt to compare a date type to a non date type has occurred [err:XPTY0004]"));
      return ((ATDateOrDerived*)atom1.get())->compare((const ATDateOrDerived::Ptr )atom2, context) < 0;
    }
    case AnyAtomicType::TIME:
    {
      // op:time-less-than(A, B)
      if(atom2->getPrimitiveTypeIndex() != AnyAtomicType::TIME) 
        XQThrow2(XPath2ErrorException,X("LessThan::less_than"), X("An attempt to compare a time type to a non time type has occurred [err:XPTY0004]"));
      return ((ATTimeOrDerived*)atom1.get())->compare((const ATTimeOrDerived::Ptr )atom2, context) < 0;
    }
    case AnyAtomicType::DATE_TIME:
    {
      // op:datetime-less-than(A, B)
      if(atom2->getPrimitiveTypeIndex() != AnyAtomicType::DATE_TIME)
        XQThrow2(XPath2ErrorException,X("LessThan::less_than"), X("An attempt to compare a dateTime type to a non dateTime type has occurred [err:XPTY0004]"));
      return ((ATDateTimeOrDerived*)atom1.get())->compare((const ATDateTimeOrDerived::Ptr)atom2, context) < 0;
    }
    case AnyAtomicType::DAY_TIME_DURATION:
    {
      // op:dayTimeDuration-less-than(A, B)
      if(atom2->getPrimitiveTypeIndex() != AnyAtomicType::DAY_TIME_DURATION)
        XQThrow2(XPath2ErrorException,X("LessThan::less_than"), X("An attempt to compare a duration type to a non duration type has occurred [err:XPTY0004]"));
      return ((ATDurationOrDerived*)atom1.get())->compare((const ATDurationOrDerived::Ptr )atom2, context) < 0;
    }
    case AnyAtomicType::YEAR_MONTH_DURATION:
    {
      // op:yearMonthDuration-less-than(A, B)
      if(atom2->getPrimitiveTypeIndex() != AnyAtomicType::YEAR_MONTH_DURATION)
        XQThrow2(XPath2ErrorException,X("LessThan::less_than"), X("An attempt to compare a duration type to a non duration type has occurred [err:XPTY0004]"));
      return ((ATDurationOrDerived*)atom1.get())->compare((const ATDurationOrDerived::Ptr )atom2, context) < 0;
    }
    default:
      XQThrow2(XPath2ErrorException,X("LessThan::less_than"), X("Unexpected data type in operator 'lt' [err:XPTY0004]"));
    }
    XQThrow2(FunctionException,X("LessThan::less_than"), X("An equality operator is not defined for the provided arguments [err:XPTY0004]"));
  }
  catch(XQException &e) {
      if(e.getXQueryLine() == 0)
        e.setXQueryPosition(info);
      throw;
  }
}

bool LessThan::execute(const AnyAtomicType::Ptr &atom1, const AnyAtomicType::Ptr &atom2, DynamicContext *context) const
{
  return less_than(atom1, atom2, 0, context, this);
}

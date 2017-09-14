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
#include <xqilla/operators/Plus.hpp>
#include <xqilla/operators/Minus.hpp>
#include <xqilla/exceptions/XPath2ErrorException.hpp>
#include <xqilla/items/ATDurationOrDerived.hpp>
#include <xqilla/items/ATDateTimeOrDerived.hpp>
#include <xqilla/items/ATDateOrDerived.hpp>
#include <xqilla/items/ATTimeOrDerived.hpp>
#include <xqilla/items/Numeric.hpp>
#include <xqilla/context/DynamicContext.hpp>

XERCES_CPP_NAMESPACE_USE;

const XMLCh Plus::name[]={ chLatin_P, chLatin_l, chLatin_u, chLatin_s, chNull };

Plus::Plus(const VectorOfASTNodes &args, XPath2MemoryManager* memMgr)
  : ArithmeticOperator(name, args, memMgr)
{
}

void Plus::calculateStaticType()
{
  const StaticType &arg0 = _args[0]->getStaticAnalysis().getStaticType();
  const StaticType &arg1 = _args[1]->getStaticAnalysis().getStaticType();

  calculateStaticTypeForNumerics(arg0, arg1);

  // Adding a duration to a date, dateTime, time, or duration
  if(arg1.containsType(StaticType::DAY_TIME_DURATION_TYPE)) {
    _src.getStaticType() |= arg0 & (StaticType::DATE_TYPE|StaticType::DATE_TIME_TYPE|StaticType::TIME_TYPE|
                                                StaticType::DAY_TIME_DURATION_TYPE);
  }
  if(arg1.containsType(StaticType::YEAR_MONTH_DURATION_TYPE)) {
    _src.getStaticType() |= arg0 & (StaticType::DATE_TYPE|StaticType::DATE_TIME_TYPE|StaticType::YEAR_MONTH_DURATION_TYPE);
  }
  if(arg0.containsType(StaticType::DAY_TIME_DURATION_TYPE)) {
    _src.getStaticType() |= arg1 & (StaticType::DATE_TYPE|StaticType::DATE_TIME_TYPE|StaticType::TIME_TYPE|
                                                StaticType::DAY_TIME_DURATION_TYPE);
  }
  if(arg0.containsType(StaticType::YEAR_MONTH_DURATION_TYPE)) {
    _src.getStaticType() |= arg1 & (StaticType::DATE_TYPE|StaticType::DATE_TIME_TYPE|StaticType::YEAR_MONTH_DURATION_TYPE);
  }
}

Item::Ptr Plus::execute(const AnyAtomicType::Ptr &atom1, const AnyAtomicType::Ptr &atom2, DynamicContext *context) const
{
  return plus(atom1, atom2, context, this);
}

Item::Ptr Plus::plus(const AnyAtomicType::Ptr &atom1, const AnyAtomicType::Ptr &atom2, DynamicContext *context, const LocationInfo *info)
{
  try {
    if(atom1 == NULLRCP || atom2 == NULLRCP) return 0;

    if(atom1->isNumericValue()) {
      if(atom2->isNumericValue()) {
        return ((Numeric*)atom1.get())->add((Numeric*)atom2.get(), context);
      }
      else {
        XQThrow2(XPath2ErrorException,X("Plus::createSequence"), X("An attempt to add a non numeric type to a numeric type has occurred [err:XPTY0004]"));
      }
    }

    switch(atom1->getPrimitiveTypeIndex()) {
    case AnyAtomicType::DATE : {
      switch(atom2->getPrimitiveTypeIndex()) {
      case AnyAtomicType::DAY_TIME_DURATION: {
        return ((ATDateOrDerived*)atom1.get())->addDayTimeDuration((const ATDurationOrDerived*)atom2.get(), context);
      }
      case AnyAtomicType::YEAR_MONTH_DURATION: {
        return ((ATDateOrDerived*)atom1.get())->addYearMonthDuration((const ATDurationOrDerived*)atom2.get(), context);
      }
      default: {
        XQThrow2(XPath2ErrorException,X("Plus::createSequence"), X("An invalid attempt to add to xs:date type has occurred [err:XPTY0004]"));
      }
      }
    }
    case AnyAtomicType::TIME : {
      switch(atom2->getPrimitiveTypeIndex()) {
      case AnyAtomicType::DAY_TIME_DURATION: {
        return ((ATTimeOrDerived*)atom1.get())->addDayTimeDuration((const ATDurationOrDerived*)atom2.get(), context );
      }
      default: {
        XQThrow2(XPath2ErrorException,X("Plus::createSequence"), X("An invalid attempt to add to xs:time type has occurred [err:XPTY0004]"));
      }
      }
    }
    case AnyAtomicType::DATE_TIME : {
      switch(atom2->getPrimitiveTypeIndex()) {
      case AnyAtomicType::DAY_TIME_DURATION: {
        return ((ATDateTimeOrDerived*)atom1.get())->addDayTimeDuration((const ATDurationOrDerived*)atom2.get(), context);
      }
      case AnyAtomicType::YEAR_MONTH_DURATION: {
        return ((ATDateTimeOrDerived*)atom1.get())->addYearMonthDuration((const ATDurationOrDerived*)atom2.get(), context);
      }
      default: {
        XQThrow2(XPath2ErrorException,X("Plus::createSequence"), X("An invalid attempt to add to xs:dateTime type has occurred [err:XPTY0004]"));
      }
      }
    }
    case AnyAtomicType::DAY_TIME_DURATION: {
      switch(atom2->getPrimitiveTypeIndex()) {
      case AnyAtomicType::DAY_TIME_DURATION: {
        return ((ATDurationOrDerived*)atom2.get())->add((const ATDurationOrDerived *)atom1.get(), context);
      }
      case AnyAtomicType::DATE: {
        return ((ATDateOrDerived*)atom2.get())->addDayTimeDuration((const ATDurationOrDerived *)atom1.get(), context);
      }
      case AnyAtomicType::DATE_TIME: {
        return ((ATDateTimeOrDerived*)atom2.get())->addDayTimeDuration((const ATDurationOrDerived *)atom1.get(), context);
      }
      case AnyAtomicType::TIME: {
        return ((ATTimeOrDerived*)atom2.get())->addDayTimeDuration((const ATDurationOrDerived *)atom1.get(), context);
      }
      default: {
        XQThrow2(XPath2ErrorException,X("Plus::createSequence"), X("An invalid attempt to add to xdt:dayTimeDuration type has occurred [err:XPTY0004]"));
      }
      }
    }
    case AnyAtomicType::YEAR_MONTH_DURATION: {
      switch(atom2->getPrimitiveTypeIndex()) {
      case AnyAtomicType::YEAR_MONTH_DURATION: {
        return ((ATDurationOrDerived*)atom2.get())->add((const ATDurationOrDerived *)atom1.get(), context);
      }
      case AnyAtomicType::DATE: {
        return ((ATDateOrDerived*)atom2.get())->addYearMonthDuration((const ATDurationOrDerived *)atom1.get(), context);
      }
      case AnyAtomicType::DATE_TIME: {
        return ((ATDateTimeOrDerived*)atom2.get())->addYearMonthDuration((const ATDurationOrDerived *)atom1.get(), context);
      }
      default: {
        XQThrow2(XPath2ErrorException,X("Plus::createSequence"), X("An invalid attempt to add to xdt:yearMonthDuration type has occurred [err:XPTY0004]"));
      }
      }
    }
    default: {
      XQThrow2(XPath2ErrorException,X("Plus::createSequence"), X("The operator add ('+') has been called on invalid operand types [err:XPTY0004]"));
    }
    }
  }
  catch(XQException &e) {
    if(e.getXQueryLine() == 0)
      e.setXQueryPosition(info);
    throw;
  }
}


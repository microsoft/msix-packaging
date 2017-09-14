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
#include <xqilla/operators/IntegerDivide.hpp>
#include <xqilla/items/ATDecimalOrDerived.hpp>
#include <xqilla/exceptions/XPath2ErrorException.hpp>
#include <xqilla/context/DynamicContext.hpp>

#include <xercesc/validators/schema/SchemaSymbols.hpp>

/*static*/ const XMLCh IntegerDivide::name[]={ XERCES_CPP_NAMESPACE_QUALIFIER chLatin_I, XERCES_CPP_NAMESPACE_QUALIFIER chLatin_D, XERCES_CPP_NAMESPACE_QUALIFIER chLatin_i, XERCES_CPP_NAMESPACE_QUALIFIER chLatin_v, XERCES_CPP_NAMESPACE_QUALIFIER chNull };

IntegerDivide::IntegerDivide(const VectorOfASTNodes &args, XPath2MemoryManager* memMgr)
  : ArithmeticOperator(name, args, memMgr)
{
  // Nothing to do
}

void IntegerDivide::calculateStaticType()
{
  const StaticType &arg0 = _args[0]->getStaticAnalysis().getStaticType();
  // untypedAtomic will be promoted to xs:double
  if(arg0.containsType(StaticType::NUMERIC_TYPE|StaticType::UNTYPED_ATOMIC_TYPE)) {
    _src.getStaticType() = StaticType(StaticType::DECIMAL_TYPE, 0, 1);
  }
}

Item::Ptr IntegerDivide::execute(const AnyAtomicType::Ptr &atom1, const AnyAtomicType::Ptr &atom2, DynamicContext *context) const
{
  if(atom1 == NULLRCP || atom2 == NULLRCP) return 0;

  if(atom1->isNumericValue() && atom2->isNumericValue()) {
    const Numeric::Ptr op1=atom1;
    const Numeric::Ptr op2=atom2;
    if(op2->isZero())
      XQThrow(XPath2ErrorException,X("IntegerDivide::execute"), X("Division by zero [err:FOAR0001]"));

    if(op1->isNaN() || op1->isInfinite() || op2->isNaN())
      XQThrow(XPath2ErrorException,X("IntegerDivide::execute"), X("Either operand is NaN or the dividend is INF or -INF [err:FOAR0002]"));

    // it divides the first argument by the second, and returns the integer obtained by truncating the fractional part 
    // of the result.
    Numeric::Ptr div=op1->divide(op2, context);
    if(div->isPositive()) 
      div = div->floor(context);
    else if(div->isNegative()) 
      div = div->ceiling(context);

    return div->castAs(AnyAtomicType::DECIMAL,
                       XERCES_CPP_NAMESPACE_QUALIFIER SchemaSymbols::fgURI_SCHEMAFORSCHEMA,
                       XERCES_CPP_NAMESPACE_QUALIFIER SchemaSymbols::fgDT_INTEGER, context);
  }
  XQThrow(XPath2ErrorException,X("IntegerDivide::execute"), X("An attempt to integer divide a non numeric type to a numeric type has occurred [err:XPTY0004]"));
}


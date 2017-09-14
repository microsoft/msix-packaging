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
#include <xqilla/operators/LessThanEqual.hpp>
#include <xqilla/operators/GreaterThan.hpp>
#include <xqilla/items/ATBooleanOrDerived.hpp>
#include <xqilla/context/impl/CodepointCollation.hpp>
#include <xqilla/context/DynamicContext.hpp>
#include <xqilla/exceptions/XPath2ErrorException.hpp>

/*static*/ const XMLCh LessThanEqual::name[]={ XERCES_CPP_NAMESPACE_QUALIFIER chLatin_L, XERCES_CPP_NAMESPACE_QUALIFIER chLatin_E, XERCES_CPP_NAMESPACE_QUALIFIER chNull };

LessThanEqual::LessThanEqual(const VectorOfASTNodes &args, XPath2MemoryManager* memMgr)
  : ComparisonOperator(name, args, memMgr)
{
}

/*static*/ bool LessThanEqual::less_than_equal(const AnyAtomicType::Ptr &arg1, const AnyAtomicType::Ptr &arg2, Collation* collation, DynamicContext* context, const LocationInfo *info)
{
  // A le B numeric               numeric                 op:numeric-less-than(A, B) or op:numeric-equal(A, B)
  // A le B xs:boolean            xs:boolean              fn:not(op:boolean-greater-than(A, B))
  // A le B xs:string             xs:string               op:numeric-less-than(fn:compare(A, B), 1)
  // A le B xs:date               xs:date                 fn:not(op:date-greater-than(A, B))
  // A le B xs:time               xs:time                 fn:not(op:time-greater-than(A, B))
  // A le B xs:dateTime           xs:dateTime             fn:not(op:datetime-greater-than(A, B))
  // A le B xdt:yearMonthDuration xdt:yearMonthDuration   fn:not(op:yearMonthDuration-greater-than(A, B))
  // A le B xdt:dayTimeDuration   xdt:dayTimeDuration     fn:not(op:dayTimeDuration-greater-than(A, B))
  // numeric values need a special comparison, for the others we can just rely on GreaterThan
  if(arg1->isNumericValue() && arg2->isNumericValue()) {
    if(((Numeric*)arg1.get())->getState() == Numeric::NaN ||
       ((Numeric*)arg2.get())->getState() == Numeric::NaN) return false;
  }

  return !GreaterThan::greater_than(arg1,arg2,collation,context,info);
}

bool LessThanEqual::execute(const AnyAtomicType::Ptr &atom1, const AnyAtomicType::Ptr &atom2, DynamicContext *context) const
{
  return less_than_equal(atom1, atom2, 0, context, this);
}

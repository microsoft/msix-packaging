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
#include <sstream>

#include <xqilla/operators/ComparisonOperator.hpp>
#include <xercesc/validators/schema/SchemaSymbols.hpp>
#include <xqilla/exceptions/XPath2TypeCastException.hpp>
#include <xqilla/items/Item.hpp>
#include <xqilla/items/AnyAtomicType.hpp>
#include <xqilla/items/ATBooleanOrDerived.hpp>
#include <xqilla/context/DynamicContext.hpp>
#include <xqilla/items/DatatypeFactory.hpp>
#include <xqilla/context/ItemFactory.hpp>
#include <xqilla/ast/XQAtomize.hpp>
#include <xqilla/ast/XQTreatAs.hpp>
#include <xqilla/schema/SequenceType.hpp>
#include <xqilla/ast/ConvertFunctionArg.hpp>
#include <xqilla/ast/XQSequence.hpp>
#include <xqilla/exceptions/StaticErrorException.hpp>
#include <xqilla/runtime/ClosureResult.hpp>

#if defined(XERCES_HAS_CPP_NAMESPACE)
XERCES_CPP_NAMESPACE_USE
#endif

ComparisonOperator::ComparisonOperator(const XMLCh* opName, const VectorOfASTNodes &args, XPath2MemoryManager* memMgr)
  : XQOperator(opName, args, memMgr)
{
}

ASTNode* ComparisonOperator::staticResolution(StaticContext *context)
{
  XPath2MemoryManager *mm = context->getMemoryManager();

  for(VectorOfASTNodes::iterator i = _args.begin(); i != _args.end(); ++i) {
    // Value comparisons are intended for comparing single values. The result of a value comparison is
    // defined by applying the following rules, in order:

    // 1. Atomization is applied to each operand. If the result, called an atomized operand, does not contain
    //    exactly one atomic value, a type error is raised.
    *i = new (mm) XQAtomize(*i, mm);
    (*i)->setLocationInfo(this);

    // 2. If the atomized operand is an empty sequence, the result of the value comparison is an empty sequence, 
    //    and the implementation need not evaluate the other operand or apply the operator. However, an 
    //    implementation may choose to evaluate the other operand in order to determine whether it raises an error.
    // 3. If the atomized operand is a sequence of length greater than one, a type error is raised [err:XPTY0004].
    SequenceType *seqType = new (mm) SequenceType(new (mm) SequenceType::ItemType(SequenceType::ItemType::TEST_ANYTHING),
                                                  SequenceType::QUESTION_MARK);
    seqType->setLocationInfo(*i);

    *i = new (mm) XQTreatAs(*i, seqType, mm);
    (*i)->setLocationInfo(this);

    // 4. Any atomized operand that has the dynamic type xdt:untypedAtomic is cast to the type xs:string.
    *i = new (mm) XQPromoteUntyped(*i, SchemaSymbols::fgURI_SCHEMAFORSCHEMA,
                                   SchemaSymbols::fgDT_STRING, mm);
    (*i)->setLocationInfo(this);

    *i = (*i)->staticResolution(context);
  }

  return this;
}

ASTNode *ComparisonOperator::staticTypingImpl(StaticContext *context)
{
  _src.clear();

  bool emptyArgument = false;
  for(VectorOfASTNodes::iterator i = _args.begin(); i != _args.end(); ++i) {
    _src.add((*i)->getStaticAnalysis());

    if(context && (*i)->getStaticAnalysis().getStaticType().getMax() == 0) {
      // The result is always empty if one of our arguments is always empty
      XPath2MemoryManager* mm = context->getMemoryManager();
      ASTNode *result = new (mm) XQSequence(mm);
      result->setLocationInfo(this);
      return result->staticTypingImpl(context);
    }

    if((*i)->getStaticAnalysis().getStaticType().getMin() == 0)
      emptyArgument = true;

    if((*i)->getStaticAnalysis().isUpdating()) {
      XQThrow(StaticErrorException,X("ComparisonOperator::staticTyping"),
              X("It is a static error for an operand of an operator "
                "to be an updating expression [err:XUST0001]"));
    }

    if((*i)->isDateOrTimeAndHasNoTimezone(context))
      _src.implicitTimezoneUsed(true);
  }

  if(emptyArgument)
    _src.getStaticType() = StaticType(StaticType::BOOLEAN_TYPE, 0, 1);
  else _src.getStaticType() = StaticType(StaticType::BOOLEAN_TYPE, 1, 1);

  return this;
}

BoolResult ComparisonOperator::boolResult(DynamicContext* context) const
{
  try {
    AnyAtomicType::Ptr left = (AnyAtomicType*)getArguments()[0]->createResult(context)->next(context).get();
    if(left.isNull()) return BoolResult::Null;
    AnyAtomicType::Ptr right = (AnyAtomicType*)getArguments()[1]->createResult(context)->next(context).get();
    if(right.isNull()) return BoolResult::Null;
    return execute(left, right, context);
  }
  catch(XQException &e) {
    if(e.getXQueryLine() == 0)
      e.setXQueryPosition(this);
    throw;
  }
}

Result ComparisonOperator::createResult(DynamicContext* context, int flags) const
{
  return (Item::Ptr)context->getItemFactory()->createBoolean(boolResult(context), context);
}

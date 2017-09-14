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
#include <sstream>

#include <xqilla/operators/ArithmeticOperator.hpp>
#include <xercesc/validators/schema/SchemaSymbols.hpp>
#include <xqilla/context/DynamicContext.hpp>
#include <xqilla/exceptions/XPath2TypeCastException.hpp>
#include <xqilla/items/Item.hpp>
#include <xqilla/items/AnyAtomicType.hpp>
#include <xqilla/ast/XQAtomize.hpp>
#include <xqilla/exceptions/XPath2ErrorException.hpp>
#include <xqilla/ast/XQTreatAs.hpp>
#include <xqilla/schema/SequenceType.hpp>
#include <xqilla/ast/ConvertFunctionArg.hpp>
#include <xqilla/ast/XQSequence.hpp>
#include <xqilla/exceptions/StaticErrorException.hpp>

#include <xercesc/framework/XMLBuffer.hpp>

#if defined(XERCES_HAS_CPP_NAMESPACE)
XERCES_CPP_NAMESPACE_USE
#endif

ArithmeticOperator::ArithmeticOperator(const XMLCh* opName, const VectorOfASTNodes &args, XPath2MemoryManager* memMgr)
  : XQOperator(opName, args, memMgr)
{
}

ASTNode* ArithmeticOperator::staticResolution(StaticContext *context)
{
  XPath2MemoryManager *mm = context->getMemoryManager();

  for(VectorOfASTNodes::iterator i = _args.begin(); i != _args.end(); ++i) {
    // An arithmetic expression is evaluated by applying the following rules, in order,
    // until an error is raised or a value is computed:

    // 1. Atomization is applied to each operand.

    *i = new (mm) XQAtomize(*i, mm);
    (*i)->setLocationInfo(this);

    // 2. If either operand is an empty sequence, the result of the operation is an empty sequence.
    // 3. If either operand is now a sequence of length greater than one, then:
    //      * If XPath 1.0 compatibility mode is true, any items after the first item in the sequence are discarded.
    //        Otherwise, a type error is raised.

    if(!context->getXPath1CompatibilityMode()) {
      SequenceType *seqType = new (mm) SequenceType(new (mm) SequenceType::ItemType(SequenceType::ItemType::TEST_ANYTHING),
                                                    SequenceType::QUESTION_MARK);
      seqType->setLocationInfo(*i);

      *i = new (mm) XQTreatAs(*i, seqType, mm);
      (*i)->setLocationInfo(this);
    }

    // 4. If either operand is now of type xdt:untypedAtomic, it is cast to the default type for the given operator.
    //    If the cast fails, a type error is raised.

    *i = new (mm) XQPromoteUntyped(*i, SchemaSymbols::fgURI_SCHEMAFORSCHEMA,
                                   SchemaSymbols::fgDT_DOUBLE, mm);
    (*i)->setLocationInfo(this);

    *i = (*i)->staticResolution(context);
  }

  return this;
}

ASTNode *ArithmeticOperator::staticTypingImpl(StaticContext *context)
{
  _src.clear();

  bool emptyArgument = false;
  for(VectorOfASTNodes::iterator i = _args.begin(); i != _args.end(); ++i) {
    if((*i)->getStaticAnalysis().isUpdating()) {
      XQThrow(StaticErrorException,X("ArithmeticOperator::staticTyping"),
              X("It is a static error for an operand of an operator "
                "to be an updating expression [err:XUST0001]"));
    }

    if((*i)->getStaticAnalysis().getStaticType().getMin() == 0)
      emptyArgument = true;
    _src.add((*i)->getStaticAnalysis());

    if((*i)->isDateOrTimeAndHasNoTimezone(context))
      _src.implicitTimezoneUsed(true);
  }

  _src.getStaticType() = StaticType();
  calculateStaticType();

  if(!emptyArgument && _src.getStaticType().getMax() == 0) {
    XMLBuffer errMsg;
    errMsg.set(X("The operator "));
    errMsg.append(_opName);
    errMsg.append(X(" has been called on invalid operand types [err:XPTY0004]"));
    XQThrow(XPath2ErrorException,X("ArithmeticOperator::staticResolution"), errMsg.getRawBuffer());
  }

  if(emptyArgument)
    _src.getStaticType().setCardinality(0, 1);
  else _src.getStaticType().setCardinality(1, 1);

  return this;
}

void ArithmeticOperator::calculateStaticTypeForNumerics(const StaticType &arg0, const StaticType &arg1)
{
  // Deal with numerics and numeric type promotion
  if(arg0.containsType(StaticType::DECIMAL_TYPE)) {
    if(arg1.containsType(StaticType::DECIMAL_TYPE)) {
      _src.getStaticType() |= StaticType::DECIMAL_TYPE;
    }
    if(arg1.containsType(StaticType::FLOAT_TYPE)) {
      _src.getStaticType() |= StaticType::FLOAT_TYPE;
    }
    // untypedAtomic will be promoted to xs:double
    if(arg1.containsType(StaticType::DOUBLE_TYPE|StaticType::UNTYPED_ATOMIC_TYPE)) {
      _src.getStaticType() |= StaticType::DOUBLE_TYPE;
    }
  }
  if(arg0.containsType(StaticType::FLOAT_TYPE)) {
    if(arg1.containsType(StaticType::DECIMAL_TYPE|StaticType::FLOAT_TYPE)) {
      _src.getStaticType() |= StaticType::FLOAT_TYPE;
    }
    // untypedAtomic will be promoted to xs:double
    if(arg1.containsType(StaticType::DOUBLE_TYPE|StaticType::UNTYPED_ATOMIC_TYPE)) {
      _src.getStaticType() |= StaticType::DOUBLE_TYPE;
    }
  }
  // untypedAtomic will be promoted to xs:double
  if(arg0.containsType(StaticType::DOUBLE_TYPE|StaticType::UNTYPED_ATOMIC_TYPE)) {
    if(arg1.containsType(StaticType::DECIMAL_TYPE|StaticType::FLOAT_TYPE|StaticType::DOUBLE_TYPE|StaticType::UNTYPED_ATOMIC_TYPE)) {
      _src.getStaticType() |= StaticType::DOUBLE_TYPE;
    }
  }
}

Result ArithmeticOperator::createResult(DynamicContext* context, int flags) const
{
  try {
    return execute((AnyAtomicType*)getArguments()[0]->createResult(context)->next(context).get(),
      getNumArgs() == 1 ? 0 : (AnyAtomicType*)getArguments()[1]->createResult(context)->next(context).get(),
      context);
  }
  catch(XQException &e) {
    if(e.getXQueryLine() == 0)
      e.setXQueryPosition(this);
    throw;
  }
}


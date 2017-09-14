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
#include <xqilla/ast/NumericFunction.hpp>
#include <xqilla/exceptions/FunctionException.hpp>
#include <xqilla/items/AnyAtomicType.hpp>
#include <xqilla/functions/FunctionSignature.hpp>
#include <xqilla/schema/SequenceType.hpp>

#if defined(XERCES_HAS_CPP_NAMESPACE)
XERCES_CPP_NAMESPACE_USE
#endif

NumericFunction::NumericFunction(const XMLCh* name, const char *signature, const VectorOfASTNodes &args, XPath2MemoryManager* memMgr)
  : XQFunction(name, signature, args, memMgr)
{ 
}

ASTNode* NumericFunction::staticResolution(StaticContext *context)
{
  resolveArguments(context, /*numericFunction*/true);
  return this;
}

ASTNode *NumericFunction::staticTypingImpl(StaticContext *context)
{
  _src.clear();
  calculateSRCForArguments(context);

  if(!_args[0]->getStaticAnalysis().getStaticType().containsType(StaticType::NUMERIC_TYPE) &&
     _args[0]->getStaticAnalysis().getStaticType().getMin() > 0)
    XQThrow(FunctionException,X("NumericFunction::staticTyping"), X("Non-numeric argument in numeric function [err:XPTY0004]"));

  _src.getStaticType() = StaticType(StaticType::NUMERIC_TYPE, 0, 1);
  return this;
}

Numeric::Ptr NumericFunction::getNumericParam(unsigned int number, DynamicContext *context, int flags) const
{
  Item::Ptr item = XQFunction::getParamNumber(number, context, flags)->next(context);

  if(item.notNull() && (!item->isAtomicValue() || !((const AnyAtomicType *)item.get())->isNumericValue()))
    XQThrow(FunctionException,X("NumericFunction::getParamNumber"), X("Non-numeric argument in numeric function [err:XPTY0004]"));

  return (const Numeric *)item.get();
}


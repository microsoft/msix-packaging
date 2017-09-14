/*
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

#include <xqilla/functions/FunctionTail.hpp>
#include <xqilla/runtime/Result.hpp>
#include <xqilla/context/DynamicContext.hpp>

XERCES_CPP_NAMESPACE_USE;

const XMLCh FunctionTail::name[] = { 't', 'a', 'i', 'l', 0 };
const unsigned int FunctionTail::minArgs = 1;
const unsigned int FunctionTail::maxArgs = 1;

/**
 * fn:tail($target as item()*) as item()*
 */

FunctionTail::FunctionTail(const VectorOfASTNodes &args, XPath2MemoryManager* memMgr)
  : XQFunction(name, "($target as item()*) as item()*", args, memMgr)
{
}

ASTNode *FunctionTail::staticTypingImpl(StaticContext *context)
{
  _src.clearExceptType();
  calculateSRCForArguments(context);

  _src.getStaticType() = _args[0]->getStaticAnalysis().getStaticType();

  unsigned int min = _src.getStaticType().getMin();
  if(min > 0) --min;

  unsigned int max = _src.getStaticType().getMax();
  if(max > 0 && max != StaticType::UNLIMITED) --max;

  _src.getStaticType().setCardinality(min, max);

  return this;
}

Result FunctionTail::createResult(DynamicContext* context, int flags) const
{
  Result result = getParamNumber(1, context);
  result->skip(1, context);
  return result;
}

const XMLCh FunctionDrop::name[] = { 'd', 'r', 'o', 'p', 0 };
const unsigned int FunctionDrop::minArgs = 2;
const unsigned int FunctionDrop::maxArgs = 2;

/**
 * xqilla:drop($target as item()*, $count as xs:double) as item()*
 */

FunctionDrop::FunctionDrop(const VectorOfASTNodes &args, XPath2MemoryManager* memMgr)
  : XQillaFunction(name, "($target as item()*, $count as xs:double) as item()*", args, memMgr)
{
}

ASTNode *FunctionDrop::staticTypingImpl(StaticContext *context)
{
  _src.clearExceptType();
  calculateSRCForArguments(context);

  _src.getStaticType() = _args[0]->getStaticAnalysis().getStaticType();
  _src.getStaticType().setCardinality(0, _src.getStaticType().getMax());

  return this;
}

Result FunctionDrop::createResult(DynamicContext* context, int flags) const
{
  Numeric::Ptr count = (Numeric*)getParamNumber(2, context)->next(context).get();
  Result result = getParamNumber(1, context);
  result->skip(count->asInt(), context);
  return result;
}


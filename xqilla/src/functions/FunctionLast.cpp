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
#include <xqilla/functions/FunctionLast.hpp>
#include <xqilla/context/DynamicContext.hpp>
#include <xqilla/items/ATDecimalOrDerived.hpp>
#include <xqilla/items/DatatypeFactory.hpp>
#include <xqilla/exceptions/DynamicErrorException.hpp>
#include <xqilla/ast/StaticAnalysis.hpp>

XERCES_CPP_NAMESPACE_USE;

const XMLCh FunctionLast::name[] = {
  chLatin_l, chLatin_a, chLatin_s, 
  chLatin_t, chNull 
};
const unsigned int FunctionLast::minArgs = 0;
const unsigned int FunctionLast::maxArgs = 0;

/**
 * fn:last() as xs:integer
**/

FunctionLast::FunctionLast(const VectorOfASTNodes &args, XPath2MemoryManager* memMgr)
  : XQFunction(name, "() as xs:integer", args, memMgr)
{
}

ASTNode *FunctionLast::staticTypingImpl(StaticContext *context)
{
  _src.clearExceptType();

  if(context && !context->getContextItemType().containsType(StaticType::ITEM_TYPE)) {
    XQThrow(DynamicErrorException,X("XQContextItem::staticTyping"),
            X("It is an error for the context item to be undefined when using it [err:XPDY0002]"));
  }

  _src.contextSizeUsed(true);
  calculateSRCForArguments(context);
  return this;
}

Sequence FunctionLast::createSequence(DynamicContext* context, int flags) const
{
  XPath2MemoryManager* memMgr = context->getMemoryManager();

  if(context->getContextItem()==NULLRCP)
    XQThrow(DynamicErrorException,X("FunctionPosition::createSequence"), X("Undefined context item in fn:last [err:XPDY0002]"));
  return Sequence(context->getItemFactory()->createInteger((long)context->getContextSize(), context), memMgr);
}

















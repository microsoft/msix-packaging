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
#include <xqilla/functions/FunctionPosition.hpp>
#include <xqilla/context/DynamicContext.hpp>
#include <xqilla/items/ATDecimalOrDerived.hpp>
#include <xqilla/items/DatatypeFactory.hpp>
#include <xqilla/exceptions/DynamicErrorException.hpp>
#include <xqilla/ast/StaticAnalysis.hpp>
#include <xqilla/runtime/ResultImpl.hpp>

XERCES_CPP_NAMESPACE_USE;

const XMLCh FunctionPosition::name[] = {
  chLatin_p, chLatin_o, chLatin_s, 
  chLatin_i, chLatin_t, chLatin_i, 
  chLatin_o, chLatin_n, chNull 
};
const unsigned int FunctionPosition::minArgs = 0;
const unsigned int FunctionPosition::maxArgs = 0;

/**
 * fn:position() as xs:integer
 */

FunctionPosition::FunctionPosition(const VectorOfASTNodes &args, XPath2MemoryManager* memMgr)
  : XQFunction(name, "() as xs:integer", args, memMgr)
{
}

ASTNode *FunctionPosition::staticTypingImpl(StaticContext *context)
{
  _src.clearExceptType();

  if(context && !context->getContextItemType().containsType(StaticType::ITEM_TYPE)) {
    XQThrow(DynamicErrorException,X("XQContextItem::staticTyping"),
            X("It is an error for the context item to be undefined when using it [err:XPDY0002]"));
  }

  _src.contextPositionUsed(true);
  calculateSRCForArguments(context);
  return this;
}

Result FunctionPosition::createResult(DynamicContext* context, int flags) const
{
  if(context->getContextItem().isNull())
    XQThrow(DynamicErrorException,X("FunctionPosition::createSequence"),
            X("Undefined context item in fn:position [err:XPDY0002]"));
  return (Item::Ptr)context->getItemFactory()->createInteger((long)context->getContextPosition(), context);
}

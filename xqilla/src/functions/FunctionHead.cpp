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

#include <xqilla/functions/FunctionHead.hpp>
#include <xqilla/runtime/Result.hpp>
#include <xqilla/context/DynamicContext.hpp>

XERCES_CPP_NAMESPACE_USE;

const XMLCh FunctionHead::name[] = {
  chLatin_h, chLatin_e, chLatin_a, 
  chLatin_d, chNull 
};
const unsigned int FunctionHead::minArgs = 1;
const unsigned int FunctionHead::maxArgs = 1;

/**
 * fn:head($target as item()*) as item()?
 */

FunctionHead::FunctionHead(const VectorOfASTNodes &args, XPath2MemoryManager* memMgr)
  : XQFunction(name, "($target as item()*) as item()?", args, memMgr)
{
}

ASTNode *FunctionHead::staticTypingImpl(StaticContext *context)
{
  _src.clearExceptType();
  calculateSRCForArguments(context);

  _src.getStaticType() = _args[0]->getStaticAnalysis().getStaticType();
  _src.getStaticType().setCardinality(_src.getStaticType().getMin() == 0 ? 0 : 1,
                                      _src.getStaticType().getMax() == 0 ? 0 : 1);

  return this;
}

Result FunctionHead::createResult(DynamicContext* context, int flags) const
{
  return getParamNumber(1, context)->next(context);
}



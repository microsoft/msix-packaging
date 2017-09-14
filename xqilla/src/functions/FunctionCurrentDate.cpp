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
#include <xqilla/functions/FunctionCurrentDate.hpp>
#include <xqilla/context/DynamicContext.hpp>
#include <xqilla/items/ATDateOrDerived.hpp>
#include "../utils/DateUtils.hpp"
#include <xqilla/ast/StaticAnalysis.hpp>

const XMLCh FunctionCurrentDate::name[] = {
  XERCES_CPP_NAMESPACE_QUALIFIER chLatin_c, XERCES_CPP_NAMESPACE_QUALIFIER chLatin_u, XERCES_CPP_NAMESPACE_QUALIFIER chLatin_r, 
  XERCES_CPP_NAMESPACE_QUALIFIER chLatin_r, XERCES_CPP_NAMESPACE_QUALIFIER chLatin_e, XERCES_CPP_NAMESPACE_QUALIFIER chLatin_n, 
  XERCES_CPP_NAMESPACE_QUALIFIER chLatin_t, XERCES_CPP_NAMESPACE_QUALIFIER chDash,    XERCES_CPP_NAMESPACE_QUALIFIER chLatin_d, 
  XERCES_CPP_NAMESPACE_QUALIFIER chLatin_a, XERCES_CPP_NAMESPACE_QUALIFIER chLatin_t, XERCES_CPP_NAMESPACE_QUALIFIER chLatin_e, 
  XERCES_CPP_NAMESPACE_QUALIFIER chNull 
};
const unsigned int FunctionCurrentDate::minArgs = 0;
const unsigned int FunctionCurrentDate::maxArgs = 0;

/**
 * fn:current-date() as xs:date
**/

FunctionCurrentDate::FunctionCurrentDate(const VectorOfASTNodes &args, XPath2MemoryManager* memMgr)
  : XQFunction(name, "() as xs:date", args, memMgr)
{
}

ASTNode *FunctionCurrentDate::staticTypingImpl(StaticContext *context)
{
  _src.clearExceptType();
  _src.currentTimeUsed(true);
  calculateSRCForArguments(context);
  return this;
}

Sequence FunctionCurrentDate::createSequence(DynamicContext* context, int flags) const
{
  return Sequence(DateUtils::getCurrentDate(context), context->getMemoryManager());
}

















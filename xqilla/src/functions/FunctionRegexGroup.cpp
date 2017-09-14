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
#include <xqilla/functions/FunctionRegexGroup.hpp>
#include <xqilla/context/DynamicContext.hpp>
#include <xqilla/ast/StaticAnalysis.hpp>
#include <xqilla/context/RegexGroupStore.hpp>

#include <xercesc/util/XMLUni.hpp>

XERCES_CPP_NAMESPACE_USE;
using namespace std;

const XMLCh FunctionRegexGroup::name[] = {
  chLatin_r, chLatin_e, chLatin_g, chLatin_e, chLatin_x, chDash, chLatin_g, chLatin_r, chLatin_o, chLatin_u, chLatin_p, chNull 
};
const unsigned int FunctionRegexGroup::minArgs = 1;
const unsigned int FunctionRegexGroup::maxArgs = 1;

/**
 * fn:regex-group($group-number as xs:integer) as xs:string  
 **/
FunctionRegexGroup::FunctionRegexGroup(const VectorOfASTNodes &args, XPath2MemoryManager* memMgr)
  : XQFunction(name, "($group-number as xs:integer) as xs:string", args, memMgr)
{
}

ASTNode *FunctionRegexGroup::staticTypingImpl(StaticContext *context)
{
  _src.clearExceptType();
  _src.forceNoFolding(true);
  calculateSRCForArguments(context);
  return this;
}

Sequence FunctionRegexGroup::createSequence(DynamicContext *context, int flags) const
{
  XPath2MemoryManager *mm = context->getMemoryManager();

  const RegexGroupStore *store = context->getRegexGroupStore();
  if(store == 0)
    return Sequence(context->getItemFactory()->createString(XMLUni::fgZeroLenString, context), mm);

  Numeric::Ptr indexItem = getParamNumber(1, context)->next(context);
  if(indexItem->isNegative())
    return Sequence(context->getItemFactory()->createString(XMLUni::fgZeroLenString, context), mm);

  const XMLCh *indexStr = indexItem->asString(context);

  int index = 0;
  while(*indexStr != 0) {
    if(*indexStr >= '0' && *indexStr <= '9') {
      index *= 10;
      index += *indexStr - '0';
    }
    ++indexStr;
  }

  const XMLCh *result = store->getGroup(index);
  if(result == 0) result = XMLUni::fgZeroLenString;

  return Sequence(context->getItemFactory()->createString(result, context), mm);
}

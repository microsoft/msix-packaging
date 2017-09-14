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
#include <xqilla/functions/FunctionStringLength.hpp>
#include <xqilla/functions/FunctionString.hpp>
#include <xqilla/utils/XPath2Utils.hpp>
#include <xqilla/context/DynamicContext.hpp>
#include <xqilla/items/ATStringOrDerived.hpp>
#include <xqilla/items/ATDecimalOrDerived.hpp>
#include <xqilla/items/DatatypeFactory.hpp>
#include <xqilla/ast/StaticAnalysis.hpp>

#include <xercesc/util/XMLString.hpp>
#include <xercesc/util/regx/RegxUtil.hpp>

XERCES_CPP_NAMESPACE_USE;

const XMLCh FunctionStringLength::name[] = {
  chLatin_s, chLatin_t, chLatin_r, 
  chLatin_i, chLatin_n, chLatin_g, 
  chDash,    chLatin_l, chLatin_e, 
  chLatin_n, chLatin_g, chLatin_t, 
  chLatin_h, chNull 
};
const unsigned int FunctionStringLength::minArgs = 0;
const unsigned int FunctionStringLength::maxArgs = 1;

/**
 * fn:string-length() as xs:integer
 * fn:string-length($arg as xs:string?) as xs:integer
**/

FunctionStringLength::FunctionStringLength(const VectorOfASTNodes &args, XPath2MemoryManager* memMgr)
  : XQFunction(name, "($arg as xs:string?) as xs:integer", args, memMgr)
{
}

ASTNode* FunctionStringLength::staticResolution(StaticContext *context)
{
  XPath2MemoryManager *mm = context->getMemoryManager();

  if(_args.empty()) {
    FunctionString *arg = new (mm) FunctionString(VectorOfASTNodes(XQillaAllocator<ASTNode*>(mm)), mm);
    arg->setLocationInfo(this);

    _args.push_back(arg);
  }

  resolveArguments(context);
  return this;
}

Sequence FunctionStringLength::createSequence(DynamicContext* context, int flags) const
{
  XPath2MemoryManager *mm = context->getMemoryManager();

  Item::Ptr strParm = getParamNumber(1,context)->next(context);
  if(strParm.isNull())
    return Sequence(context->getItemFactory()->createInteger(0, context), mm);

  const XMLCh *str = strParm->asString(context);

  long length = 0;
  while(*str) {
    ++length;
    if(RegxUtil::isHighSurrogate(*str)) ++str;
    ++str;
  }

  return Sequence(context->getItemFactory()->createInteger(length, context), mm);
}

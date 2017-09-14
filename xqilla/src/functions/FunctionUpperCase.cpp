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
#include <xqilla/functions/FunctionUpperCase.hpp>
#include <xqilla/context/DynamicContext.hpp>
#include <xqilla/utils/UnicodeTransformer.hpp>

#include <xercesc/util/XMLUni.hpp>

#if defined(XERCES_HAS_CPP_NAMESPACE)
XERCES_CPP_NAMESPACE_USE
#endif

const XMLCh FunctionUpperCase::name[] = {
  chLatin_u, chLatin_p, chLatin_p, 
  chLatin_e, chLatin_r, chDash, 
  chLatin_c, chLatin_a, chLatin_s, 
  chLatin_e, chNull 
};
const unsigned int FunctionUpperCase::minArgs = 1;
const unsigned int FunctionUpperCase::maxArgs = 1;

/*
 * fn:upper-case($arg as xs:string?) as xs:string
 */

FunctionUpperCase::FunctionUpperCase(const VectorOfASTNodes &args, XPath2MemoryManager* memMgr)
  : XQFunction(name, "($arg as xs:string?) as xs:string", args, memMgr)
{
}

Sequence FunctionUpperCase::createSequence(DynamicContext* context, int flags) const
{
  XPath2MemoryManager *memMgr = context->getMemoryManager();

  Item::Ptr arg = getParamNumber(1, context)->next(context);
  if(arg.isNull()) {
    return Sequence(context->getItemFactory()->createString(XMLUni::fgZeroLenString, context), memMgr);
  }

  AutoDeallocate<XMLCh> buf(UnicodeTransformer::upperCase(arg->asString(context), memMgr), memMgr);
  return Sequence(context->getItemFactory()->createString(buf.get(), context), memMgr);
}

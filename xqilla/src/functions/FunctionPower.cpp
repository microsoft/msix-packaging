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
#include <xqilla/functions/FunctionPower.hpp>

#include <xqilla/items/Numeric.hpp>
#include <xqilla/context/DynamicContext.hpp>

XERCES_CPP_NAMESPACE_USE

const XMLCh FunctionPower::name[] = {
  chLatin_p, chLatin_o, chLatin_w, chLatin_e,  chLatin_r,  chNull 
};
const unsigned int FunctionPower::minArgs = 2;
const unsigned int FunctionPower::maxArgs = 2;

/*
 * math:power($arg as numeric?, $arg as numeric?) as numeric?
 */
FunctionPower::FunctionPower(const VectorOfASTNodes &args, XPath2MemoryManager* memMgr)
  : EXSLTMathFunction(name, "($arg1 as xs:anyAtomicType?, $arg2 as xs:anyAtomicType?) as xs:anyAtomicType?", args, memMgr)
{
}

Result FunctionPower::createResult(DynamicContext* context, int flags) const
{
  Numeric::Ptr base = getNumericParam(1, context);
  if(base.isNull()) return 0;
  Numeric::Ptr pow = getNumericParam(2, context);
  if(pow.isNull()) return 0;
  return (Item::Ptr)base->power(pow, context);
}

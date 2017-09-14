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
#include <xqilla/functions/FunctionNormalizeSpace.hpp>
#include <xqilla/functions/FunctionString.hpp>
#include <xqilla/utils/XPath2Utils.hpp>
#include <xqilla/context/DynamicContext.hpp>
#include <xqilla/items/ATStringOrDerived.hpp>
#include <xqilla/items/DatatypeFactory.hpp>
#include <xercesc/util/XMLUni.hpp>
#include <xercesc/util/XMLString.hpp>
#include <xqilla/ast/StaticAnalysis.hpp>

XERCES_CPP_NAMESPACE_USE;

const XMLCh FunctionNormalizeSpace::name[] = {
  chLatin_n, chLatin_o, chLatin_r, 
  chLatin_m, chLatin_a, chLatin_l, 
  chLatin_i, chLatin_z, chLatin_e, 
  chDash,    chLatin_s, chLatin_p, 
  chLatin_a, chLatin_c, chLatin_e, 
  chNull 
};
const unsigned int FunctionNormalizeSpace::minArgs = 0;
const unsigned int FunctionNormalizeSpace::maxArgs = 1;

/**
 * fn:normalize-space() as xs:string
 * fn:normalize-space($arg as xs:string?) as xs:string
**/

FunctionNormalizeSpace::FunctionNormalizeSpace(const VectorOfASTNodes &args, XPath2MemoryManager* memMgr)
  : XQFunction(name, "($arg as xs:string?) as xs:string", args, memMgr)
{
}

ASTNode* FunctionNormalizeSpace::staticResolution(StaticContext *context)
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

Sequence FunctionNormalizeSpace::createSequence(DynamicContext* context, int flags) const
{
    XPath2MemoryManager *mm = context->getMemoryManager();

    Item::Ptr strParm = getParamNumber(1,context)->next(context);
    if(strParm.isNull())
      return Sequence(context->getItemFactory()->createString(XMLUni::fgZeroLenString, context), mm);

    const XMLCh *str = strParm->asString(context);

    // Skip leading whitespace
    while(*str) {
      XMLCh ch = *str;

      if((ch == 0x9) || (ch == 0xA) || (ch == 0xD) || (ch == 0x20)) {
        ++str;
        continue;
      }
      break;
    }

    XMLBuffer buf(XMLString::stringLen(str));

    // Compact whitespace, and skip trailing whitespace
    bool whitespace = false;
    while(*str) {
      XMLCh ch = *str;

      if((ch == 0x9) || (ch == 0xA) || (ch == 0xD) || (ch == 0x20)) {
        whitespace = true;
      }
      else {
        if(whitespace) buf.append(' ');
        buf.append(ch);
        whitespace = false;
      }
      ++str;
    }

    return Sequence(context->getItemFactory()->createString(buf.getRawBuffer(), context), mm);
}


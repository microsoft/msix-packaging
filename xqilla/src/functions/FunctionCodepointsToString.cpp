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
#include <xqilla/functions/FunctionCodepointsToString.hpp>
#include <xqilla/items/Item.hpp>
#include <xqilla/items/ATDecimalOrDerived.hpp>
#include <xqilla/items/ATStringOrDerived.hpp>
#include <xqilla/items/DatatypeFactory.hpp>
#include <xqilla/context/DynamicContext.hpp>
#include <xqilla/exceptions/XPath2ErrorException.hpp>
#include <xercesc/framework/XMLBuffer.hpp>
#include <xercesc/util/XMLChar.hpp>

XERCES_CPP_NAMESPACE_USE;

const XMLCh FunctionCodepointsToString::name[] = {
  chLatin_c, chLatin_o, chLatin_d, 
  chLatin_e, chLatin_p, chLatin_o, 
  chLatin_i, chLatin_n, chLatin_t, 
  chLatin_s, chDash,    chLatin_t, 
  chLatin_o, chDash,    chLatin_s, 
  chLatin_t, chLatin_r, chLatin_i, 
  chLatin_n, chLatin_g, chNull 
};
const unsigned int FunctionCodepointsToString::minArgs = 1;
const unsigned int FunctionCodepointsToString::maxArgs = 1;

/**
 * fn:codepoints-to-string($srcval as integer*) => string
 * Creates a string from a sequence of codepoints.
**/

FunctionCodepointsToString::FunctionCodepointsToString(const VectorOfASTNodes &args, XPath2MemoryManager* memMgr)
  : XQFunction(name, "($srcval as xs:integer*) as xs:string", args, memMgr) // 1 argument
{
}

Sequence FunctionCodepointsToString::createSequence(DynamicContext* context, int flags) const
{
  XMLBuffer result(1023, context->getMemoryManager());
  Sequence arg = getParamNumber(1,context)->toSequence(context);
  Sequence::iterator end = arg.end();
  for(Sequence::iterator i = arg.begin(); i != end; ++i) {
    XMLInt32 ch=((const ATDecimalOrDerived::Ptr)*i)->treatAsCodepoint(context);
	if ( ch >= 0x10000) 
    {
        XMLCh one, two;
        RegxUtil::decomposeToSurrogates(ch, one, two);
        result.append(one);
        result.append(two);
	}
    else
        result.append((XMLCh)ch);
  }
  unsigned int len = (unsigned int) result.getLen();
  const XMLCh* str=result.getRawBuffer();
  for(unsigned int j=0;j<len;j++)
  {
      if(RegxUtil::isHighSurrogate(str[j]))
        if((j+1)==len || !RegxUtil::isLowSurrogate(str[j+1]) || !XMLChar1_0::isXMLChar(str[j], str[j+1]))
          XQThrow(XPath2ErrorException, X("FunctionCodepointsToString::createSequence"), X("String contains an invalid XML character [err:FOCH0001]."));
        else
          j++;
      else if(!XMLChar1_0::isXMLChar(str[j]))
        XQThrow(XPath2ErrorException, X("FunctionCodepointsToString::createSequence"), X("String contains an invalid XML character [err:FOCH0001]."));
  }
  return Sequence(context->getItemFactory()->createString(str, context),
                  context->getMemoryManager());
}


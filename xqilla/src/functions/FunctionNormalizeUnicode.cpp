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
#include <xqilla/functions/FunctionNormalizeUnicode.hpp>

#include <xqilla/utils/XPath2Utils.hpp>
#include <xqilla/context/DynamicContext.hpp>
#include <xqilla/items/ATStringOrDerived.hpp>
#include <xqilla/utils/UnicodeTransformer.hpp>
#include <xqilla/exceptions/FunctionException.hpp>
#include <xqilla/framework/XPath2MemoryManager.hpp>
#include <xqilla/items/DatatypeFactory.hpp>

#if defined(XERCES_HAS_CPP_NAMESPACE)
XERCES_CPP_NAMESPACE_USE
#endif

const XMLCh FunctionNormalizeUnicode::name[] = {
  chLatin_n, chLatin_o, chLatin_r, 
  chLatin_m, chLatin_a, chLatin_l, 
  chLatin_i, chLatin_z, chLatin_e, 
  chDash,    chLatin_u, chLatin_n, 
  chLatin_i, chLatin_c, chLatin_o, 
  chLatin_d, chLatin_e, chNull 
};
const unsigned int FunctionNormalizeUnicode::minArgs = 1;
const unsigned int FunctionNormalizeUnicode::maxArgs = 2;

/**
 * fn:normalize-unicode($arg as xs:string?) as xs:string
 * fn:normalize-unicode($arg as xs:string?, $normalizationForm as xs:string) as xs:string
 **/

FunctionNormalizeUnicode::FunctionNormalizeUnicode(const VectorOfASTNodes &args, XPath2MemoryManager* memMgr)
  : XQFunction(name, "($arg as xs:string?, $normalizationForm as xs:string) as xs:string", args, memMgr)
{
}

Sequence FunctionNormalizeUnicode::createSequence(DynamicContext* context, int flags) const
{
  XPath2MemoryManager* memMgr = context->getMemoryManager();
  Sequence strParm=getParamNumber(1,context)->toSequence(context);
  if(strParm.isEmpty()) {
    return Sequence(context->getItemFactory()->createString(XMLUni::fgZeroLenString, context), memMgr);
  }

  const XMLCh *str = strParm.first()->asString(context);

  static const XMLCh fg_NFC[] = { chLatin_N, chLatin_F, chLatin_C, chNull };
  static const XMLCh fg_NFD[] = { chLatin_N, chLatin_F, chLatin_D, chNull };
  static const XMLCh fg_NFKC[] = { chLatin_N, chLatin_F, chLatin_K, chLatin_C, chNull };
  static const XMLCh fg_NFKD[] = { chLatin_N, chLatin_F, chLatin_K, chLatin_D, chNull };
  static const XMLCh fg_fully[] = { chLatin_F, chLatin_U, chLatin_L,
                                    chLatin_L, chLatin_Y, chDash,
                                    chLatin_N, chLatin_O, chLatin_R,
                                    chLatin_M, chLatin_A, chLatin_L,
                                    chLatin_I, chLatin_Z, chLatin_E,
                                    chLatin_D, chNull };

  const XMLCh* normalization = fg_NFC;
  if(getNumArgs()==2)
  {
    Sequence normParam=getParamNumber(2,context)->toSequence(context);
    const XMLCh *src = normParam.first()->asString(context);
    normalization = XPath2Utils::toUpper(src, memMgr);

    if (XMLString::stringLen(normalization) > 0)
    {
      unsigned int i;
      // remove leading spaces
      for(i = 0; i < XMLString::stringLen(normalization); i++) 
      {
        XMLCh ch = normalization[i];
        if((ch != 0x9) && (ch != 0xA) && (ch != 0xD) && (ch != 0x20))
          break;
      }
      const XMLCh *frontChop = XPath2Utils::subString(normalization, i, XPath2Utils::uintStrlen(normalization)-i, memMgr);

      // remove trailing spaces
      for(i = XPath2Utils::uintStrlen(frontChop)-1; i !=0 ; i--) 
      {
        XMLCh ch = frontChop[i];
        if((ch != 0x9) && (ch != 0xA) && (ch != 0xD) && (ch != 0x20))
          break;
      }
      normalization = XPath2Utils::subString(frontChop, 0, i+1, memMgr);
    }
  }

  if(XMLString::stringLen(normalization) == 0) {
    return Sequence(context->getItemFactory()->createString(str, context), memMgr);
  }
  else {
    AutoDeallocate<XMLCh> buf(0, memMgr);

    if(XPath2Utils::equals(normalization, fg_NFC)) {
      buf.set(UnicodeTransformer::normalizeC(str, memMgr));
    }
    else if(XPath2Utils::equals(normalization, fg_NFD)) {
      buf.set(UnicodeTransformer::normalizeD(str, memMgr));
    }
    else if(XPath2Utils::equals(normalization, fg_NFKC)) {
      buf.set(UnicodeTransformer::normalizeKC(str, memMgr));
    }
    else if(XPath2Utils::equals(normalization, fg_NFKD)) {
      buf.set(UnicodeTransformer::normalizeKD(str, memMgr));
    }
    else if(XPath2Utils::equals(normalization, fg_fully)) {
      XQThrow(FunctionException, X("FunctionNormalizeUnicode::createSequence"), X("Unsupported normalization form [err:FOCH0003]."));
    }
    else { 
      XQThrow(FunctionException, X("FunctionNormalizeUnicode::createSequence"), X("Invalid normalization form [err:FOCH0003]."));
   }

    return Sequence(context->getItemFactory()->createString(buf.get(), context), memMgr);
  }

  // Never reached
	return Sequence(memMgr);
}

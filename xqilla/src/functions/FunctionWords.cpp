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
 */

#include <vector>
#include "../config/xqilla_config.h"
#include <xqilla/functions/FunctionWords.hpp>
#include <xqilla/utils/UnicodeTransformer.hpp>
#include <xqilla/context/DynamicContext.hpp>
extern "C" {
#include <xqilla/utils/utf8proc.h>
}

#if defined(XERCES_HAS_CPP_NAMESPACE)
XERCES_CPP_NAMESPACE_USE
#endif

const XMLCh FunctionWords::name[] = {
  chLatin_w, chLatin_o, chLatin_r,
  chLatin_d, chLatin_s, chNull
};
const unsigned int FunctionWords::minArgs = 1;
const unsigned int FunctionWords::maxArgs = 1;

/** 
 * xqilla:words($input as xs:string?) as xs:string*
 */
  
FunctionWords::FunctionWords(const VectorOfASTNodes &args, XPath2MemoryManager* memMgr)
  : XQillaFunction(name, "($input as xs:string?) as xs:string*", args, memMgr)
{
}

Sequence FunctionWords::createSequence(DynamicContext* context, int flags) const
{
  XPath2MemoryManager* memMgr = context->getMemoryManager();

  // If the value of $operand1 is the empty sequence, the empty sequence is returned.
  Item::Ptr inputString = getParamNumber(1,context)->next(context);
  if(inputString.isNull())
    return Sequence(memMgr);

  const XMLCh *input=inputString->asString(context);
  AutoDeallocate<XMLCh> buf(UnicodeTransformer::words(input, memMgr), memMgr);

  XMLCh* rb = buf.get();
  XMLCh* start = NULL;
  Sequence resultSeq(0, memMgr);

  // Build sequence
  for (int i = 0; rb[i]; i++) {
    if (rb[i] != UTF8PROC_WB_MARK)
      continue;

    rb[i] = 0;
    if (rb[i+1] == 0 || rb[i+1] != UTF8PROC_WB_MARK) {
      if (start != NULL)
        resultSeq.addItem(context->getItemFactory()->createString(start, context));
      start = rb + (i+1);
    }
  }

  return resultSeq;
}


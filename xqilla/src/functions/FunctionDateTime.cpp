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
#include <xqilla/functions/FunctionDateTime.hpp>
#include <xqilla/context/DynamicContext.hpp>
#include <xqilla/items/ATDateTimeOrDerived.hpp>
#include <xqilla/items/ATTimeOrDerived.hpp>
#include <xqilla/items/ATDateOrDerived.hpp>
#include <xqilla/items/DatatypeFactory.hpp>
#include <xqilla/exceptions/FunctionException.hpp>
#include <xercesc/validators/schema/SchemaSymbols.hpp>

#if defined(XERCES_HAS_CPP_NAMESPACE)
XERCES_CPP_NAMESPACE_USE
#endif

const XMLCh FunctionDateTime::name[] = {
  chLatin_d, chLatin_a, chLatin_t, 
  chLatin_e, chLatin_T, chLatin_i, 
  chLatin_m, chLatin_e, chNull };
const unsigned int FunctionDateTime::minArgs = 2;
const unsigned int FunctionDateTime::maxArgs = 2;

/**
 * fn:dateTime($arg1 as xs:date?, $arg2 as xs:time?) as xs:dateTime?
**/

FunctionDateTime::FunctionDateTime(const VectorOfASTNodes &args, XPath2MemoryManager* memMgr)
  : XQFunction(name, "($arg1 as xs:date?, $arg2 as xs:time?) as xs:dateTime?", args, memMgr)
{
}

ASTNode *FunctionDateTime::staticTypingImpl(StaticContext *context)
{
  _src.clearExceptType();
  calculateSRCForArguments(context, /*checkTimezone*/true);
  return this;
}

Sequence FunctionDateTime::createSequence(DynamicContext* context, int flags) const
{
    ATDateOrDerived::Ptr date = (const ATDateOrDerived *)getParamNumber(1, context)->next(context).get();
    if(date.isNull())
        return Sequence(context->getMemoryManager());
    ATTimeOrDerived::Ptr time = (const ATTimeOrDerived *)getParamNumber(2, context)->next(context).get();
    if(time.isNull())
        return Sequence(context->getMemoryManager());

    Timezone::Ptr finalTZ;
    // determine the timezone of the result
    if(date->hasTimezone()) {
        finalTZ=date->getTimezone();
        if(time->hasTimezone()) {
          const Timezone::Ptr timeTZ=time->getTimezone();
          if(!timeTZ->equals(finalTZ))
            XQThrow(FunctionException, X("FunctionDateTime::createSequence"),
                    X("Both arguments to fn:dateTime have a timezone specified [err:FORG0008]"));
        }
    }
    else if(time->hasTimezone())
        finalTZ=time->getTimezone();

    Timezone::Ptr nulltz;
    date = date->setTimezone(nulltz, context);
    time = time->setTimezone(nulltz, context);

    // convert to a xs:dateTime string
    XMLBuffer buff(64, context->getMemoryManager());
    buff.append(date->asString(context));
    buff.append(chLatin_T);
    buff.append(time->asString(context));
    if(finalTZ != NULLRCP)
      buff.append(finalTZ->asString(context));

    return Sequence(context->getItemFactory()->createDateTime(buff.getRawBuffer(), context),
                    context->getMemoryManager());
}

















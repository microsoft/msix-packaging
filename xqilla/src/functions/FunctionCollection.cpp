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
#include <xqilla/functions/FunctionCollection.hpp>

#include <xqilla/context/DynamicContext.hpp>
#include <xqilla/exceptions/FunctionException.hpp>
#include <xqilla/ast/StaticAnalysis.hpp>
#include <xqilla/utils/XPath2Utils.hpp>

XERCES_CPP_NAMESPACE_USE;

const XMLCh FunctionCollection::name[] = {
  chLatin_c, chLatin_o, chLatin_l, 
  chLatin_l, chLatin_e, chLatin_c, 
  chLatin_t, chLatin_i, chLatin_o, 
  chLatin_n, chNull 
};
const unsigned int FunctionCollection::minArgs = 0;
const unsigned int FunctionCollection::maxArgs = 1;

/**
 * fn:collection() as node()*
 * fn:collection($arg as xs:string?) as node()*
**/

FunctionCollection::FunctionCollection(const VectorOfASTNodes &args, XPath2MemoryManager* memMgr)
  : XQFunction(name, "($arg as xs:string?) as node()*", args, memMgr),
    queryPathTree_(0)
{
}

ASTNode *FunctionCollection::staticTypingImpl(StaticContext *context)
{
  _src.clearExceptType();

  _src.setProperties(StaticAnalysis::DOCORDER | StaticAnalysis::GROUPED |
                     StaticAnalysis::SUBTREE);
  _src.getStaticType() = StaticType(StaticType::NODE_TYPE, 0, StaticType::UNLIMITED);
  _src.availableCollectionsUsed(true);
  calculateSRCForArguments(context);

  return this;
}

Sequence FunctionCollection::createSequence(DynamicContext* context, int flags) const
{
  //args 0 - URI to resolve
  if(getNumArgs() == 0)
      return context->resolveDefaultCollection(context->getProjection() ? queryPathTree_ : 0);

  Item::Ptr arg = getParamNumber(1, context)->next(context);
  if(arg.isNull())
    return context->resolveDefaultCollection(context->getProjection() ? queryPathTree_ : 0);

  const XMLCh *uri = arg->asString(context);
  if(!XPath2Utils::isValidURI(uri, context->getMemoryManager()))
    XQThrow(FunctionException, X("FunctionCollection::createSequence"), X("Invalid URI format [err:FODC0002]"));

  return context->resolveCollection(uri, this, context->getProjection() ? queryPathTree_ : 0);
}


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
#include <xqilla/functions/FunctionTrace.hpp>
#include <xqilla/context/DynamicContext.hpp>
#include <xqilla/context/MessageListener.hpp>
#include <xqilla/items/Item.hpp>
#include <xqilla/ast/StaticAnalysis.hpp>
#include <xercesc/framework/XMLBuffer.hpp>

const XMLCh FunctionTrace::name[] = {
  XERCES_CPP_NAMESPACE_QUALIFIER chLatin_t, XERCES_CPP_NAMESPACE_QUALIFIER chLatin_r, XERCES_CPP_NAMESPACE_QUALIFIER chLatin_a, 
  XERCES_CPP_NAMESPACE_QUALIFIER chLatin_c, XERCES_CPP_NAMESPACE_QUALIFIER chLatin_e, XERCES_CPP_NAMESPACE_QUALIFIER chNull 
};
const unsigned int FunctionTrace::minArgs = 2;
const unsigned int FunctionTrace::maxArgs = 2;

/*
 * fn:trace($value as item()*, $label as xs:string) as item()*
 */

FunctionTrace::FunctionTrace(const VectorOfASTNodes &args, XPath2MemoryManager* memMgr)
  : XQFunction(name, "($value as item()*, $label as xs:string) as item()*", args, memMgr)
{
}

ASTNode *FunctionTrace::staticTypingImpl(StaticContext *context)
{
  _src.clearExceptType();
  calculateSRCForArguments(context);

  _src.getStaticType() = _args.front()->getStaticAnalysis().getStaticType();
  _src.setProperties(_args.front()->getStaticAnalysis().getProperties());
  _src.forceNoFolding(true);

  return this;
}

Sequence FunctionTrace::createSequence(DynamicContext* context, int flags) const
{
  Sequence value = getParamNumber(1, context)->toSequence(context);

  if(context->getMessageListener() != 0) {
    context->getMessageListener()->trace(getParamNumber(2, context)->next(context)->asString(context),
                                         value, this, context);
  }

  return value;
}


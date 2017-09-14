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
#include <assert.h>
#include <xqilla/functions/FunctionRoot.hpp>
#include <xqilla/ast/XQContextItem.hpp>
#include <xqilla/exceptions/FunctionException.hpp>
#include <xqilla/context/DynamicContext.hpp>
#include <xqilla/runtime/Sequence.hpp>
#include <xqilla/items/Node.hpp>
#include <xqilla/ast/StaticAnalysis.hpp>
#include <xqilla/exceptions/StaticErrorException.hpp>

XERCES_CPP_NAMESPACE_USE

const XMLCh FunctionRoot::name[] = {
  chLatin_r, chLatin_o, chLatin_o, 
  chLatin_t, chNull 
};
const unsigned int FunctionRoot::minArgs = 0;
const unsigned int FunctionRoot::maxArgs = 1;

/**
 * fn:root() as node()
 * fn:root($arg as node()?) as node()?
**/

FunctionRoot::FunctionRoot(const VectorOfASTNodes &args, XPath2MemoryManager* memMgr)
  : XQFunction(name, "($arg as node()?) as node()?", args, memMgr)
{
}

ASTNode* FunctionRoot::staticResolution(StaticContext *context)
{
  XPath2MemoryManager *mm = context->getMemoryManager();

  if(_args.empty()) {
    XQContextItem *ci = new (mm) XQContextItem(mm);
    ci->setLocationInfo(this);
    _args.push_back(ci);
  }

  resolveArguments(context);
  return this;
}

ASTNode *FunctionRoot::staticTypingImpl(StaticContext *context)
{
  _src.clear();

  _src.setProperties(StaticAnalysis::DOCORDER | StaticAnalysis::GROUPED |
                     StaticAnalysis::PEER | StaticAnalysis::SAMEDOC | StaticAnalysis::ONENODE);
  _src.getStaticType() = StaticType(StaticType::NODE_TYPE, 0, 1);

  _src.add(_args[0]->getStaticAnalysis());

  if(_args[0]->getStaticAnalysis().isUpdating()) {
    XQThrow(StaticErrorException,X("FunctionRoot::staticTyping"),
            X("It is a static error for an argument to a function "
              "to be an updating expression [err:XUST0001]"));
  }

  if(_args[0]->getStaticAnalysis().getStaticType().isType(StaticType::DOCUMENT_TYPE)) {
    return _args[0];
  }

  return this;
}

Sequence FunctionRoot::createSequence(DynamicContext* context, int flags) const
{
  XPath2MemoryManager* memMgr = context->getMemoryManager();

  Node::Ptr node = (Node*)getParamNumber(1, context)->next(context).get();
  if(node.isNull()) return Sequence(memMgr);

  return Sequence(node->root(context), memMgr);
}

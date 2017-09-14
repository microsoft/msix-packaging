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

#include <xqilla/functions/FunctionExplain.hpp>
#include <xqilla/context/DynamicContext.hpp>
#include <xqilla/ast/StaticAnalysis.hpp>
#include <xqilla/events/SequenceBuilder.hpp>
#include <xqilla/events/QueryPathTreeFilter.hpp>
#include <xqilla/optimizer/ASTToXML.hpp>
#include <xqilla/simple-api/XQillaConfiguration.hpp>

XERCES_CPP_NAMESPACE_USE;
using namespace std;

const XMLCh FunctionExplain::name[] = {
  chLatin_e, chLatin_x, chLatin_p, chLatin_l, chLatin_a, chLatin_i, chLatin_n, chNull
};
const unsigned int FunctionExplain::minArgs = 1;
const unsigned int FunctionExplain::maxArgs = 1;

FunctionExplain::FunctionExplain(const VectorOfASTNodes &args, XPath2MemoryManager* memMgr)
  : XQillaFunction(name, "($expression as item()*) as element()", args, memMgr),
    queryPathTree_(0)
{
}

ASTNode *FunctionExplain::staticTypingImpl(StaticContext *context)
{
  _src.clearExceptType();

  _src.setProperties(StaticAnalysis::DOCORDER | StaticAnalysis::GROUPED |
                     StaticAnalysis::PEER | StaticAnalysis::SUBTREE | StaticAnalysis::ONENODE);
  _src.creative(true);

  calculateSRCForArguments(context);
  return this;
}

Sequence FunctionExplain::createSequence(DynamicContext* context, int flags) const
{
  AutoDelete<SequenceBuilder> builder(context->createSequenceBuilder());
  QueryPathTreeFilter qptf(queryPathTree_, builder.get());
  EventHandler *handler = queryPathTree_ ? (EventHandler*)&qptf : (EventHandler*)builder.get();

  AutoDelete<ASTToXML> a2x(context->getConfiguration()->createASTToXML());
  a2x->run(_args[0], handler, context);

  handler->endEvent();
  return builder->getSequence();
}

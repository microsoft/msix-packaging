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
#include <xqilla/functions/FunctionCount.hpp>
#include <xqilla/ast/XQLiteral.hpp>
#include <xqilla/exceptions/XQException.hpp>
#include <xqilla/runtime/Sequence.hpp>
#include <xqilla/items/ATDecimalOrDerived.hpp>
#include <xqilla/context/DynamicContext.hpp>
#include <xqilla/items/DatatypeFactory.hpp>
#include <xqilla/context/ItemFactory.hpp>
#include <xqilla/context/ContextHelpers.hpp>

const XMLCh FunctionCount::name[] = {
  XERCES_CPP_NAMESPACE_QUALIFIER chLatin_c, XERCES_CPP_NAMESPACE_QUALIFIER chLatin_o, XERCES_CPP_NAMESPACE_QUALIFIER chLatin_u, 
  XERCES_CPP_NAMESPACE_QUALIFIER chLatin_n, XERCES_CPP_NAMESPACE_QUALIFIER chLatin_t, XERCES_CPP_NAMESPACE_QUALIFIER chNull 
};
const unsigned int FunctionCount::minArgs = 1;
const unsigned int FunctionCount::maxArgs = 1;

/**
 * fn:count($arg as item()*) as xs:integer
**/

FunctionCount::FunctionCount(const VectorOfASTNodes &args, XPath2MemoryManager* memMgr)
  : XQFunction(name, "($arg as item()*) as xs:integer", args, memMgr)
{
}

ASTNode* FunctionCount::staticResolution(StaticContext *context)
{
  AutoNodeSetOrderingReset orderReset(context);
  resolveArguments(context);
  return this;
}

ASTNode *FunctionCount::staticTypingImpl(StaticContext *context)
{
  _src.clearExceptType();
  calculateSRCForArguments(context);

  if(context) {
    const StaticAnalysis &sa = _args[0]->getStaticAnalysis();
    const StaticType &sType = sa.getStaticType();
    if(sType.getMin() == sType.getMax() && !sa.areDocsOrCollectionsUsed() && !sa.isNoFoldingForced()) {
      XPath2MemoryManager* mm = context->getMemoryManager();

      try {
        AutoDelete<DynamicContext> dContext(context->createDynamicContext());
        dContext->setMemoryManager(mm);
        return XQLiteral::create(mm->createInteger(sType.getMin()), dContext, mm, this);
      }
      catch(XQException &ex) {
        // Constant folding failed
      }
    }
  }

  return this;
}

Sequence FunctionCount::createSequence(DynamicContext* context, int flags) const
{
  Result arg = getParamNumber(1,context);

  long length = 0;
  while(arg->next(context).notNull()) {
    ++length;
  }

  return Sequence(context->getItemFactory()->createInteger(length, context), context->getMemoryManager());
}


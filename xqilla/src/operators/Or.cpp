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
#include <sstream>

#include <xqilla/operators/Or.hpp>
#include <xqilla/context/DynamicContext.hpp>
#include <xqilla/items/ATBooleanOrDerived.hpp>
#include <xqilla/items/DatatypeFactory.hpp>
#include <xqilla/ast/StaticAnalysis.hpp>
#include <xqilla/ast/XQLiteral.hpp>
#include <xqilla/context/ItemFactory.hpp>
#include <xqilla/context/ContextHelpers.hpp>
#include <xqilla/exceptions/StaticErrorException.hpp>
#include <xqilla/ast/XQEffectiveBooleanValue.hpp>
#include <xqilla/runtime/ClosureResult.hpp>

XERCES_CPP_NAMESPACE_USE;

/*static*/ const XMLCh Or::name[]={ chLatin_O, chLatin_r, chNull };

Or::Or(const VectorOfASTNodes &args, XPath2MemoryManager* memMgr)
  : XQOperator(name, args, memMgr)
{
  // Nothing to do
}

ASTNode* Or::staticResolution(StaticContext *context)
{
  XPath2MemoryManager *mm = context->getMemoryManager();

  for(VectorOfASTNodes::iterator i = _args.begin(); i != _args.end(); ++i) {
    *i = new (mm) XQEffectiveBooleanValue(*i, mm);
    (*i)->setLocationInfo(this);
    *i = (*i)->staticResolution(context);
  }
  return this;
}

ASTNode *Or::staticTypingImpl(StaticContext *context)
{
  _src.clear();

  VectorOfASTNodes::iterator i;
  for(i = _args.begin(); i != _args.end(); ++i) {
    _src.add((*i)->getStaticAnalysis());

    if((*i)->getStaticAnalysis().isUpdating()) {
      XQThrow(StaticErrorException,X("Or::staticTyping"),
              X("It is a static error for an operand of an operator "
                "to be an updating expression [err:XUST0001]"));
    }
  }

  _src.getStaticType() = StaticType::BOOLEAN_TYPE;
  return this;
}

BoolResult Or::boolResult(DynamicContext* context) const
{
  VectorOfASTNodes::const_iterator i = getArguments().begin();
  for(; i != getArguments().end(); ++i) {
    if((*i)->boolResult(context)) return true;
  }
  return false;
}

Result Or::createResult(DynamicContext* context, int flags) const
{
  return (Item::Ptr)context->getItemFactory()->createBoolean(boolResult(context), context);
}

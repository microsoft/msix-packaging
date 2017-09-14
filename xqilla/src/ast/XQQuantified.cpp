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

#include <xqilla/ast/XQQuantified.hpp>
#include <xqilla/ast/XQLiteral.hpp>
#include <xqilla/ast/TupleNode.hpp>
#include <xqilla/context/DynamicContext.hpp>
#include <xqilla/context/ItemFactory.hpp>
#include <xqilla/context/ContextHelpers.hpp>
#include <xqilla/ast/XQEffectiveBooleanValue.hpp>

XQQuantified::XQQuantified(Type qtype, TupleNode *parent, ASTNode *expr, XPath2MemoryManager *mm)
  : ASTNodeImpl(QUANTIFIED, mm),
    qtype_(qtype),
    parent_(parent),
    expr_(expr)
{
}

ASTNode *XQQuantified::staticResolution(StaticContext *context)
{
  XPath2MemoryManager *mm = context->getMemoryManager();

  parent_ = parent_->staticResolution(context);

  expr_ = new (mm) XQEffectiveBooleanValue(expr_, mm);
  expr_->setLocationInfo(this);
  expr_ = expr_->staticResolution(context);

  return this;
}

ASTNode *XQQuantified::staticTypingImpl(StaticContext *context)
{
  _src.clear();

  _src.add(expr_->getStaticAnalysis());
  _src.getStaticType() = StaticType::BOOLEAN_TYPE;

  parent_ = parent_->staticTypingTeardown(context, _src);

  return this;
}

BoolResult XQQuantified::boolResult(DynamicContext* context) const
{
  bool defaultResult = (getQuantifierType() == XQQuantified::SOME) ? false : true;

  AutoVariableStoreReset reset(context);

  TupleResult::Ptr tuples = getParent()->createResult(context);
  while(tuples->next(context)) {
    context->setVariableStore(tuples);

    bool result = getExpression()->boolResult(context);
    if(defaultResult != result) {
      defaultResult = result;
      break;
    }

    reset.reset();
  }

  return defaultResult;
}

Result XQQuantified::createResult(DynamicContext* context, int flags) const
{
  return (Item::Ptr)context->getItemFactory()->createBoolean(boolResult(context), context);
}

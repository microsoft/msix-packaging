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

#include <assert.h>

#include "../config/xqilla_config.h"
#include <xqilla/framework/XQillaExport.hpp>
#include <xqilla/ast/XQContextItem.hpp>
#include <xqilla/runtime/Sequence.hpp>
#include <xqilla/context/DynamicContext.hpp>
#include <xqilla/exceptions/DynamicErrorException.hpp>
#include <xqilla/ast/StaticAnalysis.hpp>

XQContextItem::XQContextItem(XPath2MemoryManager* memMgr)
  : ASTNodeImpl(CONTEXT_ITEM, memMgr)
{
}

XQContextItem::~XQContextItem() {
  //no-op
}

ASTNode* XQContextItem::staticResolution(StaticContext *context)
{
  return this;
}

ASTNode *XQContextItem::staticTypingImpl(StaticContext *context)
{
  if(context == 0) return this;

  _src.clear();

  if(!context->getContextItemType().containsType(StaticType::ITEM_TYPE)) {
    XQThrow(DynamicErrorException,X("XQContextItem::staticTyping"),
            X("It is an error for the context item to be undefined when using it [err:XPDY0002]"));
  }

  _src.setProperties(StaticAnalysis::DOCORDER | StaticAnalysis::GROUPED |
    StaticAnalysis::PEER | StaticAnalysis::SUBTREE | StaticAnalysis::SAMEDOC |
    StaticAnalysis::ONENODE | StaticAnalysis::SELF);
  _src.getStaticType() = context->getContextItemType();
  _src.getStaticType().setCardinality(1, 1);

  _src.contextItemUsed(true);
  return this;
}

Result XQContextItem::createResult(DynamicContext* context, int flags) const
{
  const Item::Ptr item = context->getContextItem();
  if(item.isNull()) {
    XQThrow(DynamicErrorException,X("XQContextItem::ContextItemResult::createResult"),
             X("It is an error for the context item to be undefined when using it [err:XPDY0002]"));
  }
  return item;
}

Result XQContextItem::result(DynamicContext *context, const LocationInfo *info)
{
  const Item::Ptr item = context->getContextItem();
  if(item.isNull()) {
    XQThrow3(DynamicErrorException,X("XQContextItem::result"),
             X("It is an error for the context item to be undefined when using it [err:XPDY0002]"), info);
  }
  return item;
}

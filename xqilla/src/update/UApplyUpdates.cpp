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

#include <xqilla/update/UApplyUpdates.hpp>
#include <xqilla/context/DynamicContext.hpp>
#include <xqilla/context/UpdateFactory.hpp>
#include <xqilla/update/PendingUpdateList.hpp>

UApplyUpdates::UApplyUpdates(ASTNode *expr, XPath2MemoryManager* memMgr)
  : ASTNodeImpl(UAPPLY_UPDATES, memMgr),
    expr_(expr),
    valMode_(DocumentCache::VALIDATION_SKIP)
{
}

UApplyUpdates::UApplyUpdates(ASTNode *expr, DocumentCache::ValidationMode valMode, XPath2MemoryManager* memMgr)
  : ASTNodeImpl(UAPPLY_UPDATES, memMgr),
    expr_(expr),
    valMode_(valMode)
{
}

ASTNode* UApplyUpdates::staticResolution(StaticContext *context)
{
  expr_ = expr_->staticResolution(context);
  valMode_ = context->getRevalidationMode();
  return this;
}

ASTNode *UApplyUpdates::staticTypingImpl(StaticContext *context)
{
  _src.clear();

  _src.add(expr_->getStaticAnalysis());

  if(!expr_->getStaticAnalysis().isUpdating()) {
    return expr_;
  }

  _src.updating(false);
  return this;
}

Result UApplyUpdates::createResult(DynamicContext* context, int flags) const
{
  AutoDelete<UpdateFactory> ufactory(context->createUpdateFactory());
  ufactory->applyUpdates(getExpression()->createUpdateList(context), context,
                         getRevalidationMode());
  return 0;
}

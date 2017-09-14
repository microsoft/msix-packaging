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

#include <xqilla/update/UDelete.hpp>
#include <xqilla/context/DynamicContext.hpp>
#include <xqilla/items/Node.hpp>
#include <xqilla/ast/XQTreatAs.hpp>
#include <xqilla/schema/SequenceType.hpp>
#include <xqilla/exceptions/StaticErrorException.hpp>
#include <xqilla/update/PendingUpdateList.hpp>

UDelete::UDelete(ASTNode *expr, XPath2MemoryManager* memMgr)
  : ASTNodeImpl(UDELETE, memMgr),
    expr_(expr)
{
}

static const XMLCh err_XUTY0007[] = { 'e', 'r', 'r', ':', 'X', 'U', 'T', 'Y', '0', '0', '0', '7', 0 };

ASTNode *UDelete::staticResolution(StaticContext *context)
{
  XPath2MemoryManager *mm = context->getMemoryManager();

  SequenceType *seqType = new (mm) SequenceType(new (mm) SequenceType::ItemType(SequenceType::ItemType::TEST_NODE),
                                                SequenceType::STAR);
  seqType->setLocationInfo(this);

  expr_ = new (mm) XQTreatAs(expr_, seqType, mm, err_XUTY0007);
  expr_->setLocationInfo(this);

  expr_ = expr_->staticResolution(context);
  return this;
}

ASTNode *UDelete::staticTypingImpl(StaticContext *context)
{
  _src.clear();

  _src.add(expr_->getStaticAnalysis());

  if(expr_->getStaticAnalysis().isUpdating()) {
    XQThrow(StaticErrorException,X("UDelete::staticTyping"),
            X("It is a static error for the target expression of a delete expression "
              "to be an updating expression [err:XUST0001]"));
  }

  _src.updating(true);
  return this;
}

Result UDelete::createResult(DynamicContext* context, int flags) const
{
  return 0;
}

PendingUpdateList UDelete::createUpdateList(DynamicContext *context) const
{
  PendingUpdateList pul;

  Result targets = expr_->createResult(context);
  Item::Ptr item;
  while((item = targets->next(context)).notNull()) {
    pul.addUpdate(PendingUpdate(PendingUpdate::PUDELETE, (Node*)item.get(), this));
  }

  return pul;
}


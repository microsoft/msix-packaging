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

#include <xqilla/ast/ASTNodeImpl.hpp>
#include <xqilla/ast/XQNav.hpp>
#include <xqilla/utils/XPath2Utils.hpp>
#include <xqilla/utils/NumUtils.hpp>
#include <xqilla/ast/ASTNode.hpp>
#include <xqilla/runtime/Sequence.hpp>
#include <xqilla/exceptions/XPath2TypeCastException.hpp>
#include <xqilla/items/Numeric.hpp>
#include <xqilla/items/Node.hpp>
#include <xqilla/items/ATBooleanOrDerived.hpp>
#include <xqilla/items/ATDecimalOrDerived.hpp>
#include <xqilla/items/DateOrTimeType.hpp>
#include <xqilla/context/DynamicContext.hpp>
#include <xqilla/items/DatatypeFactory.hpp>
#include <xercesc/validators/schema/SchemaSymbols.hpp>
#include <xqilla/runtime/SequenceResult.hpp>
#include <xqilla/context/ItemFactory.hpp>
#include <xqilla/context/ContextHelpers.hpp>
#include <xqilla/update/PendingUpdateList.hpp>
#include <xqilla/events/EventHandler.hpp>
#include <xqilla/optimizer/ASTReleaser.hpp>
#include <xqilla/optimizer/ASTCopier.hpp>
#include <xqilla/optimizer/StaticTyper.hpp>
#include <xqilla/runtime/ClosureResult.hpp>

ASTNodeImpl::ASTNodeImpl(whichType type, XPath2MemoryManager* memMgr)
  : _src(memMgr),
    _type(type),
    _memMgr(memMgr)
{
}

ASTNodeImpl::~ASTNodeImpl()
{
}

void ASTNodeImpl::release()
{
  ASTReleaser().release(this);
}

ASTNode *ASTNodeImpl::copy(DynamicContext *context) const
{
  return ASTCopier().copy(this, context);
}

bool ASTNodeImpl::isSubsetOf(const ASTNode *other) const
{
  // TBD - jpcs
  return false;
}

bool ASTNodeImpl::isEqualTo(const ASTNode *other) const
{
  // TBD - jpcs
  return false;
}

ASTNode *ASTNodeImpl::staticTyping(StaticContext *context, StaticTyper *styper)
{
  StaticTyper defaultTyper;
  if(styper == 0) styper = &defaultTyper;
  return styper->run(this, context);
}

bool ASTNodeImpl::isConstant() const
{
  return !_src.isUsed();
}

/** Overridden in XQSequence and XQLiteral */
bool ASTNodeImpl::isDateOrTimeAndHasNoTimezone(StaticContext *context) const
{
  // To be safe, assume any value that contains a timezone might
  // not have one specified.
  return _src.getStaticType().containsType(StaticType::TIMEZONE_TYPE);
}

ASTNode::whichType ASTNodeImpl::getType() const
{
  return _type;
}

PendingUpdateList ASTNodeImpl::createUpdateList(DynamicContext *context) const
{
  return PendingUpdateList();
}

Result ASTNodeImpl::iterateResult(const Result &contextItems, DynamicContext* context) const
{
  if(_src.isContextSizeUsed()) {
    // We need the context size, so convert contextItems to a Sequence to work it out
    Sequence seq(((ResultImpl*)contextItems.get())->toSequence(context));
    return ClosureResult::create(getStaticAnalysis(), context,
      new NavStepResult(new SequenceResult(this, seq), this, seq.getLength()));
  }
  return ClosureResult::create(getStaticAnalysis(), context, new NavStepResult(contextItems, this, 0));
}

#include <iostream>
#include <xqilla/optimizer/ASTToXML.hpp>

BoolResult ASTNodeImpl::boolResult(DynamicContext* context) const
{
  Item::Ptr item = createResult(context)->next(context);
  return item.isNull() ? BoolResult::Null :
    ((ATBooleanOrDerived*)item.get())->isTrue() ? BoolResult::True :
    BoolResult::False;
}

EventGenerator::Ptr ASTNodeImpl::generateEvents(EventHandler *events, DynamicContext *context,
                                                bool preserveNS, bool preserveType) const
{
  Result result = createResult(context);
  Item::Ptr item;
  while((item = result->next(context)).notNull()) {
    item->generateEvents(events, context, preserveNS, preserveType);
  }

  return 0;
}

ASTNode *ASTNodeImpl::substitute(ASTNode *&result)
{
  ASTNode *tmp = result;
  result = 0;
  this->release();
  return tmp;
}

XPath2MemoryManager* ASTNodeImpl::getMemoryManager() const {

  return _memMgr;
}

const StaticAnalysis &ASTNodeImpl::getStaticAnalysis() const
{
  return _src;
}

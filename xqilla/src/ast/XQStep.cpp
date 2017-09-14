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

#include <sstream>

#include "../config/xqilla_config.h"
#include <xqilla/ast/XQStep.hpp>
#include <xqilla/axis/NodeTest.hpp>
#include <xqilla/items/Item.hpp>
#include <xqilla/items/ATDecimalOrDerived.hpp>
#include <xqilla/context/DynamicContext.hpp>
#include <xqilla/items/DatatypeFactory.hpp>
#include <xqilla/ast/StaticAnalysis.hpp>
#include <xqilla/exceptions/TypeErrorException.hpp>
#include <xqilla/exceptions/DynamicErrorException.hpp>
#include <xqilla/ast/XQDocumentOrder.hpp>
#include <xqilla/ast/XQContextItem.hpp>

XQStep::XQStep(Axis axis, NodeTest* nodeTest, XPath2MemoryManager* memMgr)
  : ASTNodeImpl(STEP, memMgr),
    nodeTest_(nodeTest),
    axis_(axis)
{
}

XQStep::~XQStep()
{
}

unsigned int XQStep::getAxisProperties(Axis axis)
{
  unsigned int properties = 0;
  // properties depend on the axis of the step
  switch (axis) {
  case SELF:
    properties |= StaticAnalysis::ONENODE | StaticAnalysis::SELF;
    // Fall through
  case CHILD:
  case ATTRIBUTE:
  case NAMESPACE:
    properties |= StaticAnalysis::SUBTREE | StaticAnalysis::PEER;
    break;
  case DESCENDANT:
  case DESCENDANT_OR_SELF:
    properties |= StaticAnalysis::SUBTREE;
    break;
  case FOLLOWING_SIBLING:
  case PRECEDING_SIBLING:
    properties |= StaticAnalysis::PEER;
    break;
  case PARENT:
    properties |= StaticAnalysis::PEER | StaticAnalysis::ONENODE;
    break;
  default:
    break;
  }
  properties |= StaticAnalysis::GROUPED | StaticAnalysis::SAMEDOC;

  if(isForwardAxis(axis) || axis == PARENT) {
    properties |= StaticAnalysis::DOCORDER;
  }

  return properties;
}

ASTNode* XQStep::staticResolution(StaticContext *context)
{
  nodeTest_->staticResolution(context, this);
  return this;
}

ASTNode *XQStep::staticTypingImpl(StaticContext *context)
{
  _src.clear();

  if(context && !context->getContextItemType().containsType(StaticType::ITEM_TYPE)) {
    XQThrow(DynamicErrorException,X("XQStep::staticTyping"),
            X("It is an error for the context item to be undefined when using it [err:XPDY0002]"));
  }

  _src.setProperties(getAxisProperties(axis_));
  _src.contextItemUsed(true);

  bool isExact;
  nodeTest_->getStaticType(_src.getStaticType(), context, isExact, this);
  _src.getStaticType().multiply(0, StaticType::UNLIMITED);

  switch(axis_) {
  case SELF:
    if(context)
      _src.getStaticType().typeNodeIntersect(context->getContextItemType());
    break;
  case ATTRIBUTE:
    _src.getStaticType().typeNodeIntersect(StaticType(StaticType::ATTRIBUTE_TYPE, 0, StaticType::UNLIMITED));
    break;
  case NAMESPACE:
    _src.getStaticType().typeNodeIntersect(StaticType(StaticType::NAMESPACE_TYPE, 0, StaticType::UNLIMITED));
    break;
  case CHILD:
  case DESCENDANT:
  case FOLLOWING:
  case FOLLOWING_SIBLING:
  case PRECEDING:
  case PRECEDING_SIBLING:
    _src.getStaticType().typeNodeIntersect(StaticType(StaticType::ELEMENT_TYPE | StaticType::TEXT_TYPE | StaticType::PI_TYPE |
                                            StaticType::COMMENT_TYPE, 0, StaticType::UNLIMITED));
    break;
  case ANCESTOR:
  case PARENT:
    _src.getStaticType().typeNodeIntersect(StaticType(StaticType::DOCUMENT_TYPE | StaticType::ELEMENT_TYPE, 0, StaticType::UNLIMITED));
    break;
  case DESCENDANT_OR_SELF:
  case ANCESTOR_OR_SELF:
    // Could be any type
    break;
  }

  return this;
}

bool XQStep::isForwardAxis(Axis axis)
{
  switch(axis) {
  case ANCESTOR:
  case ANCESTOR_OR_SELF:
  case PARENT:
  case PRECEDING:
  case PRECEDING_SIBLING:
    return false;

  case ATTRIBUTE:
  case CHILD:
  case DESCENDANT:
  case DESCENDANT_OR_SELF:
  case FOLLOWING:
  case FOLLOWING_SIBLING:
  case NAMESPACE:
  case SELF:
    return true;
  }
  return false;
}

NodeTest *XQStep::getNodeTest() const {
  return nodeTest_;
}

void XQStep::setNodeTest(NodeTest *nt) {
  nodeTest_ = nt;
}

XQStep::Axis XQStep::getAxis() const {
  return axis_;
}

void XQStep::setAxis(XQStep::Axis a) {
  axis_ = a;
}

class StepResult : public ResultImpl
{
public:
  StepResult(const Result &parent, const XQStep *step)
    : ResultImpl(step),
      parent_(parent),
      step_(step),
      stepResult_(0)
  {
  }

  Item::Ptr next(DynamicContext *context)
  {
    Item::Ptr result;
    while((result = stepResult_->next(context)).isNull()) {
      context->testInterrupt();

      Item::Ptr item = parent_->next(context);

      if(item.isNull()) {
        return 0;
      }
      if(!item->isNode()) {
        XQThrow(TypeErrorException,X("StepResult::next"), X("An attempt was made to perform an axis step when the Context Item was not a node [err:XPTY0020]"));
      }

      stepResult_ = ((Node*)item.get())->getAxisResult(step_->getAxis(), step_->getNodeTest(), context, this);
    }

    return result;
  }

protected:
  Result parent_;
  const XQStep *step_;
  Result stepResult_;
};

Result XQStep::createResult(DynamicContext* context, int flags) const 
{
  return new StepResult(XQContextItem::result(context, this), this);
}

Result XQStep::iterateResult(const Result &contextItems, DynamicContext* context) const
{
  return new StepResult(contextItems, this);
}


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

#include <xqilla/ast/XQNav.hpp>
#include <xqilla/ast/XQStep.hpp>
#include <xqilla/runtime/Sequence.hpp>
#include <xqilla/context/DynamicContext.hpp>
#include <xqilla/exceptions/TypeErrorException.hpp>
#include <xqilla/items/Node.hpp>
#include <xqilla/ast/StaticAnalysis.hpp>
#include <xqilla/ast/XQTreatAs.hpp>
#include <xqilla/ast/XQDocumentOrder.hpp>
#include <xqilla/ast/XQPredicate.hpp>
#include <xqilla/runtime/SequenceResult.hpp>
#include <xqilla/items/DatatypeFactory.hpp>
#include <xqilla/context/ContextHelpers.hpp>
#include <xqilla/axis/NodeTest.hpp>

XQNav::XQNav(XPath2MemoryManager* memMgr)
  : ASTNodeImpl(NAVIGATION, memMgr),
    _steps(XQillaAllocator<StepInfo>(memMgr)),
    _sortAdded(false)
{
}

XQNav::XQNav(ASTNode *ast, XPath2MemoryManager* memMgr)
  : ASTNodeImpl(NAVIGATION, memMgr),
    _steps(XQillaAllocator<StepInfo>(memMgr)),
    _sortAdded(false)
{
  addStep(ast);
}

XQNav::XQNav(const Steps &steps, bool sortAdded, XPath2MemoryManager* memMgr)
  : ASTNodeImpl(NAVIGATION, memMgr),
    _steps(XQillaAllocator<StepInfo>(memMgr)),
    _sortAdded(sortAdded)
{
  _steps = steps;
}

XQNav::~XQNav()
{
  //no-op
}

Result XQNav::createResult(DynamicContext* context, int flags) const
{
  Steps::const_iterator end = _steps.end();
  Steps::const_iterator it = _steps.begin();

  Result result(0);
  if(it != end) {
    ASTNode *step = it->step;

    result = step->createResult(context, flags);
    StaticType st = step->getStaticAnalysis().getStaticType();

    for(++it; it != end; ++it) {
      if(st.containsType(StaticType::ANY_ATOMIC_TYPE)) {
        result = new IntermediateStepCheckResult(step, result);
      }

      step = it->step;

      result = step->iterateResult(result, context);

      st = step->getStaticAnalysis().getStaticType();
    }

    // the last step allows either nodes or atomic items
    if(st.containsType(StaticType::NODE_TYPE) &&
       st.containsType(StaticType::ANY_ATOMIC_TYPE)) {
      result = new LastStepCheckResult(step, result);
    }
  }

  return result;
}

void XQNav::addStep(const StepInfo &step)
{
  if(step.step->getType() == NAVIGATION) {
    Steps &navSteps = ((XQNav*)step.step)->_steps;
    for(Steps::iterator it2 = navSteps.begin();
        it2 != navSteps.end(); ++it2) {
      _steps.push_back(*it2);
    }
  }
  else {
    _steps.push_back(step);
  }
}

void XQNav::addStepFront(ASTNode* step)
{
  if(step->getType() == NAVIGATION) {
    Steps &navSteps = ((XQNav*)step)->_steps;
    for(Steps::reverse_iterator it2 = navSteps.rbegin();
        it2 != navSteps.rend(); ++it2) {
      _steps.insert(_steps.begin(), *it2);
    }
  }
  else {
    _steps.insert(_steps.begin(), step);
  }
}

ASTNode* XQNav::staticResolution(StaticContext *context)
{
  if(!_sortAdded) {
    _sortAdded = true;
    // Wrap ourselves in a document order sort
    XPath2MemoryManager *mm = context->getMemoryManager();

    ASTNode *result = new (mm) XQDocumentOrder(this, mm);
    result->setLocationInfo(_steps.back().step);
    
    return result->staticResolution(context);
  }

  Steps::iterator begin = _steps.begin();
  Steps::iterator end = _steps.end();
  Steps::iterator it = begin;
  for(; it != end; ++it) {
    // Statically resolve our step
    it->step = it->step->staticResolution(context);
  }

  return this;
}

ASTNode *XQNav::staticTypingImpl(StaticContext *context)
{
  _src.clear();

  unsigned int props = StaticAnalysis::DOCORDER | StaticAnalysis::GROUPED |
    StaticAnalysis::PEER | StaticAnalysis::SUBTREE | StaticAnalysis::SAMEDOC |
    StaticAnalysis::ONENODE;

  AutoDelete<Steps> newSteps(0);
  if(context) newSteps.set(new Steps(XQillaAllocator<StepInfo>(context->getMemoryManager())));

  unsigned int min = 1, max = 1;

  Steps::iterator begin = _steps.begin();
  Steps::iterator end = _steps.end();
  Steps::iterator it = begin;
  for(; it != end; ++it) {
    // Statically resolve our step
    ASTNode *step = it->step;
    const StaticAnalysis &stepSrc = step->getStaticAnalysis();

    min *= stepSrc.getStaticType().getMin();
    if(max == StaticType::UNLIMITED || stepSrc.getStaticType().getMax() == StaticType::UNLIMITED)
      max = StaticType::UNLIMITED;
    else max *= stepSrc.getStaticType().getMax();

    props = combineProperties(props, stepSrc.getProperties());

    if(!context || stepSrc.areContextFlagsUsed() || _src.isNoFoldingForced() ||
       stepSrc.getStaticType().containsType(StaticType::ANY_ATOMIC_TYPE) ||
       stepSrc.isCreative()) {
      if(it != begin) {
        // Remove context item usage
        _src.addExceptContextFlags(stepSrc);
      }
      else {
        _src.add(stepSrc);
      }
    }
    else {
      // Constant fold, by clearing all our previous steps and just having our most recent step.
      // This is only possible because the result of steps has to be nodes, and
      // duplicates are removed, which means we aren't forced to execute a constant
      // step a set number of times.
      newSteps->clear();
      _src.add(stepSrc);
    }

    if(context) {
      if(step->getType() == NAVIGATION) {
        Steps &navSteps = ((XQNav*)step)->_steps;
        for(Steps::iterator it2 = navSteps.begin();
            it2 != navSteps.end(); ++it2) {
          newSteps->push_back(it2->step);
        }
      }
      else {
        newSteps->push_back(step);
      }
    }
  }

  if(context) {
    _steps = *newSteps.get();
    newSteps->clear();

    it = _steps.begin();
    end = _steps.end();
    for(; it != end; ++it) {
      // Special case, to optimise //foo or //descendant::foo
      if(it->step->getType() == STEP) {
        XQStep *step = (XQStep*)it->step;
        NodeTest *nodetest = (NodeTest*)step->getNodeTest();

        // Check for a step with a type wildcard
        if(nodetest->getTypeWildcard() && (it + 1) != end) {

          bool usesContextPositionOrSize = false;
          const ASTNode *peek = (it + 1)->step;
          while(peek->getType() == PREDICATE) {
            const XQPredicate *pred = (const XQPredicate*)peek;
            if(pred->getPredicate()->getStaticAnalysis().isContextPositionUsed() ||
               pred->getPredicate()->getStaticAnalysis().isContextSizeUsed() ||
               pred->getPredicate()->getStaticAnalysis().getStaticType().containsType(StaticType::NUMERIC_TYPE)) {
              usesContextPositionOrSize = true;
            }
            peek = pred->getExpression();
          }

          if(peek->getType() == STEP) {
            const XQStep *peekstep = (XQStep*)peek;
            // If the next node is CHILD or DESCENDANT axis, then
            // this step must have children
            if(peekstep->getAxis() == XQStep::CHILD || peekstep->getAxis() == XQStep::DESCENDANT) {

              // Check for a descendant-or-self axis
              if(step->getAxis() == XQStep::DESCENDANT_OR_SELF) {
                if(!usesContextPositionOrSize) {
                  // This is a descendant-or-self::node()/child::foo that we can optimise into descendant::foo
                  ++it;
                  const_cast<XQStep*>(peekstep)->setAxis(XQStep::DESCENDANT);
                  // Set the properties to those for descendant axis
                  const_cast<StaticAnalysis&>(peekstep->getStaticAnalysis()).
                    setProperties(StaticAnalysis::SUBTREE | StaticAnalysis::DOCORDER |
                                  StaticAnalysis::GROUPED | StaticAnalysis::SAMEDOC);
                }
              }

              nodetest->setHasChildren();
            }
            // If the next node is ATTRIBUTE axis, then this step needs to be
            // an element
            else if(peekstep->getAxis() == XQStep::ATTRIBUTE) {
              nodetest->setTypeWildcard(false);
              nodetest->setNodeType(Node::element_string);
            }
          }
        }
      }

      newSteps->push_back(*it);
    }

    _steps = *newSteps.get();
  }

  if(!_steps.empty()) {
    _src.getStaticType() = _steps.back().step->getStaticAnalysis().getStaticType();
    _src.getStaticType().multiply(min, max);
  }

  _src.setProperties(props);

  return this;
}

const XQNav::Steps &XQNav::getSteps() const {
  return _steps;
}

unsigned int XQNav::combineProperties(unsigned int prev_props, unsigned int step_props)
{
  unsigned int new_props = 0;

  if((step_props & StaticAnalysis::SELF)) {
    new_props |= prev_props;
  }

  if((prev_props & StaticAnalysis::ONENODE) && (step_props & StaticAnalysis::DOCORDER)) {
    // If there was only one input node, and the step is in document order
    // then we are still in document order
    new_props |= StaticAnalysis::DOCORDER | StaticAnalysis::GROUPED;
  }

  if((prev_props & StaticAnalysis::DOCORDER) && (prev_props & StaticAnalysis::PEER) &&
     (step_props & StaticAnalysis::DOCORDER) && (step_props & StaticAnalysis::SUBTREE)) {
    // We keep the DOCORDER property, along with a few others that come for free
    new_props |= StaticAnalysis::DOCORDER | StaticAnalysis::GROUPED | StaticAnalysis::SUBTREE;
    if(step_props & StaticAnalysis::PEER) {
      new_props |= StaticAnalysis::PEER;
    }
  }

  if((prev_props & StaticAnalysis::GROUPED) && (step_props & StaticAnalysis::SAMEDOC)) {
    // If the step keeps in the SAMEDOC, then the GROUPED property is maintained
    new_props |= StaticAnalysis::GROUPED;
  }

  if((prev_props & StaticAnalysis::SUBTREE) && (step_props & StaticAnalysis::SUBTREE)) {
    // If both are SUBTREE, then we still have SUBTREE
    new_props |= StaticAnalysis::SUBTREE;
  }

  if((prev_props & StaticAnalysis::SAMEDOC) && (step_props & StaticAnalysis::SAMEDOC)) {
    // If both are SAMEDOC, then we still have SAMEDOC
    new_props |= StaticAnalysis::SAMEDOC;
  }

  if((prev_props & StaticAnalysis::ONENODE) && (step_props & StaticAnalysis::ONENODE)) {
    // If both are ONENODE, then we still have ONENODE
    new_props |= StaticAnalysis::ONENODE;
  }

  return new_props;
}

/////////////////////////////////////
// NavStepResult
/////////////////////////////////////

NavStepResult::NavStepResult(const Result &parent, const ASTNode *step, size_t contextSize)
  : ResultImpl(step),
    parent_(parent),
    step_(step),
    stepResult_(0),
    contextPos_(0),
    contextSize_(contextSize),
    contextItem_(0)
{
}

Item::Ptr NavStepResult::next(DynamicContext *context)
{
  AutoContextInfoReset autoReset(context);

  context->setContextSize(contextSize_);
  context->setContextPosition(contextPos_);
  context->setContextItem(contextItem_);

  Item::Ptr result = stepResult_->next(context);
  while(result.isNull()) {
    context->testInterrupt();

    autoReset.resetContextInfo();

    contextItem_ = parent_->next(context);
    if(contextItem_.isNull()) {
      parent_ = 0;
      return 0;
    }
    ++contextPos_;

    context->setContextSize(contextSize_);
    context->setContextPosition(contextPos_);
    context->setContextItem(contextItem_);

    stepResult_ = step_->createResult(context);
    result = stepResult_->next(context);
  }

  return result;
}

IntermediateStepCheckResult::IntermediateStepCheckResult(const LocationInfo *o, const Result &parent)
  : ResultImpl(o),
    parent_(parent)
{
}

Item::Ptr IntermediateStepCheckResult::next(DynamicContext *context)
{
  Item::Ptr result = parent_->next(context);

  // Check it's a node
  if(!result.isNull() && !result->isNode()) {
    XQThrow(TypeErrorException,X("NavStepResult::next"),
             X("The result of a step expression (StepExpr) is not a sequence of nodes [err:XPTY0019]"));
  }

  return result;
}

LastStepCheckResult::LastStepCheckResult(const LocationInfo *o, const Result &parent)
  : ResultImpl(o),
    parent_(parent),
    _nTypeOfItemsInLastStep(0)
{
}

Item::Ptr LastStepCheckResult::next(DynamicContext *context)
{
  Item::Ptr result = parent_->next(context);

  if(result != NULLRCP) {
    // the last step allows either nodes or atomic items
    switch(_nTypeOfItemsInLastStep) {
    case 0: _nTypeOfItemsInLastStep=result->isNode()?1:2; break;
    case 1: if(!result->isNode()) 
      XQThrow(TypeErrorException,X("LastStepCheckResult::next"),
               X("The result of the last step in a path expression contains both nodes and atomic values [err:XPTY0018]"));
      break;
    case 2: if(result->isNode()) 
      XQThrow(TypeErrorException,X("LastStepCheckResult::next"),
               X("The result of the last step in a path expression contains both nodes and atomic values [err:XPTY0018]"));
      break;
    }
  }

  return result;
}


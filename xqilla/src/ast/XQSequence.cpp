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
#include <assert.h>
#include <xqilla/ast/XQSequence.hpp>
#include <xqilla/ast/XQLiteral.hpp>
#include <xqilla/update/PendingUpdateList.hpp>
#include <xqilla/context/DynamicContext.hpp>
#include <xqilla/items/Item.hpp>
#include <xqilla/exceptions/StaticErrorException.hpp>
#include <xqilla/runtime/ClosureResult.hpp>

static const unsigned int CONSTANT_FOLD_LIMIT = 4;

XQSequence *XQSequence::constantFold(Result &result, DynamicContext *context, XPath2MemoryManager* memMgr,
                                     const LocationInfo *location)
{
  XQSequence *seq = new (memMgr) XQSequence(memMgr);
  seq->setLocationInfo(location);

  Item::Ptr item;
  while((item = result->next(context)).notNull()) {
    if(seq->_astNodes.size() > CONSTANT_FOLD_LIMIT) {
      seq->release();
      return 0;
    }
    seq->addItem(XQLiteral::create(item, context, memMgr, location));
  }

  // Don't specify a context for staticTyping
  seq->staticTypingImpl(0);
  return seq;
}

XQSequence::XQSequence(XPath2MemoryManager* memMgr)
  : ASTNodeImpl(SEQUENCE, memMgr),
    _astNodes(XQillaAllocator<ASTNode*>(memMgr))
{
}

XQSequence::XQSequence(const VectorOfASTNodes &children, XPath2MemoryManager* memMgr)
  : ASTNodeImpl(SEQUENCE, memMgr),
    _astNodes(XQillaAllocator<ASTNode*>(memMgr))
{
  _astNodes = children;
}

void XQSequence::addItem(ASTNode* di)
{
  _astNodes.push_back(di);
}

ASTNode* XQSequence::staticResolution(StaticContext *context)
{
  for(VectorOfASTNodes::iterator i = _astNodes.begin(); i != _astNodes.end(); ++i) {
    *i = (*i)->staticResolution(context);
  }
  return this;
}

ASTNode *XQSequence::staticTypingImpl(StaticContext *context)
{
  _src.clear();

  bool doneOne = false;
  bool possiblyUpdating = true;
  bool nestedSeq = false;
  VectorOfASTNodes::iterator i = _astNodes.begin();

  if(i == _astNodes.end()) {
    _src.possiblyUpdating(true);
  }

  for(; i != _astNodes.end(); ++i) {
    if(_src.isUpdating()) {
      if(!(*i)->getStaticAnalysis().isUpdating() &&
         !(*i)->getStaticAnalysis().isPossiblyUpdating())
        XQThrow(StaticErrorException, X("XQSequence::staticTyping"),
                X("Mixed updating and non-updating operands [err:XUST0001]"));
    }
    else {
      if((*i)->getStaticAnalysis().isUpdating() && !possiblyUpdating)
        XQThrow(StaticErrorException, X("XQSequence::staticTyping"),
                X("Mixed updating and non-updating operands [err:XUST0001]"));
    }

    if(possiblyUpdating)
      possiblyUpdating = (*i)->getStaticAnalysis().isPossiblyUpdating();

    if(!doneOne) {
      doneOne = true;
      _src.getStaticType() = (*i)->getStaticAnalysis().getStaticType();
    } else {
      _src.getStaticType().typeConcat((*i)->getStaticAnalysis().getStaticType());
    }

    if((*i)->getType() == SEQUENCE)
      nestedSeq = true;

    _src.add((*i)->getStaticAnalysis());
  }

  if(context && nestedSeq) {
    XPath2MemoryManager *mm = context->getMemoryManager();
    VectorOfASTNodes newArgs = VectorOfASTNodes(XQillaAllocator<ASTNode*>(mm));
    for(i = _astNodes.begin(); i != _astNodes.end(); ++i) {
      if((*i)->getType() == SEQUENCE) {
        XQSequence *arg = (XQSequence*)*i;
        for(VectorOfASTNodes::iterator j = arg->_astNodes.begin(); j != arg->_astNodes.end(); ++j) {
          newArgs.push_back(*j);
        }
      }
      else {
        newArgs.push_back(*i);
      }
    }
    _astNodes = newArgs;
  }

  // Dissolve ourselves if we have only one child
  if(context && _astNodes.size() == 1) {
    return _astNodes.front();
  }
  return this;
}

EventGenerator::Ptr XQSequence::generateEvents(EventHandler *events, DynamicContext *context,
                                                        bool preserveNS, bool preserveType) const
{
  VectorOfASTNodes::const_iterator i = _astNodes.begin();
  VectorOfASTNodes::const_iterator end = _astNodes.end();
  if(i == end) return 0;
  --end;
  for(; i != end; ++i) {
    (*i)->generateAndTailCall(events, context, preserveNS, preserveType);
  }
  return new ClosureEventGenerator(*i, context, preserveNS, preserveType);
}

PendingUpdateList XQSequence::createUpdateList(DynamicContext *context) const
{
  PendingUpdateList result;
  for(VectorOfASTNodes::const_iterator i = _astNodes.begin(); i != _astNodes.end(); ++i) {
    result.mergeUpdates((*i)->createUpdateList(context));
  }
  return result;
}

class XQSequenceResult : public ResultImpl
{
public:
  XQSequenceResult(const XQSequence *ast)
    : ResultImpl(ast),
      ast_(ast),
      i_(ast->getChildren().begin()),
      result_(0)
  {
  }

  virtual Item::Ptr nextOrTail(Result &tail, DynamicContext *context)
  {
    Item::Ptr item = result_->next(context);

    while(item.isNull()) {
      const ASTNode *ast = *i_;
      ++i_;

      if(i_ == ast_->getChildren().end()) {
        // Tail call optimisation
        tail = ast->createResult(context);
        return 0;
      }

      result_ = ast->createResult(context);
      item = result_->next(context);
    }

    return item;
  }

private:
  const XQSequence *ast_;
  VectorOfASTNodes::const_iterator i_;
  Result result_;
};

Result XQSequence::createResult(DynamicContext* context, int flags) const
{
  if(_astNodes.empty()) return 0;
  return ClosureResult::create(getStaticAnalysis(), context, new XQSequenceResult(this));
}

BoolResult XQSequence::boolResult(DynamicContext* context) const
{
  if(_astNodes.empty()) return BoolResult::Null;
  if(_astNodes.size() == 1) return _astNodes[0]->boolResult(context);
  return ASTNodeImpl::boolResult(context);
}

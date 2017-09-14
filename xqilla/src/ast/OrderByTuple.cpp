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

#include <xqilla/ast/ForTuple.hpp>
#include <xqilla/ast/LetTuple.hpp>
#include <xqilla/ast/OrderByTuple.hpp>
#include <xqilla/context/DynamicContext.hpp>
#include <xqilla/context/ContextHelpers.hpp>
#include <xqilla/context/impl/VarStoreImpl.hpp>
#include <xqilla/exceptions/StaticErrorException.hpp>
#include <xqilla/utils/XStr.hpp>
#include <xqilla/operators/GreaterThan.hpp>

using namespace std;

OrderByTuple::OrderByTuple(TupleNode *parent, ASTNode *expr, int modifiers, Collation *collation, XPath2MemoryManager *mm)
  : TupleNode(ORDER_BY, parent, mm),
    expr_(expr),
    modifiers_((Modifiers)modifiers),
    collation_(collation),
    usedSrc_(mm)
{
}

TupleNode *OrderByTuple::staticResolution(StaticContext *context)
{
  parent_ = parent_->staticResolution(context);

  AutoNodeSetOrderingReset orderReset(context, (modifiers_ & UNSTABLE) == 0 ?
                                      StaticContext::ORDERING_ORDERED : StaticContext::ORDERING_UNORDERED);
  expr_ = expr_->staticResolution(context);

  return this;
}

static bool canPushOrderByBack(TupleNode *ancestor, const StaticAnalysis &exprSrc)
{
  switch(ancestor->getType()) {
  case TupleNode::FOR: {
    ForTuple *f = (ForTuple*)ancestor;
    if(!exprSrc.isVariableUsed(f->getVarURI(), f->getVarName()) &&
       !exprSrc.isVariableUsed(f->getPosURI(), f->getPosName()))
      return true;
    break;
  }
  case TupleNode::LET: {
    LetTuple *f = (LetTuple*)ancestor;
    if(!exprSrc.isVariableUsed(f->getVarURI(), f->getVarName()))
      return true;
    break;
  }
  case TupleNode::WHERE:
    return canPushOrderByBack(ancestor->getParent(), exprSrc);
  case TupleNode::COUNT:
  case TupleNode::ORDER_BY:
  case TupleNode::CONTEXT_TUPLE:
    break;
  default:
    return true;
  }
  return false;
}

TupleNode *OrderByTuple::staticTypingImpl(StaticContext *context)
{
  if(expr_->getStaticAnalysis().isUpdating()) {
    XQThrow(StaticErrorException,X("OrderByTuple::staticTypingSetup"),
            X("It is a static error for the order by expression of a FLWOR expression "
              "to be an updating expression [err:XUST0001]"));
  }

  // Push back if possible
  if(canPushOrderByBack(parent_, expr_->getStaticAnalysis())) {
    // Swap parent_ and this OrderByTuple, re-executing their staticTypingImpl() methods
    TupleNode *tmp = parent_;
    parent_ = tmp->getParent();
    tmp->setParent(this->staticTypingImpl(context));
    return tmp->staticTypingImpl(context);
  }

  min_ = parent_->getMin();
  max_ = parent_->getMax();

  return this;
}

TupleNode *OrderByTuple::staticTypingTeardown(StaticContext *context, StaticAnalysis &usedSrc)
{
  usedSrc_.clear();
  usedSrc_.add(usedSrc);

  usedSrc.add(expr_->getStaticAnalysis());
  parent_ = parent_->staticTypingTeardown(context, usedSrc);

  return this;
}

class OrderByTupleResult : public TupleResult
{
public:
  OrderByTupleResult(const OrderByTuple *ast, const TupleResult::Ptr &parent)
    : TupleResult(ast),
      ast_(ast),
      parent_(parent),
      toDo_(true),
      tuples_(),
      tupleIt_(tuples_.begin())
  {
  }

  ~OrderByTupleResult()
  {
    vector<OrderPair*>::iterator it = tuples_.begin();
    for(; it != tuples_.end(); ++it) {
      delete *it;
    }
  }

  virtual Result getVar(const XMLCh *namespaceURI, const XMLCh *name) const
  {
    return (*tupleIt_)->varStore.getVar(namespaceURI, name);
  }

  virtual void getInScopeVariables(std::vector<std::pair<const XMLCh*, const XMLCh*> > &variables) const
  {
    (*tupleIt_)->varStore.getInScopeVariables(variables);
  }

  virtual bool next(DynamicContext *context)
  {
    if(toDo_) {
      toDo_ = false;

      XPath2MemoryManager *mm = context->getMemoryManager();
      const ASTNode *expr = ast_->getExpression();
      const StaticAnalysis &usedSrc = ast_->getUsedSRC();

      while(parent_->next(context)) {
        AutoVariableStoreReset reset(context, parent_);
        tuples_.push_back(new OrderPair((AnyAtomicType*)expr->createResult(context)->
                                        next(context).get(), usedSrc, parent_, mm));
      }

      stable_sort(tuples_.begin(), tuples_.end(),
                  OrderComparison(ast_->getModifiers(), ast_->getCollation(), context, this));
      tupleIt_ = tuples_.begin();
    } else {
      delete *tupleIt_;
      *tupleIt_ = 0;
      ++tupleIt_;
    }

    return tupleIt_ != tuples_.end();
  }

private:
  class OrderComparison;
  friend class OrderComparison;

  class OrderPair
  {
  public:
    static inline bool isEmptyOrNaN(const AnyAtomicType::Ptr &si)
    {
      return si.isNull() || (si->isNumericValue() && ((Numeric*)si.get())->isNaN());
    }

    OrderPair(const AnyAtomicType::Ptr &si, const StaticAnalysis &usedSrc, const VariableStore *vars,
              XPath2MemoryManager *mm)
		: sortItem(isEmptyOrNaN(si) ? (AnyAtomicType::Ptr)0 : si),
        varStore(mm)
    {
      varStore.cacheVariableStore(usedSrc, vars);
    }

    AnyAtomicType::Ptr sortItem;
    VarStoreImpl varStore;
  };

  class OrderComparison
  {
  public:
    OrderComparison(OrderByTuple::Modifiers mod, Collation *col, DynamicContext *cn, const LocationInfo *loc)
      : modifiers(mod), collation(col), context(cn), location(loc) {}

    inline bool greaterThan(const OrderPair *w, const OrderPair *v) const
    {
      if((modifiers & OrderByTuple::EMPTY_LEAST) != 0) {
        // When the orderspec specifies empty least, a value W is considered to be greater-than a value V if one of the following is true:
        //     * V is an empty sequence and W is not an empty sequence.
        //     * V is NaN, and W is neither NaN nor an empty sequence.
        if(w->sortItem.isNull()) return false;
        if(v->sortItem.isNull()) return !w->sortItem.isNull();
      }
      else {
        // When the orderspec specifies empty greatest, a value W is considered to be greater-than a value V if one of the following is true:
        //     * W is an empty sequence and V is not an empty sequence.
        //     * W is NaN, and V is neither NaN nor an empty sequence.
        if(v->sortItem.isNull()) return false;
        if(w->sortItem.isNull()) return !v->sortItem.isNull();
      }
      //     * No collation is specified, and W gt V is true.
      //     * A specific collation C is specified, and fn:compare(V, W, C) is less than zero.
      return GreaterThan::greater_than(w->sortItem, v->sortItem, collation, context, location);
    }

    bool operator()(const OrderPair *a, const OrderPair *b) const
    {
      //    1. If V1 is greater-than V2: If the orderspec specifies descending, then T1 precedes T2 in the tuple stream; otherwise, T2 precedes T1 in the tuple stream.
      if((modifiers & OrderByTuple::DESCENDING) != 0) return greaterThan(a, b);
      //    2. If V2 is greater-than V1: If the orderspec specifies descending, then T2 precedes T1 in the tuple stream; otherwise, T1 precedes T2 in the tuple stream.
      return greaterThan(b, a);
    }

    OrderByTuple::Modifiers modifiers;
    Collation *collation;
    DynamicContext *context;
    const LocationInfo *location;
  };

  const OrderByTuple *ast_;
  TupleResult::Ptr parent_;
  bool toDo_;

  vector<OrderPair*> tuples_;
  vector<OrderPair*>::iterator tupleIt_;
};

TupleResult::Ptr OrderByTuple::createResult(DynamicContext* context) const
{
  return new OrderByTupleResult(this, parent_->createResult(context));
}


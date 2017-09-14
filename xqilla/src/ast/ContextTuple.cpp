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

#include <xqilla/ast/ContextTuple.hpp>
#include <xqilla/context/DynamicContext.hpp>

TupleNode *ContextTuple::staticResolution(StaticContext *context)
{
  return this;
}

TupleNode *ContextTuple::staticTypingImpl(StaticContext *context)
{
  min_ = 1;
  max_ = 1;
  return this;
}

TupleNode *ContextTuple::staticTypingTeardown(StaticContext *context, StaticAnalysis &usedSrc)
{
  return this;
}

class ContextTupleResult : public TupleResult
{
public:
  ContextTupleResult(const LocationInfo *loc)
    : TupleResult(loc),
      varStore_(0)
  {
  }

  virtual Result getVar(const XMLCh *namespaceURI, const XMLCh *name) const
  {
    return varStore_->getVar(namespaceURI, name);
  }

  virtual void getInScopeVariables(std::vector<std::pair<const XMLCh*, const XMLCh*> > &variables) const
  {
    varStore_->getInScopeVariables(variables);
  }

  virtual bool next(DynamicContext *context)
  {
    if(varStore_ != 0) return false;

    varStore_ = context->getVariableStore();
    return true;
  }

private:
  const VariableStore *varStore_;
};

TupleResult::Ptr ContextTuple::createResult(DynamicContext* context) const
{
  return new ContextTupleResult(this);
}


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

#include <xqilla/optimizer/Optimizer.hpp>
#include <xqilla/ast/ASTNode.hpp>
#include <xqilla/context/DynamicContext.hpp>
#include <xqilla/simple-api/XQQuery.hpp>
#include <xqilla/functions/XQUserFunction.hpp>

using namespace std;

Optimizer::Optimizer(Optimizer *parent)
  : parent_(parent)
{}

Optimizer::~Optimizer()
{
  if(parent_ != 0) {
    delete parent_;
  }
}

void Optimizer::resetInternal()
{
}

void Optimizer::startOptimize(XQQuery *query)
{
  if(parent_ != 0) {
    parent_->startOptimize(query);
  }
  
  optimize(query);
}

ASTNode *Optimizer::startOptimize(ASTNode *item)
{
  if(parent_ != 0) {
    item = parent_->startOptimize(item);
  }
  return optimize(item);
}

XQUserFunction *Optimizer::startOptimize(XQUserFunction *item)
{
  if(parent_ != 0) {
    item = parent_->startOptimize(item);
  }
  return optimizeFunctionDef(item);
}

void Optimizer::reset()
{
  if(parent_ != 0) {
    parent_->reset();
  }
  resetInternal();
}

void StaticResolver::optimize(XQQuery *query)
{
  query->staticResolution();
}

ASTNode *StaticResolver::optimize(ASTNode *item)
{
  return item->staticResolution(xpc_);
}

XQUserFunction *StaticResolver::optimizeFunctionDef(XQUserFunction *item)
{
  item->staticResolutionStage2(xpc_);
  return item;
}


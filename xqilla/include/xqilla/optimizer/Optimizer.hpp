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

#ifndef _OPTIMIZER_HPP
#define _OPTIMIZER_HPP

#include <xqilla/framework/XQillaExport.hpp>

class ASTNode;
class DynamicContext;

class XQQuery;
class XQUserFunction;

class XQILLA_API Optimizer
{
public:
  Optimizer(Optimizer *parent = 0);
  virtual ~Optimizer();

  void startOptimize(XQQuery *query);
  ASTNode *startOptimize(ASTNode *item);
  XQUserFunction *startOptimize(XQUserFunction *item);
  void reset();

protected:
  virtual void optimize(XQQuery *query) = 0;
  virtual ASTNode *optimize(ASTNode *item) = 0;
  virtual XQUserFunction *optimizeFunctionDef(XQUserFunction *item) = 0;
  virtual void resetInternal();

private:
  Optimizer(const Optimizer &o);
  Optimizer &operator=(const Optimizer &o);

  Optimizer *parent_;
};

class XQILLA_API StaticResolver : public Optimizer
{
public:
  StaticResolver(DynamicContext *xpc, Optimizer *parent = 0)
    : Optimizer(parent), xpc_(xpc) {}

protected:
  virtual void optimize(XQQuery *query);
  virtual ASTNode *optimize(ASTNode *item);
  virtual XQUserFunction *optimizeFunctionDef(XQUserFunction *item);

private:
  DynamicContext *xpc_;
};

#endif

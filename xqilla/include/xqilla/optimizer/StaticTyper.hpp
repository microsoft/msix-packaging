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

#ifndef _STATICTYPER_HPP
#define _STATICTYPER_HPP

#include <xqilla/optimizer/ASTVisitor.hpp>

class XQILLA_API StaticTyper : public ASTVisitor
{
public:
  struct PrologItem {
    PrologItem(XQGlobalVariable *g, PrologItem *p) : global(g), function(0), prev(p) {}
    PrologItem(XQUserFunction *f, PrologItem *p) : global(0), function(f), prev(p) {}

    XQGlobalVariable *global;
    XQUserFunction *function;
    PrologItem *prev;
  };

  StaticTyper()
    : ASTVisitor(), context_(0), globalsUsed_(0), globalsOrder_(0),
    trail_(0), tupleSetup_(false) {}
  StaticTyper(StaticContext *context, Optimizer *parent = 0)
    : ASTVisitor(parent), context_(context), globalsUsed_(0), globalsOrder_(0),
    trail_(0), tupleSetup_(false) {}

  GlobalVariables *&getGlobalsUsed() { return globalsUsed_; }
  GlobalVariables *&getGlobalsOrder() { return globalsOrder_; }
  PrologItem *&getTrail() { return trail_; }

  ASTNode *run(ASTNode *item, StaticContext *context);

protected:
  virtual void optimize(XQQuery *query);
  virtual XQUserFunction *optimizeFunctionDef(XQUserFunction *item);

  virtual ASTNode *optimize(ASTNode *item);
  virtual ASTNode *optimizePredicate(XQPredicate *item);
  virtual ASTNode *optimizeAnalyzeString(XQAnalyzeString *item);
  virtual ASTNode *optimizeNav(XQNav *item);
  virtual ASTNode *optimizeQuantified(XQQuantified *item);
  virtual ASTNode *optimizeMap(XQMap *item);
  virtual ASTNode *optimizeReturn(XQReturn *item);
  virtual ASTNode *optimizeTypeswitch(XQTypeswitch *item);
  virtual ASTNode *optimizeFunctionCoercion(XQFunctionCoercion *item);
  virtual ASTNode *optimizeNamespaceBinding(XQNamespaceBinding *item);
  virtual ASTNode *optimizeInlineFunction(XQInlineFunction *item);
  virtual ASTNode *optimizeEffectiveBooleanValue(XQEffectiveBooleanValue *item);
  virtual ASTNode *optimizeUTransform(UTransform *item);
  virtual ASTNode *optimizeVariable(XQVariable *item);
  virtual ASTNode *optimizeUserFunction(XQUserFunctionInstance *item);

  virtual ASTNode *optimizeFTContains(FTContains *item);
  virtual FTSelection *optimizeFTSelection(FTSelection *selection);

  virtual TupleNode *optimizeTupleNode(TupleNode *item);
  virtual TupleNode *optimizeForTuple(ForTuple *item);
  virtual TupleNode *optimizeLetTuple(LetTuple *item);
  virtual TupleNode *optimizeWhereTuple(WhereTuple *item);
  virtual TupleNode *optimizeCountTuple(CountTuple *item);
  virtual TupleNode *optimizeOrderByTuple(OrderByTuple *item);

  void optimizeCase(const StaticAnalysis &var_src, XQTypeswitch::Case *item);

  StaticContext *context_;
  GlobalVariables *globalsUsed_;
  GlobalVariables *globalsOrder_;
  PrologItem *trail_;
  bool tupleSetup_;
};

#endif

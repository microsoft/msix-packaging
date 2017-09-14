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

#ifndef _PARTIALEVALUATOR_HPP
#define _PARTIALEVALUATOR_HPP

#include <xqilla/optimizer/ASTVisitor.hpp>

class Plus;
class Minus;
class Multiply;
class Divide;
class And;
class Or;

class XQILLA_API PartialEvaluator : public ASTVisitor
{
public:
  PartialEvaluator(DynamicContext *context, Optimizer *parent = 0);

  static ASTNode *inlineFunction(const XQUserFunctionInstance *item, DynamicContext *context);

protected:
  virtual void optimize(XQQuery *query);
  virtual ASTNode *optimize(ASTNode *item);
  virtual XQUserFunction *optimizeFunctionDef(XQUserFunction *item);
  virtual ASTNode *optimizeUserFunction(XQUserFunctionInstance *item);
  virtual ASTNode *optimizeFunctionDeref(XQFunctionDeref *item);
  virtual ASTNode *optimizePartialApply(XQPartialApply *item);
  virtual ASTNode *optimizeFunctionCoercion(XQFunctionCoercion *item);
  virtual ASTNode *optimizeReturn(XQReturn *item);
  virtual ASTNode *optimizeIf(XQIf *item);
  virtual ASTNode *optimizeQuantified(XQQuantified *item);
  virtual ASTNode *optimizePredicate(XQPredicate *item);
  virtual ASTNode *optimizeEffectiveBooleanValue(XQEffectiveBooleanValue *item);
  virtual ASTNode *optimizeOperator(XQOperator *item);
  virtual ASTNode *optimizeNamespaceBinding(XQNamespaceBinding *item);
  virtual ASTNode *optimizeInlineFunction(XQInlineFunction *item);
  virtual ASTNode *optimizeFunction(XQFunction *item);
  virtual ASTNode *optimizeTypeswitch(XQTypeswitch *item);
  virtual ASTNode *optimizeTreatAs(XQTreatAs *item);

  ASTNode *optimizePlus(Plus *item);
  ASTNode *optimizeMinus(Minus *item);
  ASTNode *optimizeMultiply(Multiply *item);
  ASTNode *optimizeDivide(Divide *item);
  ASTNode *optimizeAnd(And *item);
  ASTNode *optimizeOr(Or *item);

  bool checkSizeLimit(const ASTNode *oldAST, const ASTNode *newAST);

  DynamicContext *context_;
  size_t functionInlineLimit_;
  size_t sizeLimit_;

  bool redoTyping_;
};

#endif

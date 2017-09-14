/*
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

#ifndef _ASTTOXML_HPP
#define _ASTTOXML_HPP

#include <string>
#include <iostream>

#include <xqilla/optimizer/ASTVisitor.hpp>
#include <xqilla/operators/GeneralComp.hpp>

class XQILLA_API ASTToXML : public ASTVisitor
{
public:
  ASTToXML();

  std::string print(const XQQuery *query, const DynamicContext *context);
  std::string print(const ASTNode *item, const DynamicContext *context);
  std::string print(const TupleNode *item, const DynamicContext *context);
  std::string print(const XQUserFunction *item, const DynamicContext *context);

  void run(const XQQuery *query, EventHandler *events, const DynamicContext *context);
  void run(const ASTNode *item, EventHandler *events, const DynamicContext *context);
  void run(const TupleNode *item, EventHandler *events, const DynamicContext *context);
  void run(const XQUserFunction *item, EventHandler *events, const DynamicContext *context);

protected:
  virtual void resetInternal() {}

  virtual void optimize(XQQuery *query);
  virtual ASTNode *optimize(ASTNode *item);
  virtual XQGlobalVariable *optimizeGlobalVar(XQGlobalVariable *item);
  virtual XQUserFunction *optimizeFunctionDef(XQUserFunction *item);
  virtual ASTNode *optimizeFunction(XQFunction *item);
  virtual ASTNode *optimizeOperator(XQOperator *item);
  virtual ASTNode *optimizeLiteral(XQLiteral *item);
  virtual ASTNode *optimizeQNameLiteral(XQQNameLiteral *item);
  virtual ASTNode *optimizeNumericLiteral(XQNumericLiteral *item);
  virtual ASTNode *optimizeStep(XQStep *item);
  virtual ASTNode *optimizeInlineFunction(XQInlineFunction *item);
  virtual ASTNode *optimizePromoteUntyped(XQPromoteUntyped *item);
  virtual ASTNode *optimizeVariable(XQVariable *item);
  virtual ASTNode *optimizeTreatAs(XQTreatAs *item);
  virtual ASTNode *optimizePromoteNumeric(XQPromoteNumeric *item);
  virtual ASTNode *optimizePromoteAnyURI(XQPromoteAnyURI *item);
  virtual ASTNode *optimizeUserFunction(XQUserFunctionInstance *item);
  virtual ASTNode *optimizeCastableAs(XQCastableAs *item);
  virtual ASTNode *optimizeCastAs(XQCastAs *item);
  virtual ASTNode *optimizeFunctionCoercion(XQFunctionCoercion *item);
  virtual ASTNode *optimizeTypeswitch(XQTypeswitch *item);
  virtual ASTNode *optimizePartialApply(XQPartialApply *item);
  virtual ASTNode *optimizeFunctionRef(XQFunctionRef *item);
  virtual ASTNode *optimizeFunctionConversion(XQFunctionConversion *item);
  virtual ASTNode *optimizeValidate(XQValidate *item);
  virtual ASTNode *optimizeFunctionCall(XQFunctionCall *item);
  virtual ASTNode *optimizeDOMConstructor(XQDOMConstructor *item);
  virtual ASTNode *optimizeMap(XQMap *item);
  virtual ASTNode *optimizeDirectName(XQDirectName *item);
  virtual ASTNode *optimizeUTransform(UTransform *item);
  virtual ASTNode *optimizeCopyOf(XQCopyOf *item);
  virtual ASTNode *optimizeASTDebugHook(ASTDebugHook *item);
  virtual ASTNode *optimizeCallTemplate(XQCallTemplate *item);
  virtual ASTNode *optimizeApplyTemplates(XQApplyTemplates *item);

  virtual TupleNode *optimizeTupleNode(TupleNode *item);
  virtual TupleNode *optimizeForTuple(ForTuple *item);
  virtual TupleNode *optimizeLetTuple(LetTuple *item);
  virtual TupleNode *optimizeCountTuple(CountTuple *item);
  virtual TupleNode *optimizeOrderByTuple(OrderByTuple *item);
  virtual TupleNode *optimizeTupleDebugHook(TupleDebugHook *item);

  virtual FTSelection *optimizeFTSelection(FTSelection *selection);
  virtual FTSelection *optimizeFTWords(FTWords *selection);
  virtual FTSelection *optimizeFTWord(FTWord *selection);
  virtual FTSelection *optimizeFTDistance(FTDistance *selection);
  virtual FTSelection *optimizeFTDistanceLiteral(FTDistanceLiteral *selection);
  virtual FTSelection *optimizeFTWindow(FTWindow *selection);
  virtual FTSelection *optimizeFTWindowLiteral(FTWindowLiteral *selection);
  virtual FTSelection *optimizeFTScope(FTScope *selection);
  virtual FTSelection *optimizeFTContent(FTContent *selection);

  void optimizeSequenceType(const SequenceType *type);
  void optimizeFunctionSignature(const FunctionSignature *signature);
  void optimizeCase(const XQTypeswitch::Case *cse);
  void optimizeLocation(const LocationInfo *location);
  void optimizeTemplateArgument(XQTemplateArgument *item);
  void optimizeNodeTest(const NodeTest *step);

  virtual void getElementName(ASTNode *item, XERCES_CPP_NAMESPACE_QUALIFIER XMLBuffer &buf);
  virtual void getElementName(TupleNode *item, XERCES_CPP_NAMESPACE_QUALIFIER XMLBuffer &buf);
  virtual void getElementName(FTSelection *item, XERCES_CPP_NAMESPACE_QUALIFIER XMLBuffer &buf);

  static const XMLCh *getComparisonOperationName(GeneralComp::ComparisonOperation co);
  static const XMLCh *getAxisName(XQStep::Axis axis);
  static const XMLCh *getFTRangeTypeName(FTRange::Type type);
  static const XMLCh *getFTUnitName(FTOption::FTUnit unit);

  void indent();
  void newline();
  void qname(const XMLCh *qname, const XMLCh *prefix, const XMLCh *uri, const XMLCh *name,
             XERCES_CPP_NAMESPACE_QUALIFIER XMLBuffer &buf);

  EventHandler *events_;
  const DynamicContext *context_;
  unsigned int indent_;
  XERCES_CPP_NAMESPACE_QUALIFIER XMLBuffer indentBuffer_;
  bool hasChildren_;
};

class PrintASTOptimizer : public Optimizer
{
public:
  PrintASTOptimizer(std::string label, const DynamicContext *context, Optimizer *parent = 0)
    : Optimizer(parent), label_(label), context_(context) {}
protected:
  virtual void optimize(XQQuery *query)
  {
    std::cerr << label_ << ":" << std::endl;
    std::cerr << ASTToXML().print(query, context_) << std::endl;
  }
  virtual ASTNode *optimize(ASTNode *item)
  {
    std::cerr << label_ << ":" << std::endl;
    std::cerr << ASTToXML().print(item, context_) << std::endl;
    return item;
  }
  virtual XQUserFunction *optimizeFunctionDef(XQUserFunction *item)
  {
    std::cerr << label_ << ":" << std::endl;
    std::cerr << ASTToXML().print(item, context_) << std::endl;
    return item;
  }
private:
  std::string label_;
  const DynamicContext *context_;
};

#endif

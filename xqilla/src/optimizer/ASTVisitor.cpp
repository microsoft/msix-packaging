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

#include <xqilla/optimizer/ASTVisitor.hpp>

using namespace std;

void ASTVisitor::optimize(XQQuery *query)
{
  if(query->isModuleCacheOwned()) {
    ImportedModules &modules = const_cast<ImportedModules&>(query->getModuleCache()->ordered_);
    for(ImportedModules::iterator it2 = modules.begin(); it2 != modules.end(); ++it2) {
      optimize(*it2);
    }
  }

  vector<XQGlobalVariable*> newVars;
  GlobalVariables &vars = const_cast<GlobalVariables&>(query->getVariables());
  for(GlobalVariables::iterator it = vars.begin(); it != vars.end(); ++it) {
    XQGlobalVariable *newGV = optimizeGlobalVar(*it);
    if(newGV != 0) {
      newVars.push_back(newGV);
    }
  }
  vars.clear();
  for(vector<XQGlobalVariable*>::iterator i = newVars.begin();
      i != newVars.end(); ++i) {
    vars.push_back(*i);
  }

  UserFunctions &funcs = const_cast<UserFunctions&>(query->getFunctions());
  for(UserFunctions::iterator i2 = funcs.begin(); i2 != funcs.end(); ++i2) {
    *i2 = optimizeFunctionDef(*i2);
  }

  if(query->getQueryBody() != 0) {
    query->setQueryBody(optimize(query->getQueryBody()));
  }
}

XQGlobalVariable *ASTVisitor::optimizeGlobalVar(XQGlobalVariable *item)
{
  if(item->getVariableExpr()) {
    item->setVariableExpr(optimize(const_cast<ASTNode *>(item->getVariableExpr())));
  }
  return item;
}

XQUserFunction *ASTVisitor::optimizeFunctionDef(XQUserFunction *item)
{
  if(item->getFunctionBody()) {
    item->setFunctionBody(optimize(const_cast<ASTNode*>(item->getFunctionBody())));
  }
  if(item->getPattern()) {
    VectorOfASTNodes::iterator patIt = item->getPattern()->begin();
    for(; patIt != item->getPattern()->end(); ++patIt) {
      (*patIt) = optimize(*patIt);
    }
  }
  if(item->getTemplateInstance()) {
    item->setTemplateInstance(optimize(item->getTemplateInstance()));
  }
  return item;
}

ASTNode *ASTVisitor::optimize(ASTNode *item)
{
  switch(item->getType()) {
  case ASTNode::LITERAL:
    return optimizeLiteral((XQLiteral *)item);
  case ASTNode::QNAME_LITERAL:
    return optimizeQNameLiteral((XQQNameLiteral *)item);
  case ASTNode::NUMERIC_LITERAL:
    return optimizeNumericLiteral((XQNumericLiteral *)item);
  case ASTNode::SEQUENCE:
    return optimizeSequence((XQSequence *)item);
  case ASTNode::FUNCTION:
    return optimizeFunction((XQFunction *)item);
  case ASTNode::NAVIGATION:
    return optimizeNav((XQNav *)item);
  case ASTNode::VARIABLE:
    return optimizeVariable((XQVariable *)item);
  case ASTNode::STEP:
    return optimizeStep((XQStep *)item);
  case ASTNode::IF:
    return optimizeIf((XQIf *)item);
  case ASTNode::CASTABLE_AS:
    return optimizeCastableAs((XQCastableAs *)item);
  case ASTNode::CAST_AS:
    return optimizeCastAs((XQCastAs *)item);
  case ASTNode::TREAT_AS:
    return optimizeTreatAs((XQTreatAs *)item);
  case ASTNode::FUNCTION_COERCION:
    return optimizeFunctionCoercion((XQFunctionCoercion *)item);
  case ASTNode::OPERATOR:
    return optimizeOperator((XQOperator *)item);
  case ASTNode::CONTEXT_ITEM:
    return optimizeContextItem((XQContextItem *)item);
  case ASTNode::RETURN:
    return optimizeReturn((XQReturn *)item);
  case ASTNode::QUANTIFIED:
    return optimizeQuantified((XQQuantified *)item);
  case ASTNode::TYPESWITCH:
    return optimizeTypeswitch((XQTypeswitch *)item);
  case ASTNode::VALIDATE:
    return optimizeValidate((XQValidate *)item);
  case ASTNode::FUNCTION_CALL:
    return optimizeFunctionCall((XQFunctionCall *)item);
  case ASTNode::DOM_CONSTRUCTOR:
    return optimizeDOMConstructor((XQDOMConstructor *)item);
  case ASTNode::SIMPLE_CONTENT:
    return optimizeSimpleContent((XQSimpleContent *)item);
  case ASTNode::ORDERING_CHANGE:
    return optimizeOrderingChange((XQOrderingChange *)item);
  case ASTNode::ATOMIZE:
    return optimizeAtomize((XQAtomize *)item);
  case ASTNode::EBV:
    return optimizeEffectiveBooleanValue((XQEffectiveBooleanValue *)item);
  case ASTNode::MAP:
    return optimizeMap((XQMap *)item);
  case ASTNode::PROMOTE_UNTYPED:
    return optimizePromoteUntyped((XQPromoteUntyped *)item);
  case ASTNode::PROMOTE_NUMERIC:
    return optimizePromoteNumeric((XQPromoteNumeric *)item);
  case ASTNode::PROMOTE_ANY_URI:
    return optimizePromoteAnyURI((XQPromoteAnyURI *)item);
  case ASTNode::DOCUMENT_ORDER:
    return optimizeDocumentOrder((XQDocumentOrder *)item);
  case ASTNode::PREDICATE:
    return optimizePredicate((XQPredicate *)item);
  case ASTNode::USER_FUNCTION:
    return optimizeUserFunction((XQUserFunctionInstance *)item);
  case ASTNode::NAME_EXPRESSION:
    return optimizeNameExpression((XQNameExpression *)item);
  case ASTNode::CONTENT_SEQUENCE:
    return optimizeContentSequence((XQContentSequence *)item);
  case ASTNode::DIRECT_NAME:
    return optimizeDirectName((XQDirectName *)item);
  case ASTNode::UDELETE:
    return optimizeUDelete((UDelete *)item);
  case ASTNode::URENAME:
    return optimizeURename((URename *)item);
  case ASTNode::UREPLACE:
    return optimizeUReplace((UReplace *)item);
  case ASTNode::UREPLACE_VALUE_OF:
    return optimizeUReplaceValueOf((UReplaceValueOf *)item);
  case ASTNode::UINSERT_AS_FIRST:
    return optimizeUInsertAsFirst((UInsertAsFirst *)item);
  case ASTNode::UINSERT_AS_LAST:
    return optimizeUInsertAsLast((UInsertAsLast *)item);
  case ASTNode::UINSERT_INTO:
    return optimizeUInsertInto((UInsertInto *)item);
  case ASTNode::UINSERT_AFTER:
    return optimizeUInsertAfter((UInsertAfter *)item);
  case ASTNode::UINSERT_BEFORE:
    return optimizeUInsertBefore((UInsertBefore *)item);
  case ASTNode::UTRANSFORM:
    return optimizeUTransform((UTransform *)item);
  case ASTNode::UAPPLY_UPDATES:
    return optimizeUApplyUpdates((UApplyUpdates *)item);
  case ASTNode::FTCONTAINS:
    return optimizeFTContains((FTContains *)item);
  case ASTNode::NAMESPACE_BINDING:
    return optimizeNamespaceBinding((XQNamespaceBinding *)item);
  case ASTNode::FUNCTION_CONVERSION:
    return optimizeFunctionConversion((XQFunctionConversion *)item);
  case ASTNode::ANALYZE_STRING:
    return optimizeAnalyzeString((XQAnalyzeString *)item);
  case ASTNode::COPY_OF:
    return optimizeCopyOf((XQCopyOf *)item);
  case ASTNode::COPY:
    return optimizeCopy((XQCopy *)item);
  case ASTNode::DEBUG_HOOK:
    return optimizeASTDebugHook((ASTDebugHook *)item);
  case ASTNode::CALL_TEMPLATE:
    return optimizeCallTemplate((XQCallTemplate *)item);
  case ASTNode::APPLY_TEMPLATES:
    return optimizeApplyTemplates((XQApplyTemplates *)item);
  case ASTNode::INLINE_FUNCTION:
    return optimizeInlineFunction((XQInlineFunction *)item);
  case ASTNode::FUNCTION_REF:
    return optimizeFunctionRef((XQFunctionRef *)item);
  case ASTNode::FUNCTION_DEREF:
    return optimizeFunctionDeref((XQFunctionDeref *)item);
  case ASTNode::PARTIAL_APPLY:
    return optimizePartialApply((XQPartialApply *)item);
  }
  return optimizeUnknown(item);
}

ASTNode *ASTVisitor::optimizeUnknown(ASTNode *item)
{
  return item;
}

ASTNode *ASTVisitor::optimizeFunction(XQFunction *item)
{
  VectorOfASTNodes &args = const_cast<VectorOfASTNodes &>(item->getArguments());
  for(VectorOfASTNodes::iterator i = args.begin(); i != args.end(); ++i) {
    *i = optimize(*i);
  }
  return item;
}

ASTNode *ASTVisitor::optimizeContextItem(XQContextItem *item)
{
  return item;
}

ASTNode *ASTVisitor::optimizeLiteral(XQLiteral *item)
{
  return item;
}

ASTNode *ASTVisitor::optimizeQNameLiteral(XQQNameLiteral *item)
{
  return item;
}

ASTNode *ASTVisitor::optimizeNumericLiteral(XQNumericLiteral *item)
{
  return item;
}

ASTNode *ASTVisitor::optimizeNav(XQNav *item)
{
  XQNav::Steps &args = const_cast<XQNav::Steps &>(item->getSteps());
  for(XQNav::Steps::iterator i = args.begin(); i != args.end(); ++i) {
    i->step = optimize(i->step);
  }
  return item;
}

ASTNode *ASTVisitor::optimizeSequence(XQSequence *item)
{
  VectorOfASTNodes &args = const_cast<VectorOfASTNodes &>(item->getChildren());
  for(VectorOfASTNodes::iterator i = args.begin(); i != args.end(); ++i) {
    *i = optimize(*i);
  }
  return item;
}

ASTNode *ASTVisitor::optimizeStep(XQStep *item)
{
  return item;
}

ASTNode *ASTVisitor::optimizeVariable(XQVariable *item)
{
  return item;
}

ASTNode *ASTVisitor::optimizeIf(XQIf *item)
{
  item->setTest(optimize(const_cast<ASTNode *>(item->getTest())));
  item->setWhenTrue(optimize(const_cast<ASTNode *>(item->getWhenTrue())));
  item->setWhenFalse(optimize(const_cast<ASTNode *>(item->getWhenFalse())));
  return item;
}

ASTNode *ASTVisitor::optimizeCastableAs(XQCastableAs *item)
{
  item->setExpression(optimize(const_cast<ASTNode *>(item->getExpression())));
  return item;
}

ASTNode *ASTVisitor::optimizeCastAs(XQCastAs *item)
{
  item->setExpression(optimize(const_cast<ASTNode *>(item->getExpression())));
  return item;
}

ASTNode *ASTVisitor::optimizeTreatAs(XQTreatAs *item)
{
  item->setExpression(optimize(item->getExpression()));
  return item;
}

ASTNode *ASTVisitor::optimizeFunctionCoercion(XQFunctionCoercion *item)
{
  item->setExpression(optimize(item->getExpression()));
  if(item->getFuncConvert())
    item->setFuncConvert(optimize(item->getFuncConvert()));
  return item;
}

ASTNode *ASTVisitor::optimizeOperator(XQOperator *item)
{
  VectorOfASTNodes &args = const_cast<VectorOfASTNodes &>(item->getArguments());
  for(VectorOfASTNodes::iterator i = args.begin(); i != args.end(); ++i) {
    *i = optimize(*i);
  }
  return item;
}

ASTNode *ASTVisitor::optimizeReturn(XQReturn *item)
{
  item->setParent(optimizeTupleNode(const_cast<TupleNode*>(item->getParent())));
  item->setExpression(optimize(item->getExpression()));
  return item;
}

ASTNode *ASTVisitor::optimizeQuantified(XQQuantified *item)
{
  item->setParent(optimizeTupleNode(const_cast<TupleNode*>(item->getParent())));
  item->setExpression(optimize(item->getExpression()));
  return item;
}

ASTNode *ASTVisitor::optimizeTypeswitch(XQTypeswitch *item)
{
  item->setExpression(optimize(const_cast<ASTNode *>(item->getExpression())));

  XQTypeswitch::Cases *clauses = const_cast<XQTypeswitch::Cases *>(item->getCases());
  for(XQTypeswitch::Cases::iterator i = clauses->begin(); i != clauses->end(); ++i) {
    (*i)->setExpression(optimize((*i)->getExpression()));
  }

  const_cast<XQTypeswitch::Case *>(item->getDefaultCase())->
    setExpression(optimize(item->getDefaultCase()->getExpression()));

  return item;
}

ASTNode *ASTVisitor::optimizeValidate(XQValidate *item)
{
  item->setExpression(optimize(const_cast<ASTNode *>(item->getExpression())));

  return item;
}

ASTNode *ASTVisitor::optimizeFunctionCall(XQFunctionCall *item)
{
  VectorOfASTNodes *args = const_cast<VectorOfASTNodes*>(item->getArguments());
  for(VectorOfASTNodes::iterator i = args->begin(); i != args->end(); ++i) {
    *i = optimize(*i);
  }
  return item;
}

ASTNode *ASTVisitor::optimizeUserFunction(XQUserFunctionInstance *item)
{
  VectorOfASTNodes &args = const_cast<VectorOfASTNodes &>(item->getArguments());
  for(VectorOfASTNodes::iterator i = args.begin(); i != args.end(); ++i) {
    *i = optimize(*i);
  }
  return item;
}

ASTNode *ASTVisitor::optimizeNameExpression(XQNameExpression *item)
{
  item->setExpression(optimize(const_cast<ASTNode *>(item->getExpression())));

  return item;
}

ASTNode *ASTVisitor::optimizeContentSequence(XQContentSequence *item)
{
  item->setExpression(optimize(const_cast<ASTNode *>(item->getExpression())));

  return item;
}

ASTNode *ASTVisitor::optimizeDirectName(XQDirectName *item)
{
  return item;
}

ASTNode *ASTVisitor::optimizeDOMConstructor(XQDOMConstructor *item)
{
  if(item->getName()) {
    item->setName(optimize(const_cast<ASTNode *>(item->getName())));
  }

  VectorOfASTNodes *attrs = const_cast<VectorOfASTNodes *>(item->getAttributes());
  if(attrs) {
    for(VectorOfASTNodes::iterator i = attrs->begin(); i != attrs->end(); ++i) {
      *i = optimize(*i);
    }
  }

  VectorOfASTNodes *children = const_cast<VectorOfASTNodes *>(item->getChildren());
  if(children) {
    for(VectorOfASTNodes::iterator j = children->begin(); j != children->end(); ++j) {
      *j = optimize(*j);
    }
  }

  if(item->getValue()) {
    item->setValue(optimize(const_cast<ASTNode *>(item->getValue())));
  }

  return item;
}

ASTNode *ASTVisitor::optimizeSimpleContent(XQSimpleContent *item)
{
  VectorOfASTNodes *children = const_cast<VectorOfASTNodes *>(item->getChildren());
  if(children) {
    for(VectorOfASTNodes::iterator j = children->begin(); j != children->end(); ++j) {
      *j = optimize(*j);
    }
  }
  return item;
}

ASTNode *ASTVisitor::optimizeOrderingChange(XQOrderingChange *item)
{
  item->setExpr(optimize(item->getExpr()));
  return item;
}

ASTNode *ASTVisitor::optimizeAtomize(XQAtomize *item)
{
  item->setExpression(optimize(const_cast<ASTNode *>(item->getExpression())));
  return item;
}

ASTNode *ASTVisitor::optimizeEffectiveBooleanValue(XQEffectiveBooleanValue *item)
{
  item->setExpression(optimize(const_cast<ASTNode *>(item->getExpression())));
  return item;
}

ASTNode *ASTVisitor::optimizeMap(XQMap *item)
{
  item->setArg1(optimize(item->getArg1()));
  item->setArg2(optimize(item->getArg2()));
  return item;
}

ASTNode *ASTVisitor::optimizePromoteUntyped(XQPromoteUntyped *item)
{
  item->setExpression(optimize(const_cast<ASTNode *>(item->getExpression())));
  return item;
}

ASTNode *ASTVisitor::optimizePromoteNumeric(XQPromoteNumeric *item)
{
  item->setExpression(optimize(const_cast<ASTNode *>(item->getExpression())));
  return item;
}

ASTNode *ASTVisitor::optimizePromoteAnyURI(XQPromoteAnyURI *item)
{
  item->setExpression(optimize(const_cast<ASTNode *>(item->getExpression())));
  return item;
}

ASTNode *ASTVisitor::optimizeDocumentOrder(XQDocumentOrder *item)
{
  item->setExpression(optimize(const_cast<ASTNode *>(item->getExpression())));
  return item;
}

ASTNode *ASTVisitor::optimizePredicate(XQPredicate *item)
{
  item->setExpression(optimize(const_cast<ASTNode *>(item->getExpression())));
  item->setPredicate(optimize(const_cast<ASTNode *>(item->getPredicate())));
  return item;
}

ASTNode *ASTVisitor::optimizeNamespaceBinding(XQNamespaceBinding *item)
{
  item->setExpression(optimize(const_cast<ASTNode *>(item->getExpression())));
  return item;
}

ASTNode *ASTVisitor::optimizeFunctionConversion(XQFunctionConversion *item)
{
  item->setExpression(optimize(const_cast<ASTNode *>(item->getExpression())));
  return item;
}

ASTNode *ASTVisitor::optimizeAnalyzeString(XQAnalyzeString *item)
{
  item->setExpression(optimize(const_cast<ASTNode *>(item->getExpression())));
  item->setRegex(optimize(const_cast<ASTNode *>(item->getRegex())));
  if(item->getFlags())
	  item->setFlags(optimize(const_cast<ASTNode *>(item->getFlags())));
  item->setMatch(optimize(const_cast<ASTNode *>(item->getMatch())));
  item->setNonMatch(optimize(const_cast<ASTNode *>(item->getNonMatch())));
  return item;
}

ASTNode *ASTVisitor::optimizeCopyOf(XQCopyOf *item)
{
  item->setExpression(optimize(const_cast<ASTNode *>(item->getExpression())));
  return item;
}

ASTNode *ASTVisitor::optimizeCopy(XQCopy *item)
{
  item->setExpression(optimize(const_cast<ASTNode *>(item->getExpression())));

  VectorOfASTNodes &args = const_cast<VectorOfASTNodes &>(item->getChildren());
  for(VectorOfASTNodes::iterator i = args.begin(); i != args.end(); ++i) {
    *i = optimize(*i);
  }

  return item;
}

ASTNode *ASTVisitor::optimizeASTDebugHook(ASTDebugHook *item)
{
  item->setExpression(optimize(const_cast<ASTNode *>(item->getExpression())));
  return item;
}

ASTNode *ASTVisitor::optimizeCallTemplate(XQCallTemplate *item)
{
  TemplateArguments *args = item->getArguments();
  if(args != 0) {
    for(TemplateArguments::iterator i = args->begin(); i != args->end(); ++i) {
      (*i)->value = optimize((*i)->value);
    }
  }
  if(item->getASTName())
    item->setASTName(optimize(item->getASTName()));
  return item;
}

ASTNode *ASTVisitor::optimizeApplyTemplates(XQApplyTemplates *item)
{
  item->setExpression(optimize(const_cast<ASTNode *>(item->getExpression())));

  TemplateArguments *args = item->getArguments();
  if(args != 0) {
    for(TemplateArguments::iterator i = args->begin(); i != args->end(); ++i) {
      (*i)->value = optimize((*i)->value);
    }
  }

  return item;
}

ASTNode *ASTVisitor::optimizeInlineFunction(XQInlineFunction *item)
{
  if(item->getUserFunction())
    item->setUserFunction(optimizeFunctionDef(item->getUserFunction()));
  item->setInstance(optimize(item->getInstance()));
  return item;
}

ASTNode *ASTVisitor::optimizeFunctionRef(XQFunctionRef *item)
{
  return item;
}

ASTNode *ASTVisitor::optimizeFunctionDeref(XQFunctionDeref *item)
{
  item->setExpression(optimize(item->getExpression()));

  VectorOfASTNodes *args = const_cast<VectorOfASTNodes*>(item->getArguments());
  if(args) {
    for(VectorOfASTNodes::iterator i = args->begin(); i != args->end(); ++i) {
      *i = optimize(*i);
    }
  }

  return item;
}

ASTNode *ASTVisitor::optimizePartialApply(XQPartialApply *item)
{
  item->setExpression(optimize(item->getExpression()));

  VectorOfASTNodes *args = const_cast<VectorOfASTNodes*>(item->getArguments());
  if(args) {
    for(VectorOfASTNodes::iterator i = args->begin(); i != args->end(); ++i) {
      if(*i != 0)
        *i = optimize(*i);
    }
  }

  return item;
}


////////////////////////////////////////////////////////////////////////////////////////////////////

ASTNode *ASTVisitor::optimizeUDelete(UDelete *item)
{
  item->setExpression(optimize(const_cast<ASTNode *>(item->getExpression())));
  return item;
}

ASTNode *ASTVisitor::optimizeURename(URename *item)
{
  item->setTarget(optimize(const_cast<ASTNode *>(item->getTarget())));
  item->setName(optimize(const_cast<ASTNode *>(item->getName())));
  return item;
}

ASTNode *ASTVisitor::optimizeUReplace(UReplace *item)
{
  item->setTarget(optimize(const_cast<ASTNode *>(item->getTarget())));
  item->setExpression(optimize(const_cast<ASTNode *>(item->getExpression())));
  return item;
}

ASTNode *ASTVisitor::optimizeUReplaceValueOf(UReplaceValueOf *item)
{
  item->setTarget(optimize(const_cast<ASTNode *>(item->getTarget())));
  item->setExpression(optimize(const_cast<ASTNode *>(item->getExpression())));
  return item;
}

ASTNode *ASTVisitor::optimizeUInsertAsFirst(UInsertAsFirst *item)
{
  item->setSource(optimize(const_cast<ASTNode *>(item->getSource())));
  item->setTarget(optimize(const_cast<ASTNode *>(item->getTarget())));
  return item;
}

ASTNode *ASTVisitor::optimizeUInsertAsLast(UInsertAsLast *item)
{
  item->setSource(optimize(const_cast<ASTNode *>(item->getSource())));
  item->setTarget(optimize(const_cast<ASTNode *>(item->getTarget())));
  return item;
}

ASTNode *ASTVisitor::optimizeUInsertInto(UInsertInto *item)
{
  item->setSource(optimize(const_cast<ASTNode *>(item->getSource())));
  item->setTarget(optimize(const_cast<ASTNode *>(item->getTarget())));
  return item;
}

ASTNode *ASTVisitor::optimizeUInsertAfter(UInsertAfter *item)
{
  item->setSource(optimize(const_cast<ASTNode *>(item->getSource())));
  item->setTarget(optimize(const_cast<ASTNode *>(item->getTarget())));
  return item;
}

ASTNode *ASTVisitor::optimizeUInsertBefore(UInsertBefore *item)
{
  item->setSource(optimize(const_cast<ASTNode *>(item->getSource())));
  item->setTarget(optimize(const_cast<ASTNode *>(item->getTarget())));
  return item;
}

ASTNode *ASTVisitor::optimizeUTransform(UTransform *item)
{
  VectorOfCopyBinding *bindings = const_cast<VectorOfCopyBinding*>(item->getBindings());
  for(VectorOfCopyBinding::iterator i = bindings->begin(); i != bindings->end(); ++i) {
    (*i)->expr_ = optimize((*i)->expr_);
  }

  item->setModifyExpr(optimize(const_cast<ASTNode *>(item->getModifyExpr())));
  item->setReturnExpr(optimize(const_cast<ASTNode *>(item->getReturnExpr())));

  return item;
}

ASTNode *ASTVisitor::optimizeUApplyUpdates(UApplyUpdates *item)
{
  item->setExpression(optimize(const_cast<ASTNode *>(item->getExpression())));
  return item;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

ASTNode *ASTVisitor::optimizeFTContains(FTContains *item)
{
  item->setArgument(optimize(item->getArgument()));
  item->setSelection(optimizeFTSelection(item->getSelection()));
  if(item->getIgnore())
    item->setIgnore(optimize(item->getIgnore()));
  return item;
}

FTSelection *ASTVisitor::optimizeFTSelection(FTSelection *selection)
{
  switch(selection->getType()) {
  case FTSelection::OR:
    return optimizeFTOr((FTOr*)selection);
  case FTSelection::AND:
    return optimizeFTAnd((FTAnd*)selection);
  case FTSelection::MILD_NOT:
    return optimizeFTMildnot((FTMildnot*)selection);
  case FTSelection::UNARY_NOT:
    return optimizeFTUnaryNot((FTUnaryNot*)selection);
  case FTSelection::WORDS:
    return optimizeFTWords((FTWords*)selection);
  case FTSelection::WORD:
    return optimizeFTWord((FTWord*)selection);
  case FTSelection::ORDER:
    return optimizeFTOrder((FTOrder*)selection);
  case FTSelection::DISTANCE:
    return optimizeFTDistance((FTDistance*)selection);
  case FTSelection::DISTANCE_LITERAL:
    return optimizeFTDistanceLiteral((FTDistanceLiteral*)selection);
  case FTSelection::SCOPE:
    return optimizeFTScope((FTScope*)selection);
  case FTSelection::CONTENT:
    return optimizeFTContent((FTContent*)selection);
  case FTSelection::WINDOW:
    return optimizeFTWindow((FTWindow*)selection);
  case FTSelection::WINDOW_LITERAL:
    return optimizeFTWindowLiteral((FTWindowLiteral*)selection);
  }
  return optimizeUnknownFTSelection(selection);
}

FTSelection *ASTVisitor::optimizeUnknownFTSelection(FTSelection *selection)
{
  return selection;
}

FTSelection *ASTVisitor::optimizeFTWords(FTWords *selection)
{
  selection->setExpr(optimize(selection->getExpr()));
  return selection;
}

FTSelection *ASTVisitor::optimizeFTWord(FTWord *selection)
{
  return selection;
}

FTSelection *ASTVisitor::optimizeFTOr(FTOr *selection)
{
  VectorOfFTSelections &args = const_cast<VectorOfFTSelections&>(selection->getArguments());
  for(VectorOfFTSelections::iterator i = args.begin(); i != args.end(); ++i) {
    *i = optimizeFTSelection(*i);
  }
  return selection;
}

FTSelection *ASTVisitor::optimizeFTAnd(FTAnd *selection)
{
  VectorOfFTSelections &args = const_cast<VectorOfFTSelections&>(selection->getArguments());
  for(VectorOfFTSelections::iterator i = args.begin(); i != args.end(); ++i) {
    *i = optimizeFTSelection(*i);
  }
  return selection;
}

FTSelection *ASTVisitor::optimizeFTMildnot(FTMildnot *selection)
{
  selection->setLeft(optimizeFTSelection(selection->getLeft()));
  selection->setRight(optimizeFTSelection(selection->getRight()));
  return selection;
}

FTSelection *ASTVisitor::optimizeFTUnaryNot(FTUnaryNot *selection)
{
  selection->setArgument(optimizeFTSelection(selection->getArgument()));
  return selection;
}

FTSelection *ASTVisitor::optimizeFTOrder(FTOrder *selection)
{
  selection->setArgument(optimizeFTSelection(selection->getArgument()));
  return selection;
}

FTSelection *ASTVisitor::optimizeFTDistance(FTDistance *selection)
{
  selection->setArgument(optimizeFTSelection(selection->getArgument()));

  FTRange &range = const_cast<FTRange&>(selection->getRange());
  range.arg1 = optimize(range.arg1);
  if(range.arg2) range.arg2 = optimize(range.arg2);

  return selection;
}

FTSelection *ASTVisitor::optimizeFTDistanceLiteral(FTDistanceLiteral *selection)
{
  selection->setArgument(optimizeFTSelection(selection->getArgument()));
  return selection;
}

FTSelection *ASTVisitor::optimizeFTScope(FTScope *selection)
{
  selection->setArgument(optimizeFTSelection(selection->getArgument()));
  return selection;
}

FTSelection *ASTVisitor::optimizeFTContent(FTContent *selection)
{
  selection->setArgument(optimizeFTSelection(selection->getArgument()));
  return selection;
}

FTSelection *ASTVisitor::optimizeFTWindow(FTWindow *selection)
{
  selection->setArgument(optimizeFTSelection(selection->getArgument()));
  selection->setExpr(optimize(selection->getExpr()));
  return selection;
}

FTSelection *ASTVisitor::optimizeFTWindowLiteral(FTWindowLiteral *selection)
{
  selection->setArgument(optimizeFTSelection(selection->getArgument()));
  return selection;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

TupleNode *ASTVisitor::optimizeTupleNode(TupleNode *item)
{
  switch(item->getType()) {
  case TupleNode::CONTEXT_TUPLE:
    return optimizeContextTuple((ContextTuple*)item);
  case TupleNode::FOR:
    return optimizeForTuple((ForTuple*)item);
  case TupleNode::LET:
    return optimizeLetTuple((LetTuple*)item);
  case TupleNode::WHERE:
    return optimizeWhereTuple((WhereTuple*)item);
  case TupleNode::COUNT:
    return optimizeCountTuple((CountTuple*)item);
  case TupleNode::ORDER_BY:
    return optimizeOrderByTuple((OrderByTuple*)item);
  case TupleNode::DEBUG_HOOK:
    return optimizeTupleDebugHook((TupleDebugHook*)item);
  }
  return optimizeUnknownTupleNode(item);
}

TupleNode *ASTVisitor::optimizeUnknownTupleNode(TupleNode *item)
{
  if(item->getParent())
    item->setParent(optimizeTupleNode(const_cast<TupleNode*>(item->getParent())));
  return item;
}

TupleNode *ASTVisitor::optimizeContextTuple(ContextTuple *item)
{
  return item;
}

TupleNode *ASTVisitor::optimizeForTuple(ForTuple *item)
{
  item->setParent(optimizeTupleNode(const_cast<TupleNode*>(item->getParent())));
  item->setExpression(optimize(item->getExpression()));
  return item;
}

TupleNode *ASTVisitor::optimizeLetTuple(LetTuple *item)
{
  item->setParent(optimizeTupleNode(const_cast<TupleNode*>(item->getParent())));
  item->setExpression(optimize(item->getExpression()));
  return item;
}

TupleNode *ASTVisitor::optimizeWhereTuple(WhereTuple *item)
{
  item->setParent(optimizeTupleNode(const_cast<TupleNode*>(item->getParent())));
  item->setExpression(optimize(item->getExpression()));
  return item;
}

TupleNode *ASTVisitor::optimizeCountTuple(CountTuple *item)
{
  item->setParent(optimizeTupleNode(const_cast<TupleNode*>(item->getParent())));
  return item;
}

TupleNode *ASTVisitor::optimizeOrderByTuple(OrderByTuple *item)
{
  item->setParent(optimizeTupleNode(const_cast<TupleNode*>(item->getParent())));
  item->setExpression(optimize(item->getExpression()));
  return item;
}

TupleNode *ASTVisitor::optimizeTupleDebugHook(TupleDebugHook *item)
{
  item->setParent(optimizeTupleNode(const_cast<TupleNode*>(item->getParent())));
  return item;
}


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

#include <xqilla/optimizer/StaticTyper.hpp>
#include <xqilla/ast/ASTNode.hpp>
#include <xqilla/context/DynamicContext.hpp>
#include <xqilla/simple-api/XQQuery.hpp>
#include <xqilla/context/ContextHelpers.hpp>
#include <xqilla/context/VariableTypeStore.hpp>
#include <xqilla/utils/XPath2Utils.hpp>
#include <xqilla/exceptions/XPath2TypeMatchException.hpp>
#include <xqilla/fulltext/FTSelection.hpp>
#include <xqilla/fulltext/DefaultTokenizer.hpp>
#include <xqilla/schema/SequenceType.hpp>
#include <xqilla/functions/FunctionSignature.hpp>

#include "../items/impl/FunctionRefImpl.hpp"

#include <xercesc/validators/schema/SchemaSymbols.hpp>

XERCES_CPP_NAMESPACE_USE;

ASTNode *StaticTyper::run(ASTNode *item, StaticContext *context)
{
  AutoReset<StaticContext*> autoReset(context_);
  context_ = context;
  return optimize(item);
}

void StaticTyper::optimize(XQQuery *query)
{
  assert(context_ == query->getStaticContext());

  query->staticTyping(this);
}

XQUserFunction *StaticTyper::optimizeFunctionDef(XQUserFunction *item)
{
  item->staticTypingOnce(context_, this);
  return item;
}

ASTNode *StaticTyper::optimize(ASTNode *item)
{
  ASTNode *result = ASTVisitor::optimize(item);
  if(result != item) return optimize(result);

  return item->staticTypingImpl(context_);
}

#define SUBSTITUTE(item, name) {\
    ASTNode *result = item->get ## name (); \
    item->set ##name (0); \
    item->release(); \
    return result; \
}

ASTNode *StaticTyper::optimizePredicate(XQPredicate *item)
{
  item->setExpression(optimize(const_cast<ASTNode *>(item->getExpression())));

  StaticType ciType = item->getExpression()->getStaticAnalysis().getStaticType();
  ciType.setCardinality(1, 1);
  AutoContextItemTypeReset contextTypeReset(context_, ciType);

  item->setPredicate(optimize(const_cast<ASTNode *>(item->getPredicate())));

  return item;
}

ASTNode *StaticTyper::optimizeAnalyzeString(XQAnalyzeString *item)
{
  item->setExpression(optimize(const_cast<ASTNode *>(item->getExpression())));
  item->setRegex(optimize(const_cast<ASTNode *>(item->getRegex())));
  if(item->getFlags())
	  item->setFlags(optimize(const_cast<ASTNode *>(item->getFlags())));

  StaticType ciType(StaticType::STRING_TYPE, 1, 1);
  AutoContextItemTypeReset contextTypeReset(context_, ciType);

  item->setMatch(optimize(const_cast<ASTNode *>(item->getMatch())));
  item->setNonMatch(optimize(const_cast<ASTNode *>(item->getNonMatch())));
  return item;
}

ASTNode *StaticTyper::optimizeNav(XQNav *item)
{
  AutoContextItemTypeReset contextTypeReset(context_);
  StaticType ciType;

  XQNav::Steps &args = const_cast<XQNav::Steps &>(item->getSteps());
  for(XQNav::Steps::iterator i = args.begin(); i != args.end(); ++i) {
    i->step = optimize(i->step);

    if(context_) {
      ciType = i->step->getStaticAnalysis().getStaticType();
      ciType.setCardinality(1, 1);
      context_->setContextItemType(ciType);
    }
  }

  return item;
}

ASTNode *StaticTyper::optimizeMap(XQMap *item)
{
  item->setArg1(optimize(item->getArg1()));

  AutoContextItemTypeReset contextTypeReset(context_);
  VariableTypeStore* varStore = context_ ? context_->getVariableTypeStore() : 0;

  if(context_) {
    StaticAnalysis &varSrc = const_cast<StaticAnalysis&>(item->getVarSRC());

    varSrc.getStaticType() = item->getArg1()->getStaticAnalysis().getStaticType();
    varSrc.setProperties(StaticAnalysis::DOCORDER | StaticAnalysis::GROUPED |
                         StaticAnalysis::PEER | StaticAnalysis::SUBTREE | StaticAnalysis::SAMEDOC |
                         StaticAnalysis::ONENODE | StaticAnalysis::SELF);

    if(item->getName() == 0) {
      context_->setContextItemType(varSrc.getStaticType());
    } else {
      varStore->addLogicalBlockScope();
      varStore->declareVar(item->getURI(), item->getName(), varSrc);
    }
  }

  item->setArg2(optimize(item->getArg2()));

  if(context_ && item->getName() != 0) {
    varStore->removeScope();
  }

  return item;
}

ASTNode *StaticTyper::optimizeUTransform(UTransform *item)
{
  VariableTypeStore* varStore = context_ ? context_->getVariableTypeStore() : 0;

  VectorOfCopyBinding *bindings = const_cast<VectorOfCopyBinding*>(item->getBindings());
  for(VectorOfCopyBinding::iterator i = bindings->begin(); i != bindings->end(); ++i) {
    (*i)->expr_ = optimize((*i)->expr_);
    (*i)->src_.getStaticType() = (*i)->expr_->getStaticAnalysis().getStaticType();
    (*i)->src_.setProperties((*i)->expr_->getStaticAnalysis().getProperties());

    // Declare the variable binding
    if(context_) {
      varStore->addLogicalBlockScope();
      varStore->declareVar((*i)->uri_, (*i)->name_, (*i)->src_);
    }
  }

  item->setModifyExpr(optimize(const_cast<ASTNode *>(item->getModifyExpr())));
  item->setReturnExpr(optimize(const_cast<ASTNode *>(item->getReturnExpr())));

  if(context_) {
    VectorOfCopyBinding::reverse_iterator rend = bindings->rend();
    for(VectorOfCopyBinding::reverse_iterator it = bindings->rbegin(); it != rend; ++it) {
      // Remove our variable binding and the scope we added
      varStore->removeScope();
    }
  }

  return item;
}

ASTNode *StaticTyper::optimizeQuantified(XQQuantified *item)
{
  AutoReset<bool> reset(tupleSetup_);
  tupleSetup_ = true;
  item->setParent(optimizeTupleNode(const_cast<TupleNode*>(item->getParent())));

  item->setExpression(optimize(item->getExpression()));

  tupleSetup_ = false;
  item->setParent(optimizeTupleNode(const_cast<TupleNode*>(item->getParent())));

  return item;
}

ASTNode *StaticTyper::optimizeReturn(XQReturn *item)
{
  AutoReset<bool> reset(tupleSetup_);
  tupleSetup_ = true;
  item->setParent(optimizeTupleNode(const_cast<TupleNode*>(item->getParent())));

  item->setExpression(optimize(item->getExpression()));

  tupleSetup_ = false;
  item->setParent(optimizeTupleNode(const_cast<TupleNode*>(item->getParent())));

  return item;
}

void StaticTyper::optimizeCase(const StaticAnalysis &var_src, XQTypeswitch::Case *item)
{
  AutoDelete<StaticAnalysis> caseSrc(0);
  if(context_ && item->isVariableUsed()) {
    VariableTypeStore* varStore = context_->getVariableTypeStore();

    caseSrc.set(new StaticAnalysis(context_->getMemoryManager()));
    caseSrc->copy(var_src);

    if(item->getSequenceType() != 0) {
      caseSrc->getStaticType() &= item->getTreatType();
    }

    varStore->addLogicalBlockScope();
    varStore->declareVar(item->getURI(), item->getName(), *caseSrc);
  }

  item->setExpression(optimize(item->getExpression()));

  if(context_ && item->isVariableUsed()) {
    context_->getVariableTypeStore()->removeScope();
  }
}

ASTNode *StaticTyper::optimizeTypeswitch(XQTypeswitch *item)
{
  item->setExpression(optimize(const_cast<ASTNode *>(item->getExpression())));

  XQTypeswitch::Cases *clauses = const_cast<XQTypeswitch::Cases *>(item->getCases());
  for(XQTypeswitch::Cases::iterator i = clauses->begin(); i != clauses->end(); ++i) {
    optimizeCase(item->getExpression()->getStaticAnalysis(), *i);
  }

  optimizeCase(item->getExpression()->getStaticAnalysis(),
               const_cast<XQTypeswitch::Case *>(item->getDefaultCase()));

  return item;
}

ASTNode *StaticTyper::optimizeFunctionCoercion(XQFunctionCoercion *item)
{
  item->setExpression(optimize(item->getExpression()));

  AutoDelete<StaticAnalysis> varSrc(0);
  if(item->getFuncConvert()) {
    if(context_) {
      // Could do better on the static type
      varSrc.set(new StaticAnalysis(context_->getMemoryManager()));
      varSrc->getStaticType() = StaticType::FUNCTION_TYPE;

      VariableTypeStore *varStore = context_->getVariableTypeStore();
      varStore->addLogicalBlockScope();
      varStore->declareVar(0, XQFunctionCoercion::funcVarName, *varSrc);
    }

    {
      AutoMessageListenerReset reset(context_); // Turn off warnings
      item->setFuncConvert(optimize(item->getFuncConvert()));
    }

    if(context_)
      context_->getVariableTypeStore()->removeScope();
  }

  return item;
}

ASTNode *StaticTyper::optimizeNamespaceBinding(XQNamespaceBinding *item)
{
  AutoNsScopeReset jan(context_, item->getNamespaces());

  if(context_) {
    const XMLCh *defaultElementNS = context_->getMemoryManager()->
      getPooledString(item->getNamespaces()->lookupNamespaceURI(XMLUni::fgZeroLenString));
    context_->setDefaultElementAndTypeNS(defaultElementNS);
  }

  item->setExpression(optimize(const_cast<ASTNode *>(item->getExpression())));
  return item;
}

ASTNode *StaticTyper::optimizeInlineFunction(XQInlineFunction *item)
{
  if(item->getUserFunction())
    item->getUserFunction()->staticTyping(context_, this);

  AutoDelete<StaticAnalysis> instanceVarSrc(0);
  if(context_) {
    XPath2MemoryManager *mm = context_->getMemoryManager();
    instanceVarSrc.set(new StaticAnalysis(mm));
    instanceVarSrc->getStaticType() = StaticType(StaticType::ITEM_TYPE, 0, StaticType::UNLIMITED);

    VariableTypeStore *varStore = context_->getVariableTypeStore();
    varStore->addLogicalBlockScope();

    if(item->getSignature()->argSpecs) {
      ArgumentSpecs::const_iterator argsIt = item->getSignature()->argSpecs->begin();
      for(; argsIt != item->getSignature()->argSpecs->end(); ++argsIt) {
        varStore->declareVar((*argsIt)->getURI(), (*argsIt)->getName(), *instanceVarSrc);
      }
    }
  }

  {
    // The context item is not defined
    AutoContextItemTypeReset contextTypeReset(context_, StaticType());
    // Turn off warnings
    AutoMessageListenerReset reset(context_);
    item->setInstance(optimize(item->getInstance()));
  }

  if(context_)
    context_->getVariableTypeStore()->removeScope();

  return item;
}

ASTNode *StaticTyper::optimizeEffectiveBooleanValue(XQEffectiveBooleanValue *item)
{
  AutoNodeSetOrderingReset orderReset(context_);
  item->setExpression(optimize(const_cast<ASTNode *>(item->getExpression())));
  return item;
}

ASTNode *StaticTyper::optimizeVariable(XQVariable *item)
{
  if(!context_) return item;

  XQGlobalVariable *global = 0;
  context_->getVariableTypeStore()->getVar(item->getURI(), item->getName(), &global);
  if(global) {
    // See if this is a global variable from one of the imported modules,
    // and if so make sure it's static typed first
    XQQuery *module = context_->getModule()->findModuleForVariable(item->getURI(), item->getName());
    if(module == context_->getModule()) {
      if(globalsUsed_) globalsUsed_->push_back(global);
    }
    else if(module) {
      AutoReset<StaticContext*> autoReset(context_);
      module->staticTypingOnce(this);
    }
  }
  return item;
}

ASTNode *StaticTyper::optimizeUserFunction(XQUserFunctionInstance *item)
{
  ASTNode *result = ASTVisitor::optimizeUserFunction(item);
  if(result != item || !context_) return result;

  // See if this is a function from one of the imported modules,
  // and if so make sure it's static typed first
  XQQuery *module = context_->getModule()->
    findModuleForFunction(item->getFunctionURI(), item->getFunctionName(), item->getArguments().size());
  if(module && module != context_->getModule()) {
    AutoReset<StaticContext*> autoReset(context_);
    module->staticTypingOnce(this);
  }
  return item;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

ASTNode *StaticTyper::optimizeFTContains(FTContains *item)
{
  item->setArgument(optimize(item->getArgument()));
  item->setSelection(optimizeFTSelection(item->getSelection()));

  if(context_) {
    AutoDelete<DynamicContext> dContext(context_->createDynamicContext());
    dContext->setMemoryManager(context_->getMemoryManager());

    DefaultTokenizer tokenizer;
    FTContext ftcontext(&tokenizer, 0, dContext);
    item->setSelection(item->getSelection()->optimize(&ftcontext));
  }

  if(item->getIgnore())
    item->setIgnore(optimize(item->getIgnore()));
  return item;
}

FTSelection *StaticTyper::optimizeFTSelection(FTSelection *selection)
{
  FTSelection *result = ASTVisitor::optimizeFTSelection(selection);
  if(result != selection) return result;

  return selection->staticTypingImpl(context_);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

TupleNode *StaticTyper::optimizeTupleNode(TupleNode *item)
{
  TupleNode *result = ASTVisitor::optimizeTupleNode(item);
  if(result != item) return result;

  if(tupleSetup_)
    return item->staticTypingImpl(context_);
  return item;
}

TupleNode *StaticTyper::optimizeForTuple(ForTuple *item)
{
  item->setParent(optimizeTupleNode(const_cast<TupleNode*>(item->getParent())));

  if(tupleSetup_)
    item->setExpression(optimize(item->getExpression()));

  if(context_) {
    VariableTypeStore* varStore = context_->getVariableTypeStore();

    if(tupleSetup_) {
      varStore->addLogicalBlockScope();

      if(item->getVarName()) {
        // Declare the variable binding
        StaticAnalysis &varSrc = const_cast<StaticAnalysis&>(item->getVarSRC());
        varSrc.getStaticType() = item->getExpression()->getStaticAnalysis().getStaticType();
        varSrc.getStaticType().setCardinality(1, 1);
        varSrc.setProperties(StaticAnalysis::DOCORDER | StaticAnalysis::GROUPED |
                             StaticAnalysis::PEER | StaticAnalysis::SUBTREE | StaticAnalysis::SAMEDOC |
                             StaticAnalysis::ONENODE | StaticAnalysis::SELF);
        varStore->declareVar(item->getVarURI(), item->getVarName(), varSrc);
      }

      if(item->getPosName()) {
        // Declare the positional variable binding
        StaticAnalysis &posSrc = const_cast<StaticAnalysis&>(item->getPosSRC());
        posSrc.getStaticType() = StaticType::DECIMAL_TYPE;
        varStore->declareVar(item->getPosURI(), item->getPosName(), posSrc);
      }
    }
    else {
      varStore->removeScope();
    }
  }

  return item;
}

TupleNode *StaticTyper::optimizeLetTuple(LetTuple *item)
{
  item->setParent(optimizeTupleNode(const_cast<TupleNode*>(item->getParent())));

  if(tupleSetup_)
    item->setExpression(optimize(item->getExpression()));

  if(context_) {
    VariableTypeStore* varStore = context_->getVariableTypeStore();

    if(tupleSetup_) {
      varStore->addLogicalBlockScope();

      // Declare the variable binding
      StaticAnalysis &varSrc = const_cast<StaticAnalysis&>(item->getVarSRC());
      varSrc.getStaticType() = item->getExpression()->getStaticAnalysis().getStaticType();
      varSrc.setProperties(item->getExpression()->getStaticAnalysis().getProperties());
      varStore->declareVar(item->getVarURI(), item->getVarName(), varSrc);
    }
    else {
      varStore->removeScope();
    }
  }

  return item;
}

TupleNode *StaticTyper::optimizeWhereTuple(WhereTuple *item)
{
  item->setParent(optimizeTupleNode(const_cast<TupleNode*>(item->getParent())));

  if(tupleSetup_)
    item->setExpression(optimize(item->getExpression()));

  return item;
}

TupleNode *StaticTyper::optimizeCountTuple(CountTuple *item)
{
  item->setParent(optimizeTupleNode(const_cast<TupleNode*>(item->getParent())));

  if(context_) {
    VariableTypeStore* varStore = context_->getVariableTypeStore();

    if(tupleSetup_) {
      varStore->addLogicalBlockScope();

      if(item->getVarName()) {
        // Declare the positional variable binding
        StaticAnalysis &varSrc = const_cast<StaticAnalysis&>(item->getVarSRC());
        varSrc.getStaticType() = StaticType::DECIMAL_TYPE;
        varStore->declareVar(item->getVarURI(), item->getVarName(), varSrc);
      }
    }
    else {
      varStore->removeScope();
    }
  }

  return item;
}

TupleNode *StaticTyper::optimizeOrderByTuple(OrderByTuple *item)
{
  item->setParent(optimizeTupleNode(const_cast<TupleNode*>(item->getParent())));

  if(tupleSetup_) {
    AutoNodeSetOrderingReset orderReset(context_, (item->getModifiers() & OrderByTuple::UNSTABLE) == 0 ?
                                        StaticContext::ORDERING_ORDERED : StaticContext::ORDERING_UNORDERED);
    item->setExpression(optimize(item->getExpression()));
  }

  return item;
}


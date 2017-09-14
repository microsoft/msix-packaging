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
#include <sstream>

#include <xqilla/ast/XQFunctionCoercion.hpp>
#include <xqilla/ast/XQVariable.hpp>
#include <xqilla/ast/XQFunctionDeref.hpp>
#include <xqilla/ast/XQInlineFunction.hpp>
#include <xqilla/schema/SequenceType.hpp>
#include <xqilla/context/DynamicContext.hpp>
#include <xqilla/items/Item.hpp>
#include <xqilla/utils/XPath2Utils.hpp>
#include <xqilla/items/ATUntypedAtomic.hpp>
#include <xqilla/exceptions/XPath2TypeMatchException.hpp>
#include <xqilla/context/ContextHelpers.hpp>
#include <xqilla/context/MessageListener.hpp>
#include <xqilla/context/VariableTypeStore.hpp>
#include <xqilla/functions/XQUserFunction.hpp>
#include <xqilla/context/impl/VarStoreImpl.hpp>
#include <xqilla/functions/FunctionSignature.hpp>
#include "../items/impl/FunctionRefImpl.hpp"

#include <xercesc/validators/schema/SchemaSymbols.hpp>

XERCES_CPP_NAMESPACE_USE
using namespace std;

const XMLCh XQFunctionCoercion::funcVarName[] = { '#', 'f', 'u', 'n', 'c', 'V', 'a', 'r', 0 };
static const XMLCh argVarPrefix[] = { 'a', 'r', 'g', 0 };

XQFunctionCoercion::XQFunctionCoercion(ASTNode* expr, SequenceType* exprType, XPath2MemoryManager* memMgr)
  : ASTNodeImpl(FUNCTION_COERCION, memMgr),
    _expr(expr),
    _exprType(exprType),
    _funcConvert(0)
{
}

XQFunctionCoercion::XQFunctionCoercion(ASTNode* expr, SequenceType *exprType, ASTNode *funcConvert, const StaticType &treatType, XPath2MemoryManager* memMgr)
  : ASTNodeImpl(FUNCTION_COERCION, memMgr),
    _expr(expr),
    _exprType(exprType),
    _funcConvert(funcConvert),
    _treatType(treatType)
{
}

ASTNode* XQFunctionCoercion::staticResolution(StaticContext *context)
{
  XPath2MemoryManager *mm = context->getMemoryManager();

  _exprType->staticResolution(context);
  _expr = _expr->staticResolution(context);

  bool isExact;
  _exprType->getStaticType(_treatType, context, isExact, this);

  const SequenceType::ItemType *type = _exprType->getItemType();
  if(!type || type->getItemTestType() != SequenceType::ItemType::TEST_FUNCTION ||
     type->getReturnType() == 0)
    return substitute(_expr);

  // Construct an XQInlineFunction that will convert a function reference
  // stored in a variable to the correct type, and will throw type errors
  // if it isn't the correct type

  // Simultaneously create the XQInlineFunction parameter spec and the
  // XQFunctionDeref argument list
  ArgumentSpecs *paramList = new (mm) ArgumentSpecs(XQillaAllocator<ArgumentSpec*>(mm));
  VectorOfASTNodes *argList = new (mm) VectorOfASTNodes(XQillaAllocator<ASTNode*>(mm));

  VectorOfSequenceTypes *argTypes = type->getArgumentTypes();
  for(VectorOfSequenceTypes::iterator i = argTypes->begin(); i != argTypes->end(); ++i) {
    const XMLCh *argName = context->allocateTempVarName(argVarPrefix);

    ArgumentSpec *argSpec = new (mm) ArgumentSpec(argName, *i, mm);
    argSpec->setLocationInfo(*i);
    paramList->push_back(argSpec);
    
    XQVariable *argVar = new (mm) XQVariable(0, argName, mm);
    argVar->setLocationInfo(this);
    argList->push_back(argVar);
  }

  XQVariable *funcVar = new (mm) XQVariable(0, funcVarName, mm);
  funcVar->setLocationInfo(this);

  XQFunctionDeref *body = new (mm) XQFunctionDeref(funcVar, argList, mm);
  body->setLocationInfo(this);

  FunctionSignature *signature = new (mm) FunctionSignature(paramList, type->getReturnType(), mm);

  XQUserFunction *func = new (mm) XQUserFunction(0, signature, body, false, mm);
  func->setLocationInfo(this);

  _funcConvert = new (mm) XQInlineFunction(func, mm);
  _funcConvert->setLocationInfo(this);

  _funcConvert = _funcConvert->staticResolution(context);

  return this;
}

ASTNode *XQFunctionCoercion::staticTypingImpl(StaticContext *context)
{
  _src.clear();

  // Get a better static type by looking at our expression's type too
  _src.getStaticType() = _treatType;
  _src.getStaticType() &= _expr->getStaticAnalysis().getStaticType();

  StaticType nonFunctionTypes = _expr->getStaticAnalysis().getStaticType();
  nonFunctionTypes &= StaticType(StaticType::NODE_TYPE | StaticType::ANY_ATOMIC_TYPE, 0, StaticType::UNLIMITED);

  _src.getStaticType() |= nonFunctionTypes;

  _src.setProperties(_expr->getStaticAnalysis().getProperties());
  _src.add(_expr->getStaticAnalysis());

  if(!_expr->getStaticAnalysis().getStaticType().containsType(StaticType::FUNCTION_TYPE))
    return substitute(_expr);

  return this;
}

class FunctionConversionResult : public ResultImpl
{
public:
  FunctionConversionResult(const Result &parent, const ASTNode *funcConvert,
                           const SequenceType::ItemType *itemType, const LocationInfo *location)
    : ResultImpl(location),
      parent_(parent),
      funcConvert_(funcConvert),
      itemType_(itemType)
  {
  }

  virtual Item::Ptr next(DynamicContext *context)
  {
    Item::Ptr item = parent_->next(context);

    if(item.notNull() && item->isFunction() &&
       !itemType_->matches((FunctionRef::Ptr)item, context)) {
      XPath2MemoryManager *mm = context->getMemoryManager();

      VarStoreImpl scope(mm, context->getVariableStore());
      scope.setVar(0, XQFunctionCoercion::funcVarName, item);

      AutoVariableStoreReset vsReset(context, &scope);

      // funcConvert_ only returns one item
      item = funcConvert_->createResult(context)->next(context);
    }

    return item;
  }

  string asString(DynamicContext *context, int indent) const { return ""; }

private:
  Result parent_;
  const ASTNode *funcConvert_;
  const SequenceType::ItemType *itemType_;
};

Result XQFunctionCoercion::createResult(DynamicContext* context, int flags) const
{
  return new FunctionConversionResult(_expr->createResult(context), _funcConvert, _exprType->getItemType(), this);
}


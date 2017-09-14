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

#include <xqilla/ast/XQFunctionDeref.hpp>
#include <xqilla/context/DynamicContext.hpp>
#include <xqilla/items/FunctionRef.hpp>
#include <xqilla/exceptions/XPath2TypeMatchException.hpp>
#include <xqilla/runtime/ResultImpl.hpp>
#include <xqilla/runtime/ClosureResult.hpp>
#include <xqilla/schema/SequenceType.hpp>
#include <xqilla/ast/XQTreatAs.hpp>

XERCES_CPP_NAMESPACE_USE;
using namespace std;

XQFunctionDeref::XQFunctionDeref(ASTNode *expr, VectorOfASTNodes *args, XPath2MemoryManager *mm)
  : ASTNodeImpl(FUNCTION_DEREF, mm),
    expr_(expr),
    args_(args)
{
}

ASTNode *XQFunctionDeref::staticResolution(StaticContext *context)
{
  XPath2MemoryManager *mm = context->getMemoryManager();

  SequenceType *seqType = new (mm) SequenceType(new (mm) SequenceType::ItemType(SequenceType::ItemType::TEST_FUNCTION),
                                                SequenceType::EXACTLY_ONE);
  seqType->setLocationInfo(this);

  expr_ = new (mm) XQTreatAs(expr_, seqType, mm);
  expr_->setLocationInfo(this);
  expr_ = expr_->staticResolution(context);

  if(args_) {
    for(VectorOfASTNodes::iterator i = args_->begin(); i != args_->end(); ++i) {
      *i = (*i)->staticResolution(context);
    }
  }

  return this;
}

ASTNode *XQFunctionDeref::staticTypingImpl(StaticContext *context)
{
  _src.clear();

  _src.add(expr_->getStaticAnalysis());

  if(args_) {
    for(VectorOfASTNodes::iterator i = args_->begin(); i != args_->end(); ++i) {
      _src.add((*i)->getStaticAnalysis());
    }
  }

  if(expr_->getStaticAnalysis().getStaticType().getReturnType()) {
    _src.getStaticType() = *expr_->getStaticAnalysis().getStaticType().getReturnType();
  }
  else {
    _src.getStaticType() = StaticType(StaticType::ITEM_TYPE, 0, StaticType::UNLIMITED);
  }

  return this;
}

class FunctionDerefResult : public ResultImpl
{
public:
  FunctionDerefResult(const XQFunctionDeref *ast)
    : ResultImpl(ast),
      ast_(ast)
  {
  }

  virtual Item::Ptr nextOrTail(Result &tail, DynamicContext *context)
  {
    FunctionRef::Ptr func = (FunctionRef*)ast_->getExpression()->createResult(context)->next(context).get();
    if(func.isNull()) return 0;

    VectorOfResults args;
    if(ast_->getArguments()) {
      for(VectorOfASTNodes::iterator i = ast_->getArguments()->begin(); i != ast_->getArguments()->end(); ++i) {
        args.push_back(ClosureResult::create(*i, context));
      }
    }

    tail = func->execute(args, context, this);
    return 0;
  }

private:
  const XQFunctionDeref *ast_;
};

Result XQFunctionDeref::createResult(DynamicContext *context, int flags) const
{
  FunctionRef::Ptr func = (FunctionRef*)getExpression()->createResult(context)->next(context).get();
  if(func.isNull()) return 0;

  VectorOfResults args;
  if(getArguments()) {
    for(VectorOfASTNodes::iterator i = getArguments()->begin(); i != getArguments()->end(); ++i) {
      args.push_back(ClosureResult::create(*i, context));
    }
  }

  return func->execute(args, context, this);
}

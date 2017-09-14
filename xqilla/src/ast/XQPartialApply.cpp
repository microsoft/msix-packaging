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

#include <xqilla/ast/XQPartialApply.hpp>
#include <xqilla/context/DynamicContext.hpp>
#include <xqilla/items/FunctionRef.hpp>
#include <xqilla/exceptions/XPath2TypeMatchException.hpp>
#include <xqilla/runtime/ResultImpl.hpp>
#include <xqilla/runtime/ClosureResult.hpp>
#include <xqilla/schema/SequenceType.hpp>
#include <xqilla/ast/XQTreatAs.hpp>
#include <xqilla/utils/XPath2Utils.hpp>

XERCES_CPP_NAMESPACE_USE;
using namespace std;

XQPartialApply::XQPartialApply(ASTNode *expr, VectorOfASTNodes *args, XPath2MemoryManager *mm)
  : ASTNodeImpl(PARTIAL_APPLY, mm),
    expr_(expr),
    args_(args)
{
}

ASTNode *XQPartialApply::staticResolution(StaticContext *context)
{
  XPath2MemoryManager *mm = context->getMemoryManager();

  SequenceType *seqType = new (mm) SequenceType(new (mm) SequenceType::ItemType(SequenceType::ItemType::TEST_FUNCTION),
                                                SequenceType::EXACTLY_ONE);
  seqType->setLocationInfo(this);

  expr_ = new (mm) XQTreatAs(expr_, seqType, mm);
  expr_->setLocationInfo(this);
  expr_ = expr_->staticResolution(context);

  unsigned int argCount = 0;
  if(args_) {
    for(VectorOfASTNodes::iterator i = args_->begin(); i != args_->end(); ++i) {
      if(*i != 0) {
        *i = (*i)->staticResolution(context);
        ++argCount;
      }
    }
  }

  // If no actual arguments are specified, the partial apply has no effect
  if(argCount == 0) {
    ASTNode *result = expr_;
    expr_ = 0;
    this->release();
    return result;
  }

  return this;
}

ASTNode *XQPartialApply::staticTypingImpl(StaticContext *context)
{
  _src.clear();

  _src.add(expr_->getStaticAnalysis());

  unsigned int argCount = 0;
  if(args_) {
    for(VectorOfASTNodes::iterator i = args_->begin(); i != args_->end(); ++i) {
      if(*i != 0) {
        _src.add((*i)->getStaticAnalysis());
        ++argCount;
      }
    }
  }

  const StaticType &inType = expr_->getStaticAnalysis().getStaticType();

  if(inType.getReturnType() == 0) {
    _src.getStaticType() = StaticType::FUNCTION_TYPE;
  }
  else {
    unsigned int minArgs = inType.getMinArgs();
    unsigned int maxArgs = inType.getMaxArgs();

    if(minArgs > argCount) minArgs -= argCount; else minArgs = 0;
    if(maxArgs > argCount) maxArgs -= argCount; else maxArgs = 0;

    // TBD Using getMemoryManager() might not be thread safe in DB XML - jpcs
    _src.getStaticType() = StaticType(getMemoryManager(), minArgs, maxArgs,
                                      *inType.getReturnType());
  }

  return this;
}

Result XQPartialApply::createResult(DynamicContext *context, int flags) const
{
  FunctionRef::Ptr function = (FunctionRef*)expr_->createResult(context)->next(context).get();

  if(args_->size() != function->getNumArgs()) {
    XMLBuffer buf;
    buf.set(X("The function item invoked does not accept "));
    XPath2Utils::numToBuf((unsigned int)args_->size(), buf);
    buf.append(X(" arguments - found item of type "));
    function->typeToBuffer(context, buf);
    buf.append(X(" [err:XPTY0004]"));
    XQThrow(XPath2TypeMatchException, X("XQPartialApply::createResult"), buf.getRawBuffer());
  }

  int argNum = 1;
  VectorOfASTNodes::iterator i;
  for(i = args_->begin(); i != args_->end(); ++i) {
    if(*i)
      function = function->partialApply((*i)->createResult(context), argNum, context, this);
    else ++argNum;
  }

  return (Item::Ptr)function;
}

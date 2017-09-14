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
#include <xqilla/functions/FunctionAnalyzeString.hpp>
#include <xqilla/context/DynamicContext.hpp>
#include <xqilla/exceptions/FunctionException.hpp>
#include <xqilla/ast/StaticAnalysis.hpp>
#include <xqilla/ast/XQAnalyzeString.hpp>
#include <xqilla/schema/SequenceType.hpp>
#include <xqilla/items/FunctionRef.hpp>
#include <xqilla/runtime/ClosureResult.hpp>

#include <xercesc/validators/schema/SchemaSymbols.hpp>

XERCES_CPP_NAMESPACE_USE;

const XMLCh FunctionAnalyzeString::name[] = {
  chLatin_a, chLatin_n, chLatin_a, chLatin_l, chLatin_y, chLatin_z, chLatin_e, chDash, chLatin_s, chLatin_t, chLatin_r, chLatin_i, chLatin_n, chLatin_g, chNull 
};
const unsigned int FunctionAnalyzeString::minArgs = 3;
const unsigned int FunctionAnalyzeString::maxArgs = 4;

/**
 * xqilla:analyze-string($input as xs:string?, $pattern as xs:string,
 *   $action as function(xs:string, xs:boolean) as item()*) as item()*
 * xqilla:analyze-string($input as xs:string?, $pattern as xs:string,
 *   $action as function(xs:string, xs:boolean) as item()*, $flags as xs:string) as item()*
 */
FunctionAnalyzeString::FunctionAnalyzeString(const VectorOfASTNodes &args, XPath2MemoryManager* memMgr)
  : XQillaFunction(name, "($input as xs:string?, $pattern as xs:string, "
                   "$action as function(xs:string, xs:boolean) as item()*, $flags as xs:string) as item()*", args, memMgr)
{
}

ASTNode *FunctionAnalyzeString::staticTypingImpl(StaticContext *context)
{
  _src.clearExceptType();
  _src.setProperties(0);
  calculateSRCForArguments(context);

  // TBD Precompile the regex - jpcs

  if(_args[2]->getStaticAnalysis().getStaticType().getReturnType()) {
    _src.getStaticType() = *_args[2]->getStaticAnalysis().getStaticType().getReturnType();
    _src.getStaticType().setCardinality(0, StaticType::UNLIMITED);
  }
  else {
    _src.getStaticType() = StaticType(StaticType::ITEM_TYPE, 0, StaticType::UNLIMITED);
  }

  return this;
}

class FunctionAnalyzeStringResult : public AnalyzeStringResult
{
public:
  FunctionAnalyzeStringResult(const FunctionAnalyzeString *ast)
    : AnalyzeStringResult(ast),
      ast_(ast),
      func_(0)
  {
  }

  const XMLCh *getInput(DynamicContext *context)
  {
    return ast_->getParamNumber(1, context)->next(context)->asString(context);
  }

  const XMLCh *getPattern(DynamicContext *context)
  {
    return ast_->getParamNumber(2, context)->next(context)->asString(context);
  }

  const XMLCh *getFlags(DynamicContext *context)
  {
    if(ast_->getNumArgs() == 4)
      return ast_->getParamNumber(4, context)->next(context)->asString(context);
    return XMLUni::fgZeroLenString;
  }

  Result getMatchResult(const XMLCh *matchString, size_t matchPos,
                        size_t numberOfMatches, bool match, DynamicContext *context)
  {
    if(func_.isNull())
      func_ = (FunctionRef*)ast_->getParamNumber(3, context)->next(context).get();

    VectorOfResults args;
    args.push_back(Result(context->getItemFactory()->createString(matchString, context)));
    args.push_back(Result(context->getItemFactory()->createBoolean(match, context)));

    return func_->execute(args, context, this);
  }

private:
  const FunctionAnalyzeString *ast_;
  FunctionRef::Ptr func_;
};

Result FunctionAnalyzeString::createResult(DynamicContext* context, int flags) const
{
  return ClosureResult::create(getStaticAnalysis(), context, new FunctionAnalyzeStringResult(this));
}

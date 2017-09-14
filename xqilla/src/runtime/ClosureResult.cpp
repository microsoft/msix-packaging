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
#include <xqilla/runtime/ClosureResult.hpp>
#include <xqilla/ast/ASTNode.hpp>
#include <xqilla/context/DynamicContext.hpp>
#include <xqilla/context/ContextHelpers.hpp>

using namespace std;

class LazyCreateResult : public ResultImpl
{
public:
  LazyCreateResult(const ASTNode *ast)
    : ResultImpl(ast), ast_(ast) {}

  virtual Item::Ptr nextOrTail(Result &tail, DynamicContext *context)
  {
    tail = ast_->createResult(context);
    return 0;
  }

private:
  const ASTNode *ast_;
};

Result ClosureResult::create(const StaticAnalysis &src, DynamicContext *context, ResultImpl *result)
{
  // TBD We probably need to store the regex groups here too - jpcs
  if(!src.isVariableUsed() && !src.areContextFlagsUsed())
    return result;
  return new ClosureResult(src, context, result);
}

Result ClosureResult::create(const ASTNode *ast, DynamicContext *context)
{
  return create(ast->getStaticAnalysis(), context, new LazyCreateResult(ast));
}

ClosureResult::ClosureResult(const StaticAnalysis &src, DynamicContext *context, ResultImpl *result)
  : ResultImpl(result),
    contextItem_(context->getContextItem()),
    contextPosition_(context->getContextPosition()),
    contextSize_(context->getContextSize()),
    varStore_(context->getMemoryManager()),
    result_(result)
{
  // Copy the variables we need into our local storage
  varStore_.cacheVariableStore(src, context->getVariableStore());
}

Item::Ptr ClosureResult::nextOrTail(Result &tail, DynamicContext *context)
{
  context->testInterrupt();

  // TBD Fix the DocumentCache/module import problem - jpcs
  // AutoDocumentCacheReset dcReset(context);
  // context->setDocumentCache(docCache_);
  AutoContextInfoReset ciReset(context, contextItem_, contextPosition_, contextSize_);
  AutoVariableStoreReset vsReset(context, &varStore_);

  return result_->nextOrTail(tail, context);
}

ClosureEventGenerator::ClosureEventGenerator(const ASTNode *ast, DynamicContext *context, bool preserveNS, bool preserveType)
  : ASTNodeEventGenerator(ast, preserveNS, preserveType),
    contextItem_(context->getContextItem()),
    contextPosition_(context->getContextPosition()),
    contextSize_(context->getContextSize()),
    varStore_(context->getMemoryManager())
{
  // Copy the variables we need into our local storage
  varStore_.cacheVariableStore(ast->getStaticAnalysis(), context->getVariableStore());
}

EventGenerator::Ptr ClosureEventGenerator::generateEvents(EventHandler *events, DynamicContext *context)
{
  context->testInterrupt();

  // AutoDocumentCacheReset dcReset(context);
  // context->setDocumentCache(docCache_);
  AutoContextInfoReset ciReset(context, contextItem_, contextPosition_, contextSize_);
  AutoVariableStoreReset vsReset(context, &varStore_);

  return ast_->generateEvents(events, context, preserveNS_, preserveType_);
}

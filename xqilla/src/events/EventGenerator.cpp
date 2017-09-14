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

#include <xqilla/events/EventGenerator.hpp>
#include <xqilla/ast/ASTNode.hpp>
#include <xqilla/context/DynamicContext.hpp>
#include <xqilla/events/QueryPathTreeFilter.hpp>
#include <xqilla/events/SequenceBuilder.hpp>
#include <xqilla/runtime/Sequence.hpp>

ASTNodeEventGenerator::ASTNodeEventGenerator(const ASTNode *ast, bool preserveNS, bool preserveType)
  : ast_(ast),
    preserveNS_(preserveNS),
    preserveType_(preserveType)
{
}

EventGenerator::Ptr ASTNodeEventGenerator::generateEvents(EventHandler *events, DynamicContext *context)
{
  return ast_->generateEvents(events, context, preserveNS_, preserveType_);
}

Result EventGenerator::createResult(const ASTNode *ast, const QueryPathNode *projection,
  DynamicContext *context)
{
  AutoDelete<SequenceBuilder> builder(context->createSequenceBuilder());
  if(context->getProjection() && projection != 0) {
    QueryPathTreeFilter qptf(projection, builder.get());
    ast->generateAndTailCall(&qptf, context, true, true);
    qptf.endEvent();
  }
  else {
    ast->generateAndTailCall(builder.get(), context, true, true);
    builder->endEvent();
  }

  return builder->getSequence();
}

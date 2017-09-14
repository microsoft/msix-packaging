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

#include <xqilla/ast/XQCopyOf.hpp>
#include <xqilla/context/DynamicContext.hpp>
#include <xqilla/events/SequenceBuilder.hpp>

XERCES_CPP_NAMESPACE_USE
using namespace std;

XQCopyOf::XQCopyOf(XPath2MemoryManager* memMgr)
  : ASTNodeImpl(COPY_OF, memMgr),
    expr_(0),
    copyNamespaces_(true)
{
}

XQCopyOf::XQCopyOf(ASTNode *expr, bool copyNamespaces, XPath2MemoryManager* memMgr)
  : ASTNodeImpl(COPY_OF, memMgr),
    expr_(expr),
    copyNamespaces_(copyNamespaces)
{
}

ASTNode* XQCopyOf::staticResolution(StaticContext *context)
{
  expr_ = expr_->staticResolution(context);

  return this;
}

ASTNode *XQCopyOf::staticTypingImpl(StaticContext *context)
{
  _src.clear();

  _src.copy(expr_->getStaticAnalysis());

  _src.setProperties(0);

  if(!_src.getStaticType().containsType(StaticType::NODE_TYPE)) {
    return expr_;
  }

  return this;
}

Result XQCopyOf::createResult(DynamicContext* context, int flags) const
{
  return EventGenerator::createResult(this, 0, context);
}

EventGenerator::Ptr XQCopyOf::generateEvents(EventHandler *events, DynamicContext *context,
                                        bool preserveNS, bool preserveType) const
{
  return expr_->generateEvents(events, context, copyNamespaces_, false);
}


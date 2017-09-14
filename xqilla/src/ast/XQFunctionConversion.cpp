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

#include <xqilla/ast/XQFunctionConversion.hpp>
#include <xqilla/schema/SequenceType.hpp>
#include <xqilla/context/DynamicContext.hpp>

#if defined(XERCES_HAS_CPP_NAMESPACE)
XERCES_CPP_NAMESPACE_USE
#endif

XQFunctionConversion::XQFunctionConversion(ASTNode* expr, SequenceType* seqType, XPath2MemoryManager* memMgr)
  : ASTNodeImpl(FUNCTION_CONVERSION, memMgr),
    expr_(expr),
    seqType_(seqType)
{
}

ASTNode* XQFunctionConversion::staticResolution(StaticContext *context)
{
  seqType_->staticResolution(context);
  return seqType_->convertFunctionArg(expr_, context, /*numericfunction*/false, seqType_)
    ->staticResolution(context);
}

ASTNode *XQFunctionConversion::staticTypingImpl(StaticContext *context)
{
  // Never happens
  return this;
}

Result XQFunctionConversion::createResult(DynamicContext* context, int flags) const
{
  // Never happens
  return 0;
}

ASTNode *XQFunctionConversion::getExpression() const {
  return expr_;
}

SequenceType *XQFunctionConversion::getSequenceType() const {
  return seqType_;
}

void XQFunctionConversion::setExpression(ASTNode *item) {
  expr_ = item;
}

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

#ifndef XQFUNCTIONCONVERSION_HPP
#define XQFUNCTIONCONVERSION_HPP

#include <xqilla/ast/ASTNodeImpl.hpp>

class XQILLA_API XQFunctionConversion : public ASTNodeImpl
{
public:
  XQFunctionConversion(ASTNode* expr, SequenceType *seqType, XPath2MemoryManager* memMgr);

  virtual Result createResult(DynamicContext *context, int flags=0) const;
  virtual ASTNode* staticResolution(StaticContext *context);
  virtual ASTNode *staticTypingImpl(StaticContext *context);

  ASTNode *getExpression() const;
  SequenceType *getSequenceType() const;

  void setExpression(ASTNode *item);

protected:
  ASTNode* expr_;
  SequenceType *seqType_;
};

#endif

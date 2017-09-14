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

#ifndef _XQFUNCTIONCOERCION_HPP
#define _XQFUNCTIONCOERCION_HPP

#include <xqilla/framework/XQillaExport.hpp>

#include <xqilla/ast/ASTNodeImpl.hpp>

class SequenceType;

class XQILLA_API XQFunctionCoercion : public ASTNodeImpl
{
public:
  XQFunctionCoercion(ASTNode *expr, SequenceType *exprType, XPath2MemoryManager *memMgr);
  XQFunctionCoercion(ASTNode *expr, SequenceType *exprType, ASTNode *funcConvert, const StaticType &treatType, XPath2MemoryManager *memMgr);

  virtual Result createResult(DynamicContext* context, int flags=0) const;
  virtual ASTNode *staticResolution(StaticContext *context);
  virtual ASTNode *staticTypingImpl(StaticContext *context);

  ASTNode *getExpression() const { return _expr; }
  void setExpression(ASTNode *item) { _expr = item; }
  ASTNode *getFuncConvert() const { return _funcConvert; }
  void setFuncConvert(ASTNode *item) { _funcConvert = item; }
  SequenceType *getSequenceType() const { return _exprType; }
  const StaticType &getTreatType() const { return _treatType; }

  static const XMLCh funcVarName[];

protected:
  ASTNode* _expr;
  SequenceType *_exprType;
  ASTNode *_funcConvert;
  StaticType _treatType;
};

#endif

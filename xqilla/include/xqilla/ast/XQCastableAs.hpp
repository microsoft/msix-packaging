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

#ifndef _XQCASTABLEAS_HPP
#define _XQCASTABLEAS_HPP

#include <xqilla/framework/XQillaExport.hpp>

#include <xqilla/ast/ASTNodeImpl.hpp>

class SequenceType;

class XQILLA_API XQCastableAs : public ASTNodeImpl
{
public:
  XQCastableAs(ASTNode* expr, SequenceType* exprType, XPath2MemoryManager* memMgr);
  XQCastableAs(ASTNode* expr, SequenceType* exprType, bool isPrimitive, AnyAtomicType::AtomicObjectType typeIndex, XPath2MemoryManager* memMgr);

  virtual ASTNode* staticResolution(StaticContext *context);
  virtual ASTNode *staticTypingImpl(StaticContext *context);
  virtual BoolResult boolResult(DynamicContext* context) const;
  virtual Result createResult(DynamicContext* context, int flags) const;

  ASTNode *getExpression() const;
  SequenceType *getSequenceType() const;
  bool getIsPrimitive() const { return _isPrimitive; }
  AnyAtomicType::AtomicObjectType getTypeIndex() const { return _typeIndex; }  

  void setExpression(ASTNode *item);

protected:
  ASTNode* _expr;
  SequenceType* _exprType;
  bool _isPrimitive;
  AnyAtomicType::AtomicObjectType _typeIndex;
};

#endif

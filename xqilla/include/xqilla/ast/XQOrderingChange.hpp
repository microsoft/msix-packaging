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

#ifndef XQORDERINGCHANGE_HPP
#define XQORDERINGCHANGE_HPP

#include <xqilla/ast/ASTNodeImpl.hpp>
#include <xqilla/context/StaticContext.hpp>

class XQILLA_API XQOrderingChange : public ASTNodeImpl
{
public:
  XQOrderingChange(StaticContext::NodeSetOrdering ordering, ASTNode* astNode, XPath2MemoryManager* expr);

  virtual Result createResult(DynamicContext *context, int flags=0) const;
  virtual ASTNode* staticResolution(StaticContext *context);
  virtual ASTNode *staticTypingImpl(StaticContext *context);

  StaticContext::NodeSetOrdering getOrderingValue() const;
  ASTNode* getExpr() const;
  void setExpr(ASTNode *expr);

protected:
  StaticContext::NodeSetOrdering m_nOrdering;
  ASTNode* m_pExpr;
};

#endif

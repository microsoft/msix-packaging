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

#include <sstream>

#include <xqilla/framework/XQillaExport.hpp>
#include <xqilla/ast/XQOrderingChange.hpp>

#include <xqilla/context/DynamicContext.hpp>
#include <xqilla/context/ContextHelpers.hpp>

XQOrderingChange::XQOrderingChange(StaticContext::NodeSetOrdering ordering, ASTNode* astNode, XPath2MemoryManager* expr)
  : ASTNodeImpl(ORDERING_CHANGE, expr),
    m_nOrdering(ordering),
    m_pExpr(astNode)
{
}

Result XQOrderingChange::createResult(DynamicContext *context, int flags) const
{
  return m_pExpr->createResult(context);
}

ASTNode* XQOrderingChange::staticResolution(StaticContext* context)
{
  AutoNodeSetOrderingReset orderReset(context, m_nOrdering);
  m_pExpr = m_pExpr->staticResolution(context);
  return this;
}

ASTNode *XQOrderingChange::staticTypingImpl(StaticContext *context)
{
  _src.clear();
  _src.copy(m_pExpr->getStaticAnalysis());
  return this;
}

StaticContext::NodeSetOrdering XQOrderingChange::getOrderingValue() const
{
  return m_nOrdering;
}

ASTNode* XQOrderingChange::getExpr() const
{
  return m_pExpr;
}

void XQOrderingChange::setExpr(ASTNode *expr)
{
	m_pExpr = expr;
}

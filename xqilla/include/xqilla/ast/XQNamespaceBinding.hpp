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

#ifndef XQNAMESPACEBINDING_HPP
#define XQNAMESPACEBINDING_HPP

#include <xqilla/ast/ASTNodeImpl.hpp>

#include <xercesc/dom/DOMXPathNSResolver.hpp>

class XQILLA_API XQNamespaceBinding : public ASTNodeImpl
{
public:
  XQNamespaceBinding(XERCES_CPP_NAMESPACE_QUALIFIER DOMXPathNSResolver *namespaces, ASTNode *expr, XPath2MemoryManager *mm);

  virtual ASTNode *staticResolution(StaticContext *context);
  virtual ASTNode *staticTypingImpl(StaticContext *context);

  virtual Result createResult(DynamicContext *context, int flags=0) const;
  virtual Result iterateResult(const Result &contextItems, DynamicContext *context) const;
  virtual EventGenerator::Ptr generateEvents(EventHandler *events, DynamicContext *context,
                                             bool preserveNS, bool preserveType) const;
  virtual PendingUpdateList createUpdateList(DynamicContext *context) const;

  ASTNode *getExpression() const { return expr_; }
  void setExpression(ASTNode *expr) { expr_ = expr; }

  XERCES_CPP_NAMESPACE_QUALIFIER DOMXPathNSResolver *getNamespaces() const { return namespaces_; }

protected:
  ASTNode *expr_;
  XERCES_CPP_NAMESPACE_QUALIFIER DOMXPathNSResolver *namespaces_;
};

#endif

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

#ifndef XQCOPY_HPP
#define XQCOPY_HPP

#include <xqilla/ast/ASTNodeImpl.hpp>

class XQILLA_API XQCopy : public ASTNodeImpl
{
public:
  XQCopy(XPath2MemoryManager* memMgr);
  XQCopy(ASTNode *expr, const VectorOfASTNodes &children, bool copyNamespaces, bool inheritNamespaces, XPath2MemoryManager* memMgr);

  virtual ASTNode *staticResolution(StaticContext *context);
  virtual ASTNode *staticTypingImpl(StaticContext *context);

  virtual Result createResult(DynamicContext* context, int flags=0) const;
  virtual EventGenerator::Ptr generateEvents(EventHandler *events, DynamicContext *context,
                                             bool preserveNS, bool preserveType) const;
  EventGenerator::Ptr generateEventsImpl(const Item::Ptr &toBeCopied, EventHandler *events, DynamicContext *context,
                                         bool preserveNS, bool preserveType) const;


  ASTNode *getExpression() const { return expr_; }
  void setExpression(ASTNode *item) { expr_ = item; }

  virtual const VectorOfASTNodes &getChildren() const { return children_; }
  virtual void setChildren(const VectorOfASTNodes &ch) { children_ = ch; }

  bool getCopyNamespaces() const { return copyNamespaces_; }
  void setCopyNamespaces(bool value) { copyNamespaces_ = value; }

  bool getInheritNamespaces() const { return inheritNamespaces_; }
  void setInheritNamespaces(bool value) { inheritNamespaces_ = value; }

protected:
  ASTNode *expr_;
  VectorOfASTNodes children_;
  bool copyNamespaces_;
  bool inheritNamespaces_;
};

#endif

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

#ifndef XQQUANTIFIED_HPP
#define XQQUANTIFIED_HPP

#include <xqilla/ast/ASTNodeImpl.hpp>

class TupleNode;

class XQILLA_API XQQuantified : public ASTNodeImpl
{
public:
  enum Type {
    SOME,
    EVERY
  };

  XQQuantified(Type qtype, TupleNode *parent, ASTNode *expr, XPath2MemoryManager *mm);

  Type getQuantifierType() const { return qtype_; }

  TupleNode *getParent() const { return parent_; }
  void setParent(TupleNode *parent) { parent_ = parent; }

  ASTNode *getExpression() const { return expr_; }
  void setExpression(ASTNode *expr) { expr_ = expr; }

  virtual ASTNode *staticResolution(StaticContext *context);
  virtual ASTNode *staticTypingImpl(StaticContext *context);
  virtual BoolResult boolResult(DynamicContext* context) const;
  virtual Result createResult(DynamicContext* context, int flags) const;

private:
  Type qtype_;
  TupleNode *parent_;
  ASTNode *expr_;
};

#endif

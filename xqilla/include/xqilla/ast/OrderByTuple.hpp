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

#ifndef ORDERBYTUPLE_HPP
#define ORDERBYTUPLE_HPP

#include <xqilla/ast/TupleNode.hpp>
#include <xqilla/ast/StaticAnalysis.hpp>

class ASTNode;
class Collation;

class XQILLA_API OrderByTuple : public TupleNode
{
public:
  enum Modifiers {
    ASCENDING      = 0x0,
    DESCENDING     = 0x1,

    EMPTY_GREATEST = 0x0,
    EMPTY_LEAST    = 0x2,

    STABLE         = 0x0,
    UNSTABLE       = 0x4,

    DESCENDING_EMPTY_LEAST          = DESCENDING | EMPTY_LEAST,
    DESCENDING_UNSTABLE             = DESCENDING | UNSTABLE,
    EMPTY_LEAST_UNSTABLE            = EMPTY_LEAST | UNSTABLE,
    DESCENDING_EMPTY_LEAST_UNSTABLE = DESCENDING | EMPTY_LEAST | UNSTABLE
  };

  OrderByTuple(TupleNode *parent, ASTNode *expr, int modifiers, Collation *collation, XPath2MemoryManager *mm);

  ASTNode *getExpression() const { return expr_; }
  void setExpression(ASTNode *expr) { expr_ = expr; }

  Modifiers getModifiers() const { return modifiers_; }
  Collation *getCollation() const { return collation_; }

  void setUnstable() { modifiers_ = (Modifiers)(modifiers_ | UNSTABLE); }

  const StaticAnalysis &getUsedSRC() const { return usedSrc_; }

  virtual TupleNode *staticResolution(StaticContext *context);
  virtual TupleNode *staticTypingImpl(StaticContext *context);
  virtual TupleNode *staticTypingTeardown(StaticContext *context, StaticAnalysis &usedSrc);

  virtual TupleResult::Ptr createResult(DynamicContext* context) const;

private:
  ASTNode *expr_;
  Modifiers modifiers_;
  Collation *collation_;
  StaticAnalysis usedSrc_;
};

#endif

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

#ifndef TUPLENODE_HPP
#define TUPLENODE_HPP

#include <xqilla/runtime/TupleResult.hpp>

class StaticContext;
class StaticAnalysis;
class XPath2MemoryManager;

class XQILLA_API TupleNode : public LocationInfo
{
public:
  enum Type {
    CONTEXT_TUPLE,
    FOR,
    LET,
    WHERE,
    ORDER_BY,
    COUNT,
    DEBUG_HOOK
  };

  virtual ~TupleNode() {}

  virtual void release();

  virtual Type getType() const { return type_; }
  virtual XPath2MemoryManager *getMemoryManager() const { return mm_; }

  unsigned int getMin() const { return min_; }
  unsigned int getMax() const { return max_; }
  void setMin(unsigned int v) { min_ = v; }
  void setMax(unsigned int v) { max_ = v; }

  TupleNode *getParent() const { return parent_; }
  void setParent(TupleNode *parent) { parent_ = parent; }

  void *getUserData() const { return userData_; }
  void setUserData(void *data) { userData_ = data; }

  virtual TupleNode *staticResolution(StaticContext *context) = 0;
  virtual TupleNode *staticTypingImpl(StaticContext *context) = 0;
  virtual TupleNode *staticTypingTeardown(StaticContext *context, StaticAnalysis &usedSrc) = 0;

  virtual TupleResult::Ptr createResult(DynamicContext* context) const = 0;

protected:
  TupleNode(Type type, TupleNode *parent, XPath2MemoryManager *mm)
    : type_(type), parent_(parent), min_(0), max_(0), userData_(0), mm_(mm) {}

  Type type_;
  TupleNode *parent_;
  unsigned int min_, max_;
  void *userData_;
  XPath2MemoryManager *mm_;
};

#endif

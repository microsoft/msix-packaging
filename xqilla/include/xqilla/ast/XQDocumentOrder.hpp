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

#ifndef _XQDOCUMENTORDER_HPP
#define _XQDOCUMENTORDER_HPP

#include <xqilla/framework/XQillaExport.hpp>
#include <xqilla/ast/ASTNodeImpl.hpp>
#include <xqilla/items/Node.hpp>

#include <set>

class XQILLA_API XQDocumentOrder : public ASTNodeImpl
{
public:
  XQDocumentOrder(ASTNode* expr, XPath2MemoryManager* memMgr);
  XQDocumentOrder(ASTNode* expr, bool unordered, XPath2MemoryManager* memMgr);

  virtual Result createResult(DynamicContext* context, int flags=0) const;
  virtual ASTNode* staticResolution(StaticContext *context);
  virtual ASTNode *staticTypingImpl(StaticContext *context);

  ASTNode *getExpression() const { return expr_; }
  void setExpression(ASTNode *expr) { expr_ = expr; }
  bool getUnordered() const { return unordered_; }

protected:
  ASTNode* expr_;
  bool unordered_;
};

class XQILLA_API DocumentOrderResult : public ResultImpl
{
public:
  DocumentOrderResult(const LocationInfo *location, const Result &parent)
    : ResultImpl(location), parent_(parent) {}
  virtual Item::Ptr nextOrTail(Result &tail, DynamicContext *context);
private:
  Result parent_;
};

class XQILLA_API UniqueNodesResult : public ResultImpl
{
public:
  UniqueNodesResult(const XQDocumentOrder *doc, const Result &parent, const DynamicContext *context)
    : ResultImpl(doc), parent_(parent), nTypeOfItemsInLastStep_(0),
      noDups_(uniqueLessThanCompareFn(context)) {}
  Item::Ptr next(DynamicContext *context);

private:
  class uniqueLessThanCompareFn {
  public:
    uniqueLessThanCompareFn(const DynamicContext *context)
      : context_(context) {}
    bool operator()(const Node::Ptr &first, const Node::Ptr &second)
    {
      return first->uniqueLessThan(second, context_);
    }
  private:
    const DynamicContext *context_;
  };
  typedef std::set<Node::Ptr, uniqueLessThanCompareFn> NoDuplicatesSet;

  Result parent_;
  int nTypeOfItemsInLastStep_;
  NoDuplicatesSet noDups_;
};

#endif

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

#ifndef _XQPREDICATE_HPP
#define _XQPREDICATE_HPP

#include <xqilla/ast/ASTNodeImpl.hpp>

class XQPredicate;

// NOTE: the use of "::" in front of XQillaAllocator here is
// to work around an issue in the HP-UX aCC compiler and STL
// implementation.  Do not remove it.
class VectorOfPredicates : public std::vector<XQPredicate*,::XQillaAllocator<XQPredicate*> >
{
public:
  VectorOfPredicates(XERCES_CPP_NAMESPACE_QUALIFIER MemoryManager *mm)
	  : std::vector<XQPredicate*,::XQillaAllocator<XQPredicate*> >(::XQillaAllocator<XQPredicate*>(mm)) {}
};


class XQILLA_API XQPredicate : public ASTNodeImpl
{
public:
  XQPredicate(ASTNode *predicate, XPath2MemoryManager* memMgr);
  XQPredicate(ASTNode *expr, ASTNode *predicate, XPath2MemoryManager* memMgr);
  XQPredicate(ASTNode *expr, ASTNode *predicate, bool reverse, XPath2MemoryManager* memMgr);

  virtual ASTNode* staticResolution(StaticContext *context);
  virtual ASTNode *staticTypingImpl(StaticContext *context);
  virtual Result createResult(DynamicContext* context, int flags=0) const;
  virtual Result iterateResult(const Result &contextItems, DynamicContext* context) const;

  ASTNode *getExpression() const { return expr_; }
  void setExpression(ASTNode *expr) { expr_ = expr; }

  ASTNode *getPredicate() const { return predicate_; }
  void setPredicate(ASTNode *pred) { predicate_ = pred; }

  bool getReverse() const { return reverse_; }
  void setReverse(bool reverse) { reverse_ = reverse; }

  static ASTNode *addPredicates(ASTNode *expr, VectorOfPredicates *preds);
  static ASTNode *addReversePredicates(ASTNode *expr, VectorOfPredicates *preds);

protected:
  ASTNode *expr_;
  ASTNode *predicate_;
  bool reverse_;
};

class XQILLA_API PredicateFilterResult : public ResultImpl
{
public:
	PredicateFilterResult(const Result &parent, const ASTNode *pred, size_t contextSize);
	Item::Ptr next(DynamicContext *context);
	std::string asString(DynamicContext *context, int indent) const;
private:
	bool todo_;
	Result parent_;
	const ASTNode *pred_;
	size_t contextPos_;
	size_t contextSize_;
	Item::Ptr first_;
	Item::Ptr second_;
};

class NonNumericPredicateFilterResult : public ResultImpl
{
public:
	NonNumericPredicateFilterResult(const Result &parent, const ASTNode *pred, size_t contextSize);
	Item::Ptr next(DynamicContext *context);
	std::string asString(DynamicContext *context, int indent) const;
private:
	bool todo_;
	Result parent_;
	const ASTNode *pred_;
	size_t contextPos_;
	size_t contextSize_;
};

class NumericPredicateFilterResult : public ResultImpl
{
public:
  NumericPredicateFilterResult(const Result &parent, const ASTNode *pred, size_t contextSize);
  virtual Item::Ptr nextOrTail(Result &tail, DynamicContext *context);
private:
  Result parent_;
  const ASTNode *pred_;
  size_t contextSize_;
};

#endif

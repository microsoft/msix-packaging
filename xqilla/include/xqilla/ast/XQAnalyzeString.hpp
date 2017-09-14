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

#ifndef XQANALYZESTRING_HPP
#define XQANALYZESTRING_HPP

#include <xqilla/ast/ASTNodeImpl.hpp>
#include <xqilla/context/RegexGroupStore.hpp>

#include <xercesc/util/regx/Match.hpp>
#include <xercesc/util/RefVectorOf.hpp>

class XQILLA_API XQAnalyzeString : public ASTNodeImpl
{
public:
  XQAnalyzeString(XPath2MemoryManager* memMgr);
  XQAnalyzeString(ASTNode *expr, ASTNode *regex, ASTNode *flags, ASTNode *match, ASTNode *nonMatch, XPath2MemoryManager* memMgr);

  virtual Result createResult(DynamicContext *context, int flags=0) const;
  virtual ASTNode *staticResolution(StaticContext *context);
  virtual ASTNode *staticTypingImpl(StaticContext *context);

  ASTNode *getExpression() const { return expr_; }
  void setExpression(ASTNode *item) { expr_ = item; }
  ASTNode *getRegex() const { return regex_; }
  void setRegex(ASTNode *item) { regex_ = item; }
  ASTNode *getFlags() const { return flags_; }
  void setFlags(ASTNode *item) { flags_ = item; }
  ASTNode *getMatch() const { return match_; }
  void setMatch(ASTNode *item) { match_ = item; }
  ASTNode *getNonMatch() const { return nonMatch_; }
  void setNonMatch(ASTNode *item) { nonMatch_ = item; }

protected:
  ASTNode *expr_;
  ASTNode *regex_;
  ASTNode *flags_;
  ASTNode *match_;
  ASTNode *nonMatch_;
};

class XQILLA_API AnalyzeStringResult : public ResultImpl, private RegexGroupStore
{
public:
  AnalyzeStringResult(const LocationInfo *info);

  virtual const XMLCh *getInput(DynamicContext *context) = 0;
  virtual const XMLCh *getPattern(DynamicContext *context) = 0;
  virtual const XMLCh *getFlags(DynamicContext *context) = 0;
  virtual Result getMatchResult(const XMLCh *matchString, size_t matchPos, size_t numberOfMatches,
                                bool match, DynamicContext *context) = 0;

  virtual Item::Ptr next(DynamicContext *context);

private:
  virtual const XMLCh *getGroup(int index) const;

  const XMLCh *input_;
  XERCES_CPP_NAMESPACE_QUALIFIER RefVectorOf<XERCES_CPP_NAMESPACE_QUALIFIER Match> matches_;
  std::vector<std::pair<const XMLCh*, XERCES_CPP_NAMESPACE_QUALIFIER Match*> > strings_;

  size_t contextPos_;
  XERCES_CPP_NAMESPACE_QUALIFIER Match *currentMatch_;
  XPath2MemoryManager *mm_;

  Result result_;
};

#endif

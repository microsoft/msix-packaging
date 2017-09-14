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

#ifndef _FTMILDNOT_HPP
#define _FTMILDNOT_HPP

#include <xqilla/fulltext/FTSelection.hpp>

#include <set>

class XQILLA_API FTMildnot : public FTSelection
{
public:
  FTMildnot(FTSelection *left, FTSelection *right, XPath2MemoryManager *memMgr);

  virtual FTSelection *staticResolution(StaticContext *context);
  virtual FTSelection *staticTypingImpl(StaticContext *context);
  virtual FTSelection *optimize(FTContext *context) const;
  virtual AllMatches *execute(FTContext *ftcontext) const;

  FTSelection *getLeft() const { return left_; }
  void setLeft(FTSelection *expr) { left_ = expr; }
  FTSelection *getRight() const { return right_; }
  void setRight(FTSelection *expr) { right_ = expr; }

private:
  FTSelection *left_, *right_;
};

class FTMildnotMatches : public AllMatches
{
public:
  FTMildnotMatches(const LocationInfo *info, AllMatches *left, AllMatches *right)
    : AllMatches(info), left_(left), right_(right) {}
  ~FTMildnotMatches();
  bool next(DynamicContext *context);
  const StringMatches &getStringIncludes();
  const StringMatches &getStringExcludes();

private:
  AllMatches *left_;
  AllMatches *right_;
  std::set<unsigned int> badTokens_;
};

#endif

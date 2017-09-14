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

#ifndef _FTOR_HPP
#define _FTOR_HPP

#include <xqilla/fulltext/FTSelection.hpp>

#include <vector>

class XQILLA_API FTOr : public FTSelection
{
public:
  FTOr(XPath2MemoryManager *memMgr);
  FTOr(FTSelection *left, FTSelection *right, XPath2MemoryManager *memMgr);

  virtual FTSelection *staticResolution(StaticContext *context);
  virtual FTSelection *staticTypingImpl(StaticContext *context);
  virtual FTSelection *optimize(FTContext *context) const;
  virtual AllMatches *execute(FTContext *ftcontext) const;

  const VectorOfFTSelections &getArguments() const { return args_; }
  void addArg(FTSelection *sel) { args_.push_back(sel); }

private:
  VectorOfFTSelections args_;
};

class FTDisjunctionMatches : public AllMatches
{
public:
  FTDisjunctionMatches(const LocationInfo *info);
  ~FTDisjunctionMatches();

  void addMatches(AllMatches *m)
  {
    args_.push_back(m);
  }

  bool next(DynamicContext *context);
  const StringMatches &getStringIncludes();
  const StringMatches &getStringExcludes();

private:
  bool toDo_;
  std::vector<AllMatches*> args_;
  std::vector<AllMatches*>::iterator it_;
};

#endif

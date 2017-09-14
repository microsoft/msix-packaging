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

#ifndef _FTAND_HPP
#define _FTAND_HPP

#include <xqilla/fulltext/FTSelection.hpp>

#include <list>

class XQILLA_API FTAnd : public FTSelection
{
public:
  FTAnd(XPath2MemoryManager *memMgr);
  FTAnd(FTSelection *left, FTSelection *right, XPath2MemoryManager *memMgr);

  virtual FTSelection *staticResolution(StaticContext *context);
  virtual FTSelection *staticTypingImpl(StaticContext *context);
  virtual FTSelection *optimize(FTContext *context) const;
  virtual AllMatches *execute(FTContext *ftcontext) const;

  const VectorOfFTSelections &getArguments() const { return args_; }
  void addArg(FTSelection *sel) { args_.push_back(sel); }

private:
  VectorOfFTSelections args_;
};

class BufferedMatches : public AllMatches
{
public:
  BufferedMatches(const LocationInfo *info, AllMatches *matches);

  bool next(DynamicContext *context);
  void reset();

  AllMatches *getAllMatches();

  const StringMatches &getStringIncludes() { return *includeIt_; }
  const StringMatches &getStringExcludes() { return *excludeIt_; }

private:
  AllMatches *matches_;
  bool reset_;
  std::vector<StringMatches> includeBuffer_;
  std::vector<StringMatches> excludeBuffer_;
  std::vector<StringMatches>::iterator includeIt_;
  std::vector<StringMatches>::iterator excludeIt_;
};

class FTConjunctionMatches : public AllMatches
{
public:
  FTConjunctionMatches(const LocationInfo *info);
  ~FTConjunctionMatches();

  void addMatches(AllMatches *m);

  bool next(DynamicContext *context);
  const StringMatches &getStringIncludes();
  const StringMatches &getStringExcludes();

private:
  void deleteMatches();

private:
  void addStringIncludes(const StringMatches &sMatches);
  void addStringExcludes(const StringMatches &sMatches);
private:
  bool toDo_;
  std::vector<BufferedMatches> args_;
  StringMatches includes_;
  StringMatches excludes_;
};

#endif

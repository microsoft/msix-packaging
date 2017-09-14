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

#ifndef _FTUNARYNOT_HPP
#define _FTUNARYNOT_HPP

#include <xqilla/fulltext/FTSelection.hpp>

class XQILLA_API FTUnaryNot : public FTSelection
{
public:
  FTUnaryNot(FTSelection *arg, XPath2MemoryManager *memMgr);

  virtual FTSelection *staticResolution(StaticContext *context);
  virtual FTSelection *staticTypingImpl(StaticContext *context);
  virtual FTSelection *optimize(FTContext *context) const;
  virtual AllMatches *execute(FTContext *ftcontext) const;

  FTSelection *getArgument() const { return arg_; }
  void setArgument(FTSelection *expr) { arg_ = expr; }

private:
  FTSelection *arg_;
};

class FTUnaryNotMatches : public AllMatches
{
public:
  FTUnaryNotMatches(const LocationInfo *info, AllMatches *arg)
    : AllMatches(info), toDo_(true), arg_(arg), includes_(), excludes_() {}
  ~FTUnaryNotMatches();
  bool next(DynamicContext *context);
  const StringMatches &getStringIncludes();
  const StringMatches &getStringExcludes();

private:
  void addStringIncludes(const StringMatches &sMatches);
  void addStringExcludes(const StringMatches &sMatches);

private:
  bool toDo_;
  AllMatches *arg_;
  StringMatches includes_;
  StringMatches excludes_;
};

#endif

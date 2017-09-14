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

#ifndef _FTORDER_HPP
#define _FTORDER_HPP

#include <xqilla/fulltext/FTOption.hpp>

class XQILLA_API FTOrder : public FTOption
{
public:
  FTOrder(XPath2MemoryManager *memMgr)
    : FTOption(FTSelection::ORDER, memMgr), arg_(0) {}
  FTOrder(FTSelection *arg, XPath2MemoryManager *memMgr)
    : FTOption(FTSelection::ORDER, memMgr), arg_(arg) {}

  virtual FTSelection *staticResolution(StaticContext *context);
  virtual FTSelection *staticTypingImpl(StaticContext *context);
  virtual FTSelection *optimize(FTContext *context) const;
  virtual AllMatches *execute(FTContext *ftcontext) const;

  virtual void setArgument(FTSelection *arg) { arg_ = arg; }
  FTSelection *getArgument() const { return arg_; }

private:
  FTSelection *arg_;
};

class FTOrderMatches : public AllMatches
{
public:
  FTOrderMatches(const LocationInfo *info, AllMatches *arg)
    : AllMatches(info), arg_(arg), excludes_() {}
  ~FTOrderMatches();
  bool next(DynamicContext *context);
  const StringMatches &getStringIncludes();
  const StringMatches &getStringExcludes();

private:
  AllMatches *arg_;
  StringMatches excludes_;
};

#endif

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

#ifndef _FTCONTENT_HPP
#define _FTCONTENT_HPP

#include <xqilla/fulltext/FTOption.hpp>

class XQILLA_API FTContent : public FTOption
{
public:
  enum Type {
    AT_START,
    AT_END,
    ENTIRE_CONTENT
  };

  FTContent(Type type, XPath2MemoryManager *memMgr)
    : FTOption(FTSelection::CONTENT, memMgr), type_(type), arg_(0) {}
  FTContent(FTSelection *arg, Type type, XPath2MemoryManager *memMgr)
    : FTOption(FTSelection::CONTENT, memMgr), type_(type), arg_(arg) {}

  virtual FTSelection *staticResolution(StaticContext *context);
  virtual FTSelection *staticTypingImpl(StaticContext *context);
  virtual FTSelection *optimize(FTContext *context) const;
  virtual AllMatches *execute(FTContext *ftcontext) const;

  virtual void setArgument(FTSelection *arg) { arg_ = arg; }
  FTSelection *getArgument() const { return arg_; }
  Type getType() const { return type_; }

private:
  Type type_;
  FTSelection *arg_;
};

class FTContentAtStartMatches : public AllMatches
{
public:
  FTContentAtStartMatches(const LocationInfo *info, AllMatches *arg)
    : AllMatches(info), arg_(arg), includes_() {}
  ~FTContentAtStartMatches();
  bool next(DynamicContext *context);
  const StringMatches &getStringIncludes();
  const StringMatches &getStringExcludes();

private:
  AllMatches *arg_;
  StringMatches includes_;
};

class FTContentAtEndMatches : public AllMatches
{
public:
  FTContentAtEndMatches(const LocationInfo *info, AllMatches *arg)
    : AllMatches(info), arg_(arg), includes_() {}
  ~FTContentAtEndMatches();
  bool next(DynamicContext *context);
  const StringMatches &getStringIncludes();
  const StringMatches &getStringExcludes();

private:
  AllMatches *arg_;
  StringMatches includes_;
};

#endif

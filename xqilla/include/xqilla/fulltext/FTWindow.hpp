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

#ifndef _FTWINDOW_HPP
#define _FTWINDOW_HPP

#include <xqilla/fulltext/FTOption.hpp>

#include <set>

class ASTNode;

class XQILLA_API FTWindow : public FTOption
{
public:
  FTWindow(ASTNode *expr, FTUnit unit, XPath2MemoryManager *memMgr)
    : FTOption(FTSelection::WINDOW, memMgr), expr_(expr), unit_(unit), arg_(0) {}
  FTWindow(FTSelection *arg, ASTNode *expr, FTUnit unit, XPath2MemoryManager *memMgr)
    : FTOption(FTSelection::WINDOW, memMgr), expr_(expr), unit_(unit), arg_(arg) {}

  virtual FTSelection *staticResolution(StaticContext *context);
  virtual FTSelection *staticTypingImpl(StaticContext *context);
  virtual FTSelection *optimize(FTContext *context) const;
  virtual AllMatches *execute(FTContext *ftcontext) const;

  virtual void setArgument(FTSelection *arg) { arg_ = arg; }
  FTSelection *getArgument() const { return arg_; }
  ASTNode *getExpr() const { return expr_; }
  void setExpr(ASTNode *expr) { expr_ = expr; }
  FTUnit getUnit() const { return unit_; }

private:
  ASTNode *expr_;
  FTUnit unit_;
  FTSelection *arg_;
};

class XQILLA_API FTWindowLiteral : public FTOption
{
public:
  FTWindowLiteral(FTSelection *arg, unsigned int distance, FTUnit unit,
                  XPath2MemoryManager *memMgr)
    : FTOption(WINDOW_LITERAL, memMgr), arg_(arg), distance_(distance),
      unit_(unit) {}

  virtual FTSelection *staticResolution(StaticContext *context);
  virtual FTSelection *staticTypingImpl(StaticContext *context);
  virtual FTSelection *optimize(FTContext *context) const;
  virtual AllMatches *execute(FTContext *ftcontext) const;

  virtual void setArgument(FTSelection *arg) { arg_ = arg; }
  FTSelection *getArgument() const { return arg_; }
  unsigned int getDistance() const { return distance_; }
  FTUnit getUnit() const { return unit_; }

private:
  FTSelection *arg_;
  unsigned int distance_;
  FTUnit unit_;
};

class FTWindowMatches : public AllMatches
{
public:
  FTWindowMatches(const LocationInfo *info, unsigned int distance, FTOption::FTUnit unit, AllMatches *arg)
    : AllMatches(info), distance_(distance), unit_(unit), arg_(arg), found_(true), excludes_() {}
  ~FTWindowMatches();
  bool next(DynamicContext *context);
  const StringMatches &getStringIncludes();
  const StringMatches &getStringExcludes();

private:
  unsigned int distance_;
  FTOption::FTUnit unit_;
  AllMatches *arg_;
  bool found_;
  std::set<unsigned int> excludeValues_;
  std::set<unsigned int>::iterator excludeIt_;
  StringMatches excludes_;
};

#endif

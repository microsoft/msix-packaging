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

#include "../config/xqilla_config.h"
#include <xqilla/fulltext/FTContent.hpp>
#include <xqilla/fulltext/FTDistance.hpp>
#include <xqilla/context/DynamicContext.hpp>

FTSelection *FTContent::staticResolution(StaticContext *context)
{
  arg_ = arg_->staticResolution(context);
  return this;
}

FTSelection *FTContent::staticTypingImpl(StaticContext *context)
{
  src_.clear();
  src_.add(arg_->getStaticAnalysis());
  return this;
}

FTSelection *FTContent::optimize(FTContext *ftcontext) const
{
  XPath2MemoryManager *mm = ftcontext->context->getMemoryManager();

  FTSelection *newarg = arg_->optimize(ftcontext);
  if(newarg == 0) return 0;

  if(type_ == ENTIRE_CONTENT) {
    newarg = new (mm) FTDistanceLiteral(newarg, FTRange::EXACTLY, 0, 0, FTOption::WORDS, mm);
    newarg->setLocationInfo(this);
    newarg = new (mm) FTContent(newarg, AT_START, mm);
    newarg->setLocationInfo(this);
    newarg = new (mm) FTContent(newarg, AT_END, mm);
    newarg->setLocationInfo(this);
  }
  else {
	  newarg = new (mm) FTContent(newarg, type_, mm);
	  newarg->setLocationInfo(this);
  }

  return newarg;
}

AllMatches *FTContent::execute(FTContext *ftcontext) const
{
  switch(type_) {
  case AT_START:
    return new FTContentAtStartMatches(this, arg_->execute(ftcontext));
  case AT_END:
    return new FTContentAtEndMatches(this, arg_->execute(ftcontext));
  default:
    // ENTIRE_CONTENT is handled by optimize()
    assert(0);
    break;
  }
  return 0;
}
FTContentAtStartMatches::~FTContentAtStartMatches()
{
  delete arg_;
}

bool FTContentAtStartMatches::next(DynamicContext *context)
{
  if(!arg_) return false;

  StringMatches::const_iterator smallest;
  bool found = false;
  while(arg_->next(context)) {
    StringMatches::const_iterator i = arg_->getStringIncludes().begin();
    StringMatches::const_iterator end = arg_->getStringIncludes().end();
    if(i != end) {
      smallest = i;
      found = true;
      for(++i; i != end; ++i) {
        if(i->tokenInfo.position_ < smallest->tokenInfo.position_) {
          smallest = i;
        }
      }
      break;
    }
  }
  if (!found) {
    delete arg_;
    arg_ = 0;
    return false;
  }

  includes_.clear();
  StringMatches::const_iterator i = arg_->getStringIncludes().begin();
  StringMatches::const_iterator end = arg_->getStringIncludes().end();
  for(; i != end; ++i) {
    if(i == smallest) {
      StringMatch sm(*i);
      sm.startToken = true;
      includes_.push_back(sm);
    }
    else {
      includes_.push_back(*i);
    }
  }
  return true;
}

const StringMatches &FTContentAtStartMatches::getStringIncludes()
{
  assert(arg_);
  return includes_;
}
const StringMatches &FTContentAtStartMatches::getStringExcludes()
{
  assert(arg_);
  return arg_->getStringExcludes();
}

FTContentAtEndMatches::~FTContentAtEndMatches()
{
  delete arg_;
}

bool FTContentAtEndMatches::next(DynamicContext *context)
{
  if(!arg_) return false;

  StringMatches::const_iterator largest;
  bool found = false;
  while(arg_->next(context)) {
    StringMatches::const_iterator i = arg_->getStringIncludes().begin();
    StringMatches::const_iterator end = arg_->getStringIncludes().end();
    if(i != end) {
      largest = i;
      found = true;
      for(++i; i != end; ++i) {
        if(i->tokenInfo.position_ > largest->tokenInfo.position_) {
          largest = i;
        }
      }
      break;
    }
  }
  if (!found) {
    delete arg_;
    arg_ = 0;
    return false;
  }

  includes_.clear();
  StringMatches::const_iterator i = arg_->getStringIncludes().begin();
  StringMatches::const_iterator end = arg_->getStringIncludes().end();
  for(; i != end; ++i) {
    if(i == largest) {
      StringMatch sm(*i);
      sm.endToken = true;
      includes_.push_back(sm);
    }
    else {
      includes_.push_back(*i);
    }
  }
  return true;
}

const StringMatches &FTContentAtEndMatches::getStringIncludes()
{
  assert(arg_);
  return includes_;
}

const StringMatches &FTContentAtEndMatches::getStringExcludes()
{
  assert(arg_);
  return arg_->getStringExcludes();
}

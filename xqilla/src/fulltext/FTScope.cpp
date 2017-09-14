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
#include <xqilla/fulltext/FTScope.hpp>
#include <xqilla/context/DynamicContext.hpp>

using namespace std;

FTSelection *FTScope::staticResolution(StaticContext *context)
{
  arg_ = arg_->staticResolution(context);
  return this;
}

FTSelection *FTScope::staticTypingImpl(StaticContext *context)
{
  src_.clear();
  src_.add(arg_->getStaticAnalysis());
  return this;
}

FTSelection *FTScope::optimize(FTContext *ftcontext) const
{
  XPath2MemoryManager *mm = ftcontext->context->getMemoryManager();

  FTSelection *newarg = arg_->optimize(ftcontext);
  if(newarg == 0) return 0;

  if(newarg->getType() == WORD) {
    return newarg;
  }

  newarg = new (mm) FTScope(newarg, type_, unit_, mm);
  newarg->setLocationInfo(this);
  return newarg;
}

AllMatches *FTScope::execute(FTContext *ftcontext) const
{
  switch(type_) {
  case SAME:
    return new FTScopeSameMatches(this, unit_, arg_->execute(ftcontext));
  case DIFFERENT:
    return new FTScopeDifferentMatches(this, unit_, arg_->execute(ftcontext));
  default:
    assert(0);
    break;
  }
  return 0;
}

FTScopeSameMatches::~FTScopeSameMatches()
{
  delete arg_;
}

bool FTScopeSameMatches::next(DynamicContext *context)
{
  excludes_.clear();
  if(!arg_) return false;

  unitValue_ = 0;
  bool found = arg_->next(context);
  if (!found) {
    delete arg_;
    arg_ = 0;
    return false;
  }
  while(found) {
    StringMatches::const_iterator i = arg_->getStringIncludes().begin();
    StringMatches::const_iterator end = arg_->getStringIncludes().end();
    if(i != end && (i+1) != end) {
      unitValue_ = FTOption::tokenUnit(i->tokenInfo, unit_);
      for(++i; i != end; ++i) {
        found = false;
        if(FTOption::tokenUnit(i->tokenInfo, unit_) != unitValue_) {
          found = arg_->next(context);
          if (!found) {
            delete arg_;
            arg_ = 0;
            return false;
          }
          break;
        }
      }
    }
    else {
      found = arg_->next(context);
      if (!found) {
        delete arg_;
        arg_ = 0;
        return false;
      }
    }
  }

  return true;
}

const StringMatches &FTScopeSameMatches::getStringIncludes()
{
  assert(arg_);
  return arg_->getStringIncludes();
}

const StringMatches &FTScopeSameMatches::getStringExcludes()
{
  if (arg_ && !excludes_.empty()) {
    StringMatches::const_iterator end = arg_->getStringExcludes().end();
    for(StringMatches::const_iterator i = arg_->getStringExcludes().begin();
        i != end; ++i) {
      if(FTOption::tokenUnit(i->tokenInfo, unit_) == unitValue_) {
        excludes_.push_back(*i);
      }
    }
  }
  return excludes_;
}

FTScopeDifferentMatches::~FTScopeDifferentMatches()
{
  delete arg_;
}

bool FTScopeDifferentMatches::next(DynamicContext *context)
{
  excludes_.clear();
  if(!arg_) return false;

  bool found = arg_->next(context);
  if (!found) {
    delete arg_;
    arg_ = 0;
    return false;
  }
  while(found) {
    found = false;
    unitValuesSeen_.clear();
    StringMatches::const_iterator end = arg_->getStringIncludes().end();
    StringMatches::const_iterator i = arg_->getStringIncludes().begin();
    for(; i != end; ++i) {
      if(!unitValuesSeen_.insert(FTOption::tokenUnit(i->tokenInfo, unit_)).second) {
        found = arg_->next(context);
        if (!found) {
          delete arg_;
          arg_ = 0;
          return false;
        }
        break;
      }
    }
  }

  return true;
}

const StringMatches &FTScopeDifferentMatches::getStringIncludes()
{
  assert(arg_);
  return arg_->getStringIncludes();
}

const StringMatches &FTScopeDifferentMatches::getStringExcludes()
{
  if (arg_ && !excludes_.empty()) {
    StringMatches::const_iterator end = arg_->getStringExcludes().end();
    StringMatches::const_iterator i = arg_->getStringExcludes().begin();
    for(; i != end; ++i) {
      if(unitValuesSeen_.find(FTOption::tokenUnit(i->tokenInfo, unit_)) != unitValuesSeen_.end()) {
        excludes_.push_back(*i);
      }
    }
  }
  return excludes_;
}

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
#include <xqilla/fulltext/FTUnaryNot.hpp>
#include <xqilla/context/DynamicContext.hpp>

FTUnaryNot::FTUnaryNot(FTSelection *arg, XPath2MemoryManager *memMgr)
    : FTSelection(FTSelection::UNARY_NOT, memMgr),
      arg_(arg)
{
}

FTSelection *FTUnaryNot::staticResolution(StaticContext *context)
{
  arg_ = arg_->staticResolution(context);
  return this;
}

FTSelection *FTUnaryNot::staticTypingImpl(StaticContext *context)
{
  src_.clear();
  src_.add(arg_->getStaticAnalysis());
  return this;
}

FTSelection *FTUnaryNot::optimize(FTContext *ftcontext) const
{
  XPath2MemoryManager *mm = ftcontext->context->getMemoryManager();

  FTSelection *newarg = arg_->optimize(ftcontext);
  if(newarg == 0) return 0;

  newarg = new (mm) FTUnaryNot(newarg, mm);
  newarg->setLocationInfo(this);
  return newarg;
}

AllMatches *FTUnaryNot::execute(FTContext *ftcontext) const
{
  return new FTUnaryNotMatches(this, arg_->execute(ftcontext));
}

FTUnaryNotMatches::~FTUnaryNotMatches()
{
  delete arg_;
}

bool FTUnaryNotMatches::next(DynamicContext *context)
{
  // TBD incorrect implementation - jpcs
  // TBD need to check for StringInclude / StringExclude contradictions - jpcs
  if(toDo_) {
    toDo_ = false;

    if(arg_) {
      while(arg_->next(context)) {
        addStringExcludes(arg_->getStringIncludes());
        addStringIncludes(arg_->getStringExcludes());
      }
    }

    delete arg_;
    arg_ = 0;
    return true;
  }
  includes_.clear();
  excludes_.clear();
  return false;
}

const StringMatches &FTUnaryNotMatches::getStringIncludes()
{
  return includes_;
}
const StringMatches &FTUnaryNotMatches::getStringExcludes()
{
  return excludes_;
}

void FTUnaryNotMatches::addStringIncludes(const StringMatches &sMatches)
{
  for(StringMatches::const_iterator j = sMatches.begin(); j != sMatches.end(); ++j) {
    includes_.push_back(*j);
  }
}

void FTUnaryNotMatches::addStringExcludes(const StringMatches &sMatches)
{
  for(StringMatches::const_iterator j = sMatches.begin(); j != sMatches.end(); ++j) {
    excludes_.push_back(*j);
  }
}
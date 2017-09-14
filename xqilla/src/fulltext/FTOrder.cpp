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
#include <xqilla/fulltext/FTOrder.hpp>
#include <xqilla/context/DynamicContext.hpp>

FTSelection *FTOrder::staticResolution(StaticContext *context)
{
  arg_ = arg_->staticResolution(context);
  return this;
}

FTSelection *FTOrder::staticTypingImpl(StaticContext *context)
{
  src_.clear();
  src_.add(arg_->getStaticAnalysis());
  return this;
}

FTSelection *FTOrder::optimize(FTContext *ftcontext) const
{
  XPath2MemoryManager *mm = ftcontext->context->getMemoryManager();

  FTSelection *newarg = arg_->optimize(ftcontext);
  if(newarg == 0) return 0;

  if(newarg->getType() == WORD) {
    return newarg;
  }

  newarg = new (mm) FTOrder(newarg, mm);
  newarg->setLocationInfo(this);
  return newarg;
}

AllMatches *FTOrder::execute(FTContext *ftcontext) const
{
  return new FTOrderMatches(this, arg_->execute(ftcontext));
}

FTOrderMatches::~FTOrderMatches()
{
  delete arg_;
}

bool FTOrderMatches::next(DynamicContext *context)
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
    StringMatches::const_iterator begin = arg_->getStringIncludes().begin();
    StringMatches::const_iterator end = arg_->getStringIncludes().end();
    StringMatches::const_iterator i, j;
    for(i = begin; i != end; ++i) {
      for(j = i, ++j; j != end; ++j) {
        if((i->queryPos > j->queryPos && i->tokenInfo.position_ < j->tokenInfo.position_) ||
           (i->queryPos < j->queryPos && i->tokenInfo.position_ > j->tokenInfo.position_)) {
          found = arg_->next(context);
          if (!found) {
            delete arg_;
            arg_ = 0;
            return false;
          }
          break;
        }
      }
      if (found) break;
    }
  }

  return true;
}

const StringMatches &FTOrderMatches::getStringIncludes()
{
  assert(arg_);
  return arg_->getStringIncludes();
}

const StringMatches &FTOrderMatches::getStringExcludes()
{
  if (arg_ && !excludes_.empty()) {
    StringMatches::const_iterator e_end = arg_->getStringExcludes().end();
    StringMatches::const_iterator e = arg_->getStringExcludes().begin();
    StringMatches::const_iterator i_begin = arg_->getStringIncludes().begin();
    StringMatches::const_iterator i_end = arg_->getStringIncludes().end();
    StringMatches::const_iterator i;
    for(; e != e_end; ++e) {
      for(i = i_begin; i != i_end; ++i) {
        if((i->queryPos <= e->queryPos && i->tokenInfo.position_ <= e->tokenInfo.position_) ||
           (i->queryPos >= e->queryPos && i->tokenInfo.position_ >= e->tokenInfo.position_)) {
          excludes_.push_back(*e);
        }
      }
    }
  } 
  return excludes_;
}

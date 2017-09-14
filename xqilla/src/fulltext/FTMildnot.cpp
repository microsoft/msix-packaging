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
#include <xqilla/fulltext/FTMildnot.hpp>
#include <xqilla/context/DynamicContext.hpp>
#include <xqilla/exceptions/XPath2ErrorException.hpp>

FTMildnot::FTMildnot(FTSelection *left, FTSelection *right, XPath2MemoryManager *memMgr)
    : FTSelection(FTSelection::MILD_NOT, memMgr),
      left_(left),
      right_(right)
{
}

FTSelection *FTMildnot::staticResolution(StaticContext *context)
{
  left_ = left_->staticResolution(context);
  right_ = right_->staticResolution(context);
  return this;
}

FTSelection *FTMildnot::staticTypingImpl(StaticContext *context)
{
  src_.clear();
  src_.add(left_->getStaticAnalysis());
  src_.add(right_->getStaticAnalysis());
  return this;
}

FTSelection *FTMildnot::optimize(FTContext *ftcontext) const
{
  XPath2MemoryManager *mm = ftcontext->context->getMemoryManager();

  FTSelection *newleft = left_->optimize(ftcontext);
  if(newleft == 0) return 0;

  FTSelection *newright = right_->optimize(ftcontext);
  if(newright == 0) return newleft;

  FTSelection *result = new (mm) FTMildnot(newleft, newright, mm);
  result->setLocationInfo(this);
  return result;
}

AllMatches *FTMildnot::execute(FTContext *ftcontext) const
{
  AllMatches *leftMatches = left_->execute(ftcontext);
  AllMatches *rightMatches = right_->execute(ftcontext);
  return new FTMildnotMatches(this, leftMatches, rightMatches);
}

FTMildnotMatches::~FTMildnotMatches()
{
  delete left_;
  delete right_;
}

bool FTMildnotMatches::next(DynamicContext *context)
{
  if(!left_) return false;

  if(right_) {
    while(right_->next(context)) {
      if(!right_->getStringExcludes().empty())
        XQThrow(XPath2ErrorException, X("FTMildnotMatches::next"),
                X("Invalid expression on the right-hand side of a not-in [err:FTDY0017]"));

      StringMatches::const_iterator end = right_->getStringIncludes().end();
      for(StringMatches::const_iterator i = right_->getStringIncludes().begin();
          i != end; ++i) {
        badTokens_.insert(i->tokenInfo.position_);
      }
    }
    delete right_;
    right_ = 0;
  }

  while(left_->next(context)) {
    if(!left_->getStringExcludes().empty())
      XQThrow(XPath2ErrorException, X("FTMildnotMatches::next"),
              X("Invalid expression on the left-hand side of a not-in [err:FTDY0017]"));

    bool found = false;
    StringMatches::const_iterator end = left_->getStringIncludes().end();
    for(StringMatches::const_iterator i = left_->getStringIncludes().begin();
        i != end; ++i) {
      if(badTokens_.find(i->tokenInfo.position_) != badTokens_.end()) {
        found = true;
        break;
      }
    }
    if (!found) return true;
  }

  delete left_;
  left_ = 0;
  return false;
}

const StringMatches &FTMildnotMatches::getStringIncludes()
{
  assert(left_);
  return left_->getStringIncludes();
}

const StringMatches &FTMildnotMatches::getStringExcludes()
{
  assert(left_);
  return left_->getStringExcludes();
}
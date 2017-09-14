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
#include <xqilla/fulltext/FTOr.hpp>
#include <xqilla/context/DynamicContext.hpp>

using namespace std;

FTOr::FTOr(XPath2MemoryManager *memMgr)
  : FTSelection(FTSelection::OR, memMgr),
    args_(memMgr)
{
}

FTOr::FTOr(FTSelection *left, FTSelection *right, XPath2MemoryManager *memMgr)
    : FTSelection(FTSelection::OR, memMgr),
      args_(memMgr)
{
  args_.push_back(left);
  args_.push_back(right);
}

FTSelection *FTOr::staticResolution(StaticContext *context)
{
  for(VectorOfFTSelections::iterator i = args_.begin();
      i != args_.end(); ++i) {
    *i = (*i)->staticResolution(context);
  }

  return this;
}

FTSelection *FTOr::staticTypingImpl(StaticContext *context)
{
  src_.clear();

  for(VectorOfFTSelections::iterator i = args_.begin();
      i != args_.end(); ++i) {
    src_.add((*i)->getStaticAnalysis());
  }

  return this;
}

FTSelection *FTOr::optimize(FTContext *ftcontext) const
{
  XPath2MemoryManager *mm = ftcontext->context->getMemoryManager();

  FTOr *ftor = new (mm) FTOr(mm);
  ftor->setLocationInfo(this);

  for(VectorOfFTSelections::const_iterator i = args_.begin();
      i != args_.end(); ++i) {
    FTSelection *arg = (*i)->optimize(ftcontext);
    if(arg != 0)
      ftor->addArg(arg);
  }

  if(ftor->args_.empty()) {
    return 0;
  }
  if(ftor->args_.size() == 1) {
    return ftor->args_.back();
  }

  return ftor;
}

AllMatches *FTOr::execute(FTContext *ftcontext) const
{
  FTDisjunctionMatches *disjunction = new FTDisjunctionMatches(this);
  AllMatches *result(disjunction);

  for(VectorOfFTSelections::const_iterator i = args_.begin();
      i != args_.end(); ++i) {
    disjunction->addMatches((*i)->execute(ftcontext));
  }

  return result;
}

FTDisjunctionMatches::FTDisjunctionMatches(const LocationInfo *info)
  : AllMatches(info),
    toDo_(true)
{
}

FTDisjunctionMatches::~FTDisjunctionMatches()
{
  for(it_ = args_.begin(); it_ != args_.end(); it_++) {
    delete *(it_);
  }
}

bool FTDisjunctionMatches::next(DynamicContext *context)
{
  // TBD AllMatches normalization
  if(toDo_) {
    toDo_ = false;
    it_ = args_.begin();
  }

  bool found = false;
  while(it_ != args_.end()) {
    if(!(*it_)->next(context)) {
      delete *(it_);
      *it_ = 0;
      ++it_;
    }
    else {
      found = true;
      break;
    }
  }

  return found;
}

const StringMatches &FTDisjunctionMatches::getStringIncludes()
{
  assert(it_ != args_.end());
  return (*it_)->getStringIncludes();
}
const StringMatches &FTDisjunctionMatches::getStringExcludes()
{
  assert(it_ != args_.end());
  return (*it_)->getStringExcludes();
}

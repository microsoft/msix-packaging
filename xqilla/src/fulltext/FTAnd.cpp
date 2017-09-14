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
#include <xqilla/fulltext/FTAnd.hpp>
#include <xqilla/context/DynamicContext.hpp>

using namespace std;

FTAnd::FTAnd(XPath2MemoryManager *memMgr)
  : FTSelection(AND, memMgr),
    args_(memMgr)
{
}

FTAnd::FTAnd(FTSelection *left, FTSelection *right, XPath2MemoryManager *memMgr)
    : FTSelection(AND, memMgr),
      args_(memMgr)
{
  args_.push_back(left);
  args_.push_back(right);
}

FTSelection *FTAnd::staticResolution(StaticContext *context)
{
  for(VectorOfFTSelections::iterator i = args_.begin();
      i != args_.end(); ++i) {
    *i = (*i)->staticResolution(context);
  }

  return this;
}

FTSelection *FTAnd::staticTypingImpl(StaticContext *context)
{
  src_.clear();

  for(VectorOfFTSelections::iterator i = args_.begin();
      i != args_.end(); ++i) {
    src_.add((*i)->getStaticAnalysis());
  }

  return this;
}

FTSelection *FTAnd::optimize(FTContext *ftcontext) const
{
  XPath2MemoryManager *mm = ftcontext->context->getMemoryManager();

  FTAnd *ftand = new (mm) FTAnd(mm);
  ftand->setLocationInfo(this);

  for(VectorOfFTSelections::const_iterator i = args_.begin();
      i != args_.end(); ++i) {
    FTSelection *arg = (*i)->optimize(ftcontext);
    if(arg == 0) return 0;
    ftand->addArg(arg);
  }

  if(ftand->args_.empty()) {
    return 0;
  }
  if(ftand->args_.size() == 1) {
    return ftand->args_.back();
  }

  return ftand;
}

AllMatches *FTAnd::execute(FTContext *ftcontext) const
{
  FTConjunctionMatches *conjunction = new FTConjunctionMatches(this);
  AllMatches *result(conjunction);

  for(VectorOfFTSelections::const_iterator i = args_.begin();
      i != args_.end(); ++i) {
    conjunction->addMatches((*i)->execute(ftcontext));
  }

  return result;
}

BufferedMatches::BufferedMatches(const LocationInfo *info, AllMatches *matches)
  : AllMatches(info),
    matches_(matches),
    reset_(false),
    includeBuffer_(),
    excludeBuffer_(),
    includeIt_(includeBuffer_.begin()),
    excludeIt_(excludeBuffer_.begin())
{
}

bool BufferedMatches::next(DynamicContext *context)
{
  if(includeIt_ != includeBuffer_.end()) {
    if (reset_) {
      reset_ = false;
    } else {
      includeIt_++;
      excludeIt_++;
    }
    if(includeIt_ != includeBuffer_.end()) 
      return true;
  }

  if(matches_) {
    if(matches_->next(context)) {
      includeBuffer_.push_back(matches_->getStringIncludes());
      excludeBuffer_.push_back(matches_->getStringExcludes());
      includeIt_ = includeBuffer_.end();
      excludeIt_ = excludeBuffer_.end();
      includeIt_--;
      excludeIt_--; 
      return true;
    }
  }

  return false;
}

void BufferedMatches::reset()
{
  reset_ = true;
  includeIt_ = includeBuffer_.begin();
  excludeIt_ = excludeBuffer_.begin();
}

AllMatches *BufferedMatches::getAllMatches()
{
  return matches_;
}

FTConjunctionMatches::FTConjunctionMatches(const LocationInfo *info)
  : AllMatches(info),
    toDo_(true),
    includes_(),
    excludes_()
{
}

FTConjunctionMatches::~FTConjunctionMatches()
{
  deleteMatches();
}

void FTConjunctionMatches::addMatches(AllMatches *m)
{
  if (m) {
    BufferedMatches buf(this, m);
    args_.push_back(buf);
  }
}

bool FTConjunctionMatches::next(DynamicContext *context)
{
  // TBD need to check for StringInclude / StringExclude contradictions

  includes_.clear();
  excludes_.clear();
  vector<BufferedMatches>::reverse_iterator rend = args_.rend();
  vector<BufferedMatches>::reverse_iterator rbegin = args_.rbegin();
  vector<BufferedMatches>::reverse_iterator it;

  bool initialisationState = false;
  if(toDo_) {
    toDo_ = false;
    initialisationState = true;
  }

  if(initialisationState) {
    it = rend;
  }
  else {
    it = rbegin;
  }

  while(true) {
    if(initialisationState) {
      if(it == rbegin) {
        break;
      }
      else {
        --it;
        it->reset();
        if(!it->next(context)) {
          initialisationState = false;
          ++it;
        }
      }
    }
    else {
      if(it == rend) {
        deleteMatches();
        return false;
      }
      else {
        if(it->next(context)) {
          initialisationState = true;
        }
        else {
          ++it;
        }
      }
    }
  }

  return true;
}

const StringMatches &FTConjunctionMatches::getStringIncludes()
{
  if (includes_.empty()) {
    for(vector<BufferedMatches>::iterator i = args_.begin();
        i != args_.end(); ++i) {
      addStringIncludes(i->getStringIncludes());
    }
  }
  return includes_;
}

const StringMatches &FTConjunctionMatches::getStringExcludes()
{
  if (excludes_.empty()) {
    for(vector<BufferedMatches>::iterator i = args_.begin();
        i != args_.end(); ++i) {
      addStringExcludes(i->getStringExcludes());
    }
  }
  return excludes_;
}

void FTConjunctionMatches::deleteMatches()
{
  for (vector<BufferedMatches>::iterator it = args_.begin(); 
    it != args_.end(); it++) {
      delete it->getAllMatches();
  }
  args_.clear();
}

void FTConjunctionMatches::addStringIncludes(const StringMatches &sMatches)
{
  for(StringMatches::const_iterator j = sMatches.begin(); j != sMatches.end(); ++j) {
    includes_.push_back(*j);
  }
}

void FTConjunctionMatches::addStringExcludes(const StringMatches &sMatches)
{
  for(StringMatches::const_iterator j = sMatches.begin(); j != sMatches.end(); ++j) {
    excludes_.push_back(*j);
  }
}

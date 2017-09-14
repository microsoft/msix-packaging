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
#include <xqilla/fulltext/FTDistance.hpp>
#include <xqilla/context/DynamicContext.hpp>
#include <xqilla/context/ItemFactory.hpp>
#include <xqilla/schema/SequenceType.hpp>
#include <xqilla/utils/UTF8Str.hpp>
#include <xqilla/ast/XQAtomize.hpp>
#include <xqilla/ast/XQTreatAs.hpp>

#include <xercesc/validators/schema/SchemaSymbols.hpp>

#include <algorithm>
#include <stdlib.h>

#if defined(XERCES_HAS_CPP_NAMESPACE)
XERCES_CPP_NAMESPACE_USE
#endif

FTSelection *FTDistance::staticResolution(StaticContext *context)
{
  XPath2MemoryManager *mm = context->getMemoryManager();

  SequenceType *seqType = new (mm) SequenceType(SchemaSymbols::fgURI_SCHEMAFORSCHEMA,
                                                SchemaSymbols::fgDT_INTEGER,
                                                SequenceType::EXACTLY_ONE, mm);
  seqType->setLocationInfo(this);

  arg_ = arg_->staticResolution(context);

  range_.arg1 = new (mm) XQAtomize(range_.arg1, mm);
  range_.arg1->setLocationInfo(this);
  range_.arg1 = new (mm) XQTreatAs(range_.arg1, seqType, mm);
  range_.arg1->setLocationInfo(this);
  range_.arg1 = range_.arg1->staticResolution(context);

  if(range_.arg2 != NULL) {
    range_.arg2 = new (mm) XQAtomize(range_.arg2, mm);
    range_.arg2->setLocationInfo(this);
    range_.arg2 = new (mm) XQTreatAs(range_.arg2, seqType, mm);
    range_.arg2->setLocationInfo(this);
    range_.arg2 = range_.arg2->staticResolution(context);
  }

  return this;
}

FTSelection *FTDistance::staticTypingImpl(StaticContext *context)
{
  src_.clear();

  src_.add(arg_->getStaticAnalysis());
  src_.add(range_.arg1->getStaticAnalysis());

  if(range_.arg2 != NULL) {
    src_.add(range_.arg2->getStaticAnalysis());
  }

  return this;
}

FTSelection *FTDistance::optimize(FTContext *ftcontext) const
{
  XPath2MemoryManager *mm = ftcontext->context->getMemoryManager();

  if(range_.arg1->isConstant()) {
    switch(range_.type) {
    case FTRange::EXACTLY:
    case FTRange::AT_LEAST:
    case FTRange::AT_MOST: {
      Numeric::Ptr num = (Numeric*)range_.arg1->createResult(ftcontext->context)->next(ftcontext->context).get();

      FTSelection *result = new (mm) FTDistanceLiteral(arg_, range_.type, num->asInt(), 0, unit_, mm);
      result->setLocationInfo(this);
      return result->optimize(ftcontext);
    }
    case FTRange::FROM_TO: {
      if(range_.arg2->isConstant()) {
        Numeric::Ptr num = (Numeric*)range_.arg1->createResult(ftcontext->context)->next(ftcontext->context).get();
        Numeric::Ptr num2 = (Numeric*)range_.arg2->createResult(ftcontext->context)->next(ftcontext->context).get();

        FTSelection *result = new (mm) FTDistanceLiteral(arg_, FTRange::FROM_TO, num->asInt(), num2->asInt(), unit_, mm);
        result->setLocationInfo(this);
        return result->optimize(ftcontext);
      }
    }
    }
  }

  FTSelection *newarg = arg_->optimize(ftcontext);
  if(newarg == 0) return 0;

  if(newarg->getType() == WORD) {
    return newarg;
  }

  newarg = new (mm) FTDistance(range_, unit_, newarg, mm);
  newarg->setLocationInfo(this);
  return newarg;
}

AllMatches *FTDistance::execute(FTContext *ftcontext) const
{
  Numeric::Ptr num = (Numeric*)range_.arg1->createResult(ftcontext->context)->next(ftcontext->context).get();

  switch(range_.type) {
  case FTRange::EXACTLY: {
    return new FTDistanceExactlyMatches(this, num->asInt(), unit_, arg_->execute(ftcontext));
  }
  case FTRange::AT_LEAST: {
    return new FTDistanceAtLeastMatches(this, num->asInt(), unit_, arg_->execute(ftcontext));
  }
  case FTRange::AT_MOST: {
    return new FTDistanceAtMostMatches(this, num->asInt(), unit_, arg_->execute(ftcontext));
  }
  case FTRange::FROM_TO: {
    Numeric::Ptr num2 = (Numeric*)range_.arg2->createResult(ftcontext->context)->next(ftcontext->context).get();
    return new FTDistanceFromToMatches(this, num->asInt(), num2->asInt(), unit_, arg_->execute(ftcontext));
  }
  }
  return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

FTSelection *FTDistanceLiteral::staticResolution(StaticContext *context)
{
  arg_ = arg_->staticResolution(context);
  return this;
}

FTSelection *FTDistanceLiteral::staticTypingImpl(StaticContext *context)
{
  src_.clear();
  src_.add(arg_->getStaticAnalysis());
  return this;
}

FTSelection *FTDistanceLiteral::optimize(FTContext *ftcontext) const
{
  XPath2MemoryManager *mm = ftcontext->context->getMemoryManager();

  FTSelection *newarg = arg_->optimize(ftcontext);
  if(newarg == 0) return 0;

  if(newarg->getType() == WORD) {
    return newarg;
  }
  
  newarg = new (mm) FTDistanceLiteral(newarg, type_, distance_, distance2_, unit_, mm);
  newarg->setLocationInfo(this);
  return newarg;
}

AllMatches *FTDistanceLiteral::execute(FTContext *ftcontext) const
{
    switch(type_) {
    case FTRange::EXACTLY: {
      return new FTDistanceExactlyMatches(this, distance_, unit_, arg_->execute(ftcontext));
    }
    case FTRange::AT_LEAST: {
      return new FTDistanceAtLeastMatches(this, distance_, unit_, arg_->execute(ftcontext));
    }
    case FTRange::AT_MOST: {
      return new FTDistanceAtMostMatches(this, distance_, unit_, arg_->execute(ftcontext));
    }
    case FTRange::FROM_TO: {
      return new FTDistanceFromToMatches(this, distance_, distance2_, unit_, arg_->execute(ftcontext));
    }
    }
    return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

static bool lessThanCompareFn(const StringMatch &first, const StringMatch &second)
{
	return first.tokenInfo.position_ < second.tokenInfo.position_;
}

FTDistanceMatches::~FTDistanceMatches()
{
  delete arg_;
}

bool FTDistanceMatches::next(DynamicContext *context)
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
    if(arg_->getStringIncludes().size() > 1) {
      StringMatches sMatches = arg_->getStringIncludes();
      std::sort(sMatches.begin(), sMatches.end(), lessThanCompareFn);

      StringMatches::iterator end = sMatches.end();
      StringMatches::iterator a = sMatches.begin();
      StringMatches::iterator b = a; ++b;
      for(; b != end; ++a, ++b) {
        unsigned int actual = FTOption::tokenDistance(a->tokenInfo, b->tokenInfo, unit_);
        if(!distanceMatches(actual)) {
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
  }

  return true;
}

const StringMatches &FTDistanceMatches::getStringIncludes()
{
  assert(arg_);
  return arg_->getStringIncludes();
}

const StringMatches &FTDistanceMatches::getStringExcludes()
{
  if (arg_ && !excludes_.empty()) {
    for(StringMatches::const_iterator i = arg_->getStringExcludes().begin();
        i != arg_->getStringExcludes().end(); ++i) {
      for(StringMatches::const_iterator j = arg_->getStringIncludes().begin();
          j != arg_->getStringIncludes().end(); ++j) {
        unsigned int actual = FTOption::tokenDistance(i->tokenInfo, j->tokenInfo, unit_);
        if(distanceMatches(actual)) {
          excludes_.push_back(*i);
          break;
        }
      }
    }
  }
  return excludes_;
}

bool FTDistanceExactlyMatches::distanceMatches(unsigned int actual) const
{
  return actual == distance_;
}

bool FTDistanceAtLeastMatches::distanceMatches(unsigned int actual) const
{
  return actual >= distance_;
}

bool FTDistanceAtMostMatches::distanceMatches(unsigned int actual) const
{
  return actual <= distance_;
}

bool FTDistanceFromToMatches::distanceMatches(unsigned int actual) const
{
  return distance_ <= actual && actual <= distance2_;
}

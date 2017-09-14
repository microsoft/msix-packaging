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
#include <xqilla/fulltext/FTWindow.hpp>
#include <xqilla/context/DynamicContext.hpp>
#include <xqilla/schema/SequenceType.hpp>
#include <xqilla/utils/UTF8Str.hpp>
#include <xqilla/ast/XQAtomize.hpp>
#include <xqilla/ast/XQTreatAs.hpp>

#include <xercesc/validators/schema/SchemaSymbols.hpp>

#include <stdlib.h>

#if defined(XERCES_HAS_CPP_NAMESPACE)
XERCES_CPP_NAMESPACE_USE
#endif

FTSelection *FTWindow::staticResolution(StaticContext *context)
{
  XPath2MemoryManager *mm = context->getMemoryManager();

  SequenceType *seqType = new (mm) SequenceType(SchemaSymbols::fgURI_SCHEMAFORSCHEMA,
                                                SchemaSymbols::fgDT_INTEGER,
                                                SequenceType::EXACTLY_ONE, mm);
  seqType->setLocationInfo(this);

  arg_ = arg_->staticResolution(context);

  expr_ = new (mm) XQAtomize(expr_, mm);
  expr_->setLocationInfo(this);
  expr_ = new (mm) XQTreatAs(expr_, seqType, mm);
  expr_->setLocationInfo(this);
  expr_ = expr_->staticResolution(context);

  return this;
}

FTSelection *FTWindow::staticTypingImpl(StaticContext *context)
{
  src_.clear();
  src_.add(arg_->getStaticAnalysis());
  src_.add(expr_->getStaticAnalysis());
  return this;
}

FTSelection *FTWindow::optimize(FTContext *ftcontext) const
{
  XPath2MemoryManager *mm = ftcontext->context->getMemoryManager();

  if(expr_->isConstant()) {
    Numeric::Ptr num = (Numeric*)expr_->createResult(ftcontext->context)->next(ftcontext->context).get();

    FTSelection *result = new (mm) FTWindowLiteral(arg_, num->asInt(), unit_, mm);
    result->setLocationInfo(this);
    return result->optimize(ftcontext);
  }

  FTSelection *newarg = arg_->optimize(ftcontext);
  if(newarg == 0) return 0;

  if(newarg->getType() == WORD) {
    return newarg;
  }

  newarg = new (mm) FTWindow(newarg, expr_, unit_, mm);
  newarg->setLocationInfo(this);
  return newarg;
}

AllMatches *FTWindow::execute(FTContext *ftcontext) const
{
  Numeric::Ptr num = (Numeric*)expr_->createResult(ftcontext->context)->next(ftcontext->context).get();
  return new FTWindowMatches(this, num->asInt(), unit_, arg_->execute(ftcontext));
}

////////////////////////////////////////////////////////////////////////////////////////////////////

FTSelection *FTWindowLiteral::staticResolution(StaticContext *context)
{
  arg_ = arg_->staticResolution(context);
  return this;
}

FTSelection *FTWindowLiteral::staticTypingImpl(StaticContext *context)
{
  src_.clear();
  src_.add(arg_->getStaticAnalysis());
  return this;
}

FTSelection *FTWindowLiteral::optimize(FTContext *ftcontext) const
{
  XPath2MemoryManager *mm = ftcontext->context->getMemoryManager();

  FTSelection *newarg = arg_->optimize(ftcontext);
  if(newarg == 0) return 0;

  if(newarg->getType() == WORD) {
    return newarg;
  }

  newarg = new (mm) FTWindowLiteral(newarg, distance_, unit_, mm);
  newarg->setLocationInfo(this);
  return newarg;
}

AllMatches *FTWindowLiteral::execute(FTContext *ftcontext) const
{
  return new FTWindowMatches(this, distance_, unit_, arg_->execute(ftcontext));
}

////////////////////////////////////////////////////////////////////////////////////////////////////

FTWindowMatches::~FTWindowMatches()
{
  delete arg_;
}

bool FTWindowMatches::next(DynamicContext *context)
{
  excludes_.clear();
  if(!arg_ || !found_) return false;

  unsigned int unitVal;
  while(found_) {
    found_ = arg_->next(context);
    if(!found_) {
      delete arg_;
      arg_ = 0;
      return false;
    }

    StringMatches::const_iterator end = arg_->getStringIncludes().end();
    StringMatches::const_iterator i = arg_->getStringIncludes().begin();
    if(i != end) {
      unsigned int includeMin = FTOption::tokenUnit(i->tokenInfo, unit_);
      unsigned int includeMax = includeMin;
      for(++i; i != end; ++i) {
        unitVal = FTOption::tokenUnit(i->tokenInfo, unit_);
        if(unitVal < includeMin) {
          includeMin = unitVal;
        }
        else if(unitVal > includeMax) {
          includeMax = unitVal;
        }
      }

      if((includeMax - includeMin + 1) <= distance_) {
        unsigned int tokenStart = 0;
        if(includeMax > distance_) {
          tokenStart = includeMax - distance_ + 1;
        }
        unsigned int tokenEnd = includeMin + distance_ - 1;

        excludeValues_.clear();
        end = arg_->getStringIncludes().end();
        i = arg_->getStringIncludes().begin();
        for(; i != end; ++i) {
          unitVal = FTOption::tokenUnit(i->tokenInfo, unit_);
          if(tokenStart <= unitVal && unitVal <= tokenEnd) {
            excludeValues_.insert(unitVal);
          }
        }
        if(excludeValues_.empty()) {
          // Add a dummy value, so we return a match
          excludeValues_.insert(includeMin);
        }
        excludeIt_ = excludeValues_.begin();
        break;
      }
    }
  }

  for(StringMatches::const_iterator i = arg_->getStringExcludes().begin();
      i != arg_->getStringExcludes().end(); ++i) {
    unitVal = FTOption::tokenUnit(i->tokenInfo, unit_);
    if(*excludeIt_ <= unitVal && unitVal <= (*excludeIt_ + distance_ - 1)) {
      excludes_.push_back(*i);
    }
  }

  ++excludeIt_;
  if(excludeIt_ == excludeValues_.end()) {
    found_ = false;
  }

  return true;
}

const StringMatches &FTWindowMatches::getStringIncludes()
{
  assert(arg_);
  return arg_->getStringIncludes();
}

const StringMatches &FTWindowMatches::getStringExcludes()
{
  return excludes_;
}

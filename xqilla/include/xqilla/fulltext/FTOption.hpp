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

#ifndef _FTOPTION_HPP
#define _FTOPTION_HPP

#include <xqilla/fulltext/FTSelection.hpp>
#include <xqilla/fulltext/TokenInfo.hpp>

class XQILLA_API FTOption : public FTSelection
{
public:
  enum FTUnit {
    WORDS,
    SENTENCES,
    PARAGRAPHS
  };

  virtual void setArgument(FTSelection *arg) = 0;

  static unsigned int tokenUnit(const TokenInfo &t, FTUnit unit)
  {
    switch(unit) {
    case WORDS:
      return t.position_;
    case SENTENCES:
      return t.sentence_;
    case PARAGRAPHS:
      return t.paragraph_;
    }
    return 0;
  }

  static unsigned int tokenDistance(const TokenInfo &a, const TokenInfo &b, FTUnit unit)
  {
    switch(unit) {
    case WORDS:
      return (unsigned int)labs((long)a.position_ - (long)b.position_) - 1;
    case SENTENCES:
      return (unsigned int)labs((long)a.sentence_ - (long)b.sentence_);
    case PARAGRAPHS:
      return (unsigned int)labs((long)a.paragraph_ - (long)b.paragraph_);
    }
    return 0;
  }

  static bool lessThanFunc(const TokenInfo &a, const TokenInfo &b)
  {
    if (a.paragraph_ != b.paragraph_)
      return a.paragraph_ < b.paragraph_;
    if (a.sentence_ != b.sentence_)
      return a.sentence_ < b.sentence_;
    return a.position_ < b.position_;
  }

protected:
  FTOption(Type type, XPath2MemoryManager *memMgr)
    : FTSelection(type, memMgr) {}
};

typedef std::vector<FTOption*,XQillaAllocator<FTOption*> > VectorOfFTOptions;

#endif

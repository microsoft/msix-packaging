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

#ifndef _ALLMATCHES_HPP
#define _ALLMATCHES_HPP


#include <xqilla/framework/XQillaExport.hpp>
#include <xqilla/framework/ReferenceCounted.hpp>
#include <xqilla/fulltext/TokenInfo.hpp>

class DynamicContext;

class XQILLA_API StringMatch
{
public:
  StringMatch()
    : tokenInfo(), queryPos(0), startToken(false), endToken(false) 
  {}
  StringMatch(unsigned int qPos, const TokenInfo &tInfo)
    : tokenInfo(tInfo.word_, tInfo.position_, tInfo.sentence_, 
    tInfo.paragraph_), queryPos(qPos), startToken(false), endToken(false) 
  {}

  StringMatch(const StringMatch &other)
    : tokenInfo(other.tokenInfo.word_, other.tokenInfo.position_, 
      other.tokenInfo.sentence_, other.tokenInfo.paragraph_), queryPos(other.queryPos),
      startToken(other.startToken), endToken(other.endToken)
  {}

  TokenInfo tokenInfo;
  unsigned int queryPos;
  bool startToken, endToken;
};

typedef std::vector<StringMatch> StringMatches;

class XQILLA_API AllMatches : public LocationInfo
{
public:
  virtual ~AllMatches() {}

  /// Get the next Match from the iterator. Returns false if there is no next value.
  virtual bool next(DynamicContext *context) = 0;

  virtual const StringMatches &getStringIncludes() = 0;
  virtual const StringMatches &getStringExcludes() = 0;

protected:
  AllMatches(const LocationInfo *o)
  {
    setLocationInfo(o);
  }
};

#endif

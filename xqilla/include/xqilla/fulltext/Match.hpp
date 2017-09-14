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

#ifndef _MATCH_HPP
#define _MATCH_HPP


#include <vector>

#include <xqilla/framework/XQillaExport.hpp>
#include <xqilla/framework/ReferenceCounted.hpp>
#include <xqilla/fulltext/TokenInfo.hpp>

class XQILLA_API StringMatch
{
public:
  StringMatch()
    : queryPos(0), tokenInfo(0), startToken(false), endToken(false) {}
  StringMatch(unsigned int qPos, const TokenInfo::Ptr &tInfo)
    : queryPos(qPos), tokenInfo(tInfo), startToken(false), endToken(false) {}

  unsigned int queryPos;
  TokenInfo::Ptr tokenInfo;
  bool startToken, endToken;
};

typedef std::vector<StringMatch> StringMatches;

class XQILLA_API Match : public ReferenceCounted
{
public:
  typedef RefCountPointer<Match> Ptr;

  Match() {}

  const StringMatches &getStringIncludes() const { return includes_; }
  const StringMatches &getStringExcludes() const { return excludes_; }

  void addStringInclude(unsigned int queryPos, const TokenInfo::Ptr &tokenInfo)
  {
    includes_.push_back(StringMatch(queryPos, tokenInfo));
  }
  void addStringExclude(unsigned int queryPos, const TokenInfo::Ptr &tokenInfo)
  {
    excludes_.push_back(StringMatch(queryPos, tokenInfo));
  }
  void addStringInclude(const StringMatch &match)
  {
    includes_.push_back(match);
  }
  void addStringExclude(const StringMatch &match)
  {
    excludes_.push_back(match);
  }
  void addStringIncludes(const StringMatches &sMatches)
  {
    for(StringMatches::const_iterator j = sMatches.begin(); j != sMatches.end(); ++j) {
      includes_.push_back(*j);
    }
  }
  void addStringExcludes(const StringMatches &sMatches)
  {
    for(StringMatches::const_iterator j = sMatches.begin(); j != sMatches.end(); ++j) {
      excludes_.push_back(*j);
    }
  }
  void add(const Match::Ptr &match) {
    addStringIncludes(match->getStringIncludes());
    addStringExcludes(match->getStringExcludes());
  }

private:
  StringMatches includes_;
  StringMatches excludes_;
};

#endif

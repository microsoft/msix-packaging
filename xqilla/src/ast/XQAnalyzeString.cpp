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
#include <assert.h>
#include <sstream>

#include <xqilla/ast/XQAnalyzeString.hpp>
#include <xqilla/ast/XQFunctionConversion.hpp>
#include <xqilla/context/DynamicContext.hpp>
#include <xqilla/context/ContextHelpers.hpp>
#include <xqilla/context/ItemFactory.hpp>
#include <xqilla/schema/SequenceType.hpp>
#include <xqilla/exceptions/ASTException.hpp>
#include <xqilla/utils/XPath2Utils.hpp>
#include <xqilla/runtime/ClosureResult.hpp>

#include <xercesc/validators/schema/SchemaSymbols.hpp>
#include <xercesc/util/regx/RegularExpression.hpp>
#include <xercesc/util/regx/Match.hpp>
#include <xercesc/util/XMLExceptMsgs.hpp>
#include <xercesc/util/ParseException.hpp>

XERCES_CPP_NAMESPACE_USE
using namespace std;

XQAnalyzeString::XQAnalyzeString(XPath2MemoryManager* memMgr)
  : ASTNodeImpl(ANALYZE_STRING, memMgr),
    expr_(0),
    regex_(0),
    flags_(0),
    match_(0),
    nonMatch_(0)
{
}

XQAnalyzeString::XQAnalyzeString(ASTNode *expr, ASTNode *regex, ASTNode *flags, ASTNode *match, ASTNode *nonMatch, XPath2MemoryManager* memMgr)
  : ASTNodeImpl(ANALYZE_STRING, memMgr),
    expr_(expr),
    regex_(regex),
    flags_(flags),
    match_(match),
    nonMatch_(nonMatch)
{
}

ASTNode* XQAnalyzeString::staticResolution(StaticContext *context)
{
  XPath2MemoryManager *mm = context->getMemoryManager();

  SequenceType *seqType = new (mm) SequenceType(SchemaSymbols::fgURI_SCHEMAFORSCHEMA,
                                                SchemaSymbols::fgDT_STRING,
                                                SequenceType::EXACTLY_ONE, mm);
  seqType->setLocationInfo(this);

  expr_ = new (mm) XQFunctionConversion(expr_, seqType, mm);
  expr_ = expr_->staticResolution(context);

  regex_ = regex_->staticResolution(context);

  if(flags_)
    flags_ = flags_->staticResolution(context);

  match_ = match_->staticResolution(context);
  nonMatch_ = nonMatch_->staticResolution(context);

  return this;
}

ASTNode *XQAnalyzeString::staticTypingImpl(StaticContext *context)
{
  _src.clear();

  _src.add(expr_->getStaticAnalysis());

  // TBD Precompile the regex - jpcs
  _src.add(regex_->getStaticAnalysis());

  if(flags_) {
    _src.add(flags_->getStaticAnalysis());
  }

  _src.add(match_->getStaticAnalysis());
  _src.getStaticType() = match_->getStaticAnalysis().getStaticType();

  _src.add(nonMatch_->getStaticAnalysis());
  _src.getStaticType().typeConcat(nonMatch_->getStaticAnalysis().getStaticType());

  _src.getStaticType().multiply(0, StaticType::UNLIMITED);

  return this;
}

AnalyzeStringResult::AnalyzeStringResult(const LocationInfo *info)
  : ResultImpl(info),
    input_(0),
    matches_(10, true),
    contextPos_(0),
    currentMatch_(0),
    mm_(0),
    result_(0)
{
}

Item::Ptr AnalyzeStringResult::next(DynamicContext *context)
{
#ifdef HAVE_ALLMATCHES
  XPath2MemoryManager *mm = context->getMemoryManager();

  if(input_ == 0) {
    input_ = getInput(context);
    const XMLCh *pattern = getPattern(context);
    const XMLCh *options = getFlags(context);
  
    //Check that the options are valid - throw an exception if not (can have s,m,i and x)
    //Note: Are allowed to duplicate the letters.
    const XMLCh* cursor = options;
    for(; *cursor != 0; ++cursor){
      switch(*cursor) {
      case chLatin_s:
      case chLatin_m:
      case chLatin_i:
      case chLatin_x:
        break;
      default:
        XQThrow(ASTException, X("AnalyzeStringResult::next"),X("Invalid regular expression flags [err:XTDE1145]."));
      }
    }

    mm_ = mm;

    // Always turn off head character optimisation, since it is broken
    XMLBuffer optionsBuf;
    optionsBuf.set(options);
    optionsBuf.append(chLatin_H);

    size_t length = XMLString::stringLen(input_);
    try {
      // Parse and execute the regular expression
      RegularExpression regEx(pattern, optionsBuf.getRawBuffer(), mm);

      if(regEx.matches(XMLUni::fgZeroLenString))
        XQThrow(ASTException, X("AnalyzeStringResult::next"), X("The pattern matches the zero-length string [err:XTDE1150]"));

      regEx.allMatches(input_, 0, length, &matches_);
    }
    catch (ParseException &e){ 
      XMLBuffer buf;
      buf.set(X("Invalid regular expression: "));
      buf.append(e.getMessage());
      buf.append(X(" [err:XTDE1140]"));
      XQThrow(ASTException, X("AnalyzeStringResult::next"), buf.getRawBuffer());
    }
    catch (RuntimeException &e){ 
      if(e.getCode()==XMLExcepts::Regex_RepPatMatchesZeroString)
        XQThrow(ASTException, X("AnalyzeStringResult::next"), X("The pattern matches the zero-length string [err:XTDE1150]"));
      else 
        XQThrow(ASTException, X("AnalyzeStringResult::next"), e.getMessage());
    }

    int tokStart = 0;

    unsigned int i = 0;
    for(; i < matches_.size(); ++i) {
      Match *match = matches_.elementAt(i);
      int matchStart = match->getStartPos(0);
      int matchEnd = match->getEndPos(0);

      if(tokStart < matchStart) {
        const XMLCh *str = XPath2Utils::subString(input_, tokStart, matchStart - tokStart, mm);
        strings_.push_back(pair<const XMLCh*, Match*>(str, static_cast<Match*>(0)));
      }

      const XMLCh *str = XPath2Utils::subString(input_, matchStart, matchEnd - matchStart, mm);
      strings_.push_back(pair<const XMLCh*, Match*>(str, match));

      tokStart = matchEnd;
    }

    if(tokStart < (int) length) {
      const XMLCh *str = XPath2Utils::subString(input_, tokStart,
                                                (unsigned int)(length - tokStart), mm);
      strings_.push_back(pair<const XMLCh*, Match*>(str, static_cast<Match*>(0)));
    }
  }

  AutoRegexGroupStoreReset regexReset(context, this);

  Item::Ptr item;
  while((item = result_->next(context)).isNull()) {
    context->testInterrupt();

    regexReset.reset();

    if(contextPos_ < strings_.size()) {
      const XMLCh *matchString = strings_[contextPos_].first;
      currentMatch_ = strings_[contextPos_].second;
      ++contextPos_;

      context->setRegexGroupStore(this);

      result_ = getMatchResult(matchString, contextPos_, strings_.size(),
                               currentMatch_ != 0, context);
    }
    else {
      return 0;
    }
  }

  return item;
#else
  XQThrow(ASTException, X("AnalyzeStringResult::next"),X("xsl:analyze-string is only supported with Xerces-C 3.0 or newer."));
#endif
}

const XMLCh *AnalyzeStringResult::getGroup(int index) const
{
  if(currentMatch_ == 0 || index < 0 || index >= currentMatch_->getNoGroups())
    return 0;

  int matchStart = currentMatch_->getStartPos(index);
  return XPath2Utils::subString(input_, matchStart, currentMatch_->getEndPos(index) - matchStart, mm_);
}

class XslAnalyzeStringResult : public AnalyzeStringResult
{
public:
  XslAnalyzeStringResult(const XQAnalyzeString *ast)
    : AnalyzeStringResult(ast),
      ast_(ast)
  {
  }

  const XMLCh *getInput(DynamicContext *context)
  {
    return ast_->getExpression()->createResult(context)->next(context)->asString(context);
  }

  const XMLCh *getPattern(DynamicContext *context)
  {
    return ast_->getRegex()->createResult(context)->next(context)->asString(context);
  }

  const XMLCh *getFlags(DynamicContext *context)
  {
    if(ast_->getFlags())
      return ast_->getFlags()->createResult(context)->next(context)->asString(context);
    return XMLUni::fgZeroLenString;
  }

  Result getMatchResult(const XMLCh *matchString, size_t matchPos,
                        size_t numberOfMatches, bool match, DynamicContext *context)
  {
    AutoContextInfoReset autoReset(context);

    context->setContextSize(numberOfMatches);
    context->setContextPosition(matchPos);
    context->setContextItem(context->getItemFactory()->createString(matchString, context));

    if(match) return ast_->getMatch()->createResult(context);
    return ast_->getNonMatch()->createResult(context);
  }

private:
  const XQAnalyzeString *ast_;
};

Result XQAnalyzeString::createResult(DynamicContext* context, int flags) const
{
  return ClosureResult::create(getStaticAnalysis(), context, new XslAnalyzeStringResult(this));
}


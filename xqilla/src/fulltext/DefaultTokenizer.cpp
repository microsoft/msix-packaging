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
#include <xqilla/fulltext/DefaultTokenizer.hpp>
#include <xqilla/framework/XPath2MemoryManager.hpp>
#include <xqilla/context/DynamicContext.hpp>
#include <xqilla/utils/UnicodeTransformer.hpp>
extern "C" {
#include <xqilla/utils/utf8proc.h>
}

#include <xercesc/util/XMLString.hpp>

#if defined(XERCES_HAS_CPP_NAMESPACE)
XERCES_CPP_NAMESPACE_USE
#endif

TokenStream::Ptr DefaultTokenizer::tokenize(const Node::Ptr &node, DynamicContext *context) const
{
  return new DefaultTokenStream(node->dmStringValue(context), context->getMemoryManager());
}

TokenStream::Ptr DefaultTokenizer::tokenize(const XMLCh *str, XPath2MemoryManager *mm) const
{
  return new DefaultTokenStream(str, mm);
}

DefaultTokenizer::DefaultTokenStream::DefaultTokenStream(const XMLCh *str, XPath2MemoryManager *mm)
  : tokenStart_(0),
    position_(0),
    sentence_(0),
    paragraph_(0),
    seenEndOfSentence_(false),
    mm_(mm)
{
  //string_ will point to memory allocated in wordsAndSentences
  string_ = UnicodeTransformer::wordsAndSentences(str, mm_); 
  current_ = string_;

  //skip over the first <WB> and <SB>
  current_++;
  current_++;
}

DefaultTokenizer::DefaultTokenStream::~DefaultTokenStream()
{
  mm_->deallocate(string_);
}

TokenInfo DefaultTokenizer::DefaultTokenStream::next()
{
  TokenInfo result;
  memset(&result, 0, sizeof(TokenInfo));

  // in case that utf8proc return 0
  if(current_ == 0)
    return result;

  while(result.word_ == 0) {
    switch(*current_){
      case UTF8PROC_SB_MARK: {
        ++sentence_;
        break;
      }  
      case UTF8PROC_WB_MARK:{
        if(tokenStart_ != 0 
          && !UnicodeTransformer::isSpacesOrPunctuations(tokenStart_, current_-1)) {
          //report a token
          //token is form tokenStart_ to current_-1
          result.word_ = mm_->getPooledString(tokenStart_, current_-tokenStart_);
          result.paragraph_ = paragraph_;
          result.position_ = position_;
          result.sentence_ = sentence_;
          ++position_;
          tokenStart_ = 0;
        }

        break;
      }
      case 0:{//end of the string
        return result;
      }
      default: {
        // if current is a single space seperator and after is <WB>
        // like: "<WB> <WB>", ignore it
        if(tokenStart_ == 0 
          && *(current_+1) == UTF8PROC_WB_MARK
          && UnicodeTransformer::isSpaceSeparator(current_)){
          break;  
        }

        // if current is a single punctuation and after is <WB>, 
        // like: "<WB>)<WB>", ignore it
        int codeSize;
        if(tokenStart_ == 0  
          && (*(current_+1) == UTF8PROC_WB_MARK || *(current_+2) == UTF8PROC_WB_MARK) 
          && UnicodeTransformer::isPunctuation(current_, codeSize)){
          if (*(current_ + codeSize) == UTF8PROC_WB_MARK)
            current_ = current_ + codeSize-1;

          break;
        }

        if(tokenStart_ == 0) {
          tokenStart_ = current_;
        }
      }
    }
    ++current_;
  }

  return result;
}

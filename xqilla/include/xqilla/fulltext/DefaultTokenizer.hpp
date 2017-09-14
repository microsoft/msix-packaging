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

#ifndef _DEFAULTTOKENIZER_HPP
#define _DEFAULTTOKENIZER_HPP

#include <xqilla/framework/XQillaExport.hpp>
#include <xqilla/fulltext/Tokenizer.hpp>

class XQILLA_API DefaultTokenizer : public Tokenizer
{
public:
  DefaultTokenizer() {}

  virtual TokenStream::Ptr tokenize(const Node::Ptr &node, DynamicContext *context) const;
  virtual TokenStream::Ptr tokenize(const XMLCh *str, XPath2MemoryManager *mm) const;

private:
  class DefaultTokenStream : public TokenStream
  {
  public:
    DefaultTokenStream(const XMLCh *str, XPath2MemoryManager *mm);
    virtual ~DefaultTokenStream();
    virtual TokenInfo next();

  private:
    XMLCh *string_;
    XMLCh *current_;
    XMLCh *tokenStart_;

    unsigned int position_;
    unsigned int sentence_;
    unsigned int paragraph_;

    bool seenEndOfSentence_;

    XPath2MemoryManager *mm_;
  };
};

#endif

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

#ifndef _DEFAULTTOKENSTORE_HPP
#define _DEFAULTTOKENSTORE_HPP

#include <xqilla/framework/XQillaExport.hpp>
#include <xqilla/fulltext/TokenStore.hpp>
#include <xqilla/items/Node.hpp>

#include <xercesc/util/RefHashTableOf.hpp>
#include <xercesc/framework/XMLBuffer.hpp>

#include <vector>

class XPath2MemoryManager;
class DynamicContext;

class XQILLA_API DefaultTokenStore : public TokenStore
{
public:
  typedef std::vector<TokenInfo> Tokens;

  class TokenEntryStream : public TokenStream
  {
  public:
    TokenEntryStream(const Tokens &tokens)
      : it_(tokens.begin()), end_(tokens.end()) {}

    virtual TokenInfo next()
    {
      if(it_ != end_)
        return *it_++;
      TokenInfo token;
      memset(&token, 0, sizeof(TokenInfo));
      return token;
    }

  private:
    Tokens::const_iterator it_;
    Tokens::const_iterator end_;
  };

  DefaultTokenStore(const Node::Ptr &node, const Tokenizer *tokenizer,
                    DynamicContext *context);
  DefaultTokenStore(XPath2MemoryManager *mm);

  virtual TokenStream::Ptr findTokens(const XMLCh *searchString) const;

  virtual unsigned int getStartTokenPosition() const { return 0; }
  virtual unsigned int getEndTokenPosition() const { return numTokens_ - 1; }

private:
  class TokenEntry
  {
  public:
    void addToken(const TokenInfo &token)
    {
      tokens_.push_back(token);
    }
    TokenStream::Ptr getTokenStream() const
    {
      return new TokenEntryStream(tokens_);
    }

  private:
    Tokens tokens_;
  };

private:
  XERCES_CPP_NAMESPACE_QUALIFIER RefHashTableOf<TokenEntry> tokens_;
  unsigned int numTokens_;
  XPath2MemoryManager *mm_;
  DynamicContext *context_;
};

#endif

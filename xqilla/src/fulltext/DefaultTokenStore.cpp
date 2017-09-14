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
#include <xqilla/fulltext/DefaultTokenStore.hpp>
#include <xqilla/fulltext/Tokenizer.hpp>
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

DefaultTokenStore::DefaultTokenStore(const Node::Ptr &node, const Tokenizer *tokenizer,
                                     DynamicContext *context)
  : tokens_(73, /*adoptElems*/true, context->getMemoryManager()),
    numTokens_(0),
    mm_(context->getMemoryManager()),
    context_(context)
{
  TokenStream::Ptr stream = tokenizer->tokenize(node, context);
  TokenInfo token;
	
  int options = 0;
  //consider stem, stop words, thesaurus, case options
  if(context_->getFTCase() == StaticContext::CASE_INSENSITIVE) {
    options = UTF8PROC_CASEFOLD;
  }


  while((token = stream->next()).word_ != 0) {
    ++numTokens_;
    AutoDeallocate<XMLCh> buf(UnicodeTransformer::transform(token.word_, options, mm_), mm_);
    TokenEntry *entry = tokens_.get(buf.get());
    if(entry == 0) {
      entry = new TokenEntry();
      tokens_.put((void*)mm_->getPooledString(buf.get()), entry);
    }
    entry->addToken(token);
  }
}

DefaultTokenStore::DefaultTokenStore(XPath2MemoryManager *mm)
  : tokens_(3, /*adoptElems*/true, mm),
    numTokens_(0),
    mm_(mm)
{
}

TokenStream::Ptr DefaultTokenStore::findTokens(const XMLCh *searchString) const
{
  int options = 0;
  if(context_->getFTCase() == StaticContext::CASE_INSENSITIVE){
    options = UTF8PROC_CASEFOLD;
  } else if(context_->getFTCase() == StaticContext::UPPERCASE){
    options = UTF8PROC_UPPERCASE;
  } else if(context_->getFTCase() == StaticContext::LOWERCASE){
    options = UTF8PROC_LOWERCASE;
  }

  AutoDeallocate<XMLCh> buf(UnicodeTransformer::transform(searchString, options, mm_), mm_);
  const TokenEntry *entry = tokens_.get(buf.get());
  if(entry == 0) return 0;
  return entry->getTokenStream();
}

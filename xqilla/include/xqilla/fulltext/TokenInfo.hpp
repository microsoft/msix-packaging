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

#ifndef _TOKENINFO_HPP
#define _TOKENINFO_HPP

#include <xqilla/framework/XQillaExport.hpp>
#include <xqilla/framework/ReferenceCounted.hpp>

#include <stdlib.h>

struct XQILLA_API TokenInfo
{
  const XMLCh *word_;
  unsigned int position_;
  unsigned int sentence_;
  unsigned int paragraph_; 

  TokenInfo() 
  {
    word_ = 0;
    position_ = 0;
    sentence_ = 0;
    paragraph_ = 0;
  }

  TokenInfo(const XMLCh *word, unsigned int position, 
                         unsigned int sentence, unsigned int paragraph) 
  {
    word_ = word;
    position_ = position;
    sentence_ = sentence;
    paragraph_ = paragraph;
  }
};

class XQILLA_API TokenStream : public ReferenceCounted
{
public:
  typedef RefCountPointer<TokenStream> Ptr;

  virtual ~TokenStream() {}

  virtual TokenInfo next() = 0;

protected:
  TokenStream() {}
};

#endif

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
#include <xercesc/util/XMLString.hpp>
#include <xqilla/utils/UTF8Str.hpp>

XERCES_CPP_NAMESPACE_USE;

UTF8Str::UTF8Str(const XMLCh* const toTranscode, MemoryManager *mm)
  : str_(0),
    mm_(mm)
{
  if(toTranscode == 0) {
    str_ = mm_ ? (XMLByte*)mm_->allocate(1 * sizeof(XMLByte)) : new XMLByte[1];
    str_[0] = 0;
  }
  else {
    XERCES_CPP_NAMESPACE_QUALIFIER XMLUTF8Transcoder t(0, 512);
    size_t l = XERCES_CPP_NAMESPACE_QUALIFIER XMLString::stringLen(toTranscode);
    const size_t needed = l * 3 + 1; // 3 bytes per XMLCh is the worst case, + '\0'
    str_ = mm_ ? (XMLByte*)mm_->allocate(needed * sizeof(XMLByte)) : new XMLByte[needed];
#if _XERCES_VERSION >= 30000
    XMLSize_t charsEaten= 0;
    t.transcodeTo(toTranscode, l+1, str_,
                  needed, charsEaten, XERCES_CPP_NAMESPACE_QUALIFIER XMLTranscoder::UnRep_Throw);
#else
    unsigned int charsEaten= 0;
    t.transcodeTo(toTranscode, (unsigned int)l+1, str_, 
                  (unsigned int)needed, charsEaten, XERCES_CPP_NAMESPACE_QUALIFIER XMLTranscoder::UnRep_Throw);
#endif
  }
}

UTF8Str::~UTF8Str()
{
  if(str_) {
    if(mm_) mm_->deallocate(str_);
    else delete [] str_;
  }
}

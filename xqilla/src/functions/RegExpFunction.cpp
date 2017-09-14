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
#include <xqilla/functions/RegExpFunction.hpp>
#include <xqilla/exceptions/FunctionException.hpp>

#include <xercesc/util/regx/RegularExpression.hpp>
#include <xercesc/util/ParseException.hpp>
#include <xercesc/util/XMLUni.hpp>

#if defined(XERCES_HAS_CPP_NAMESPACE)
XERCES_CPP_NAMESPACE_USE
#endif


RegExpFunction::RegExpFunction(const XMLCh* name, const char *signature, const VectorOfASTNodes &args, XPath2MemoryManager* memMgr)
  : XQFunction(name, signature, args, memMgr),
    regExp_(0), pattern_(0), options_(0)
{
}


void RegExpFunction::checkRegexpOpts(const XMLCh* opts, const char* sourceMsg) const
{
  const XMLCh* cursor = opts;
  for(; *cursor != 0; ++cursor){
    switch(*cursor) {
    case chLatin_s:
    case chLatin_m:
    case chLatin_i:
    case chLatin_x:
      break;
    default:
      XQThrow(FunctionException, X(sourceMsg),X("Invalid regular expression flags [err:FORX0001]."));
    }
  }
}

void RegExpFunction::processParseException(ParseException &e, const char* sourceMsg, XPath2MemoryManager* memMgr) const
{
  XMLBuffer buf(1023, memMgr);
  buf.set(X("Invalid regular expression: "));
  buf.append(e.getMessage());
  buf.append(X(" [err:FORX0002]"));
  XQThrow(FunctionException, X(sourceMsg), buf.getRawBuffer());
}

void RegExpFunction::copyRegExp(RegExpFunction* source, XPath2MemoryManager* memMgr)
{
  if(source->regExp_)
  {
    pattern_ = memMgr->getPooledString(source->pattern_);
    options_ = memMgr->getPooledString(source->options_);

    // Always turn off head character optimisation, since it is broken
    XMLBuffer optionsBuf;
    optionsBuf.set(options_);
    optionsBuf.append(chLatin_H);

    //compiling regexp again
    try
    {
      regExp_ = new (memMgr) RegularExpression(pattern_, optionsBuf.getRawBuffer(), memMgr);
    } catch (ParseException &e){
      processParseException(e, "RegExpFunction::copyRegExp", memMgr);
    }
  }
}

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

#ifndef _REGEXPFUNCTION_HPP
#define _REGEXPFUNCTION_HPP

#include <xqilla/ast/XQFunction.hpp>

XERCES_CPP_NAMESPACE_BEGIN
class RegularExpression;
class ParseException;
XERCES_CPP_NAMESPACE_END


class XQILLA_API RegExpFunction: public XQFunction
{

public:
  RegExpFunction(const XMLCh* name, const char *signature, const VectorOfASTNodes &args, XPath2MemoryManager* memMgr);

  //getting a pre-compiled regular expression
  const XERCES_CPP_NAMESPACE_QUALIFIER RegularExpression* getRegExp() const {
    return regExp_;
  };

  //this method is needed for copying a pre-compiled regular expression,
  //it copies @pattern_ and @options_ and then compiles a new regular expression
  //@source is the regexp function which we copy a regexp from
  virtual void copyRegExp(RegExpFunction *source, XPath2MemoryManager* memMgr);


protected:
  //if a regular expession is a constant, then we will store a compiled regexp here,
  //and also pattern, and options if presented. We need those values when copying this function
  const XERCES_CPP_NAMESPACE_QUALIFIER RegularExpression *regExp_;
  const XMLCh *pattern_;
  const XMLCh *options_;

  //helper functions
  virtual void checkRegexpOpts(const XMLCh* opts, const char* sourceMsg) const;
  virtual void processParseException(XERCES_CPP_NAMESPACE_QUALIFIER ParseException &e, const char* sourceMsg,
                             XPath2MemoryManager* memMgr) const;


};

#endif


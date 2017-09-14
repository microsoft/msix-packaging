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

#ifndef _FUNCTIONREPLACE_HPP
#define _FUNCTIONREPLACE_HPP

#include <xqilla/functions/RegExpFunction.hpp>

XERCES_CPP_NAMESPACE_BEGIN
class RuntimeException;
class RegularExpression;
class ParseException;
XERCES_CPP_NAMESPACE_END


/** 
 * Returns the string that is obtained by replacing all non-overlapping substrings of $input 
 * that match the given $pattern with an occurrence of the $replacement string.  
 *
 * xf:replace(string? $input, string $pattern, string $replacement) => string?
 * xf:replace(string? $input, string $pattern, string  $replacement, 
 *            string  $flags) => string?
 */
class XQILLA_API FunctionReplace : public RegExpFunction
{

public:
  static const XMLCh name[];
  static const unsigned int minArgs;
  static const unsigned int maxArgs;

  /**
   * Constructor.
   */
  FunctionReplace(const VectorOfASTNodes &args, XPath2MemoryManager* memMgr);

  virtual ASTNode *staticTypingImpl(StaticContext *context);

  /**
   * Returns $input with non-overlapping matches to $pattern 
   * replaced by $replacement
   */
  Sequence createSequence(DynamicContext* context, int flags=0) const;

  static const XMLCh *replace(const XMLCh *input, const XMLCh *pattern, const XMLCh *replacement,
                              const XMLCh *options = 0, XERCES_CPP_NAMESPACE_QUALIFIER MemoryManager *mm =
                              XERCES_CPP_NAMESPACE_QUALIFIER XMLPlatformUtils::fgMemoryManager);

  static const XMLCh *replace(const XMLCh *input, const XERCES_CPP_NAMESPACE_QUALIFIER RegularExpression *regExp,
                              const XMLCh *replacement,
                              XERCES_CPP_NAMESPACE_QUALIFIER MemoryManager *mm =
                              XERCES_CPP_NAMESPACE_QUALIFIER XMLPlatformUtils::fgMemoryManager);

private:

  //helper functions
  void processRuntimeException(XERCES_CPP_NAMESPACE_QUALIFIER RuntimeException &e, const char* sourceMsg) const;
};

#endif


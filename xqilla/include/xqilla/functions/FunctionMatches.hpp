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

#ifndef _FUNCTIONMATCHES_HPP
#define _FUNCTIONMATCHES_HPP

#include <xqilla/functions/RegExpFunction.hpp>

XERCES_CPP_NAMESPACE_BEGIN
class XMLException;
class RegularExpression;
class ParseException;
XERCES_CPP_NAMESPACE_END


class XQILLA_API FunctionMatches : public RegExpFunction {
public:
  static const XMLCh name[];
  static const unsigned int minArgs;
  static const unsigned int maxArgs;

  FunctionMatches(const VectorOfASTNodes &args, XPath2MemoryManager* memMgr);

  virtual ASTNode *staticTypingImpl(StaticContext *context);
  virtual BoolResult boolResult(DynamicContext* context) const;
  virtual Result createResult(DynamicContext* context, int flags) const;

  static bool matches(const XMLCh *input, const XMLCh *pattern, const XMLCh *options = 0);
  static bool matches(const XMLCh *input, const XERCES_CPP_NAMESPACE_QUALIFIER RegularExpression* regExp);

private:
  void processXMLException(XERCES_CPP_NAMESPACE_QUALIFIER XMLException &e, const char* sourceMsg) const;
};

#endif

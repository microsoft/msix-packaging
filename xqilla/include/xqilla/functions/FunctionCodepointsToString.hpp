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

/*
  codepoints-to-string function
*/

#ifndef _FUNCTIONCODEPOINTSTOSTRING_HPP
#define _FUNCTIONCODEPOINTSTOSTRING_HPP

#include <xqilla/framework/XQillaExport.hpp>

#include <xqilla/ast/XQFunction.hpp>

/** codepoints-to-string function */
class XQILLA_API FunctionCodepointsToString : public XQFunction
{
public:
  static const XMLCh name[];
  static const unsigned int minArgs;
  static const unsigned int maxArgs;

  ///Constructor.
  FunctionCodepointsToString(const VectorOfASTNodes &args, XPath2MemoryManager* memMgr);
  
protected:
  /** takes a series of integers (codepoints) and turns them into a unicode string */
  Sequence createSequence(DynamicContext* context, int flags=0) const;
};
#endif // _FUNCTIONCODEPOINTSTOSTRING_HPP


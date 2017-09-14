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
   Union function -- takes two sequences, and merge them
*/

#ifndef _UNION_HPP
#define _UNION_HPP

#include <xqilla/framework/XQillaExport.hpp>

#include <xqilla/ast/XQNav.hpp>
#include <xqilla/ast/XQOperator.hpp>
#include <xqilla/runtime/ResultImpl.hpp>

/** Union operator function*/
class XQILLA_API Union : public XQOperator
{
public:
  static const XMLCh name[];

  Union(const VectorOfASTNodes &args, XPath2MemoryManager* memMgr);
  
  ASTNode* staticResolution(StaticContext *context);
  virtual ASTNode *staticTypingImpl(StaticContext *context);
  Result createResult(DynamicContext* context, int flags=0) const;

private:
  class UnionResult : public ResultImpl
  {
  public:
    UnionResult(const Union *op, int flags);

    Item::Ptr next(DynamicContext *context);
  private:
    const Union *_op;
    unsigned int _flags;
    unsigned int _index;
    Result _result;
  };

  bool sortAdded_;
};

#endif


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

#ifndef _XQCONTEXTITEM_HPP
#define _XQCONTEXTITEM_HPP

#include <xqilla/framework/XQillaExport.hpp>

#include <xqilla/ast/ASTNodeImpl.hpp>

/** Returns the context item. Raises [err:XP0002] if the context item is null. */
class XQILLA_API XQContextItem : public ASTNodeImpl
{
public:
  XQContextItem(XPath2MemoryManager* memMgr);
  ~XQContextItem();

  /**
   * Called during static analysis of expression to determine is statically
   * correct.  If not an appropriate error is thrown.
   */
  virtual ASTNode* staticResolution(StaticContext *context);
  virtual ASTNode *staticTypingImpl(StaticContext *context);

  virtual Result createResult(DynamicContext* context, int flags=0) const;
  static Result result(DynamicContext *context, const LocationInfo *info);
};

#endif


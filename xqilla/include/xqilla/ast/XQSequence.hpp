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

#ifndef _XQSEQUENCE_HPP
#define _XQSEQUENCE_HPP

#include <xqilla/ast/ASTNodeImpl.hpp>

class XQILLA_API XQSequence : public ASTNodeImpl
{
public:
  XQSequence(XPath2MemoryManager* memMgr);	
  XQSequence(const VectorOfASTNodes &children, XPath2MemoryManager* memMgr);	

  ///adds a ASTNode to this XQSequence
  void addItem(ASTNode* di);

  virtual ASTNode *staticResolution(StaticContext *context);
  virtual ASTNode *staticTypingImpl(StaticContext *context);

  virtual Result createResult(DynamicContext* context, int flags=0) const;
  virtual BoolResult boolResult(DynamicContext* context) const;
  virtual EventGenerator::Ptr generateEvents(EventHandler *events, DynamicContext *context,
                              bool preserveNS, bool preserveType) const;
  virtual PendingUpdateList createUpdateList(DynamicContext *context) const;

  const VectorOfASTNodes &getChildren() const { return _astNodes; }

  static XQSequence *constantFold(Result &result, DynamicContext *context, XPath2MemoryManager* memMgr,
                                  const LocationInfo *location);

private:
  VectorOfASTNodes _astNodes;
};

#endif

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

#ifndef _ASTRELEASER_HPP
#define _ASTRELEASER_HPP

#include <xqilla/optimizer/ASTVisitor.hpp>

class XQILLA_API ASTReleaser : public ASTVisitor
{
public:
  ASTReleaser();

  void release(ASTNode *item);
  void release(TupleNode *item);
  void release(XQUserFunction *item);

protected:
  virtual ASTNode *optimize(ASTNode *item);
  virtual TupleNode *optimizeTupleNode(TupleNode *item);
  virtual FTSelection *optimizeFTSelection(FTSelection *selection);

  ALL_ASTVISITOR_METHODS()
};

#endif

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

#ifndef FORTUPLE_HPP
#define FORTUPLE_HPP

#include <xqilla/ast/TupleNode.hpp>
#include <xqilla/ast/StaticAnalysis.hpp>

class ASTNode;

class XQILLA_API ForTuple : public TupleNode
{
public:
  ForTuple(TupleNode *parent, const XMLCh *varQName, const XMLCh *posQName, ASTNode *expr,
           XPath2MemoryManager *mm);
  ForTuple(TupleNode *parent, const XMLCh *varURI, const XMLCh *varName,
	  const XMLCh *posURI, const XMLCh *posName, ASTNode *expr, XPath2MemoryManager *mm);

  const XMLCh *getVarQName() const { return varQName_; }
  const XMLCh *getVarURI() const { return varURI_; }
  const XMLCh *getVarName() const { return varName_; }

  const XMLCh *getPosQName() const { return posQName_; }
  const XMLCh *getPosURI() const { return posURI_; }
  const XMLCh *getPosName() const { return posName_; }

  ASTNode *getExpression() const { return expr_; }
  void setExpression(ASTNode *expr) { expr_ = expr; }

  const StaticAnalysis &getVarSRC() const { return varSrc_; }
  const StaticAnalysis &getPosSRC() const { return posSrc_; }

  virtual TupleNode *staticResolution(StaticContext *context);
  virtual TupleNode *staticTypingImpl(StaticContext *context);
  virtual TupleNode *staticTypingTeardown(StaticContext *context, StaticAnalysis &usedSrc);

  virtual TupleResult::Ptr createResult(DynamicContext* context) const;

private:
  const XMLCh *varQName_, *varURI_, *varName_;
  const XMLCh *posQName_, *posURI_, *posName_;
  StaticAnalysis varSrc_, posSrc_;
  ASTNode *expr_;
};

#endif

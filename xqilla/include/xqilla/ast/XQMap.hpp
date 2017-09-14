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

#ifndef _XQMAP_HPP
#define _XQMAP_HPP

#include <xqilla/framework/XQillaExport.hpp>
#include <xqilla/ast/ASTNodeImpl.hpp>
#include <xqilla/context/VariableStore.hpp>

class XQILLA_API XQMap : public ASTNodeImpl
{
public:
  XQMap(ASTNode *arg1, ASTNode *arg2, XPath2MemoryManager* memMgr);
  XQMap(ASTNode *arg1, ASTNode *arg2, const XMLCh *uri, const XMLCh *name, XPath2MemoryManager* memMgr);

  ASTNode *getArg1() const { return arg1_; }
  void setArg1(ASTNode *a) { arg1_ = a; }
  ASTNode *getArg2() const { return arg2_; }
  void setArg2(ASTNode *a) { arg2_ = a; }

  const XMLCh *getURI() const { return uri_; }
  const XMLCh *getName() const { return name_; }

  const StaticAnalysis &getVarSRC() const { return varSrc_; }

  virtual ASTNode *staticResolution(StaticContext *context);
  virtual ASTNode *staticTypingImpl(StaticContext *context);
  virtual Result createResult(DynamicContext* context, int flags=0) const;

private:
  ASTNode *arg1_;
  ASTNode *arg2_;

  const XMLCh *uri_;
  const XMLCh *name_;
  StaticAnalysis varSrc_;
};

class XQILLA_API MapResult : public ResultImpl, public VariableStore
{
public:
  MapResult(const Result &parent, const ASTNode *step, const XMLCh *uri, const XMLCh *name);

  virtual Result getVar(const XMLCh *namespaceURI, const XMLCh *name) const;
  virtual void getInScopeVariables(std::vector<std::pair<const XMLCh*, const XMLCh*> > &variables) const;

  Item::Ptr next(DynamicContext *context);

private:
  Result parent_;
  const ASTNode *step_;
  const XMLCh *uri_;
  const XMLCh *name_;

  Result stepResult_;

  Item::Ptr item_;
  const VariableStore *scope_;
};

#endif

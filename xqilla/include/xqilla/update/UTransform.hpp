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

#ifndef _UTRANSFORM_HPP
#define _UTRANSFORM_HPP

#include <xqilla/ast/ASTNodeImpl.hpp>
#include <xqilla/context/impl/VarStoreImpl.hpp>
#include <xqilla/schema/DocumentCache.hpp>

class XQILLA_API CopyBinding : public LocationInfo
{
public:
  CopyBinding(XPath2MemoryManager* memMgr, const XMLCh* variable, ASTNode* allValues);
  CopyBinding(XPath2MemoryManager* memMgr, const CopyBinding &o);

  const XMLCh *qname_, *uri_, *name_;

  StaticAnalysis src_;

  ASTNode *expr_;

private:
  CopyBinding(const CopyBinding &o);
};

typedef std::vector<CopyBinding*, XQillaAllocator<CopyBinding*> > VectorOfCopyBinding;

class XQILLA_API UTransform : public ASTNodeImpl
{
public:
  UTransform(VectorOfCopyBinding* bindings, ASTNode *modifyExpr, ASTNode* returnExpr, XPath2MemoryManager *expr);
  UTransform(VectorOfCopyBinding* bindings, ASTNode *modifyExpr, ASTNode* returnExpr, DocumentCache::ValidationMode valMode, XPath2MemoryManager *expr);

  virtual ASTNode *staticResolution(StaticContext* context);
  virtual ASTNode *staticTypingImpl(StaticContext *context);
  virtual Result createResult(DynamicContext* context, int flags) const;

  const VectorOfCopyBinding *getBindings() const { return bindings_; }

  ASTNode *getModifyExpr() const { return modify_; }
  void setModifyExpr(ASTNode *modify) { modify_ = modify; }

  ASTNode *getReturnExpr() const { return return_; }
  void setReturnExpr(ASTNode *ret) { return_ = ret; }

  DocumentCache::ValidationMode getRevalidationMode() const { return valMode_; }

protected:
  VectorOfCopyBinding* bindings_;
  ASTNode *modify_;
  ASTNode *return_;
  DocumentCache::ValidationMode valMode_;
};

#endif

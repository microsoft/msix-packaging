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

#ifndef _CONVERTFUNCTIONARG_HPP
#define _CONVERTFUNCTIONARG_HPP

#include <xqilla/framework/XQillaExport.hpp>
#include <xqilla/ast/ASTNodeImpl.hpp>

class XQILLA_API XQPromoteUntyped : public ASTNodeImpl
{
public:
  XQPromoteUntyped(ASTNode* expr, const XMLCh *uri, const XMLCh *name, XPath2MemoryManager* memMgr);
  XQPromoteUntyped(ASTNode* expr, const XMLCh *uri, const XMLCh *name, bool isPrimitive, AnyAtomicType::AtomicObjectType typeIndex, XPath2MemoryManager* memMgr);

  virtual Result createResult(DynamicContext* context, int flags=0) const;
  virtual ASTNode* staticResolution(StaticContext *context);
  virtual ASTNode *staticTypingImpl(StaticContext *context);

  ASTNode *getExpression() const { return expr_; }
  void setExpression(ASTNode *expr) { expr_ = expr; }
  const XMLCh *getTypeURI() const { return uri_; }
  const XMLCh *getTypeName() const { return name_; }
  bool getIsPrimitive() const { return isPrimitive_; }
  AnyAtomicType::AtomicObjectType getTypeIndex() const { return typeIndex_; }  

protected:
  ASTNode* expr_;
  const XMLCh *uri_, *name_;
  bool isPrimitive_;
  AnyAtomicType::AtomicObjectType typeIndex_;
};

class PromoteUntypedResult : public ResultImpl
{
public:
  PromoteUntypedResult(const XQPromoteUntyped *di, const Result &parent);
  PromoteUntypedResult(const LocationInfo *location, const Result &parent,
                       bool isPrimitive, AnyAtomicType::AtomicObjectType typeIndex,
                       const XMLCh *uri, const XMLCh *name);

  Item::Ptr next(DynamicContext *context);
  std::string asString(DynamicContext *context, int indent) const { return "promoteuntypedresult"; }
private:
  Result parent_;
  bool isPrimitive_;
  AnyAtomicType::AtomicObjectType typeIndex_;
  const XMLCh *uri_, *name_;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

class XQILLA_API XQPromoteNumeric : public ASTNodeImpl
{
public:
  XQPromoteNumeric(ASTNode* expr, const XMLCh *uri, const XMLCh *name, XPath2MemoryManager* memMgr);
  XQPromoteNumeric(ASTNode* expr, const XMLCh *uri, const XMLCh *name, AnyAtomicType::AtomicObjectType typeIndex, XPath2MemoryManager* memMgr);

  virtual Result createResult(DynamicContext* context, int flags=0) const;
  virtual ASTNode* staticResolution(StaticContext *context);
  virtual ASTNode *staticTypingImpl(StaticContext *context);

  ASTNode *getExpression() const { return expr_; }
  void setExpression(ASTNode *expr) { expr_ = expr; }
  const XMLCh *getTypeURI() const { return uri_; }
  const XMLCh *getTypeName() const { return name_; }
  AnyAtomicType::AtomicObjectType getTypeIndex() const { return typeIndex_; }

protected:
  ASTNode* expr_;
  const XMLCh *uri_, *name_;
  AnyAtomicType::AtomicObjectType typeIndex_;
};

class PromoteNumericResult : public ResultImpl
{
public:
  PromoteNumericResult(const LocationInfo *location, const Result &parent, AnyAtomicType::AtomicObjectType typeIndex)
    : ResultImpl(location), parent_(parent), typeIndex_(typeIndex) {}

  Item::Ptr next(DynamicContext *context);
  std::string asString(DynamicContext *context, int indent) const { return "promotenumericresult"; }
private:
  Result parent_;
  AnyAtomicType::AtomicObjectType typeIndex_;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

class XQILLA_API XQPromoteAnyURI : public ASTNodeImpl
{
public:
  XQPromoteAnyURI(ASTNode* expr, const XMLCh *uri, const XMLCh *name, XPath2MemoryManager* memMgr);

  virtual Result createResult(DynamicContext* context, int flags=0) const;
  virtual ASTNode* staticResolution(StaticContext *context);
  virtual ASTNode *staticTypingImpl(StaticContext *context);

  ASTNode *getExpression() const { return expr_; }
  void setExpression(ASTNode *expr) { expr_ = expr; }
  const XMLCh *getTypeURI() const { return uri_; }
  const XMLCh *getTypeName() const { return name_; }

protected:
  ASTNode* expr_;
  const XMLCh *uri_, *name_;
};

class PromoteAnyURIResult : public ResultImpl
{
public:
  PromoteAnyURIResult(const LocationInfo *location, const Result &parent)
    : ResultImpl(location), parent_(parent) {}

  Item::Ptr next(DynamicContext *context);
  std::string asString(DynamicContext *context, int indent) const { return "promoteanyuriresult"; }
private:
  Result parent_;
};

#endif

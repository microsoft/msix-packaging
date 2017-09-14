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

#ifndef _XQLITERAL_HPP
#define _XQLITERAL_HPP

#include <xqilla/ast/ASTNodeImpl.hpp>
#include <xqilla/mapm/m_apm.h>

class XQILLA_API XQLiteral : public ASTNodeImpl
{
public:
  XQLiteral(const XMLCh* typeURI, const XMLCh* typeName, const XMLCh* value,
            AnyAtomicType::AtomicObjectType primitiveType, XPath2MemoryManager* memMgr);

  virtual bool isDateOrTimeAndHasNoTimezone(StaticContext* context) const;

  virtual ASTNode *staticResolution(StaticContext *context);
  virtual ASTNode *staticTypingImpl(StaticContext *context);
  virtual BoolResult boolResult(DynamicContext* context) const;
  virtual Result createResult(DynamicContext* context, int flags=0) const;
  virtual EventGenerator::Ptr generateEvents(EventHandler *events, DynamicContext *context,
                                             bool preserveNS, bool preserveType) const;

  AnyAtomicType::AtomicObjectType getPrimitiveType() const { return primitiveType_; }
  const XMLCh *getTypeURI() const { return typeURI_; }
  const XMLCh *getTypeName() const { return typeName_; }
  const XMLCh *getValue() const { return value_; }

  static ASTNode *create(const Item::Ptr &item, DynamicContext *context, XPath2MemoryManager* memMgr,
                         const LocationInfo *location);
  static ASTNode *create(bool value, XPath2MemoryManager* memMgr, const LocationInfo *location);

private:
  const XMLCh *typeURI_;
  const XMLCh *typeName_;
  AnyAtomicType::AtomicObjectType primitiveType_;
  const XMLCh *value_;
};

class XQILLA_API XQQNameLiteral : public ASTNodeImpl
{
public:
  XQQNameLiteral(const XMLCh* typeURI, const XMLCh* typeName, const XMLCh* uri,
                 const XMLCh* prefix, const XMLCh* localname, XPath2MemoryManager* memMgr);

  virtual bool isDateOrTimeAndHasNoTimezone(StaticContext* context) const;

  virtual ASTNode *staticResolution(StaticContext *context);
  virtual ASTNode *staticTypingImpl(StaticContext *context);
  virtual Result createResult(DynamicContext* context, int flags=0) const;
  virtual EventGenerator::Ptr generateEvents(EventHandler *events, DynamicContext *context,
                                             bool preserveNS, bool preserveType) const;

  const XMLCh *getTypeURI() const { return typeURI_; }
  const XMLCh *getTypeName() const { return typeName_; }
  const XMLCh *getURI() const { return uri_; }
  const XMLCh *getLocalname() const { return localname_; }
  const XMLCh *getPrefix() const { return prefix_; }

private:
  const XMLCh *typeURI_;
  const XMLCh *typeName_;
  const XMLCh *uri_, *prefix_, *localname_;
};

class XQILLA_API XQNumericLiteral : public ASTNodeImpl
{
public:
  XQNumericLiteral(const XMLCh* typeURI, const XMLCh* typeName, const MAPM& value,
                   AnyAtomicType::AtomicObjectType primitiveType, XPath2MemoryManager* memMgr);

  virtual bool isDateOrTimeAndHasNoTimezone(StaticContext* context) const;

  virtual ASTNode *staticResolution(StaticContext *context);
  virtual ASTNode *staticTypingImpl(StaticContext *context);
  virtual Result createResult(DynamicContext* context, int flags=0) const;
  virtual EventGenerator::Ptr generateEvents(EventHandler *events, DynamicContext *context,
                                             bool preserveNS, bool preserveType) const;

  AnyAtomicType::AtomicObjectType getPrimitiveType() const { return primitiveType_; }
  const XMLCh *getTypeURI() const { return typeURI_; }
  const XMLCh *getTypeName() const { return typeName_; }
  MAPM getValue() const;
  const M_APM_struct &getRawValue() const { return value_; }

private:
  const XMLCh *typeURI_;
  const XMLCh *typeName_;
  AnyAtomicType::AtomicObjectType primitiveType_;
  M_APM_struct value_;
};

#endif


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

#ifndef XQVALIDATE_HPP
#define XQVALIDATE_HPP

#include <xqilla/ast/ASTNodeImpl.hpp>
#include <xqilla/events/EventHandler.hpp>
#include <xqilla/schema/DocumentCache.hpp>

class XQILLA_API XQValidate : public ASTNodeImpl
{
public:
  XQValidate(ASTNode *expr, DocumentCache::ValidationMode mode, XPath2MemoryManager *mm);

  virtual ASTNode* staticResolution(StaticContext *context);
  virtual ASTNode *staticTypingImpl(StaticContext *context);
  virtual Result createResult(DynamicContext* context, int flags=0) const;
  virtual EventGenerator::Ptr generateEvents(EventHandler *events, DynamicContext *context,
                              bool preserveNS, bool preserveType) const;

  ASTNode *getExpression() const { return expr_; }
  DocumentCache::ValidationMode getMode() const { return mode_; }

  void setExpression(ASTNode *expr) { expr_ = expr; }

private:
  ASTNode *expr_;
  DocumentCache::ValidationMode mode_;
};

class XQILLA_API ValidateArgumentCheckFilter : public EventFilter
{
public:
  ValidateArgumentCheckFilter(EventHandler *next, DocumentCache::ValidationMode mode, DynamicContext *context,
                              const LocationInfo *loc);

  virtual void startDocumentEvent(const XMLCh *documentURI, const XMLCh *encoding);
  virtual void endDocumentEvent();
  virtual void startElementEvent(const XMLCh *prefix, const XMLCh *uri, const XMLCh *localname);
  virtual void endElementEvent(const XMLCh *prefix, const XMLCh *uri, const XMLCh *localname,
                               const XMLCh *typeURI, const XMLCh *typeName);
  virtual void textEvent(const XMLCh *value);
  virtual void textEvent(const XMLCh *chars, unsigned int length);
  virtual void piEvent(const XMLCh *target, const XMLCh *value);
  virtual void commentEvent(const XMLCh *value);
  virtual void attributeEvent(const XMLCh *prefix, const XMLCh *uri, const XMLCh *localname, const XMLCh *value,
                              const XMLCh *typeURI, const XMLCh *typeName);
  virtual void namespaceEvent(const XMLCh *prefix, const XMLCh *uri);
  virtual void atomicItemEvent(AnyAtomicType::AtomicObjectType type, const XMLCh *value, const XMLCh *typeURI,
                               const XMLCh *typeName);

private:
  DocumentCache::ValidationMode mode_;
  const LocationInfo *info_;
  DynamicContext *context_;
  bool inDocumentNode_;
  bool seenDocElem_;
  unsigned int level_;
  bool seenOne_;
};

#endif

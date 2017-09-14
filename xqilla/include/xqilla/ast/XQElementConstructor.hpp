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

#ifndef XQELEMENTCONSTRUCTOR_HPP
#define XQELEMENTCONSTRUCTOR_HPP

#include <xqilla/ast/XQDOMConstructor.hpp>
#include <xqilla/events/EventHandler.hpp>

#include <xercesc/util/RefHashTableOf.hpp>

#include <set>

class XQILLA_API XQElementConstructor : public XQDOMConstructor
{
public:
  XQElementConstructor(ASTNode* name, VectorOfASTNodes* attrList, VectorOfASTNodes* children, XPath2MemoryManager *mm);
  XQElementConstructor(ASTNode* name, VectorOfASTNodes* attrList, VectorOfASTNodes* children,
                       XERCES_CPP_NAMESPACE_QUALIFIER RefHashTableOf< XMLCh > *namespaces, XPath2MemoryManager *mm);

  virtual EventGenerator::Ptr generateEvents(EventHandler *events, DynamicContext *context,
                              bool preserveNS, bool preserveType) const;
  virtual ASTNode* staticResolution(StaticContext *context);
  virtual ASTNode *staticTypingImpl(StaticContext *context);

  virtual const XMLCh* getNodeType() const;
  virtual ASTNode *getName() const;
  virtual const VectorOfASTNodes *getAttributes() const;
  void setAttributes(VectorOfASTNodes *a) { m_attrList = a; }
  virtual const VectorOfASTNodes *getChildren() const;
  void setChildren(VectorOfASTNodes *c) { m_children = c; }

  XERCES_CPP_NAMESPACE_QUALIFIER RefHashTableOf< XMLCh > *getNamespaces() const { return m_namespaces; }

  virtual void setName(ASTNode *name);

  // Used for parsing only
  ASTNode *namespaceExpr;

protected:
  ASTNode* m_name;
  VectorOfASTNodes* m_attrList, *m_children;
  XERCES_CPP_NAMESPACE_QUALIFIER RefHashTableOf< XMLCh >* m_namespaces;
};

class ElemConstructFilter : public EventFilter
{
public:
  ElemConstructFilter(EventHandler *next, const LocationInfo *location, XPath2MemoryManager *mm);

  virtual void startElementEvent(const XMLCh *prefix, const XMLCh *uri, const XMLCh *localname);
  virtual void endElementEvent(const XMLCh *prefix, const XMLCh *uri, const XMLCh *localname,
                               const XMLCh *typeURI, const XMLCh *typeName);
  virtual void piEvent(const XMLCh *target, const XMLCh *value);
  virtual void textEvent(const XMLCh *value);
  virtual void textEvent(const XMLCh *chars, unsigned int length);
  virtual void commentEvent(const XMLCh *value);
  virtual void attributeEvent(const XMLCh *prefix, const XMLCh *uri, const XMLCh *localname, const XMLCh *value,
                              const XMLCh *typeURI, const XMLCh *typeName);
  virtual void namespaceEvent(const XMLCh *prefix, const XMLCh *uri);

private:
  struct AttrRecord {
    AttrRecord(const XMLCh *u, const XMLCh *n, XPath2MemoryManager *mm);

    bool operator<(const AttrRecord &o) const;

    const XMLCh *uri;
    const XMLCh *name;
  };

  XPath2MemoryManager *mm_;
  const LocationInfo *location_;
  unsigned int level_;
  bool seenContent_;
  std::set<AttrRecord> attrs_;
};

#endif

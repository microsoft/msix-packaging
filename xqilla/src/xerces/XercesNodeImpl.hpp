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

#ifndef _XQILLANODEIMPL_HPP
#define _XQILLANODEIMPL_HPP

#include <xqilla/items/Node.hpp>
#include <xqilla/utils/XMLChCompare.hpp>
#include "Axis.hpp"

#include <xercesc/util/XercesDefs.hpp>
#include <xercesc/framework/XMLBuffer.hpp>

class DynamicContext;
class XPath2MemoryManager;
class Sequence;
class DocumentCache;
class XercesURIResolver;

XERCES_CPP_NAMESPACE_BEGIN
class DatatypeValidator;
class DOMNode;
XERCES_CPP_NAMESPACE_END

class XercesNodeImpl : public Node, private AxisNodeFactory
{
public:
  typedef RefCountPointer<const XercesNodeImpl> Ptr;

  XercesNodeImpl(const XERCES_CPP_NAMESPACE_QUALIFIER DOMNode *node, XercesURIResolver *resolver);
  ~XercesNodeImpl();

  virtual void *getInterface(const XMLCh *name) const;

  virtual const XMLCh* asString(const DynamicContext* context) const;
  virtual bool hasInstanceOfType(const XMLCh* typeURI, const XMLCh* typeName, const DynamicContext* context) const;

  virtual Sequence dmBaseURI(const DynamicContext* context) const;
  virtual const XMLCh* dmNodeKind() const;
  virtual ATQNameOrDerived::Ptr dmNodeName(const DynamicContext* context) const;
  virtual const XMLCh* dmStringValue(const DynamicContext* context) const;
  virtual Sequence dmTypedValue(DynamicContext* context) const;

  virtual Sequence dmDocumentURI(const DynamicContext* context) const;
  virtual ATQNameOrDerived::Ptr dmTypeName(const DynamicContext* context) const;

  virtual ATBooleanOrDerived::Ptr dmNilled(const DynamicContext* context) const;

  virtual bool lessThan(const Node::Ptr &other, const DynamicContext *context) const;
  virtual bool equals(const Node::Ptr &other) const;
  virtual bool uniqueLessThan(const Node::Ptr &other, const DynamicContext *context) const;

  virtual const XERCES_CPP_NAMESPACE_QUALIFIER DOMNode* getDOMNode() const;

  virtual Node::Ptr root(const DynamicContext* context) const;
  virtual Node::Ptr dmParent(const DynamicContext* context) const;
  virtual Result dmAttributes(const DynamicContext* context, const LocationInfo *info) const;
  virtual Result dmNamespaceNodes(const DynamicContext* context, const LocationInfo *info) const;
  virtual Result dmChildren(const DynamicContext *context, const LocationInfo *info) const;

  virtual Result getAxisResult(XQStep::Axis axis, const NodeTest *nodeTest, const DynamicContext *context, const LocationInfo *info) const;

  virtual ATBooleanOrDerived::Ptr dmIsId(const DynamicContext* context) const;
  virtual ATBooleanOrDerived::Ptr dmIsIdRefs(const DynamicContext* context) const;

  virtual const XMLCh* getTypeURI() const;
  virtual const XMLCh* getTypeName() const;

  virtual void generateEvents(EventHandler *events, const DynamicContext *context,
                              bool preserveNS = true, bool preserveType = true) const;

  static void typeUriAndName(const XERCES_CPP_NAMESPACE_QUALIFIER DOMNode *node, const XMLCh *&uri, const XMLCh *&name);

  static void generateEvents(const XERCES_CPP_NAMESPACE_QUALIFIER DOMNode *node, EventHandler *events, bool preserveNS = true, bool preserveType = true);

protected:
  virtual Node::Ptr createNode(const XERCES_CPP_NAMESPACE_QUALIFIER DOMNode *node, const DynamicContext *context) const;

  virtual void getTypeUriAndName(const XMLCh*& uri, const XMLCh*& name) const;
  virtual void getMemberTypeUriAndName(const XMLCh*& uri, const XMLCh*& name) const;
  Sequence getListTypeTypedValue(XERCES_CPP_NAMESPACE_QUALIFIER DatatypeValidator *dtv, const DynamicContext* context) const;
  void addStringValueToBuffer(const XERCES_CPP_NAMESPACE_QUALIFIER DOMNode* node, XERCES_CPP_NAMESPACE_QUALIFIER XMLBuffer& buffer) const;

  const XERCES_CPP_NAMESPACE_QUALIFIER DOMNode* fNode;
  XercesURIResolver *resolver_;
};
#endif

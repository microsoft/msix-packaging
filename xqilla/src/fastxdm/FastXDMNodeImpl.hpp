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

#ifndef _FASTXDMNODEIMPL_HPP
#define _FASTXDMNODEIMPL_HPP

#include <xqilla/items/Node.hpp>
#include "FastXDMDocument.hpp"

XERCES_CPP_NAMESPACE_BEGIN
class DatatypeValidator;
XERCES_CPP_NAMESPACE_END

class XQILLA_API FastXDMNodeImpl : public Node
{
public:
  typedef RefCountPointer<const FastXDMNodeImpl> Ptr;

  FastXDMNodeImpl(const FastXDMDocument::Ptr &document, const FastXDMDocument::Node *node);
  ~FastXDMNodeImpl();

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

  const FastXDMDocument::Ptr &getDocument() const { return document_; }
  const FastXDMDocument::Node *getNode() const { return node_; } 

  static const XMLCh fastxdm_string[];

protected:
  virtual void getTypeUriAndName(const XMLCh*& uri, const XMLCh*& name) const;

  FastXDMDocument::Ptr document_;
  const FastXDMDocument::Node *node_;
};

class XQILLA_API FastXDMAttributeNodeImpl : public Node
{
public:
  typedef RefCountPointer<const FastXDMAttributeNodeImpl> Ptr;

  FastXDMAttributeNodeImpl(const FastXDMDocument::Ptr &document, const FastXDMDocument::Attribute *attr);
  ~FastXDMAttributeNodeImpl();

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

  const FastXDMDocument::Ptr &getDocument() const { return document_; }
  const FastXDMDocument::Attribute *getAttribute() const { return attr_; } 

  static const XMLCh fastxdm_attr_string[];

protected:
  virtual void getTypeUriAndName(const XMLCh*& uri, const XMLCh*& name) const;

  FastXDMDocument::Ptr document_;
  const FastXDMDocument::Attribute *attr_;
};

class XQILLA_API FastXDMNamespaceNodeImpl : public Node
{
public:
  typedef RefCountPointer<const FastXDMNamespaceNodeImpl> Ptr;

  FastXDMNamespaceNodeImpl(const FastXDMNodeImpl::Ptr &owner, const XMLCh *prefix, const XMLCh *uri,
                           unsigned int order);
  ~FastXDMNamespaceNodeImpl();

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

  const FastXDMNodeImpl::Ptr &getOwner() const { return owner_; }
  const XMLCh *getPrefix() const { return prefix_; }
  const XMLCh *getURI() const { return uri_; }
  unsigned int getOrder() const { return order_; }

  static const XMLCh fastxdm_ns_string[];

protected:
  FastXDMNodeImpl::Ptr owner_;
  const XMLCh *prefix_;
  const XMLCh *uri_;
  unsigned int order_;
};

#endif

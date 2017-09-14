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

#ifndef _XQILLANODE_HPP
#define _XQILLANODE_HPP

#include <xqilla/items/ATQNameOrDerived.hpp>
#include <xqilla/items/ATBooleanOrDerived.hpp>
#include <xqilla/ast/XQStep.hpp>

class DynamicContext;
class XPath2MemoryManager;
class Sequence;
class Result;
class NodeTest;

class XQILLA_API Node : public Item
{
public:
  typedef RefCountPointer<const Node> Ptr;

  /** Returns true, since this Item is a Node */
  virtual bool isNode() const;

  /** Returns false, since this Item is a Node */
  virtual bool isAtomicValue() const;

  virtual bool isFunction() const;

  virtual void typeToBuffer(DynamicContext *context, XERCES_CPP_NAMESPACE_QUALIFIER XMLBuffer &buffer) const;

  /** Serialize this node */
  virtual const XMLCh* asString(const DynamicContext* context) const = 0;

  /** check if the underlying type of this node is instance of a certain type */
  virtual bool hasInstanceOfType(const XMLCh* typeURI, const XMLCh* typeName, const DynamicContext* context) const = 0;
  
  /** Accessor: returns the base-uri of this node as a
   * xs:anyURI, if one exists */
  virtual Sequence dmBaseURI(const DynamicContext* context) const = 0;

  /** Accessor: returns the kind of node as a string */
  virtual const XMLCh* dmNodeKind() const = 0;

  /** Accessor: returns the name of this node */
  virtual ATQNameOrDerived::Ptr dmNodeName(const DynamicContext* context) const = 0;

  /** Accessor: returns the string value of this node */
  virtual const XMLCh* dmStringValue(const DynamicContext* context) const = 0;

  /** Accessor: returns the typed value of this node, which is a sequence of zero or 
   * more atomic values derived from the string-value of the node and its type in such 
   * a way as to be consistent with validation */
  virtual Sequence dmTypedValue(DynamicContext* context) const = 0;

  /** Accessor:  returns the absolute URI of the resource from which the document node 
   * was constructed, if the absolute URI is available*/
  virtual Sequence dmDocumentURI(const DynamicContext* context) const = 0;

  /** Accessor: returns the name of the type of a node */
  virtual ATQNameOrDerived::Ptr dmTypeName(const DynamicContext* context) const = 0;

  /** Accessor: returns the setting of the nilled property of an element node */
  virtual ATBooleanOrDerived::Ptr dmNilled(const DynamicContext* context) const = 0;

  /** Returns true if 'this' is before 'other' in document order,
   * false otherwise */
  virtual bool lessThan(const Node::Ptr &other, const DynamicContext *context) const = 0;

  /** Returns true if 'other' is equal (identity equal) to 'this'*/
  virtual bool equals(const Node::Ptr &other) const = 0;

  /** Specifies a sorting order for uniqueness removal. This may or may not
      return the same as lessThan. */
  virtual bool uniqueLessThan(const Node::Ptr &other, const DynamicContext *context) const = 0;

  /** Returns the root of this node */
  virtual Node::Ptr root(const DynamicContext* context) const = 0;

  /** Returns the parent of this node */
  virtual Node::Ptr dmParent(const DynamicContext* context) const = 0;

  /** Returns the attributes of this node */
  virtual Result dmAttributes(const DynamicContext* context, const LocationInfo *info) const = 0;

  /** Returns the namespace nodes of this node */
  virtual Result dmNamespaceNodes(const DynamicContext* context, const LocationInfo *info) const = 0;

  /** Returns the children of this node */
  virtual Result dmChildren(const DynamicContext *context, const LocationInfo *info) const = 0;

  /**
   * Returns a Result containing the nodes in the axis given, relative to this node.
   * Forward axis results should be in document order.
   * Reverse axis results should be in reverse document order.
   */
  virtual Result getAxisResult(XQStep::Axis axis, const NodeTest *nodeTest, const DynamicContext *context, const LocationInfo *info) const = 0;

  /** Returns if the node is an id node or not */
  virtual ATBooleanOrDerived::Ptr dmIsId(const DynamicContext* context) const = 0;

  /** Returns if the node is an idref node or not */
  virtual ATBooleanOrDerived::Ptr dmIsIdRefs(const DynamicContext* context) const = 0;

  /* Get the namespace URI for the DOM type */
  virtual const XMLCh* getTypeURI() const = 0;

  /* Get the name of the DOM type  (ie "integer" for xs:integer) */
  virtual const XMLCh* getTypeName() const = 0;

  static const XMLCh document_string[];
  static const XMLCh element_string[];
  static const XMLCh attribute_string[];
  static const XMLCh text_string[];
  static const XMLCh processing_instruction_string[];
  static const XMLCh comment_string[];
  static const XMLCh namespace_string[];
};
#endif

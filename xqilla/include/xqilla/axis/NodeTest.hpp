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

#ifndef _NODETEST_HPP
#define _NODETEST_HPP

#include <xqilla/framework/XQillaExport.hpp>

#include <xqilla/runtime/Result.hpp>
#include <xqilla/runtime/ResultImpl.hpp>
#include <xqilla/schema/SequenceType.hpp>

class StaticContext;
class DynamicContext;

/**
 * This class represents a NodeTest production from the grammar, 
 * and provides methods for filtering a Result containing Node items,
 * and determining if a single Item matches.
 */
class XQILLA_API NodeTest
{
public:
  NodeTest();
  NodeTest(const XMLCh *nodeType, const XMLCh *uri = 0, const XMLCh *name = 0);
  NodeTest(const NodeTest *other);
  virtual ~NodeTest();

  /** Method for returning arbitrary interfaces from the implementations */
  virtual void *getInterface(const XMLCh *name) const;

  /// Filters the given Result, returning only Items that match the NodeTest
  Result filterResult(const Result &toFilter, const LocationInfo *info) const;
  ///check if a node has to be placed in the result set
  bool filterNode(Node::Ptr node, DynamicContext* context) const;

  void staticResolution(StaticContext *context, const LocationInfo *location);

  ///gets the sequence type used to match the nodes
  SequenceType::ItemType* getItemType() const;
  ///set function for private member
  void setItemType(SequenceType::ItemType* type);
  
  ///gets the name of the node we are selecting
  const XMLCh* getNodeName() const;
  ///set function for private member
  void setNodeName(const XMLCh* name);

  ///gets the uri of the node we are selecting
  const XMLCh* getNodeUri() const;
  ///set function for private member
  void setNodeUri(const XMLCh* uri);

  ///gets the prefix of the node we are selecting
  const XMLCh* getNodePrefix() const;
  ///set function for private member
  void setNodePrefix(const XMLCh* prefix);
  ///returns whether the prefix has been set (instead of the uri)
  bool isNodePrefixSet() const;

  ///gets the node type we are selecting
  const XMLCh *getNodeType() const;
  ///set node type looked for
  void setNodeType(const XMLCh *type);
  ///retuns whether the type has been set in this NavStep
  bool isNodeTypeSet() const;

  ///returns true if the Node Name ia a wildcard
  bool getNameWildcard() const;
  ///sets the wildcard nodeName bool
  void setNameWildcard(bool value = true);

  ///returns true if the Node prefix ia a wildcard
  bool getNamespaceWildcard() const;
  ///sets the prefix wildcard bool
  void setNamespaceWildcard(bool value = true);

  ///returns true if the type is a wildcard
  bool getTypeWildcard() const;
  ///sets the type wildcard bool
  void setTypeWildcard(bool value = true);

  /// returns true if the following step is a child axis (for optimisation reasons)
  bool getHasChildren() const;
  /// sets whether the following step is a child axis
  void setHasChildren(bool value = true);

  void getStaticType(StaticType &st, const StaticContext *context,
                     bool &isExact, const LocationInfo *location) const;

  bool isSubsetOf(const NodeTest *o) const;
  static bool isSubsetOf(const NodeTest *a, const NodeTest *b);

protected:
  ///check a node's type against _type, considers wildcard
  bool checkNodeType(Node::Ptr node) const;

  ///check a node's name against _name, considers wildcard
  bool checkNodeName(Node::Ptr node, const DynamicContext *context) const;

  /**
   * Performs filtering based on type or name.
   */
  class XQILLA_API FilterResult : public ResultImpl
  {
  public:
    FilterResult(const LocationInfo *info, const Result &toFilter, const NodeTest *nodeTest);
    virtual ~FilterResult() {}

    Item::Ptr next(DynamicContext *context);

  protected:
    Result toFilter_;
    const NodeTest *nodeTest_;
  };

protected:
  const XMLCh* _name,*_uri,*_prefix;
  const XMLCh* _type;
  bool _wildcardName, _wildcardNamespace, _wildcardType, _usePrefix, _hasChildren;
  SequenceType::ItemType* _itemType;
};

#endif

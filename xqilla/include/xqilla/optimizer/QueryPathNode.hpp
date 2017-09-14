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

#ifndef _QUERYPATHNODE_HPP
#define _QUERYPATHNODE_HPP

#include <vector>
#include <string>

#include <xqilla/framework/XPath2MemoryManager.hpp>

#include <xercesc/util/XMemory.hpp>

class NodeTest;

class XQILLA_API QueryPathNode : public XERCES_CPP_NAMESPACE_QUALIFIER XMemory
{
public:
  typedef enum {
    ATTRIBUTE,
    CHILD,
    DESCENDANT,
    DESCENDANT_ATTR,
    ROOT
  } Type;

  typedef std::vector<QueryPathNode*> Vector;
  typedef std::vector<QueryPathNode*, XQillaAllocator<QueryPathNode*> > MVector;


  QueryPathNode(const NodeTest *nodeTest, Type type, XERCES_CPP_NAMESPACE_QUALIFIER MemoryManager *mm);
  virtual ~QueryPathNode();

  Type getType() const { return type_; }
  void setType(Type t) { type_ = t; }

  bool isWildcardURI() const;
  bool isWildcardName() const;
  bool isWildcardNodeType() const;

  virtual bool isWildcard() const;

  const XMLCh *getURI() const;
  const XMLCh *getName() const;

  const NodeTest *getNodeTest() const { return nodeTest_; }

  QueryPathNode *getParent() const { return parent_; }
  QueryPathNode *getNextSibling() const { return nextSibling_; }
  QueryPathNode *getPreviousSibling() const { return prevSibling_; }
  QueryPathNode *getFirstChild() const { return firstChild_; }
  QueryPathNode *getLastChild() const { return lastChild_; }

  QueryPathNode *getRoot() const;

  QueryPathNode *appendChild(QueryPathNode *childToAdopt);
  void removeChild(QueryPathNode *child, bool deleteChild = true);
  void stealChildren(QueryPathNode *victim);
  virtual void markSubtreeValue();
  virtual void markSubtreeResult();

  virtual std::string toString(int level = 0) const;

  virtual bool equals(const QueryPathNode *o) const;
  virtual bool isSubsetOf(const QueryPathNode *node) const;

protected:
  QueryPathNode(const QueryPathNode &);
  QueryPathNode &operator=(const QueryPathNode &);

  Type type_;

  const NodeTest *nodeTest_;

  QueryPathNode *parent_;
  QueryPathNode *nextSibling_;
  QueryPathNode *prevSibling_;
  QueryPathNode *firstChild_;
  QueryPathNode *lastChild_;

  XERCES_CPP_NAMESPACE_QUALIFIER MemoryManager *memMgr_;
};

#endif

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

#ifndef _QUERYPATHTREEGENERATOR_HPP
#define _QUERYPATHTREEGENERATOR_HPP

#include <map>

#include <xqilla/optimizer/ASTVisitor.hpp>
#include <xqilla/optimizer/QueryPathNode.hpp>
#include <xqilla/context/impl/VariableStoreTemplate.hpp>
#include <xqilla/framework/XPath2MemoryManagerImpl.hpp>

class NodeTest;

typedef std::map<const XMLCh *, QueryPathNode*> QPNMap;

/**
 * Generates QueryPathNode trees of the paths
 * in the documents that will be navigated.
 */
class XQILLA_API QueryPathTreeGenerator : public ASTVisitor
{
public:
  QueryPathTreeGenerator(DynamicContext *context, Optimizer *parent = 0);
  virtual ~QueryPathTreeGenerator() { varStore_.clear(); }

  class XQILLA_API PathResult {
  public:
    void join(const QueryPathNode::MVector &o);
    void join(const PathResult &o);
    void join(QueryPathNode *o);

    void markSubtreeValue() const;
    void markSubtreeResult() const;
    void markRoot() const;

    QueryPathNode::Vector returnPaths;
  };
protected:
  virtual QueryPathNode *createQueryPathNode(const NodeTest *nodeTest, QueryPathNode::Type type);
  virtual NodeTest *createNodeTest(const XMLCh *nodeType, const XMLCh *uri = 0, const XMLCh *name = 0);

  virtual void resetInternal();

  virtual void optimize(XQQuery *query);
  virtual ASTNode *optimize(ASTNode *item);
  ALL_ASTVISITOR_METHODS();

  void push(PathResult result);
  PathResult pop();
  PathResult generate(ASTNode *item);

  void generateBuiltInStep(QueryPathNode *target, QueryPathNode &node,
                           PathResult &result);
  void generateParentStep(QueryPathNode *target, QueryPathNode &node,
                          PathResult &result);
  void generateSelfStep(QueryPathNode *target, QueryPathNode &node,
                        PathResult &result);
  void generateAncestorStep(QueryPathNode *target,
                            QueryPathNode &node, PathResult &result);
  void generateAncestorOrSelfStep(QueryPathNode *target,
                                  QueryPathNode &node,
                                  PathResult &result);
  void generateFollowingStep(QueryPathNode *target,
                             QueryPathNode &node, PathResult &result);
  void generatePrecedingStep(QueryPathNode *target,
                             QueryPathNode &node, PathResult &result);
  void generateSiblingStep(QueryPathNode *target, QueryPathNode &node,
                           PathResult &result);

  void createAnyNodeResult(PathResult &result);

  const PathResult &getCurrentContext() const;
  void setCurrentContext(const PathResult &value);
  void setVariable(const XMLCh *uri, const XMLCh *name,
                   const PathResult &value);

  PathResult copyNodes(const PathResult &r);

  XPath2MemoryManager *mm_;
  DynamicContext *context_;

  std::set<const ASTNode*> userFunctionStack_;

  typedef VarHashEntry<PathResult> VarStoreRef;
  typedef VariableStoreTemplate<PathResult> VarStore;

  XPath2MemoryManagerImpl varStoreMemMgr_;
  VarStore varStore_; ///< Memory owned by varStoreMemMgr_

  std::vector<PathResult> results_;

  QPNMap projectionMap_;
};

#endif

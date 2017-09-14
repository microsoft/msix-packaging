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

#include "../config/xqilla_config.h"
#include <xqilla/optimizer/QueryPathTreeGenerator.hpp>
#include <xqilla/axis/NodeTest.hpp>

#include <xqilla/context/DynamicContext.hpp>

#include <xqilla/operators/And.hpp>
#include <xqilla/operators/Or.hpp>
#include <xqilla/operators/Equals.hpp>
#include <xqilla/operators/NotEquals.hpp>
#include <xqilla/operators/Plus.hpp>
#include <xqilla/operators/Minus.hpp>
#include <xqilla/operators/Multiply.hpp>
#include <xqilla/operators/Divide.hpp>
#include <xqilla/operators/IntegerDivide.hpp>
#include <xqilla/operators/Mod.hpp>
#include <xqilla/operators/LessThan.hpp>
#include <xqilla/operators/GreaterThan.hpp>
#include <xqilla/operators/LessThanEqual.hpp>
#include <xqilla/operators/GreaterThanEqual.hpp>
#include <xqilla/operators/UnaryMinus.hpp>
#include <xqilla/operators/NodeComparison.hpp>
#include <xqilla/operators/OrderComparison.hpp>
#include <xqilla/operators/GeneralComp.hpp>
#include <xqilla/operators/Intersect.hpp>
#include <xqilla/operators/Except.hpp>
#include <xqilla/operators/Union.hpp>

#include <xqilla/functions/FunctionBaseURI.hpp>
#include <xqilla/functions/FunctionCollection.hpp>
#include <xqilla/functions/FunctionDoc.hpp>
#include <xqilla/functions/FunctionDocument.hpp>
#include <xqilla/functions/FunctionId.hpp>
#include <xqilla/functions/FunctionIdref.hpp>
#include <xqilla/functions/FunctionLang.hpp>
#include <xqilla/functions/FunctionNormalizeSpace.hpp>
#include <xqilla/functions/FunctionNumber.hpp>
#include <xqilla/functions/FunctionParseXML.hpp>
#include <xqilla/functions/FunctionParseJSON.hpp>
#include <xqilla/functions/FunctionExplain.hpp>
#include <xqilla/functions/FunctionRoot.hpp>
#include <xqilla/functions/FunctionString.hpp>
#include <xqilla/functions/FunctionStringLength.hpp>
#include <xqilla/functions/FunctionTrace.hpp>
#include <xqilla/functions/FunctionUnordered.hpp>
#include <xqilla/functions/FunctionHead.hpp>
#include <xqilla/functions/FunctionTail.hpp>
#include <xqilla/functions/FunctionSignature.hpp>

#include <xqilla/update/FunctionPut.hpp>
#include <xqilla/utils/XPath2Utils.hpp>

#include "../items/impl/FunctionRefImpl.hpp"

#include <xercesc/util/XMLUri.hpp>

XERCES_CPP_NAMESPACE_USE;
using namespace std;

static const XMLCh XMLChDot[] = { chColon, chColon, chLatin_d, chLatin_o, chLatin_t, chNull };

QueryPathTreeGenerator::QueryPathTreeGenerator(DynamicContext *context, Optimizer *parent)
  : ASTVisitor(parent),
    mm_(context->getMemoryManager()),
    context_(context),
    varStore_(&varStoreMemMgr_)
{
  resetInternal();
}

QueryPathNode *QueryPathTreeGenerator::createQueryPathNode(const NodeTest *nodeTest, QueryPathNode::Type type)
{
  return new (mm_) QueryPathNode(nodeTest, type, mm_);
}

NodeTest *QueryPathTreeGenerator::createNodeTest(const XMLCh *nodeType, const XMLCh *uri, const XMLCh *name)
{
  return new (mm_) NodeTest(nodeType, uri, name);
}

void QueryPathTreeGenerator::createAnyNodeResult(PathResult &result)
{
  NodeTest *nt = createNodeTest(Node::document_string);
  QueryPathNode *root = createQueryPathNode(nt, QueryPathNode::ROOT);
  result.join(root);

  QueryPathNode *pisn = root->appendChild(createQueryPathNode(createNodeTest((XMLCh*)0),
                                                              QueryPathNode::DESCENDANT));
  result.join(pisn);

  QueryPathNode *cisn = pisn->appendChild(createQueryPathNode(createNodeTest(Node::attribute_string),
                                                              QueryPathNode::ATTRIBUTE));
  result.join(cisn);
}

void QueryPathTreeGenerator::resetInternal()
{
  varStore_.clear();

  // The initial context item could be a node of any type from anywhere -
  // set up a set of paths that reflect that
  PathResult contextResult;
  createAnyNodeResult(contextResult);
  varStore_.setGlobalVar(XMLChDot, XMLChDot, contextResult);
}

void QueryPathTreeGenerator::push(PathResult result)
{
  results_.push_back(result);
}

QueryPathTreeGenerator::PathResult QueryPathTreeGenerator::pop()
{
  PathResult result = results_.back();
  results_.pop_back();
  return result;
}

QueryPathTreeGenerator::PathResult QueryPathTreeGenerator::generate(ASTNode *item)
{
  optimize(item);
  return pop();
}

ASTNode *QueryPathTreeGenerator::optimize(ASTNode *item)
{
  return ASTVisitor::optimize(item);
}

void QueryPathTreeGenerator::optimize(XQQuery *query)
{
  ASTVisitor::optimize(query);

  if(query->getQueryBody() != 0) {
    pop().markSubtreeResult();
  }
}

XQUserFunction *QueryPathTreeGenerator::optimizeFunctionDef(XQUserFunction *item)
{
  if(item->getFunctionBody() != 0) {
    const ArgumentSpecs *params = item->getSignature()->argSpecs;
    if(params) {
      varStore_.addScope(VarStore::MyScope::LOCAL_SCOPE);

      for(ArgumentSpecs::const_iterator it = params->begin();
          it != params->end(); ++it) {
          PathResult paramRes;
          if((*it)->getStaticAnalysis().getStaticType().containsType(StaticType::NODE_TYPE))
            createAnyNodeResult(paramRes);
          setVariable((*it)->getURI(), (*it)->getName(), paramRes);
      }
    }

    generate(const_cast<ASTNode*>(item->getFunctionBody()));

    if(params) {
      delete varStore_.popScope();
    }
  }

  return item;
}

XQGlobalVariable *QueryPathTreeGenerator::optimizeGlobalVar(XQGlobalVariable *item)
{
  PathResult result;

  if(item->getVariableExpr()) {
    result.join(generate(const_cast<ASTNode *>(item->getVariableExpr())));
  } else {
    if(item->getStaticAnalysis().getStaticType().containsType(StaticType::NODE_TYPE))
      createAnyNodeResult(result);
  }

  varStore_.setGlobalVar(item->getVariableURI(), item->getVariableLocalName(), result);

  return item;
}

static QueryPathNode::Type getTypeFromAxis(XQStep::Axis axis)
{
  QueryPathNode::Type result = (QueryPathNode::Type)-1;
  switch(axis) {
  case XQStep::CHILD: {
    result = QueryPathNode::CHILD;
    break;
  }
  case XQStep::ATTRIBUTE: {
    result = QueryPathNode::ATTRIBUTE;
    break;
  }
  case XQStep::DESCENDANT_OR_SELF:
  case XQStep::DESCENDANT: {
    result = QueryPathNode::DESCENDANT;
    break;
  }
  default: {
    // Do nothing
    break;
  }
  }
  return result;
}

const QueryPathTreeGenerator::PathResult &QueryPathTreeGenerator::getCurrentContext() const
{
  return varStore_.getVar(XMLChDot, XMLChDot)->getValue();
}

void QueryPathTreeGenerator::setCurrentContext(const PathResult &value)
{
  setVariable(XMLChDot, XMLChDot, value);
}

void QueryPathTreeGenerator::setVariable(const XMLCh *uri, const XMLCh *name, const PathResult &value)
{
  varStore_.declareVar(uri, name, value);
}

ASTNode *QueryPathTreeGenerator::optimizeStep(XQStep *item)
{
  XQStep::Axis axis = item->getAxis();
  const NodeTest *nodeTest = item->getNodeTest();

  PathResult result;

  QueryPathNode dummy(nodeTest, getTypeFromAxis(axis), mm_);

  const PathResult &currentContext = getCurrentContext();
  for(QueryPathNode::Vector::const_iterator it = currentContext.returnPaths.begin();
      it != currentContext.returnPaths.end(); ++it) {

    switch(axis) {
    case XQStep::PARENT: {
      generateParentStep(*it, dummy, result);
      break;
    }
    case XQStep::DESCENDANT_OR_SELF: {
      generateSelfStep(*it, dummy, result);
      generateBuiltInStep(*it, dummy, result);
      break;
    }
    case XQStep::SELF: {
      generateSelfStep(*it, dummy, result);
      break;
    }
    case XQStep::ANCESTOR: {
      generateAncestorStep(*it, dummy, result);
      break;
    }
    case XQStep::ANCESTOR_OR_SELF: {
      generateAncestorOrSelfStep(*it, dummy, result);
      break;
    }
    case XQStep::FOLLOWING: {
      generateFollowingStep(*it, dummy, result);
      break;
    }
    case XQStep::PRECEDING: {
      generatePrecedingStep(*it, dummy, result);
      break;
    }
    case XQStep::PRECEDING_SIBLING:
    case XQStep::FOLLOWING_SIBLING: {
      generateSiblingStep(*it, dummy, result);
      break;
    }
    case XQStep::ATTRIBUTE:
    case XQStep::CHILD:
    case XQStep::DESCENDANT: {
      generateBuiltInStep(*it, dummy, result);
      break;
    }
    case XQStep::NAMESPACE:
    default: {
      // Do nothing
      break;
    }
    }
  }

  push(result);
  return item;
}

void QueryPathTreeGenerator::generateBuiltInStep(QueryPathNode *target, QueryPathNode &node, PathResult &result)
{
  switch(target->getType()) {
  case QueryPathNode::ATTRIBUTE: {
    // Does nothing
    break;
  }
  case QueryPathNode::CHILD:
  case QueryPathNode::DESCENDANT:
  case QueryPathNode::ROOT: {
    QueryPathNode *newNode = createQueryPathNode(node.getNodeTest(), node.getType());
    result.join(target->appendChild(newNode));
    break;
  }
  default: break;
  }
}

void QueryPathTreeGenerator::generateParentStep(QueryPathNode *target, QueryPathNode &node, PathResult &result)
{
  QueryPathNode *parent = target->getParent();
  if(parent == 0) return;

  switch(target->getType()) {
  case QueryPathNode::ATTRIBUTE:
  case QueryPathNode::CHILD: {
    if(parent->isSubsetOf(&node)) {
      result.join(parent);
    } else if(node.isSubsetOf(parent)) {
      QueryPathNode *parent_parent = parent->getParent();
      if(parent_parent != 0) {
        QueryPathNode *newNode = createQueryPathNode(node.getNodeTest(), parent->getType());
        result.join(parent_parent->appendChild(newNode));
      }
    }
    break;
  }
  case QueryPathNode::DESCENDANT: {
    if(parent->isSubsetOf(&node)) {
      result.join(parent);
    }

    QueryPathNode *newNode = createQueryPathNode(node.getNodeTest(), QueryPathNode::DESCENDANT);
    result.join(parent->appendChild(newNode));
    break;
  }
  case QueryPathNode::ROOT: {
    // There is no parent
    break;
  }
  default: break;
  }
}

void QueryPathTreeGenerator::generateSelfStep(QueryPathNode *target, QueryPathNode &node, PathResult &result)
{
  if(target->isSubsetOf(&node)) {
    result.join(target);
  } else if(node.isSubsetOf(target)) {
    QueryPathNode *parent = target->getParent();
    if(parent != 0) {
      QueryPathNode *newNode = createQueryPathNode(node.getNodeTest(), target->getType());
      result.join(parent->appendChild(newNode));
    }
  }
}

void QueryPathTreeGenerator::generateAncestorStep(QueryPathNode *target, QueryPathNode &node, PathResult &result)
{
  QueryPathNode *parent = target->getParent();
  if(parent == 0) return;

  switch(target->getType()) {
  case QueryPathNode::ATTRIBUTE:
  case QueryPathNode::CHILD: {
    if(parent->isSubsetOf(&node)) {
      result.join(parent);
    } else if(node.isSubsetOf(parent)) {
      QueryPathNode *parent_parent = parent->getParent();
      if(parent_parent != 0) {
        QueryPathNode *newNode = createQueryPathNode(node.getNodeTest(), parent->getType());
        result.join(parent_parent->appendChild(newNode));
      }
    }
    generateAncestorStep(parent, node, result);
    break;
  }
  case QueryPathNode::DESCENDANT: {
    if(parent->isSubsetOf(&node)) {
      result.join(parent);
    }

    QueryPathNode *newNode = createQueryPathNode(node.getNodeTest(), QueryPathNode::DESCENDANT);
    result.join(parent->appendChild(newNode));

    generateAncestorStep(parent, node, result);
    break;
  }
  case QueryPathNode::ROOT: {
    // Has no ancestors
    break;
  }
  default: break;
  }
}

void QueryPathTreeGenerator::generateAncestorOrSelfStep(QueryPathNode *target, QueryPathNode &node, PathResult &result)
{
  generateSelfStep(target, node, result);
  generateAncestorStep(target, node, result);
}

void QueryPathTreeGenerator::generateFollowingStep(QueryPathNode *target, QueryPathNode &node, PathResult &result)
{
  QueryPathNode *newNode = createQueryPathNode(node.getNodeTest(), QueryPathNode::DESCENDANT);
  result.join(target->getRoot()->appendChild(newNode));
}

void QueryPathTreeGenerator::generatePrecedingStep(QueryPathNode *target, QueryPathNode &node, PathResult &result)
{
  QueryPathNode *root = target->getRoot();

  if(root->isSubsetOf(&node)) {
    result.join(root);
  }

  QueryPathNode *newNode = createQueryPathNode(node.getNodeTest(), QueryPathNode::DESCENDANT);
  result.join(root->appendChild(newNode));
}

void QueryPathTreeGenerator::generateSiblingStep(QueryPathNode *target, QueryPathNode &node, PathResult &result)
{
  QueryPathNode *parent = target->getParent();
  if(parent == 0) return;

  switch(target->getType()) {
  case QueryPathNode::CHILD:
  case QueryPathNode::DESCENDANT: {
    QueryPathNode *newNode = createQueryPathNode(node.getNodeTest(), target->getType());
    result.join(parent->appendChild(newNode));
    break;
  }
  case QueryPathNode::ATTRIBUTE:
  case QueryPathNode::ROOT: {
    // Have no siblings
    break;
  }
  default: break;
  }
}

ASTNode *QueryPathTreeGenerator::optimizeNav(XQNav *item)
{
  PathResult result = getCurrentContext();

  bool setContext = false;
  XQNav::Steps &args = const_cast<XQNav::Steps &>(item->getSteps());
  for(XQNav::Steps::iterator i = args.begin(); i != args.end(); ++i) {
    if(setContext) {
      varStore_.addScope(VarStore::MyScope::LOGICAL_BLOCK_SCOPE);
      setCurrentContext(result);
    }

    result = generate(i->step);

    if(setContext) {
      delete varStore_.popScope();
    }
    setContext = true;
  }

  push(result);
  return item;
}

static const XMLCh *resolveURIArg(const ASTNode *arg, DynamicContext *context, bool &defaultCollection)
{
  if(!arg->isConstant()) return 0;

  Item::Ptr item = arg->createResult(context)->next(context);
  if(item.isNull()) {
    defaultCollection = true;
    return 0;
  }

  const XMLCh *uriArg = item->asString(context);
  if(uriArg == 0) return 0;

  const XMLCh *baseUri = context->getBaseURI();

  try {
    XMLUri uri;
    if(baseUri != 0 && *baseUri != 0) {
      XMLUri base(baseUri);
      uri = XMLUri(&base, uriArg);
    } else {
      uri = XMLUri(uriArg);
    }
    return context->getMemoryManager()->getPooledString(uri.getUriText());
  }
  catch(...)  {
    // don't throw; it's just that the URI isn't valid.
  }

  return 0;
}

ASTNode *QueryPathTreeGenerator::optimizeFunction(XQFunction *item)
{
  VectorOfASTNodes &args = const_cast<VectorOfASTNodes &>(item->getArguments());
  const XMLCh *uri = item->getFunctionURI();
  const XMLCh *name = item->getFunctionName();

  PathResult result;

  if(uri == XQFunction::XMLChFunctionURI) {

    // fn:root()
    if(name == FunctionRoot::name) {
      const PathResult &currentContext = getCurrentContext();

      // root() gets the root of it's argument, if it has one.
      // Otherwise it gets the root of the current context
      PathResult ret;
      if(args.empty()) {
        ret = currentContext;
      }
      else {
        ret = generate(args[0]);
      }

      for(QueryPathNode::Vector::iterator it = ret.returnPaths.begin();
          it != ret.returnPaths.end(); ++it) {
        result.join((*it)->getRoot());
      }
    }

    else if(name == FunctionDoc::name) {
      ASTNode *arg = args[0];
      generate(arg);

      QueryPathNode *root = ((FunctionDoc*)item)->getQueryPathTree();
      if(!root) {
        // Check criteria for safe document projection
        if(!arg->isConstant()) context_->setProjection(false);

        // Check to see if this document URI has already been accessed
        bool defaultCollection = false;
        const XMLCh *uriArg = resolveURIArg(arg, context_, defaultCollection);
        if(uriArg != 0) root = projectionMap_[uriArg];

        // If we've not found a root QueryPathNode, create a new one
        if(!root) {
          NodeTest *nt = createNodeTest(Node::document_string);
          root = createQueryPathNode(nt, QueryPathNode::ROOT);
          if(uriArg != 0) projectionMap_[uriArg] = root;
        }

        ((FunctionDoc*)item)->setQueryPathTree(root);
      }

      result.join(root);
    }

    else if(name == FunctionDocument::name) {
      ASTNode *arg = args[0];
      generate(arg);

      QueryPathNode *root = ((FunctionDoc*)item)->getQueryPathTree();
      if(!root) {
        // Check criteria for safe document projection
        if(!arg->isConstant() || args.size() > 1) context_->setProjection(false);

        // Check to see if this document URI has already been accessed
        bool defaultCollection = false;
        const XMLCh *uriArg = resolveURIArg(arg, context_, defaultCollection);
        if(uriArg != 0) root = projectionMap_[uriArg];

        // If we've not found a root QueryPathNode, create a new one
        if(!root) {
          NodeTest *nt = createNodeTest(Node::document_string);
          root = createQueryPathNode(nt, QueryPathNode::ROOT);
          if(uriArg != 0) projectionMap_[uriArg] = root;
        }

        ((FunctionDocument*)item)->setQueryPathTree(root);
      }

      result.join(root);
    }

    else if(name == FunctionCollection::name) {
      ASTNode *arg = 0;
      if(!args.empty()) {
        arg = args[0];
        generate(arg);
      }

      QueryPathNode *root = ((FunctionCollection*)item)->getQueryPathTree();
      if(!root) {
        const XMLCh *uriArg = 0;
        bool defaultCollection = false;
        if(arg != 0) {
          // Check criteria for safe document projection
          if(!arg->isConstant()) context_->setProjection(false);

          // Check to see if this document URI has already been accessed
          uriArg = resolveURIArg(arg, context_, defaultCollection);
        }
        else defaultCollection = true;

        if(uriArg != 0 || defaultCollection) root = projectionMap_[uriArg];

        // If we've not found a root QueryPathNode, create a new one
        if(!root) {
          NodeTest *nt = createNodeTest(Node::document_string);
          root = createQueryPathNode(nt, QueryPathNode::ROOT);
          if(uriArg != 0 || defaultCollection) projectionMap_[uriArg] = root;
        }

        ((FunctionCollection*)item)->setQueryPathTree(root);
      }

      result.join(root);
    }

    else if(name == FunctionPut::name) {
      generate(args[0]).markSubtreeResult();
      if(args.size() > 1)
        generate(args[1]).markSubtreeValue();
    }
    else if(name == FunctionTrace::name) {
      // trace() returns it's first argument, and prints the string
      // value of it's second argument.

      result = generate(args[0]);
      generate(args[1]);
    }
    else if(name == FunctionHead::name ||
            name == FunctionTail::name ||
            name == FunctionUnordered::name) {
      // return their argument
      result = generate(args[0]);
    }
    else if(name == FunctionId::name ||
            name == FunctionIdref::name) {
      PathResult nodes = generate(args[1]);

      // We don't handle these, so we mark the trees as unoptimisable
      nodes.markRoot();

      generate(args[0]);
    }
    else if(name == FunctionLang::name) {
      generate(args[0]);

      PathResult nodeResult = generate(args[1]);

      // We want all the xml:lang attributes from all our lookup node's ancestors

      NodeTest *nt = createNodeTest(Node::element_string);
      QueryPathNode dummy(nt, QueryPathNode::CHILD, mm_);
      PathResult ancestorResult;
      QueryPathNode::Vector::const_iterator it;
      for(it = nodeResult.returnPaths.begin(); it != nodeResult.returnPaths.end(); ++it) {
        generateAncestorOrSelfStep(*it, dummy, ancestorResult);
      }

      NodeTest *nt2 = createNodeTest(Node::attribute_string, XMLUni::fgXMLURIName, mm_->getPooledString("lang"));
      QueryPathNode dummy2(nt2, QueryPathNode::ATTRIBUTE, mm_);
      PathResult attrResult;
      for(it = ancestorResult.returnPaths.begin(); it != ancestorResult.returnPaths.end(); ++it) {
        generateBuiltInStep(*it, dummy2, attrResult);
      }

      attrResult.markSubtreeValue();
    }
    else if(name == FunctionBaseURI::name) {
      PathResult nodeResult = generate(args[0]);

      // We want all the xml:base attributes from all our lookup node's ancestors

      NodeTest *nt = createNodeTest(Node::element_string);
      QueryPathNode dummy(nt, QueryPathNode::CHILD, mm_);
      PathResult ancestorResult;
      QueryPathNode::Vector::const_iterator it;
      for(it = nodeResult.returnPaths.begin(); it != nodeResult.returnPaths.end(); ++it) {
        generateAncestorOrSelfStep(*it, dummy, ancestorResult);
      }

      NodeTest *nt2 = createNodeTest(Node::attribute_string, XMLUni::fgXMLURIName, mm_->getPooledString("base"));
      QueryPathNode dummy2(nt2, QueryPathNode::ATTRIBUTE, mm_);
      PathResult attrResult;
      for(it = ancestorResult.returnPaths.begin(); it != ancestorResult.returnPaths.end(); ++it) {
        generateBuiltInStep(*it, dummy2, attrResult);
      }

      attrResult.markSubtreeValue();
    }

    // These implicitly use the value of the current context if they have no parameters
    else if((name == FunctionStringLength::name ||
             name == FunctionNormalizeSpace::name ||
             name == FunctionNumber::name ||
             name == FunctionString::name) &&
            args.empty()) {
      const PathResult &currentContext = getCurrentContext();
      currentContext.markSubtreeValue();
    }

    else if(name == FunctionString::name) {
      generate(args[0]).markSubtreeValue();
    }

    else {
      for(VectorOfASTNodes::iterator i = args.begin(); i != args.end(); ++i) {
        generate(*i);
      }
    }

  }

  else if(uri == XQillaFunction::XMLChFunctionURI) {
    if(name == FunctionParseXML::name) {
      // Returns a document, much the same as fn:doc()
      generate(args[0]);

      QueryPathNode *root = ((FunctionParseXML*)item)->getQueryPathTree();
      if(!root) {
        NodeTest *nt = createNodeTest(Node::document_string);
        root = createQueryPathNode(nt, QueryPathNode::ROOT);
        ((FunctionParseXML*)item)->setQueryPathTree(root);
      }

      result.join(root);
    }

    else if(name == FunctionParseJSON::name) {
      // Returns a sequence of elements
      generate(args[0]);

      QueryPathNode *root = ((FunctionParseJSON*)item)->getQueryPathTree();
      if(!root) {
        NodeTest *nt = createNodeTest(Node::element_string);
        root = createQueryPathNode(nt, QueryPathNode::CHILD);
        ((FunctionParseJSON*)item)->setQueryPathTree(root);
      }

      result.join(root);
    }

    else if(name == FunctionExplain::name) {
      // Returns a sequence of elements
      generate(args[0]);

      QueryPathNode *root = ((FunctionExplain*)item)->getQueryPathTree();
      if(!root) {
        NodeTest *nt = createNodeTest(Node::element_string);
        root = createQueryPathNode(nt, QueryPathNode::CHILD);
        ((FunctionExplain*)item)->setQueryPathTree(root);
      }

      result.join(root);
    }

    else {
      // We don't know about this function, assume it will only need the descendants of it's node arguments
      for(VectorOfASTNodes::iterator i = args.begin(); i != args.end(); ++i) {
        generate(*i).markSubtreeResult();
      }
    }
  }

  else {
    // We don't know about this function, assume it will only need the descendants of it's node arguments
    for(VectorOfASTNodes::iterator i = args.begin(); i != args.end(); ++i) {
      generate(*i).markSubtreeResult();
    }
  }

  push(result);
  return item;
}

class ArgHolder {
public:
  ArgHolder(const XMLCh *u, const XMLCh *n, const QueryPathTreeGenerator::PathResult &v)
    : uri(u), name(n), value(v) {}
  
  const XMLCh *uri, *name;
  QueryPathTreeGenerator::PathResult value;
};

ASTNode *QueryPathTreeGenerator::optimizeUserFunction(XQUserFunctionInstance *item)
{
  PathResult result;

  VectorOfASTNodes &args = const_cast<VectorOfASTNodes &>(item->getArguments());
  const XQUserFunction *functionDef = item->getFunctionDefinition();

  // Evaluate the arguments in the current scope, declare them in the function's scope
  vector<ArgHolder> evaluatedArgs;
  if(functionDef->getSignature()->argSpecs) {
    ArgumentSpecs::const_iterator binding = functionDef->getSignature()->argSpecs->begin();
    for(VectorOfASTNodes::iterator arg = args.begin(); arg != args.end() && binding != functionDef->getSignature()->argSpecs->end(); ++arg, ++binding) {

      PathResult r = generate(*arg);

      if((*binding)->isUsed()) {
        evaluatedArgs.push_back(ArgHolder((*binding)->getURI(), (*binding)->getName(), r));
      }
    }
  }

  if(functionDef->getFunctionBody() != 0) {
    // Check to see if this user function is recursing into a user function
    // that has already been called.
    pair<set<const ASTNode*>::iterator, bool> ins = userFunctionStack_.insert(functionDef->getFunctionBody());
    if(ins.second) {
      varStore_.addScope(VarStore::MyScope::LOCAL_SCOPE);

      for(vector<ArgHolder>::iterator it = evaluatedArgs.begin(); it != evaluatedArgs.end(); ++it) {
        setVariable(it->uri, it->name, it->value);
      }

      PathResult ret = generate(const_cast<ASTNode *>(functionDef->getFunctionBody()));
      result.join(ret);

      delete varStore_.popScope();

      userFunctionStack_.erase(ins.first);

      push(result);
      return item;
    }
  }

  // All we can really do if it is recursive is to mark the entire tree as needed
  // TBD see if we can handle recursive user functions better - jpcs
  for(vector<ArgHolder>::iterator it = evaluatedArgs.begin(); it != evaluatedArgs.end(); ++it) {
    it->value.markRoot();
  }

  push(result);
  return item;
}

ASTNode *QueryPathTreeGenerator::optimizeOperator(XQOperator *item)
{
  PathResult result;

  VectorOfASTNodes &args = const_cast<VectorOfASTNodes &>(item->getArguments());
  const XMLCh *name = item->getOperatorName();

  if(name == Union::name) {
    PathResult ret = generate(args[0]);
    result.join(ret);

    ret = generate(args[1]);
    result.join(ret);
  }

  else if(name == Intersect::name) {
    PathResult ret = generate(args[0]);
    result.join(ret);

    ret = generate(args[1]);
    result.join(ret);
  }

  else if(name == Except::name) {
    result = generate(args[0]);
    generate(args[1]);
  }

  else {
    for(VectorOfASTNodes::iterator i = args.begin(); i != args.end(); ++i) {
      generate(*i);
    }
  }

  push(result);
  return item;
}

ASTNode *QueryPathTreeGenerator::optimizeSequence(XQSequence *item)
{
  VectorOfASTNodes &args = const_cast<VectorOfASTNodes &>(item->getChildren());
  PathResult result;
  for(VectorOfASTNodes::iterator i = args.begin(); i != args.end(); ++i) {
    PathResult ret = generate(*i);
    result.join(ret);
  }

  push(result);
  return item;
}

ASTNode *QueryPathTreeGenerator::optimizeVariable(XQVariable *item)
{
  VarStoreRef *ref = varStore_.getVar(item->getURI(), item->getName());

  PathResult result;
  if(ref != 0) {
    result = ref->getValue();
  } else {
    // Undeclared global variable - we'll declare it here
    createAnyNodeResult(result);
    varStore_.setGlobalVar(item->getURI(), item->getName(), result);
  }

  push(result);
  return item;
}

ASTNode *QueryPathTreeGenerator::optimizeIf(XQIf *item)
{
  PathResult result;

  PathResult test = generate(const_cast<ASTNode *>(item->getTest()));

  PathResult ret = generate(const_cast<ASTNode *>(item->getWhenTrue()));
  result.join(ret);

  ret = generate(const_cast<ASTNode *>(item->getWhenFalse()));
  result.join(ret);

  push(result);
  return item;
}

TupleNode *QueryPathTreeGenerator::optimizeForTuple(ForTuple *item)
{
  optimizeTupleNode(const_cast<TupleNode*>(item->getParent()));

  PathResult r = generate(item->getExpression());

  if(item->getVarName()) {
    setVariable(item->getVarURI(), item->getVarName(), r);
  }
  if(item->getPosName()) {
    setVariable(item->getPosURI(), item->getPosName(), PathResult());
  }

  return item;
}

TupleNode *QueryPathTreeGenerator::optimizeLetTuple(LetTuple *item)
{
  optimizeTupleNode(const_cast<TupleNode*>(item->getParent()));

  PathResult r = generate(item->getExpression());

  if(item->getVarName()) {
    setVariable(item->getVarURI(), item->getVarName(), r);
  }

  return item;
}

TupleNode *QueryPathTreeGenerator::optimizeWhereTuple(WhereTuple *item)
{
  optimizeTupleNode(const_cast<TupleNode*>(item->getParent()));

  generate(item->getExpression());

  return item;
}

TupleNode *QueryPathTreeGenerator::optimizeCountTuple(CountTuple *item)
{
  optimizeTupleNode(const_cast<TupleNode*>(item->getParent()));
  return item;
}

TupleNode *QueryPathTreeGenerator::optimizeOrderByTuple(OrderByTuple *item)
{
  optimizeTupleNode(const_cast<TupleNode*>(item->getParent()));

  generate(item->getExpression());

  return item;
}

TupleNode *QueryPathTreeGenerator::optimizeTupleDebugHook(TupleDebugHook *item)
{
  optimizeTupleNode(const_cast<TupleNode*>(item->getParent()));
  return item;
}

ASTNode *QueryPathTreeGenerator::optimizeReturn(XQReturn *item)
{
  varStore_.addScope(VarStore::MyScope::LOGICAL_BLOCK_SCOPE);

  optimizeTupleNode(const_cast<TupleNode*>(item->getParent()));
  PathResult result = generate(item->getExpression());
  
  delete varStore_.popScope();

  push(result);
  return item;
}

ASTNode *QueryPathTreeGenerator::optimizeQuantified(XQQuantified *item)
{
  varStore_.addScope(VarStore::MyScope::LOGICAL_BLOCK_SCOPE);

  optimizeTupleNode(const_cast<TupleNode*>(item->getParent()));
  generate(item->getExpression());
  
  delete varStore_.popScope();

  push(PathResult());
  return item;
}

ASTNode *QueryPathTreeGenerator::optimizeTypeswitch(XQTypeswitch *item)
{
  PathResult result;

  PathResult expr = generate(const_cast<ASTNode *>(item->getExpression()));

  XQTypeswitch::Cases *clauses = const_cast<XQTypeswitch::Cases *>(item->getCases());
  for(XQTypeswitch::Cases::iterator i = clauses->begin(); i != clauses->end(); ++i) {

    if((*i)->isVariableUsed()) {
      varStore_.addScope(VarStore::MyScope::LOGICAL_BLOCK_SCOPE);
      setVariable((*i)->getURI(), (*i)->getName(), expr);
    }

    PathResult ret = generate((*i)->getExpression());
    result.join(ret);

    if((*i)->isVariableUsed()) {
      delete varStore_.popScope();
    }
  }

  XQTypeswitch::Case *def = const_cast<XQTypeswitch::Case *>(item->getDefaultCase());
  if(def->isVariableUsed()) {
    varStore_.addScope(VarStore::MyScope::LOGICAL_BLOCK_SCOPE);
    setVariable(def->getURI(), def->getName(), expr);
  }

  PathResult ret = generate(def->getExpression());
  result.join(ret);

  if(def->isVariableUsed()) {
    delete varStore_.popScope();
  }

  push(result);
  return item;
}

ASTNode *QueryPathTreeGenerator::optimizeDOMConstructor(XQDOMConstructor *item)
{
  PathResult result;

  if(item->getName()) {
    generate(const_cast<ASTNode *>(item->getName()));
  }

  VectorOfASTNodes *attrs = const_cast<VectorOfASTNodes *>(item->getAttributes());
  if(attrs) {
    for(VectorOfASTNodes::iterator i = attrs->begin(); i != attrs->end(); ++i) {
      generate(*i).markSubtreeResult();
    }
  }

  VectorOfASTNodes *children = const_cast<VectorOfASTNodes *>(item->getChildren());
  if(children) {
    for(VectorOfASTNodes::iterator j = children->begin(); j != children->end(); ++j) {
      generate(*j).markSubtreeResult();
    }
  }

  if(item->getValue()) {
    generate(const_cast<ASTNode *>(item->getValue()));
  }

  if(!item->getQueryPathTree()) {
    if(item->getNodeType() == Node::document_string) {
      item->setQueryPathTree(createQueryPathNode(createNodeTest(Node::document_string), QueryPathNode::ROOT));
    }
    else if(item->getNodeType() == Node::element_string) {
      item->setQueryPathTree(createQueryPathNode(createNodeTest(Node::element_string), QueryPathNode::CHILD));
    }
    else if(item->getNodeType() == Node::attribute_string) {
      item->setQueryPathTree(createQueryPathNode(createNodeTest(Node::attribute_string), QueryPathNode::ATTRIBUTE));
    }
    else {
      item->setQueryPathTree(createQueryPathNode(createNodeTest((XMLCh*)0), QueryPathNode::CHILD));
    }
  }


  result.join(item->getQueryPathTree());
  push(result);
  return item;
}

ASTNode *QueryPathTreeGenerator::optimizeSimpleContent(XQSimpleContent *item)
{
  VectorOfASTNodes *children = const_cast<VectorOfASTNodes *>(item->getChildren());
  if(children) {
    for(VectorOfASTNodes::iterator j = children->begin(); j != children->end(); ++j) {
      generate(*j);
    }
  }

  push(PathResult());
  return item;
}

QueryPathTreeGenerator::PathResult QueryPathTreeGenerator::copyNodes(const PathResult &r)
{
  // Emulate copying the nodes
  PathResult copyResult;

  r.markSubtreeResult();

  NodeTest *nt = createNodeTest(Node::document_string);
  QueryPathNode *root = createQueryPathNode(nt, QueryPathNode::ROOT);

  QueryPathNode::Vector::const_iterator j = r.returnPaths.begin();
  for(; j != r.returnPaths.end(); ++j) {
    switch((*j)->getType()) {
    case QueryPathNode::ATTRIBUTE: {
      QueryPathNode *pisn = createQueryPathNode(createNodeTest(Node::element_string),
                                                QueryPathNode::DESCENDANT);
      pisn = root->appendChild(pisn);
      copyResult.join(pisn->appendChild(createQueryPathNode((*j)->getNodeTest(),
                                                            QueryPathNode::ATTRIBUTE)));
      break;
    }
    case QueryPathNode::CHILD:
    case QueryPathNode::DESCENDANT: {
      copyResult.join(root->appendChild(createQueryPathNode((*j)->getNodeTest(),
                                                            QueryPathNode::DESCENDANT)));
      break;
    }
    case QueryPathNode::ROOT: {
      copyResult.join(root);
      break;
    }
    default: break;
    }
  }

  return copyResult;
}

ASTNode *QueryPathTreeGenerator::optimizeContentSequence(XQContentSequence *item)
{
  push(copyNodes(generate(const_cast<ASTNode *>(item->getExpression()))));
  return item;
}

ASTNode *QueryPathTreeGenerator::optimizeDirectName(XQDirectName *item)
{
  push(PathResult());
  return item;
}

ASTNode *QueryPathTreeGenerator::optimizeContextItem(XQContextItem *item)
{
  push(getCurrentContext());
  return item;
}

ASTNode *QueryPathTreeGenerator::optimizeLiteral(XQLiteral *item)
{
  push(PathResult());
  return item;
}

ASTNode *QueryPathTreeGenerator::optimizeQNameLiteral(XQQNameLiteral *item)
{
  push(PathResult());
  return item;
}

ASTNode *QueryPathTreeGenerator::optimizeNumericLiteral(XQNumericLiteral *item)
{
  push(PathResult());
  return item;
}

ASTNode *QueryPathTreeGenerator::optimizeFunctionRef(XQFunctionRef *item)
{
  push(PathResult());
  return item;
}

ASTNode *QueryPathTreeGenerator::optimizeAtomize(XQAtomize *item)
{
  generate(const_cast<ASTNode *>(item->getExpression())).markSubtreeValue();
  push(PathResult());
  return item;
}

ASTNode *QueryPathTreeGenerator::optimizeEffectiveBooleanValue(XQEffectiveBooleanValue *item)
{
  generate(const_cast<ASTNode *>(item->getExpression()));
  push(PathResult());
  return item;
}

ASTNode *QueryPathTreeGenerator::optimizeMap(XQMap *item)
{
  PathResult result = generate(item->getArg1());

  varStore_.addScope(VarStore::MyScope::LOGICAL_BLOCK_SCOPE);
  if(item->getName() == 0) {
    setCurrentContext(result);
  } else {
    setVariable(item->getURI(), item->getName(), result);
  }

  result = generate(item->getArg2());

  delete varStore_.popScope();

  push(result);
  return item;
}

ASTNode *QueryPathTreeGenerator::optimizeValidate(XQValidate *item)
{
  push(copyNodes(generate(const_cast<ASTNode *>(item->getExpression()))));
  return item;
}

ASTNode *QueryPathTreeGenerator::optimizePredicate(XQPredicate *item)
{
  PathResult result = generate(const_cast<ASTNode *>(item->getExpression()));

  varStore_.addScope(VarStore::MyScope::LOGICAL_BLOCK_SCOPE);
  setCurrentContext(result);

  generate(const_cast<ASTNode *>(item->getPredicate()));

  delete varStore_.popScope();
  push(result);
  return item;
}

ASTNode *QueryPathTreeGenerator::optimizeUDelete(UDelete *item)
{
  // Any tree that is updated can't be document projected
  generate(const_cast<ASTNode *>(item->getExpression())).markRoot();
  push(PathResult());
  return item;
}

ASTNode *QueryPathTreeGenerator::optimizeURename(URename *item)
{
  // Any tree that is updated can't be document projected
  generate(const_cast<ASTNode *>(item->getTarget())).markRoot();
  generate(const_cast<ASTNode *>(item->getName()));
  push(PathResult());
  return item;
}

ASTNode *QueryPathTreeGenerator::optimizeUReplace(UReplace *item)
{
  // Any tree that is updated can't be document projected
  generate(const_cast<ASTNode *>(item->getTarget())).markRoot();
  generate(const_cast<ASTNode *>(item->getExpression())).markSubtreeResult();
  push(PathResult());
  return item;
}

ASTNode *QueryPathTreeGenerator::optimizeUReplaceValueOf(UReplaceValueOf *item)
{
  // Any tree that is updated can't be document projected
  generate(const_cast<ASTNode *>(item->getTarget())).markRoot();
  generate(const_cast<ASTNode *>(item->getExpression()));
  push(PathResult());
  return item;
}

ASTNode *QueryPathTreeGenerator::optimizeUInsertAsFirst(UInsertAsFirst *item)
{
  // Any tree that is updated can't be document projected
  generate(const_cast<ASTNode *>(item->getTarget())).markRoot();
  generate(const_cast<ASTNode *>(item->getSource())).markSubtreeResult();
  push(PathResult());
  return item;
}

ASTNode *QueryPathTreeGenerator::optimizeUInsertAsLast(UInsertAsLast *item)
{
  // Any tree that is updated can't be document projected
  generate(const_cast<ASTNode *>(item->getTarget())).markRoot();
  generate(const_cast<ASTNode *>(item->getSource())).markSubtreeResult();
  push(PathResult());
  return item;
}

ASTNode *QueryPathTreeGenerator::optimizeUInsertInto(UInsertInto *item)
{
  // Any tree that is updated can't be document projected
  generate(const_cast<ASTNode *>(item->getTarget())).markRoot();
  generate(const_cast<ASTNode *>(item->getSource())).markSubtreeResult();
  push(PathResult());
  return item;
}

ASTNode *QueryPathTreeGenerator::optimizeUInsertAfter(UInsertAfter *item)
{
  // Any tree that is updated can't be document projected
  generate(const_cast<ASTNode *>(item->getTarget())).markRoot();
  generate(const_cast<ASTNode *>(item->getSource())).markSubtreeResult();
  push(PathResult());
  return item;
}

ASTNode *QueryPathTreeGenerator::optimizeUInsertBefore(UInsertBefore *item)
{
  // Any tree that is updated can't be document projected
  generate(const_cast<ASTNode *>(item->getTarget())).markRoot();
  generate(const_cast<ASTNode *>(item->getSource())).markSubtreeResult();
  push(PathResult());
  return item;
}

ASTNode *QueryPathTreeGenerator::optimizeUTransform(UTransform *item)
{
  varStore_.addScope(VarStore::MyScope::LOGICAL_BLOCK_SCOPE);

  VectorOfCopyBinding *bindings = const_cast<VectorOfCopyBinding*>(item->getBindings());
  for(VectorOfCopyBinding::iterator i = bindings->begin(); i != bindings->end(); ++i) {
    setVariable((*i)->uri_, (*i)->name_, generate((*i)->expr_));
  }

  generate(const_cast<ASTNode *>(item->getModifyExpr()));
  PathResult result = generate(const_cast<ASTNode *>(item->getReturnExpr()));

  delete varStore_.popScope();

  push(result);
  return item;
}

ASTNode *QueryPathTreeGenerator::optimizeFunctionCall(XQFunctionCall *item)
{
  // Shouldn't happen
  VectorOfASTNodes *args = const_cast<VectorOfASTNodes*>(item->getArguments());
  for(VectorOfASTNodes::iterator i = args->begin(); i != args->end(); ++i) {
    generate(*i);
  }
  push(PathResult());
  return item;
}

ASTNode *QueryPathTreeGenerator::optimizeFTContains(FTContains *item)
{
  generate(item->getArgument());
  optimizeFTSelection(item->getSelection());
  if(item->getIgnore()) generate(item->getIgnore());
  push(PathResult());
  return item;
}

ASTNode *QueryPathTreeGenerator::optimizeAnalyzeString(XQAnalyzeString *item)
{
  PathResult result;

  generate(item->getExpression());
  generate(item->getRegex());
  if(item->getFlags())
    generate(item->getFlags());

  varStore_.addScope(VarStore::MyScope::LOGICAL_BLOCK_SCOPE);
  setCurrentContext(PathResult());

  result.join(generate(item->getMatch()));
  result.join(generate(item->getNonMatch()));

  delete varStore_.popScope();

  push(result);
  return item;
}

ASTNode *QueryPathTreeGenerator::optimizeCopyOf(XQCopyOf *item)
{
  push(copyNodes(generate(const_cast<ASTNode *>(item->getExpression()))));
  return item;
}

ASTNode *QueryPathTreeGenerator::optimizeCopy(XQCopy *item)
{
  VectorOfASTNodes &children = const_cast<VectorOfASTNodes&>(item->getChildren());
  for(VectorOfASTNodes::iterator j = children.begin(); j != children.end(); ++j) {
    generate(*j).markSubtreeResult();
  }

  push(copyNodes(generate(const_cast<ASTNode *>(item->getExpression()))));
  return item;
}

ASTNode *QueryPathTreeGenerator::optimizeCallTemplate(XQCallTemplate *item)
{
  // TBD Could be better - jpcs
  context_->setProjection(false);

  TemplateArguments *args = item->getArguments();
  if(args != 0) {
    for(TemplateArguments::iterator i = args->begin(); i != args->end(); ++i) {
      generate((*i)->value).markRoot();
    }
  }
  if(item->getASTName())
    generate(item->getASTName());
  push(PathResult());
  return item;
}

ASTNode *QueryPathTreeGenerator::optimizeApplyTemplates(XQApplyTemplates *item)
{
  // TBD Could be better - jpcs
  context_->setProjection(false);

  PathResult res = generate(const_cast<ASTNode *>(item->getExpression()));

//   varStore_.addScope(VarStore::MyScope::LOGICAL_BLOCK_SCOPE);
//   setCurrentContext(res);

//   delete varStore_.popScope();

  TemplateArguments *args = item->getArguments();
  if(args != 0) {
    for(TemplateArguments::iterator i = args->begin(); i != args->end(); ++i) {
      generate((*i)->value).markRoot();
    }
  }

  push(PathResult());
  return item;
}

ASTNode *QueryPathTreeGenerator::optimizeInlineFunction(XQInlineFunction *item)
{
  if(item->getUserFunction())
    optimizeFunctionDef(item->getUserFunction());

  const ArgumentSpecs *params = item->getSignature()->argSpecs;
  if(params) {
    varStore_.addScope(VarStore::MyScope::LOCAL_SCOPE);

    ArgumentSpecs::const_iterator it = params->begin();
    unsigned int c = 0;
    for(; it != params->end(); ++it, ++c) {
      PathResult paramRes;
      if((*it)->getStaticAnalysis().getStaticType().containsType(StaticType::NODE_TYPE))
        createAnyNodeResult(paramRes);
      setVariable((*it)->getURI(), (*it)->getName(), paramRes);
    }
  }

  generate(item->getInstance()).markSubtreeResult();

  if(params) {
    delete varStore_.popScope();
  }

  push(PathResult());
  return item;
}

ASTNode *QueryPathTreeGenerator::optimizeFunctionDeref(XQFunctionDeref *item)
{
  // TBD Could be better - jpcs
  context_->setProjection(false);

  // We could trace which function instances could possibly be called here
  // and treat the function dereference as the union of all the functions
  // it could call.

  generate(item->getExpression());

  VectorOfASTNodes *args = const_cast<VectorOfASTNodes*>(item->getArguments());
  if(args) {
    for(VectorOfASTNodes::iterator i = args->begin(); i != args->end(); ++i) {
      generate(*i).markRoot();
    }
  }

  push(PathResult());
  return item;
}

ASTNode *QueryPathTreeGenerator::optimizePartialApply(XQPartialApply *item)
{
  // TBD Could be better - jpcs
  context_->setProjection(false);

  // We could trace which function instances could possibly be called here
  // and treat the function dereference as the union of all the functions
  // it could call.

  generate(item->getExpression());

  VectorOfASTNodes *args = const_cast<VectorOfASTNodes*>(item->getArguments());
  if(args) {
    for(VectorOfASTNodes::iterator i = args->begin(); i != args->end(); ++i) {
      if(*i != 0)
        generate(*i).markRoot();
    }
  }

  push(PathResult());
  return item;
}

#define UNCHANGED_FULL(methodname, classname) \
ASTNode *QueryPathTreeGenerator::optimize ## methodname (classname *item) \
{ \
  return ASTVisitor::optimize ## methodname (item); \
}

#define UNCHANGED_XQ(name) UNCHANGED_FULL(name, XQ ## name)
#define UNCHANGED(name) UNCHANGED_FULL(name, name)

UNCHANGED_XQ(NameExpression)
UNCHANGED_XQ(CastAs)
UNCHANGED_XQ(TreatAs)
UNCHANGED_XQ(FunctionCoercion)
UNCHANGED_XQ(OrderingChange)
UNCHANGED_XQ(CastableAs)
UNCHANGED_XQ(PromoteUntyped)
UNCHANGED_XQ(PromoteNumeric)
UNCHANGED_XQ(PromoteAnyURI)
UNCHANGED_XQ(DocumentOrder)
UNCHANGED(UApplyUpdates)
UNCHANGED_XQ(NamespaceBinding)
UNCHANGED_XQ(FunctionConversion)
UNCHANGED(ASTDebugHook)

#define UNCHANGED_FT(classname) \
FTSelection *QueryPathTreeGenerator::optimize ## classname (classname *item) \
{ \
  return ASTVisitor::optimize ## classname (item); \
}

UNCHANGED_FT(FTWords)
UNCHANGED_FT(FTWord)
UNCHANGED_FT(FTMildnot)
UNCHANGED_FT(FTUnaryNot)
UNCHANGED_FT(FTOrder)
UNCHANGED_FT(FTDistance)
UNCHANGED_FT(FTDistanceLiteral)
UNCHANGED_FT(FTScope)
UNCHANGED_FT(FTContent)
UNCHANGED_FT(FTWindow)
UNCHANGED_FT(FTWindowLiteral)
UNCHANGED_FT(FTOr)
UNCHANGED_FT(FTAnd)

ASTNode *QueryPathTreeGenerator::optimizeUnknown(ASTNode *item)
{
  return ASTVisitor::optimizeUnknown(item);
}

TupleNode *QueryPathTreeGenerator::optimizeUnknownTupleNode(TupleNode *item)
{
  return ASTVisitor::optimizeUnknownTupleNode(item);
}

FTSelection *QueryPathTreeGenerator::optimizeUnknownFTSelection(FTSelection *selection)
{
  return ASTVisitor::optimizeUnknownFTSelection(selection);
}

TupleNode *QueryPathTreeGenerator::optimizeContextTuple(ContextTuple *item)
{
  return ASTVisitor::optimizeContextTuple(item);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void QueryPathTreeGenerator::PathResult::join(const QueryPathNode::MVector &o)
{
  returnPaths.insert(returnPaths.begin(), o.begin(), o.end());
}

void QueryPathTreeGenerator::PathResult::join(const PathResult &o)
{
  returnPaths.insert(returnPaths.begin(), o.returnPaths.begin(), o.returnPaths.end());
}

void QueryPathTreeGenerator::PathResult::join(QueryPathNode *o)
{
  returnPaths.push_back(o);
}

void QueryPathTreeGenerator::PathResult::markSubtreeValue() const
{
  // Mark the paths as needing the subtree
  for(QueryPathNode::Vector::const_iterator it = returnPaths.begin(); it != returnPaths.end(); ++it) {
    (*it)->markSubtreeValue();
  }
}

void QueryPathTreeGenerator::PathResult::markSubtreeResult() const
{
  // Mark the paths as needing the subtree
  for(QueryPathNode::Vector::const_iterator it = returnPaths.begin(); it != returnPaths.end(); ++it) {
    (*it)->markSubtreeResult();
  }
}

void QueryPathTreeGenerator::PathResult::markRoot() const
{
  // Mark the paths as needing the entire tree
  for(QueryPathNode::Vector::const_iterator it = returnPaths.begin(); it != returnPaths.end(); ++it) {
    (*it)->getRoot()->markSubtreeResult();
  }
}


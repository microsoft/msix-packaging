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

#include <xqilla/optimizer/QueryPathNode.hpp>
#include <xqilla/utils/UTF8Str.hpp>
#include <xqilla/axis/NodeTest.hpp>
#include <xqilla/utils/XPath2Utils.hpp>
#include <sstream>

XERCES_CPP_NAMESPACE_USE;
using namespace std;

QueryPathNode::QueryPathNode(const NodeTest *nodeTest, Type type, MemoryManager *mm)
  : type_(type),
    nodeTest_(nodeTest),
    parent_(0),
    nextSibling_(0),
    prevSibling_(0),
    firstChild_(0),
    lastChild_(0),
    memMgr_(mm)
{
}

QueryPathNode::~QueryPathNode()
{
  QueryPathNode *child = firstChild_;
  QueryPathNode *next;
  while(child) {
    next = child->nextSibling_;
    delete child;
    child = next;
  }
}

bool QueryPathNode::isWildcardURI() const
{
  if(nodeTest_ == 0) return true;

  return nodeTest_->getItemType() || nodeTest_->getNamespaceWildcard();
}

bool QueryPathNode::isWildcardName() const
{
  if(nodeTest_ == 0) return true;

  return nodeTest_->getItemType() || nodeTest_->getNameWildcard();
}

bool QueryPathNode::isWildcardNodeType() const
{
  if(nodeTest_ == 0) return true;

  return nodeTest_->getItemType() || nodeTest_->getTypeWildcard() ||
    (nodeTest_->isNodeTypeSet() && nodeTest_->getNodeType() != Node::element_string);
}

bool QueryPathNode::isWildcard() const
{
  if(nodeTest_ == 0 || nodeTest_->getItemType()) return true;

  return nodeTest_->getNamespaceWildcard() || nodeTest_->getNameWildcard() || nodeTest_->getTypeWildcard() ||
    (type_ != ATTRIBUTE && type_ != DESCENDANT_ATTR && nodeTest_->getNodeType() != Node::element_string);
}

const XMLCh *QueryPathNode::getURI() const
{
  if(nodeTest_->getItemType() || nodeTest_->getNamespaceWildcard())
    return 0;
  return nodeTest_->getNodeUri();
}

const XMLCh *QueryPathNode::getName() const
{
  if(nodeTest_->getItemType() || nodeTest_->getNameWildcard())
    return 0;
  return nodeTest_->getNodeName();
}

QueryPathNode *QueryPathNode::getRoot() const
{
  QueryPathNode *result = const_cast<QueryPathNode*>(this);
  QueryPathNode *parent = parent_;
  while(parent) {
    result = parent;
    parent = parent->parent_;
  }
  return result;
}

QueryPathNode *QueryPathNode::appendChild(QueryPathNode *childToAdopt)
{
  if(childToAdopt->parent_ == this) return childToAdopt;

  if(childToAdopt->parent_ != 0) {
    childToAdopt->parent_->removeChild(childToAdopt, false);
  }

  // Search to see if the child exists already
  QueryPathNode *compare = firstChild_;
  for(; compare; compare = compare->nextSibling_) {
    if(compare->equals(childToAdopt)) {
      break;
    }
  }

  if(compare) {
    // Child exists already
    compare->stealChildren(childToAdopt);
    delete childToAdopt;
    return compare;
  }
  else {
    // Child doesn't already exist, so add
    childToAdopt->parent_ = this;
    if(firstChild_ == 0) {
      firstChild_ = childToAdopt;
    }
    if(lastChild_) {
      lastChild_->nextSibling_ = childToAdopt;
      childToAdopt->prevSibling_ = lastChild_;
    }
    lastChild_ = childToAdopt;

    return childToAdopt;
  }
}

void QueryPathNode::removeChild(QueryPathNode *child, bool deleteChild)
{
  if(firstChild_ == child) {
    firstChild_ = child->nextSibling_;
  }

  if(lastChild_ == child) {
    lastChild_ = child->prevSibling_;
  }

  if(child->prevSibling_) {
    child->prevSibling_->nextSibling_ = child->nextSibling_;
  }

  if(child->nextSibling_) {
    child->nextSibling_->prevSibling_ = child->prevSibling_;
  }

  if(deleteChild) {
    delete child;
  }
  else {
    child->parent_ = 0;
    child->nextSibling_ = 0;
    child->prevSibling_ = 0;
  }
}

void QueryPathNode::stealChildren(QueryPathNode *victim)
{
  // Loop over the children of "victim", adding them to us
  for(QueryPathNode *i = victim->firstChild_; i != 0; i = victim->firstChild_) {
    appendChild(i); // Removes it from "victim"
  }
}

void QueryPathNode::markSubtreeValue()
{
  if(type_ != ATTRIBUTE && type_ != DESCENDANT_ATTR) {
    appendChild(new (memMgr_) QueryPathNode(0, DESCENDANT, memMgr_));
  }
}

void QueryPathNode::markSubtreeResult()
{
  if(type_ != ATTRIBUTE && type_ != DESCENDANT_ATTR) {
    appendChild(new (memMgr_) QueryPathNode(0, DESCENDANT_ATTR, memMgr_));
    appendChild(new (memMgr_) QueryPathNode(0, DESCENDANT, memMgr_));
  }
}

bool QueryPathNode::equals(const QueryPathNode *o) const
{
  return type_ == o->type_ && NodeTest::isSubsetOf(nodeTest_, o->nodeTest_)
    && NodeTest::isSubsetOf(o->nodeTest_, nodeTest_);

//   return isWildcardURI() == o->isWildcardURI() &&
//     isWildcardName() == o->isWildcardName() &&
//     isWildcardNodeType() == o->isWildcardNodeType() &&
//     type_ == o->type_ &&
//     (isWildcardURI() || XPath2Utils::equals(getURI(), o->getURI())) &&
//     (isWildcardName() || XPath2Utils::equals(getName(), o->getName()));
}

bool QueryPathNode::isSubsetOf(const QueryPathNode *node) const
{
  return NodeTest::isSubsetOf(nodeTest_, node->nodeTest_);
}

static string getIndent(int level)
{
  ostringstream s;

  for(int i = level; i != 0; --i) {
    s << "  ";
  }

  return s.str();
}

static string typeToString(QueryPathNode::Type type) {
  switch(type) {
  case QueryPathNode::ATTRIBUTE: {
    return "attribute";
    break;
  }
  case QueryPathNode::CHILD: {
    return "child";
    break;
  }
  case QueryPathNode::DESCENDANT: {
    return "descendant";
    break;
  }
  case QueryPathNode::DESCENDANT_ATTR: {
    return "descendant-attr";
    break;
  }
  case QueryPathNode::ROOT: {
    return "root";
    break;
  }
  }
  return "UNKNOWN";
}

string QueryPathNode::toString(int level) const
{
  ostringstream s;
  string in(getIndent(level));
  string type(typeToString(type_));

  s << in << "<" << type;

  if(isWildcardURI()) {
    s << " uri=\"*\"";
  } else if(nodeTest_->getNodeUri() == 0) {
    s << " uri=\"\"";
  } else {
    s << " uri=\"" << UTF8(nodeTest_->getNodeUri()) << "\"";
  }
  if(isWildcardName()) {
    s << " name=\"*\"";
  } else if(nodeTest_->getNodeName() == 0) {
    s << " name=\"\"";
  } else {
    s << " name=\"" << UTF8(nodeTest_->getNodeName()) << "\"";
  }
  if(isWildcardNodeType()) {
    s << " nodeType=\"*\"";
  }

  if(firstChild_) {
    s << ">" << endl;

    QueryPathNode *child = firstChild_;
    while(child) {
      s << child->toString(level + 1);
      child = child->nextSibling_;
    }

    s << in << "</" << type << ">" << endl;
  }
  else {
    s << "/>" << endl;
  }

  return s.str();
}


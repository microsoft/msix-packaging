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
#include <assert.h>
#include <sstream>

#include <xqilla/axis/NodeTest.hpp>

#include <xqilla/utils/XPath2Utils.hpp>
#include <xqilla/items/Node.hpp>
#include <xqilla/context/DynamicContext.hpp>
#include <xqilla/utils/XStr.hpp>

#if defined(XERCES_HAS_CPP_NAMESPACE)
XERCES_CPP_NAMESPACE_USE
#endif

NodeTest::NodeTest()
  : _name(0),
    _uri(0),
    _prefix(0),
    _type(0),
    _wildcardName(false),
    _wildcardNamespace(false),
    _wildcardType(false),
    _usePrefix(false),
    _hasChildren(false),
    _itemType(0)
{
}

NodeTest::NodeTest(const XMLCh *nodeType, const XMLCh *uri, const XMLCh *name)
  : _name(name),
    _uri(uri),
    _prefix(0),
    _type(nodeType),
    _wildcardName(name == 0),
    _wildcardNamespace(name == 0),
    _wildcardType(nodeType == 0),
    _usePrefix(false),
    _hasChildren(false),
    _itemType(0)
{
}

NodeTest::NodeTest(const NodeTest *o)
  : _name(o->_name),
    _uri(o->_uri),
    _prefix(o->_prefix),
    _type(o->_type),
    _wildcardName(o->_wildcardName),
    _wildcardNamespace(o->_wildcardNamespace),
    _wildcardType(o->_wildcardType),
    _usePrefix(o->_usePrefix),
    _hasChildren(o->_hasChildren),
    _itemType(o->_itemType)
{
}

NodeTest::~NodeTest()
{
  delete _itemType;
}

void *NodeTest::getInterface(const XMLCh *name) const
{
  return 0;
}

void NodeTest::getStaticType(StaticType &st, const StaticContext *context,
                             bool &isExact, const LocationInfo *location) const
{
  if(_itemType) {
    _itemType->getStaticType(st, context, isExact, location);
  }
  else {
    if(_wildcardType) {
      if(_hasChildren) {
        st = StaticType::ELEMENT_TYPE | StaticType::DOCUMENT_TYPE;
      }
      else {
        st = StaticType::NODE_TYPE;
      }
    }
    else if(_type == Node::document_string) {
      st = StaticType::DOCUMENT_TYPE;
    }
    else if(_type == Node::element_string) {
      st = StaticType::ELEMENT_TYPE;
    }
    else if(_type == Node::attribute_string) {
      st = StaticType::ATTRIBUTE_TYPE;
    }
    else if(_type == Node::namespace_string) {
      st = StaticType::NAMESPACE_TYPE;
    }
    else if(_type == Node::processing_instruction_string) {
      st = StaticType::PI_TYPE;
    }
    else if(_type == Node::comment_string) {
      st = StaticType::COMMENT_TYPE;
    }
    else if(_type == Node::text_string) {
      st = StaticType::TEXT_TYPE;
    }

    if(_wildcardName && _wildcardNamespace)
      isExact = true;
    else isExact = false;
  }
}

Result NodeTest::filterResult(const Result &toFilter, const LocationInfo *info) const
{
  return new FilterResult(info, toFilter, this);
}

bool NodeTest::filterNode(Node::Ptr node, DynamicContext* context) const
{
  if(_itemType) return _itemType->matches(node, context);
  else return checkNodeType(node) && checkNodeName(node, context);
}

bool NodeTest::checkNodeType(Node::Ptr node) const
{
  assert(node.notNull());

  if(_wildcardType) {
    if(_hasChildren) {
      return node->dmNodeKind() == Node::element_string || node->dmNodeKind() == Node::document_string;
    }
    else return true;
  }

  return node->dmNodeKind() == _type;
}

bool NodeTest::checkNodeName(Node::Ptr node, const DynamicContext *context) const
{
  ATQNameOrDerived::Ptr name = node->dmNodeName(context);

  const XMLCh *uri = _uri;
  if(uri == 0 && node->dmNodeKind() == Node::element_string) {
    uri = context->getDefaultElementAndTypeNS();
  }

  return (_wildcardName || (name.notNull() && XPath2Utils::equals(((ATQNameOrDerived*)name.get())->getName(), _name)))
    && (_wildcardNamespace || (name.notNull() && XPath2Utils::equals(((ATQNameOrDerived*)name.get())->getURI(), uri)));
}


void NodeTest::setNodeName(const XMLCh* name)
{
  _name = name;
}

void NodeTest::setNodePrefix(const XMLCh* prefix)
{
  _prefix=prefix;
  _usePrefix=true;
}

void NodeTest::setNodeUri(const XMLCh* uri)
{
  _uri = uri;
  _usePrefix=false;
}

void NodeTest::setNodeType(const XMLCh *type)
{
  _type = type;
}

void NodeTest::setNameWildcard(bool value)
{
  _wildcardName = value;
}

void NodeTest::setNamespaceWildcard(bool value)
{
  _wildcardNamespace = value;
}

void NodeTest::setTypeWildcard(bool value)
{
  _wildcardType = value;
}

bool NodeTest::getHasChildren() const
{
  return _hasChildren;
}

void NodeTest::setHasChildren(bool value)
{
  _hasChildren = value;
}


const XMLCh *NodeTest::getNodeType() const
{
  return _type;
}

const XMLCh* NodeTest::getNodeUri() const
{
  return _uri;
}

const XMLCh* NodeTest::getNodePrefix() const
{
  return _prefix;
}

const XMLCh* NodeTest::getNodeName() const
{
  return _name;
}

bool NodeTest::getNameWildcard() const
{
  return _wildcardName;
}

bool NodeTest::getNamespaceWildcard() const
{
  return _wildcardNamespace;
}

bool NodeTest::getTypeWildcard() const
{
  return _wildcardType;
}


bool NodeTest::isNodeTypeSet() const
{
  return _type != 0;
}

void NodeTest::staticResolution(StaticContext *context, const LocationInfo *location)
{
  if(isNodePrefixSet()) {
    setNodeUri(context->getUriBoundToPrefix(getNodePrefix(), location));
  }
  if(!_wildcardNamespace && _uri == 0 &&
     !_wildcardType && _type == Node::element_string) {
    _uri = context->getDefaultElementAndTypeNS();
  }

  // Convert certain NodeTest objects that use an ItemType to ones that don't,
  // for efficiency and simplicity of comparison.
  if(_itemType != 0) {
    _itemType->staticResolution(context, location);

    switch(_itemType->getItemTestType()) {
    case SequenceType::ItemType::TEST_NODE:
      _wildcardType = true;
      _wildcardNamespace = true;
      _wildcardName = true;
      _itemType = 0;
      break;
    case SequenceType::ItemType::TEST_DOCUMENT:
      if(_itemType->getName() == 0 && _itemType->getType() == 0) {
        _wildcardType = false;
        _type = Node::document_string;
        _wildcardNamespace = true;
        _wildcardName = true;
        _itemType = 0;
      }
      break;
    case SequenceType::ItemType::TEST_TEXT:
      _wildcardType = false;
      _type = Node::text_string;
      _wildcardNamespace = true;
      _wildcardName = true;
      _itemType = 0;
      break;
    case SequenceType::ItemType::TEST_COMMENT:
      _wildcardType = false;
      _type = Node::comment_string;
      _wildcardNamespace = true;
      _wildcardName = true;
      _itemType = 0;
      break;
    case SequenceType::ItemType::TEST_PI:
      _wildcardType = false;
      _type = Node::processing_instruction_string;
      _wildcardNamespace = true;
      if(_itemType->getName() == 0) {
        _wildcardName = true;
      } else {
        _wildcardName = false;
        _name = _itemType->getName()->getName();
      }
      _itemType = 0;
      break;
    case SequenceType::ItemType::TEST_ATTRIBUTE:
      if(_itemType->getType() == 0) {
        _wildcardType = false;
        _type = Node::attribute_string;
        if(_itemType->getName() == 0) {
          _wildcardNamespace = true;
          _wildcardName = true;
        } else {
          _wildcardNamespace = false;
          _uri = context->getUriBoundToPrefix(_itemType->getName()->getPrefix(), location);
          _wildcardName = false;
          _name = _itemType->getName()->getName();
        }
        _itemType = 0;
      }
      break;
    case SequenceType::ItemType::TEST_ELEMENT:
      if(_itemType->getType() == 0) {
        _wildcardType = false;
        _type = Node::element_string;
        if(_itemType->getName() == 0) {
          _wildcardNamespace = true;
          _wildcardName = true;
        } else {
          _wildcardNamespace = false;
          _uri = context->getUriBoundToPrefix(_itemType->getName()->getPrefix(), location);
          _wildcardName = false;
          _name = _itemType->getName()->getName();
        }
        _itemType = 0;
      }
      break;
    default: break;
    }
  }
}

bool NodeTest::isNodePrefixSet() const {
  return _usePrefix;
}

SequenceType::ItemType* NodeTest::getItemType() const {
  return _itemType;
}

void NodeTest::setItemType(SequenceType::ItemType* type) {
  _itemType=type;
}

bool NodeTest::isSubsetOf(const NodeTest *o) const
{
  if(o->_itemType != 0 || _itemType != 0)
    return false;

  if(!o->_wildcardType && (_wildcardType || o->_type != _type))
    return false;
  if(!o->_wildcardNamespace && (_wildcardNamespace || !XPath2Utils::equals(o->_uri, _uri)))
    return false;
  if(!o->_wildcardName && (_wildcardName || !XPath2Utils::equals(o->_name, _name)))
    return false;
  return true;
}

bool NodeTest::isSubsetOf(const NodeTest *a, const NodeTest *b)
{
  if(b == 0) return true;
  if(a == 0) return b->_itemType == 0 && b->_wildcardType &&
               b->_wildcardNamespace && b->_wildcardName;

  return a->isSubsetOf(b);
}

/////////////////////////////////////
// FilterResult
/////////////////////////////////////

NodeTest::FilterResult::FilterResult(const LocationInfo *info, const Result &toFilter, const NodeTest *nodeTest)
  : ResultImpl(info),
    toFilter_(toFilter),
    nodeTest_(nodeTest)
{
}

Item::Ptr NodeTest::FilterResult::next(DynamicContext *context)
{
  Node::Ptr result = 0;
  while((result = toFilter_->next(context)).notNull() && !nodeTest_->filterNode(result, context)) {}

  return result;
}


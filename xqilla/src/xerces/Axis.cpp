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
#include "Axis.hpp"
#include <xqilla/axis/NodeTest.hpp>
#include <xqilla/utils/XPath2Utils.hpp>
#include <xqilla/utils/XPath2NSUtils.hpp>
#include <xqilla/schema/SequenceType.hpp>
#include <xqilla/context/DynamicContext.hpp>
#include "../dom-api/impl/XPathNamespaceImpl.hpp"

#if defined(XERCES_HAS_CPP_NAMESPACE)
XERCES_CPP_NAMESPACE_USE
#endif

Axis::Axis(const LocationInfo *info, const XERCES_CPP_NAMESPACE_QUALIFIER DOMNode *contextNode, const Node *nodeObj,
           const NodeTest *nodeTest, const AxisNodeFactory &factory)
  : ResultImpl(info),
    nodeObj_(nodeObj),
    factory_(factory),
    contextNode_(contextNode),
    nodeTest_(nodeTest),
    toDo_(true)
{
}

Item::Ptr Axis::next(DynamicContext *context)
{
  const DOMNode *node = 0;
  while((node = nextNode(context)) != 0)
  {
    context->testInterrupt();

    if(nodeTest_ == 0) return factory_.createNode(node, context);

    SequenceType::ItemType *itemType = nodeTest_->getItemType();
    if(itemType != 0)
    {
      Node::Ptr result = factory_.createNode(node, context);
      if(itemType->matches(result, context)) {
        return result;
      }
    }
    else
    {
      switch(node->getNodeType())
      {      
        case DOMNode::DOCUMENT_NODE: {
          if(!nodeTest_->getTypeWildcard() && nodeTest_->getNodeType() != Node::document_string) continue;
          if(!nodeTest_->getNameWildcard() || !nodeTest_->getNamespaceWildcard()) continue;
          break;
        }
        case DOMNode::ELEMENT_NODE: {
          if(!nodeTest_->getTypeWildcard() && nodeTest_->getNodeType() != Node::element_string) continue;
          if(!nodeTest_->getNameWildcard() && !XPath2Utils::equals(getLocalName(node), nodeTest_->getNodeName())) continue;
          if(!nodeTest_->getNamespaceWildcard() && !XPath2Utils::equals(node->getNamespaceURI(), nodeTest_->getNodeUri())) continue;
          break;
        }
        case DOMNode::ATTRIBUTE_NODE: {
          if(nodeTest_->getTypeWildcard()) { if(nodeTest_->getHasChildren()) continue; }
          else if(nodeTest_->getNodeType() != Node::attribute_string) continue;
          if(!nodeTest_->getNameWildcard() && !XPath2Utils::equals(getLocalName(node), nodeTest_->getNodeName())) continue;
          if(!nodeTest_->getNamespaceWildcard() && !XPath2Utils::equals(node->getNamespaceURI(), nodeTest_->getNodeUri())) continue;
          break;
        }
        case DOMNode::CDATA_SECTION_NODE:
        case DOMNode::TEXT_NODE: {
          if(nodeTest_->getTypeWildcard()) { if(nodeTest_->getHasChildren()) continue; }
          else if(nodeTest_->getNodeType() != Node::text_string) continue;
          if(!nodeTest_->getNameWildcard() || !nodeTest_->getNamespaceWildcard()) continue;
          break;
        }
        case DOMNode::PROCESSING_INSTRUCTION_NODE: {
          if(nodeTest_->getTypeWildcard()) { if(nodeTest_->getHasChildren()) continue; }
          else if(nodeTest_->getNodeType() != Node::processing_instruction_string) continue;
          if(!nodeTest_->getNameWildcard() && !XPath2Utils::equals(node->getNodeName(), nodeTest_->getNodeName())) continue;
          if(!nodeTest_->getNamespaceWildcard()) continue;
          break;
        }
        case DOMNode::COMMENT_NODE: {
          if(nodeTest_->getTypeWildcard()) { if(nodeTest_->getHasChildren()) continue; }
          else if(nodeTest_->getNodeType() != Node::comment_string) continue;
          if(!nodeTest_->getNameWildcard() || !nodeTest_->getNamespaceWildcard()) continue;
          break;
        }
        case DOMXPathNamespace::XPATH_NAMESPACE_NODE: {
          if(nodeTest_->getTypeWildcard()) { if(nodeTest_->getHasChildren()) continue; }
          else if(nodeTest_->getNodeType() != Node::namespace_string) continue;
          if(!nodeTest_->getNameWildcard() && !XPath2Utils::equals(node->getPrefix(), nodeTest_->getNodeName())) continue;
          if(!nodeTest_->getNamespaceWildcard() && !XPath2Utils::equals(XMLUni::fgZeroLenString, nodeTest_->getNodeUri())) continue;
          break;
        }
        default: {
          continue;
        }
      }

      return factory_.createNode(node, context);
    }
    
  }

  return 0;
}

const DOMNode *Axis::getParent(const DOMNode *fNode)
{
  DOMNode *parent = XPath2NSUtils::getParent(fNode);

  // Skip out of entity reference nodes
  while(parent != 0 && parent->getNodeType() == DOMNode::ENTITY_REFERENCE_NODE) {
    parent = parent->getParentNode();
  }

  return parent;
}

const DOMNode *Axis::getFirstChild(const DOMNode *fNode)
{
  if(fNode->getNodeType() == DOMNode::ATTRIBUTE_NODE)
    return 0;

  DOMNode *result = fNode->getFirstChild();
  if(result == 0) return 0;

  // Skip into the contents of entity reference nodes
  while(result->getNodeType() == DOMNode::ENTITY_REFERENCE_NODE) {
    result = result->getFirstChild();
  }

  // Skip any other unused types
  while(result->getNodeType() == DOMNode::DOCUMENT_TYPE_NODE) {
    result = result->getNextSibling();
  }

  return result;
}

const DOMNode *Axis::getLastChild(const DOMNode *fNode)
{
  if(fNode->getNodeType() == DOMNode::ATTRIBUTE_NODE)
    return 0;

  DOMNode *result = fNode->getLastChild();
  if(result == 0) return 0;

  // Skip into the contents of entity reference nodes
  while(result->getNodeType() == DOMNode::ENTITY_REFERENCE_NODE) {
    result = result->getLastChild();
  }

  // Skip any other unused types
  while(result->getNodeType() == DOMNode::DOCUMENT_TYPE_NODE) {
    result = result->getPreviousSibling();
  }

  return result;
}

const DOMNode *Axis::getNextSibling(const DOMNode *fNode)
{
  DOMNode *result = fNode->getNextSibling();

  if(result == 0) {
    // Skip out of the contents of entity reference nodes
    DOMNode *parent = fNode->getParentNode();
    while(result == 0 && parent != 0 && parent->getNodeType() == DOMNode::ENTITY_REFERENCE_NODE) {
      result = parent->getNextSibling();
      parent = parent->getParentNode();
    }
  }

  // Skip any other unused types
  while(result != 0 && result->getNodeType() == DOMNode::DOCUMENT_TYPE_NODE) {
    result = result->getNextSibling();
  }

  return result;
}

const DOMNode *Axis::getPreviousSibling(const DOMNode *fNode)
{
  DOMNode *result = fNode->getPreviousSibling();

  if(result == 0) {
    // Skip out of the contents of entity reference nodes
    DOMNode *parent = fNode->getParentNode();
    while(result == 0 && parent != 0 && parent->getNodeType() == DOMNode::ENTITY_REFERENCE_NODE) {
      result = parent->getPreviousSibling();
      parent = parent->getParentNode();
    }
  }

  // Skip any other unused types
  while(result != 0 && result->getNodeType() == DOMNode::DOCUMENT_TYPE_NODE) {
    result = result->getPreviousSibling();
  }

  return result;
}


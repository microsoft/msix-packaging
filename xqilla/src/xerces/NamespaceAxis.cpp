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
#include "NamespaceAxis.hpp"
#include <xqilla/utils/XPath2NSUtils.hpp>
#include <xqilla/context/DynamicContext.hpp>
#include <xqilla/context/ItemFactory.hpp>

#include "../dom-api/impl/XPathNamespaceImpl.hpp"

#include <xercesc/dom/impl/DOMDocumentImpl.hpp>
#include <xercesc/dom/DOMElement.hpp>
#include <xercesc/util/XMLString.hpp>
#include <xercesc/util/XMLUniDefs.hpp>

#if defined(XERCES_HAS_CPP_NAMESPACE)
XERCES_CPP_NAMESPACE_USE
#endif

NamespaceAxis::NamespaceAxis(const LocationInfo *info, const XERCES_CPP_NAMESPACE_QUALIFIER DOMNode *contextNode, const Node *nodeObj,
       const NodeTest *nodeTest, const AxisNodeFactory &factory)
  : Axis(info, contextNode, nodeObj, nodeTest, factory),
    node_(contextNode),
    originalNode_(contextNode),
    nodeMap_(node_->getAttributes()),
    i_(0),
    state_(CHECK_ELEMENT)
{
}

static const DOMNode *createNamespaceNode(const XMLCh* prefix, const XMLCh* uri, const DOMNode* parentNode)
{
  if(parentNode->getNodeType()!=DOMNode::ELEMENT_NODE)
      return NULL;

  DOMDocument *ownerDocument = parentNode->getOwnerDocument();
  return new ((DOMDocumentImpl *)ownerDocument, (DOMDocumentImpl::NodeObjectType)XPathNamespaceImpl::XPATH_NAMESPACE_OBJECT)
    XPathNamespaceImpl(prefix, uri, static_cast<DOMElement*>(const_cast<DOMNode*>(parentNode)), ownerDocument);
}

const DOMNode *NamespaceAxis::nextNode(DynamicContext *context)
{
  const DOMNode *result = 0;

  while(result == 0) {
    switch(state_) {
    case CHECK_ELEMENT: {
      const XMLCh* uri = node_->getNamespaceURI();
      const XMLCh* prefix = node_->getPrefix();
      if(done_.insert(prefix).second && uri && *uri) {
        result = createNamespaceNode(prefix, uri, originalNode_);
      }
      state_ = CHECK_ATTR;
      break;
    }
    case CHECK_ATTR: {
      if(nodeMap_ != 0 && i_ < nodeMap_->getLength()) {
        DOMNode *tmpAttr = nodeMap_->item(i_);
        ++i_;

        const XMLCh* attrName = tmpAttr->getNodeName();

        // Check to see if this attribute starts with xmlns
        if(!XMLString::startsWith(attrName, XMLUni::fgXMLNSString)) {
          const XMLCh* uri = tmpAttr->getNamespaceURI();
          const XMLCh* prefix = tmpAttr->getPrefix();
          if(uri && *uri && done_.insert(prefix).second) {
            result = createNamespaceNode(prefix, uri, originalNode_);
          }
        }
        else {
          // Get uri
          const XMLCh* uri = tmpAttr->getNodeValue();

          // Figure out prefix
          const XMLCh* prefix = 0;
          if(XMLString::stringLen(attrName) != 5) {
            // A prefix was given

            // If the name doesn't start with xmlns: (and its not xmlns) then skip it
            // XXX: Is this necessary/allowed?
            if(attrName[5] != chColon) {
              continue;
            }

            prefix = attrName + 6;
          }

          if(done_.insert(prefix).second && uri && *uri) {
            result = createNamespaceNode(prefix, uri, originalNode_);
          }
        }
      }
      else {
        node_ = XPath2NSUtils::getParent(node_);
        if(node_ == 0 || node_->getNodeType() != DOMNode::ELEMENT_NODE) {
          state_ = DO_XML;
        }
        else {
          nodeMap_ = node_->getAttributes();
          i_ = 0;
          state_ = CHECK_ELEMENT;
        }
      }
      break;
    }
    case DO_XML: {
      if(done_.insert(XMLUni::fgXMLString).second) {
        result = createNamespaceNode(XMLUni::fgXMLString, XMLUni::fgXMLURIName, originalNode_);
      }
      state_ = DONE;
      break;
    }
    case DONE:
      return 0;
    }
  }

  return result;
}

std::string NamespaceAxis::asString(DynamicContext *context, int indent) const
{
  return "NamespaceNodesResult";
}


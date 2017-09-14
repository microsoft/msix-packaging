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

#include "../../config/xqilla_config.h"
#include "XPathNamespaceImpl.hpp"
#include <xqilla/utils/XPath2Utils.hpp>
#include <xqilla/utils/XStr.hpp>
#include <xercesc/dom/DOMException.hpp>
#include <xercesc/dom/DOMDocument.hpp>
#include <xercesc/dom/impl/DOMDocumentImpl.hpp>
#include <xercesc/dom/DOMAttr.hpp>
#include <xercesc/util/XercesVersion.hpp>
#include <xercesc/dom/impl/DOMCasts.hpp>
#include <xercesc/dom/impl/DOMNodeImpl.hpp>

XERCES_CPP_NAMESPACE_USE;

XPathNamespaceImpl::XPathNamespaceImpl(const XMLCh* const nsPrefix, 
		const XMLCh* const nsUri, DOMElement *owner, DOMDocument *docOwner) 
	: fNode(docOwner)
{
    DOMNodeImpl *argImpl = castToNodeImpl(this);

    if(owner) {
      argImpl->fOwnerNode = owner;
      argImpl->isOwned(true);
    } else {
      argImpl->fOwnerNode = docOwner;
      argImpl->isOwned(false);
    }
    argImpl->setIsLeafNode(true);

    DOMDocumentImpl *docImpl = (DOMDocumentImpl *)docOwner;
    uri = docImpl->getPooledString(nsUri);
    prefix = docImpl->getPooledString(nsPrefix);

    //	this->ownerElement = owner;
}

XPathNamespaceImpl::XPathNamespaceImpl(const XPathNamespaceImpl &other) 
	: fNode(other.fNode), uri(other.uri), prefix(other.prefix)
{
}

XPathNamespaceImpl::~XPathNamespaceImpl()
{
}

DOMNode * XPathNamespaceImpl::cloneNode(bool deep) const 
{
	throw DOMException::NOT_SUPPORTED_ERR;
}

const XMLCh *XPathNamespaceImpl::getNodeName() const
{
	return prefix;
}

#if _XERCES_VERSION >= 30000
DOMNode::NodeType XPathNamespaceImpl::getNodeType() const
#else
short XPathNamespaceImpl::getNodeType() const
#endif
{
  return (DOMNode::NodeType)DOMXPathNamespace::XPATH_NAMESPACE_NODE;
}

const XMLCh *XPathNamespaceImpl::getPrefix() const
{
	return prefix;
}


const XMLCh *XPathNamespaceImpl::getNamespaceURI() const 
{
	return uri;
}

DOMElement *XPathNamespaceImpl::getOwnerElement() const 
{
    return (DOMElement *) (fNode.isOwned() ? fNode.fOwnerNode : 0);
}

const XMLCh *XPathNamespaceImpl::getNodeValue() const 
{
	return 0;
}

void XPathNamespaceImpl::setNodeValue(const XMLCh *value)
{
	throw DOMException(DOMException::NO_MODIFICATION_ALLOWED_ERR, 0);
}

void* XPathNamespaceImpl::setUserData(const XMLCh* key, void* data, DOMUserDataHandler* handler)
{
	throw DOMException(DOMException::NO_MODIFICATION_ALLOWED_ERR, 0);
}

void XPathNamespaceImpl::setPrefix(const XMLCh *prefix)
{
	throw DOMException(DOMException::NO_MODIFICATION_ALLOWED_ERR, 0);
}



#if _XERCES_VERSION >= 30000
short            XPathNamespaceImpl::compareDocumentPosition(const DOMNode* other) const
#else
short            XPathNamespaceImpl::compareTreePosition(const DOMNode* other) const
#endif
{

    //note: order of namespace nodes currently has issues (number 51). For our purposes
    //namespace nodes belong to the element on which they were originally declared
    //this is proberbly incorecct but easiest to implement. Will change when the idea
    //is firmed up REVISIT

    // Questions of clarification for this method - to be answered by the
    // DOM WG.   Current assumptions listed - LM
    //
    // 1. How do ENTITY nodes compare?
    //    Current assumption: TREE_POSITION_DISCONNECTED, as ENTITY nodes
    //    aren't really 'in the tree'
    //
    // 2. How do NOTATION nodes compare?
    //    Current assumption: TREE_POSITION_DISCONNECTED, as NOTATION nodes
    //    aren't really 'in the tree'
    //
    // 3. Are TREE_POSITION_ANCESTOR and TREE_POSITION_DESCENDANT
    //    only relevant for nodes that are "part of the document tree"?
    //     <outer>
    //         <inner  myattr="true"/>
    //     </outer>
    //    Is the element node "outer" considered an ancestor of "myattr"?
    //    Current assumption: No.
    //
    // 4. How do children of ATTRIBUTE nodes compare (with eachother, or
    //    with children of other attribute nodes with the same element)
    //    Current assumption: Children of ATTRIBUTE nodes are treated as if
    //    they are the attribute node itself, unless the 2 nodes
    //    are both children of the same attribute.
    //
    // 5. How does an ENTITY_REFERENCE node compare with it's children?
    //    Given the DOM, it should precede its children as an ancestor.
    //    Given "document order",  does it represent the same position?
    //    Current assumption: An ENTITY_REFERENCE node is an ancestor of its
    //    children.
    //
    // 6. How do children of a DocumentFragment compare?
    //    Current assumption: If both nodes are part of the same document
    //    fragment, there are compared as if they were part of a document.



    const DOMNode* thisNode = this;

    // If the nodes are the same...
    if (thisNode == other)
#if _XERCES_VERSION >= 30000
        return 0;
#else
        return (DOMNode::TREE_POSITION_SAME_NODE | DOMNode::TREE_POSITION_EQUIVALENT);
#endif

    // If either node is of type ENTITY or NOTATION, compare as disconnected
    short thisType = thisNode->getNodeType();
    short otherType = other->getNodeType();

    // If either node is of type ENTITY or NOTATION, compare as disconnected
    if (thisType == DOMNode::ENTITY_NODE ||
            thisType == DOMNode::NOTATION_NODE ||
            otherType == DOMNode::ENTITY_NODE ||
            otherType == DOMNode::NOTATION_NODE ) {
#if _XERCES_VERSION >= 30000
        return DOMNode::DOCUMENT_POSITION_DISCONNECTED;
#else
        return DOMNode::TREE_POSITION_DISCONNECTED;
#endif
    }

    //if it is a custom node and bigger than us we must ask it for the order
    if(otherType > DOMXPathNamespace::XPATH_NAMESPACE_NODE) {
        DOMNodeImpl tmp(0);
#if _XERCES_VERSION >= 30000
        return tmp.reverseTreeOrderBitPattern(other->compareDocumentPosition(this));
#else
        return tmp.reverseTreeOrderBitPattern(other->compareTreePosition(this));
#endif
    }

    // Find the ancestor of each node, and the distance each node is from
    // its ancestor.
    // During this traversal, look for ancestor/descendent relationships
    // between the 2 nodes in question.
    // We do this now, so that we get this info correct for attribute nodes
    // and their children.

    const DOMNode *node;
    const DOMNode *thisAncestor = this;
    const DOMNode *otherAncestor = other;
    int thisDepth=0;
    int otherDepth=0;
    for (node = this; node != 0; node = node->getParentNode()) {
        thisDepth +=1;
        if (node == other)
            // The other node is an ancestor of this one.
#if _XERCES_VERSION >= 30000
            return (DOMNode::DOCUMENT_POSITION_CONTAINS | DOMNode::DOCUMENT_POSITION_PRECEDING);
#else
            return (DOMNode::TREE_POSITION_ANCESTOR | DOMNode::TREE_POSITION_PRECEDING);
#endif
        thisAncestor = node;
    }

    for (node=other; node != 0; node = node->getParentNode()) {
        otherDepth +=1;
        if (node == this)
            // The other node is a descendent of the reference node.
#if _XERCES_VERSION >= 30000
            return (DOMNode::DOCUMENT_POSITION_CONTAINED_BY | DOMNode::DOCUMENT_POSITION_FOLLOWING);
#else
            return (DOMNode::TREE_POSITION_DESCENDANT | DOMNode::TREE_POSITION_FOLLOWING);
#endif
        otherAncestor = node;
    }


    const DOMNode *otherNode = other;

    short thisAncestorType = thisAncestor->getNodeType();
    short otherAncestorType = otherAncestor->getNodeType();

    // if the ancestor is an attribute, get owning element.
    // we are now interested in the owner to determine position.

    if (thisAncestorType == DOMNode::ATTRIBUTE_NODE) {
        thisNode = ((DOMAttr *)thisAncestor)->getOwnerElement();
    }

    if (thisAncestorType == DOMXPathNamespace::XPATH_NAMESPACE_NODE) {
        thisNode = ((XPathNamespaceImpl *)thisAncestor)->getOwnerElement();
    }

    if (otherAncestorType == DOMNode::ATTRIBUTE_NODE) {
        otherNode = ((DOMAttr *)otherAncestor)->getOwnerElement();
    }

    if (otherAncestorType == DOMXPathNamespace::XPATH_NAMESPACE_NODE) {
        otherNode = ((XPathNamespaceImpl *)otherAncestor)->getOwnerElement();
    }


    // Before proceeding, we should check if both ancestor nodes turned
    // out to be attributes for the same element
    if (thisAncestorType == DOMNode::ATTRIBUTE_NODE &&
        otherAncestorType == DOMNode::ATTRIBUTE_NODE &&
            thisNode==otherNode)
#if _XERCES_VERSION >= 30000
        return DOMNode::DOCUMENT_POSITION_IMPLEMENTATION_SPECIFIC | (thisAncestor<otherAncestor ? DOMNode::DOCUMENT_POSITION_FOLLOWING:DOMNode::DOCUMENT_POSITION_PRECEDING);
#else
        return DOMNode::TREE_POSITION_EQUIVALENT;
#endif

    //now do the same for namespace nodes
    if (thisAncestorType == DOMXPathNamespace::XPATH_NAMESPACE_NODE &&
        otherAncestorType == DOMXPathNamespace::XPATH_NAMESPACE_NODE &&
            thisNode==otherNode)
#if _XERCES_VERSION >= 30000
        return DOMNode::DOCUMENT_POSITION_IMPLEMENTATION_SPECIFIC | (thisAncestor<otherAncestor ? DOMNode::DOCUMENT_POSITION_FOLLOWING:DOMNode::DOCUMENT_POSITION_PRECEDING);
#else
        return DOMNode::TREE_POSITION_EQUIVALENT;
#endif

    //now do comparison between attrs
    if (thisAncestorType == DOMXPathNamespace::XPATH_NAMESPACE_NODE &&
        otherAncestorType == DOMNode::ATTRIBUTE_NODE &&
            thisNode==otherNode)
#if _XERCES_VERSION >= 30000
        return DOMNode::DOCUMENT_POSITION_FOLLOWING;
#else
        return DOMNode::TREE_POSITION_FOLLOWING;
#endif

    //now do comparison between attrs
    if (thisAncestorType == DOMNode::ATTRIBUTE_NODE &&
        otherAncestorType == DOMXPathNamespace::XPATH_NAMESPACE_NODE &&
            thisNode==otherNode)
#if _XERCES_VERSION >= 30000
        return DOMNode::DOCUMENT_POSITION_PRECEDING;
#else
        return DOMNode::TREE_POSITION_PRECEDING;
#endif


    // Now, find the ancestor of the owning element, if the original
    // ancestor was an attribute

    if (thisAncestorType == DOMNode::ATTRIBUTE_NODE || thisAncestorType == DOMXPathNamespace::XPATH_NAMESPACE_NODE) {
        thisDepth=0;
        for (node=thisNode; node != 0; node = node->getParentNode()) {
            thisDepth +=1;
            if (node == otherNode)
                // The other node is an ancestor of the owning element
#if _XERCES_VERSION >= 30000
                return DOMNode::DOCUMENT_POSITION_PRECEDING;
#else
                return DOMNode::TREE_POSITION_PRECEDING;
#endif
            thisAncestor = node;
        }
        for (node=otherNode; node != 0; node = node->getParentNode()) {
            if (node == thisNode)
                // The other node is an ancestor of the owning element
#if _XERCES_VERSION >= 30000
                return DOMNode::DOCUMENT_POSITION_FOLLOWING;
#else
                return DOMNode::TREE_POSITION_FOLLOWING;
#endif
        }
    }

    // Now, find the ancestor of the owning element, if the original
    // ancestor was an attribute
    if (otherAncestorType == DOMNode::ATTRIBUTE_NODE || otherAncestorType == DOMXPathNamespace::XPATH_NAMESPACE_NODE) {
        otherDepth=0;
        for (node=otherNode; node != 0; node = node->getParentNode()) {
            otherDepth +=1;
            if (node == thisNode)
                // The other node is a descendent of the reference
                // node's element
#if _XERCES_VERSION >= 30000
                return DOMNode::DOCUMENT_POSITION_FOLLOWING;
#else
                return DOMNode::TREE_POSITION_FOLLOWING;
#endif
            otherAncestor = node;
        }
        for (node=thisNode; node != 0; node = node->getParentNode()) {
            if (node == otherNode)
                // The other node is an ancestor of the owning element
#if _XERCES_VERSION >= 30000
                return DOMNode::DOCUMENT_POSITION_PRECEDING;
#else
                return DOMNode::TREE_POSITION_PRECEDING;
#endif
        }
    }

    // thisAncestor and otherAncestor must be the same at this point,
    // otherwise, we are not in the same tree or document fragment
    if (thisAncestor != otherAncestor)
#if _XERCES_VERSION >= 30000
        return DOMNode::DOCUMENT_POSITION_DISCONNECTED | DOMNode::DOCUMENT_POSITION_IMPLEMENTATION_SPECIFIC |
              (thisAncestor<otherAncestor ? DOMNode::DOCUMENT_POSITION_PRECEDING : DOMNode::DOCUMENT_POSITION_FOLLOWING);
#else
        return DOMNode::TREE_POSITION_DISCONNECTED;
#endif

    // Determine which node is of the greatest depth.
    if (thisDepth > otherDepth) {
        for (int i= 0 ; i < thisDepth - otherDepth; i++)
            thisNode = thisNode->getParentNode();
    }
    else {
        for (int i = 0; i < otherDepth - thisDepth; i++)
            otherNode = otherNode->getParentNode();
    }

    // We now have nodes at the same depth in the tree.  Find a common
    // ancestor.
    DOMNode *thisNodeP, *otherNodeP;
    for (thisNodeP = thisNode->getParentNode(),
                 otherNodeP = otherNode->getParentNode();
             thisNodeP != otherNodeP;) {
        thisNode = thisNodeP;
        otherNode = otherNodeP;
        thisNodeP = thisNodeP->getParentNode();
        otherNodeP = otherNodeP->getParentNode();
    }

    // See whether thisNode or otherNode is the leftmost
    for (DOMNode *current = thisNodeP->getFirstChild();
             current != 0;
             current = current->getNextSibling()) {
        if (current == otherNode) {
#if _XERCES_VERSION >= 30000
            return DOMNode::DOCUMENT_POSITION_PRECEDING;
#else
            return DOMNode::TREE_POSITION_PRECEDING;
#endif
        }
        else if (current == thisNode) {
#if _XERCES_VERSION >= 30000
            return DOMNode::DOCUMENT_POSITION_FOLLOWING;
#else
            return DOMNode::TREE_POSITION_FOLLOWING;
#endif
        }
    }
    // REVISIT:  shouldn't get here.   Should probably throw an
    // exception
    return 0;
}

void  XPathNamespaceImpl::release() {
    if (fNode.isOwned() && !fNode.isToBeReleased())
        throw DOMException(DOMException::INVALID_ACCESS_ERR,0);

    DOMDocumentImpl* doc = (DOMDocumentImpl*) getOwnerDocument();
    if (doc) {
        fNode.callUserDataHandlers(DOMUserDataHandler::NODE_DELETED, 0, 0);
        doc->release(this, (DOMDocumentImpl::NodeObjectType)XPathNamespaceImpl::XPATH_NAMESPACE_OBJECT);
    }
    else {
        // shouldn't reach here
        throw DOMException(DOMException::INVALID_ACCESS_ERR,0);
    }

}

const XMLCh* XPathNamespaceImpl::getTextContent() const {
  return getNamespaceURI(); 
}

const XMLCh* XPathNamespaceImpl::getLocalName() const {
  return this->getPrefix();
}

//
//   Functions inherited from Node
//

           DOMNamedNodeMap* XPathNamespaceImpl::getAttributes() const                   {return fNode.getAttributes (); };
           DOMDocument*     XPathNamespaceImpl::getOwnerDocument() const                {return getOwnerElement()->getOwnerDocument(); };
           bool             XPathNamespaceImpl::hasAttributes() const                   {return fNode.hasAttributes(); };
           bool             XPathNamespaceImpl::isEqualNode(const DOMNode* arg) const   {return fNode.isEqualNode(arg); };

           DOMNode*         XPathNamespaceImpl::appendChild(DOMNode *newChild)          {return fNode.appendChild (newChild); };
           DOMNodeList*     XPathNamespaceImpl::getChildNodes() const                   {return fNode.getChildNodes (); };
           DOMNode*         XPathNamespaceImpl::getFirstChild() const                   {return fNode.getFirstChild (); };
           DOMNode*         XPathNamespaceImpl::getLastChild() const                    {return fNode.getLastChild (); };
           DOMNode*         XPathNamespaceImpl::getNextSibling() const                  {return fNode.getNextSibling (); };

//The exception is wrong - we need to sort out all these methods when we can compile again.
           DOMNode*         XPathNamespaceImpl::getParentNode() const                   {return fNode.getParentNode ();};
           DOMNode*         XPathNamespaceImpl::getPreviousSibling() const              {return fNode.getParentNode ();};
           bool             XPathNamespaceImpl::hasChildNodes() const                   {return fNode.hasChildNodes ();};
           DOMNode*         XPathNamespaceImpl::insertBefore(DOMNode *newChild, DOMNode *refChild)
                                                                                    {return fNode.insertBefore (newChild, refChild); };
           void             XPathNamespaceImpl::normalize()                             {fNode.normalize (); };
           DOMNode*         XPathNamespaceImpl::removeChild(DOMNode *oldChild)          {return fNode.removeChild (oldChild); };
           DOMNode*         XPathNamespaceImpl::replaceChild(DOMNode *newChild, DOMNode *oldChild)
                                                                                    {return fNode.replaceChild (newChild, oldChild); };
           bool             XPathNamespaceImpl::isSupported(const XMLCh *feature, const XMLCh *version) const
                                                                                    {return fNode.isSupported (feature, version); };
           bool             XPathNamespaceImpl::isSameNode(const DOMNode* other) const       {return fNode.isSameNode(other); };
           void*            XPathNamespaceImpl::getUserData(const XMLCh* key) const     {return fNode.getUserData(key); };
           void             XPathNamespaceImpl::setTextContent(const XMLCh* textContent){fNode.setTextContent(textContent); };
#if _XERCES_VERSION >= 20100
#if _XERCES_VERSION >= 30000
           const XMLCh*     XPathNamespaceImpl::lookupPrefix(const XMLCh* namespaceURI) const {return fNode.lookupPrefix(namespaceURI); };
#else
           const XMLCh*     XPathNamespaceImpl::lookupNamespacePrefix(const XMLCh* namespaceURI, bool useDefault) const {return fNode.lookupNamespacePrefix(namespaceURI, useDefault); };
#endif
           bool             XPathNamespaceImpl::isDefaultNamespace(const XMLCh* namespaceURI) const {return fNode.isDefaultNamespace(namespaceURI); };
           const XMLCh*     XPathNamespaceImpl::lookupNamespaceURI(const XMLCh* prefix) const {return fNode.lookupNamespaceURI(prefix); };
#else
           const XMLCh*     XPathNamespaceImpl::lookupNamespacePrefix(const XMLCh* namespaceURI, bool useDefault) {return fNode.lookupNamespacePrefix(namespaceURI, useDefault); };
           bool             XPathNamespaceImpl::isDefaultNamespace(const XMLCh* namespaceURI) {return fNode.isDefaultNamespace(namespaceURI); };
           const XMLCh*     XPathNamespaceImpl::lookupNamespaceURI(const XMLCh* prefix) {return fNode.lookupNamespaceURI(prefix); };
#endif
 
           const XMLCh*     XPathNamespaceImpl::getBaseURI() const                      {return fNode.getBaseURI(); };

#if _XERCES_VERSION >= 30000
           void*            XPathNamespaceImpl::getFeature(const XMLCh* feature, const XMLCh* version) const {return fNode.getFeature(feature, version); }
#else
           DOMNode*         XPathNamespaceImpl::getInterface(const XMLCh* feature)      {return fNode.getInterface(feature); };
#endif

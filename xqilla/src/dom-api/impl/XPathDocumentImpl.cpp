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
#include "XPathDocumentImpl.hpp"
#include <xercesc/dom/impl/DOMDocumentTypeImpl.hpp>
#include <xercesc/dom/impl/DOMRangeImpl.hpp>
#include <xercesc/dom/impl/DOMCasts.hpp>
#include <xercesc/dom/DOMException.hpp>
#include <xercesc/util/OutOfMemoryException.hpp>

#if defined(XERCES_HAS_CPP_NAMESPACE)
XERCES_CPP_NAMESPACE_USE
#endif

XPathDocumentImpl::XPathDocumentImpl(DOMImplementation* domImpl, MemoryManager* memMgr)
#if _XERCES_VERSION >= 30000
  : DOMDocumentImpl(domImpl, memMgr),
#else
  : DOMDocumentImpl(memMgr),
#endif
    fMyDocType(NULL),
    fMyDocElement(NULL)
{
#if _XERCES_VERSION >= 30000
  setXmlVersion(XMLUni::fgVersion1_1);
#else
  setVersion(XMLUni::fgVersion1_1);
#endif
}

XPathDocumentImpl::~XPathDocumentImpl()
{
}

DOMNode *XPathDocumentImpl::insertBefore(DOMNode *newChild, DOMNode *refChild)
{
    // if the newChild is a documenttype node created from domimplementation, set the ownerDoc first
    if ((newChild->getNodeType() == DOMNode::DOCUMENT_TYPE_NODE) && !newChild->getOwnerDocument())
        ((DOMDocumentTypeImpl*)newChild)->setOwnerDocument(this);

    if(newChild==NULL)
        throw DOMException(DOMException::HIERARCHY_REQUEST_ERR,0, getMemoryManager());

    DOMNodeImpl *thisNodeImpl = castToNodeImpl(this);
    if (thisNodeImpl->isReadOnly())
        throw DOMException(DOMException::NO_MODIFICATION_ALLOWED_ERR, 0, getMemoryManager());

    DOMNode* thisNode = castToNode(&fParent);
    if (newChild->getOwnerDocument() != thisNode)
        throw DOMException(DOMException::WRONG_DOCUMENT_ERR, 0, getMemoryManager());

    // refChild must in fact be a child of this node (or 0)
    if (refChild!=0 && refChild->getParentNode() != thisNode)
        throw DOMException(DOMException::NOT_FOUND_ERR,0, getMemoryManager());

    // if the new node has to be placed before itself, we don't have to do anything 
    // (even worse, we would crash if we continue, as we assume they are two distinct nodes)
    if (refChild!=0 && newChild->isSameNode(refChild))
        return newChild;

    if (newChild->getNodeType() == DOMNode::DOCUMENT_FRAGMENT_NODE)
    {
        // SLOW BUT SAFE: We could insert the whole subtree without
        // juggling so many next/previous pointers. (Wipe out the
        // parent's child-list, patch the parent pointers, set the
        // ends of the list.) But we know some subclasses have special-
        // case behavior they add to insertBefore(), so we don't risk it.
        // This approch also takes fewer bytecodes.

        while(newChild->hasChildNodes())     // Move
            insertBefore(newChild->getFirstChild(),refChild);
    }

    else
    {
        DOMNode *oldparent=newChild->getParentNode();
        if(oldparent!=0)
            oldparent->removeChild(newChild);

        // Attach up
        castToNodeImpl(newChild)->fOwnerNode = thisNode;
        castToNodeImpl(newChild)->isOwned(true);

        // Attach before and after
        // Note: fFirstChild.previousSibling == lastChild!!
        if (fParent.fFirstChild == 0) {
            // this our first and only child
            fParent.fFirstChild = newChild;
            castToNodeImpl(newChild)->isFirstChild(true);
            // castToChildImpl(newChild)->previousSibling = newChild;
            DOMChildNode *newChild_ci = castToChildImpl(newChild);
            newChild_ci->previousSibling = newChild;
        } else {
            if (refChild == 0) {
                // this is an append
                DOMNode *lastChild = castToChildImpl(fParent.fFirstChild)->previousSibling;
                castToChildImpl(lastChild)->nextSibling = newChild;
                castToChildImpl(newChild)->previousSibling = lastChild;
                castToChildImpl(fParent.fFirstChild)->previousSibling = newChild;
            } else {
                // this is an insert
                if (refChild == fParent.fFirstChild) {
                    // at the head of the list
                    castToNodeImpl(fParent.fFirstChild)->isFirstChild(false);
                    castToChildImpl(newChild)->nextSibling = fParent.fFirstChild;
                    castToChildImpl(newChild)->previousSibling = castToChildImpl(fParent.fFirstChild)->previousSibling;
                    castToChildImpl(fParent.fFirstChild)->previousSibling = newChild;
                    fParent.fFirstChild = newChild;
                    castToNodeImpl(newChild)->isFirstChild(true);
                } else {
                    // somewhere in the middle
                    DOMNode *prev = castToChildImpl(refChild)->previousSibling;
                    castToChildImpl(newChild)->nextSibling = refChild;
                    castToChildImpl(prev)->nextSibling = newChild;
                    castToChildImpl(refChild)->previousSibling = newChild;
                    castToChildImpl(newChild)->previousSibling = prev;
                }
            }
        }
    }

    changed();

    Ranges* ranges = getRanges();
    if ( ranges != 0) {
        XMLSize_t sz = ranges->size();
        if (sz != 0) {
            for (XMLSize_t i =0; i<sz; i++) {
                ranges->elementAt(i)->updateRangeForInsertedNode(newChild);
            }
        }
    }

    // If insert succeeded, cache the kid appropriately
    if(newChild->getNodeType() == DOMNode::ELEMENT_NODE)
        fMyDocElement=(DOMElement *)newChild;
    else if(newChild->getNodeType() == DOMNode::DOCUMENT_TYPE_NODE)
        fMyDocType=(DOMDocumentType *)newChild;

    return newChild;
}

DOMNode* XPathDocumentImpl::replaceChild(DOMNode *newChild, DOMNode *oldChild) {
    DOMDocumentType* tempDocType = fMyDocType;
    DOMElement* tempDocElement = fMyDocElement;

    if(oldChild->getNodeType() == DOMNode::DOCUMENT_TYPE_NODE)
        fMyDocType=0;
    else if(oldChild->getNodeType() == DOMNode::ELEMENT_NODE)
        fMyDocElement=0;

    try {
        insertBefore(newChild, oldChild);
        // changed() already done.

        if((oldChild->getNodeType() == DOMNode::DOCUMENT_TYPE_NODE)
        || (oldChild->getNodeType() == DOMNode::ELEMENT_NODE))
            return fParent.removeChild(oldChild);
        else
            return removeChild(oldChild);
    }
    catch(const OutOfMemoryException&)
    {
        throw;
    }
    catch(...) {
        fMyDocType = tempDocType;
        fMyDocElement = tempDocElement;
        throw;
    }
}

DOMNode *XPathDocumentImpl::removeChild(DOMNode *oldChild)
{
    fParent.removeChild(oldChild);

    // If remove succeeded, un-cache the kid appropriately
    if(oldChild->getNodeType() == DOMNode::ELEMENT_NODE)
        fMyDocElement=0;
    else if(oldChild->getNodeType() == DOMNode::DOCUMENT_TYPE_NODE)
        fMyDocType=0;

    return oldChild;
}

DOMDocumentType *XPathDocumentImpl::getDoctype() const
{
    return fMyDocType;
}

DOMElement *XPathDocumentImpl::getDocumentElement() const
{
    return fMyDocElement;
}

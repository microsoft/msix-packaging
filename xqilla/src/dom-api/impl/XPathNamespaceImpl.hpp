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

#ifndef __XPATHNAMESPACEIMPL_HPP
#define __XPATHNAMESPACEIMPL_HPP

#include <xqilla/framework/XQillaExport.hpp>

#include <xercesc/util/XercesDefs.hpp>
#include <xercesc/dom/impl/DOMNodeImpl.hpp>
#include <xercesc/dom/DOMXPathNamespace.hpp>

XERCES_CPP_NAMESPACE_BEGIN
class DOMElement;
class DOMDocument;
XERCES_CPP_NAMESPACE_END


class XQILLA_API XPathNamespaceImpl : public XERCES_CPP_NAMESPACE_QUALIFIER DOMXPathNamespace
{
public:
	XERCES_CPP_NAMESPACE_QUALIFIER DOMNodeImpl fNode;
    const XMLCh *uri;
    const XMLCh *prefix;

    enum XPathObjectType {
      XPATH_NAMESPACE_OBJECT = 13
    };

public:
	//Main constructor
	XPathNamespaceImpl(const XMLCh* const prefix, const XMLCh* const uri, XERCES_CPP_NAMESPACE_QUALIFIER DOMElement *owner, XERCES_CPP_NAMESPACE_QUALIFIER DOMDocument *docOwner);
	XPathNamespaceImpl(const XPathNamespaceImpl &other);
	virtual ~XPathNamespaceImpl();

	// Declare functions from DOMNode.  They all must be implemented by this class
    virtual       XERCES_CPP_NAMESPACE_QUALIFIER DOMNode*         appendChild(XERCES_CPP_NAMESPACE_QUALIFIER DOMNode *newChild) ;
    virtual       XERCES_CPP_NAMESPACE_QUALIFIER DOMNode*         cloneNode(bool deep) const ;
    virtual       XERCES_CPP_NAMESPACE_QUALIFIER DOMNamedNodeMap* getAttributes() const ;
    virtual       XERCES_CPP_NAMESPACE_QUALIFIER DOMNodeList*     getChildNodes() const ;
    virtual       XERCES_CPP_NAMESPACE_QUALIFIER DOMNode*         getFirstChild() const ;
    virtual       XERCES_CPP_NAMESPACE_QUALIFIER DOMNode*         getLastChild() const ;
    virtual const XMLCh*           getLocalName() const ;
    virtual const XMLCh*           getNamespaceURI() const ;
    virtual       XERCES_CPP_NAMESPACE_QUALIFIER DOMNode*         getNextSibling() const ;
    virtual const XMLCh*           getNodeName() const ;
#if _XERCES_VERSION >= 30000
    virtual       NodeType         getNodeType() const;
#else
    virtual       short            getNodeType() const;
#endif
    virtual const XMLCh*           getNodeValue() const ;
    virtual       XERCES_CPP_NAMESPACE_QUALIFIER DOMDocument*     getOwnerDocument() const ;
    virtual const XMLCh*           getPrefix() const ;
    virtual       XERCES_CPP_NAMESPACE_QUALIFIER DOMNode*         getParentNode() const ;
    virtual       XERCES_CPP_NAMESPACE_QUALIFIER DOMNode*         getPreviousSibling() const ;
    virtual       bool             hasChildNodes() const ;
    virtual       XERCES_CPP_NAMESPACE_QUALIFIER DOMNode*         insertBefore(XERCES_CPP_NAMESPACE_QUALIFIER DOMNode *newChild, XERCES_CPP_NAMESPACE_QUALIFIER DOMNode *refChild) ;
    virtual       void             normalize() ;
    virtual       XERCES_CPP_NAMESPACE_QUALIFIER DOMNode*         removeChild(XERCES_CPP_NAMESPACE_QUALIFIER DOMNode *oldChild) ;
    virtual       XERCES_CPP_NAMESPACE_QUALIFIER DOMNode*         replaceChild(XERCES_CPP_NAMESPACE_QUALIFIER DOMNode *newChild, XERCES_CPP_NAMESPACE_QUALIFIER DOMNode *oldChild) ;
    virtual       void             setNodeValue(const XMLCh  *nodeValue) ;
    virtual       bool             isSupported(const XMLCh *feature, const XMLCh *version) const ;
    virtual       bool             hasAttributes() const ;
    virtual       void             setPrefix(const XMLCh * prefix) ;
    virtual       void*            setUserData(const XMLCh* key, void* data, XERCES_CPP_NAMESPACE_QUALIFIER DOMUserDataHandler* handler) ;
    virtual       void*            getUserData(const XMLCh* key) const ;
    virtual       bool             isSameNode(const XERCES_CPP_NAMESPACE_QUALIFIER DOMNode* other) const;
    virtual       bool             isEqualNode(const XERCES_CPP_NAMESPACE_QUALIFIER DOMNode* arg) const;
    virtual const XMLCh*           getBaseURI() const ;
#if _XERCES_VERSION >= 30000
    virtual short                  compareDocumentPosition(const XERCES_CPP_NAMESPACE_QUALIFIER DOMNode* other) const ;
#else
    virtual short                  compareTreePosition(const XERCES_CPP_NAMESPACE_QUALIFIER DOMNode* other) const ;
#endif
    virtual const XMLCh*           getTextContent() const ;
    virtual void                   setTextContent(const XMLCh* textContent) ;
#if _XERCES_VERSION >= 30000
    virtual const XMLCh*      lookupPrefix(const XMLCh* namespaceURI) const;
#else
    virtual const XMLCh*           lookupNamespacePrefix(const XMLCh* namespaceURI, bool useDefault) const  ;
#endif
    virtual bool                   isDefaultNamespace(const XMLCh* namespaceURI) const;
    virtual const XMLCh*           lookupNamespaceURI(const XMLCh* prefix) const  ;
#if _XERCES_VERSION >= 30000
    virtual void*             getFeature(const XMLCh* feature, const XMLCh* version) const;
#else
    virtual       XERCES_CPP_NAMESPACE_QUALIFIER DOMNode*         getInterface(const XMLCh* feature) ;
#endif
    virtual       void             release();


	virtual XERCES_CPP_NAMESPACE_QUALIFIER DOMElement *getOwnerElement() const;
};



#endif

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

#ifndef __XPATHDOCUMENTIMPL_HPP
#define __XPATHDOCUMENTIMPL_HPP

#include <xqilla/framework/XQillaExport.hpp>

#include <xercesc/util/XercesDefs.hpp>
#include <xercesc/dom/impl/DOMDocumentImpl.hpp>

class XQILLA_API XPathDocumentImpl : public XERCES_CPP_NAMESPACE_QUALIFIER DOMDocumentImpl
{
public:
  XPathDocumentImpl(XERCES_CPP_NAMESPACE_QUALIFIER DOMImplementation* domImpl, XERCES_CPP_NAMESPACE_QUALIFIER MemoryManager* memMgr);
  virtual ~XPathDocumentImpl();

  virtual XERCES_CPP_NAMESPACE_QUALIFIER DOMNode*             insertBefore(XERCES_CPP_NAMESPACE_QUALIFIER DOMNode *newChild, XERCES_CPP_NAMESPACE_QUALIFIER DOMNode *refChild);
  virtual XERCES_CPP_NAMESPACE_QUALIFIER DOMNode*             removeChild(XERCES_CPP_NAMESPACE_QUALIFIER DOMNode *oldChild);
  virtual XERCES_CPP_NAMESPACE_QUALIFIER DOMNode*             replaceChild(XERCES_CPP_NAMESPACE_QUALIFIER DOMNode *newChild, XERCES_CPP_NAMESPACE_QUALIFIER DOMNode *oldChild);
  virtual XERCES_CPP_NAMESPACE_QUALIFIER DOMDocumentType*     getDoctype() const;
  virtual XERCES_CPP_NAMESPACE_QUALIFIER DOMElement*          getDocumentElement() const;

protected:
  XERCES_CPP_NAMESPACE_QUALIFIER DOMDocumentType*      fMyDocType;
  XERCES_CPP_NAMESPACE_QUALIFIER DOMElement*           fMyDocElement;

};



#endif

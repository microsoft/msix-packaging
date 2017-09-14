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
#include "XPath2NodeSerializer.hpp"

#include <xercesc/dom/DOMXPathNamespace.hpp>
#include <xercesc/dom/DOMNode.hpp>
#include <xercesc/framework/XMLFormatter.hpp>
#include <xercesc/util/XMLUniDefs.hpp>
#include <xercesc/util/XercesDefs.hpp>

XERCES_CPP_NAMESPACE_USE;

XPath2NodeSerializer::XPath2NodeSerializer(MemoryManager* const manager)
  :
#if _XERCES_VERSION >= 30000
  DOMLSSerializerImpl(manager)
#else
  DOMWriterImpl(manager)
#endif
{
}
  
XPath2NodeSerializer::~XPath2NodeSerializer() {
  // nothing to do
}


bool XPath2NodeSerializer::customNodeSerialize(const DOMNode* const nodeToWrite, int level) {

  switch (nodeToWrite->getNodeType ()) {
    case DOMXPathNamespace::XPATH_NAMESPACE_NODE: 
      {
        const XMLCh* localName = nodeToWrite->getLocalName();
        const XMLCh* namespaceURI = nodeToWrite->getNamespaceURI();

        *fFormatter << XMLFormatter::NoEscapes
                    << chOpenSquare
                    << localName
                    << chEqual 
                    << chDoubleQuote
                    << namespaceURI
                    << chDoubleQuote 
                    << chCloseSquare;
       
        return true;
        break;

      }
    default: 
      {
        return false;
        break;
      }
  }

}

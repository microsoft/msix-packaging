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
#include <xqilla/utils/XPath2NSUtils.hpp>
#include <xqilla/utils/XPath2Utils.hpp>
#include <xqilla/context/DynamicContext.hpp>
#include <xqilla/utils/XStr.hpp>

#include <xercesc/dom/DOMAttr.hpp>
#include <xercesc/dom/DOMElement.hpp>
#include <xercesc/dom/DOMNamedNodeMap.hpp>
#include <xercesc/util/XMLUniDefs.hpp>
#include <xercesc/util/XMLString.hpp>

XERCES_CPP_NAMESPACE_USE;

const XMLCh* XPath2NSUtils::getLocalName(const XMLCh* name)
{
  unsigned int len=XPath2Utils::uintStrlen(name);

  for(unsigned int i = 0; i < len; i++) {
    if(name[i] == chColon) {
      return name+i+1;
    }
  }
  return name;
}

const XMLCh* XPath2NSUtils::getPrefix(const XMLCh* name, XPath2MemoryManager* memMgr)
{
  unsigned int len=XPath2Utils::uintStrlen(name);
  
  for(unsigned int i = 0; i < len; i++) {
    if(name[i] == chColon) {
      return XPath2Utils::subString(name, 0, i, memMgr);
    }
  }
  return XMLUni::fgZeroLenString;
}

const XMLCh* XPath2NSUtils::qualifyName(const XMLCh* prefix, const XMLCh* name, XPath2MemoryManager* memMgr)
{
  if (prefix != NULL) {
    XMLCh colon[2] = {chColon, chNull};
    return XPath2Utils::concatStrings(prefix, colon, name, memMgr);
  } else {
    return name;
  }
}

DOMNode *XPath2NSUtils::getParent(const DOMNode *node)
{
  if(node->getNodeType() == DOMNode::ATTRIBUTE_NODE) {
    return (static_cast<const DOMAttr *>(node))->getOwnerElement();
  }
  else {
    return node->getParentNode();
  }
}


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
#include "AttributeAxis.hpp"
#include <xqilla/axis/NodeTest.hpp>

#include <xercesc/util/XMLUniDefs.hpp>

#if defined(XERCES_HAS_CPP_NAMESPACE)
XERCES_CPP_NAMESPACE_USE
#endif

AttributeAxis::AttributeAxis(const LocationInfo *info, const XERCES_CPP_NAMESPACE_QUALIFIER DOMNode *contextNode, const Node *nodeObj,
       const NodeTest *nodeTest, const AxisNodeFactory &factory)
  : Axis(info, contextNode, nodeObj, nodeTest, factory),
    nodeMap_(contextNode->getAttributes()),
    i_(0)
{
}

const DOMNode *AttributeAxis::nextNode(DynamicContext *context)
{
  const DOMNode *result = 0;
  if(nodeMap_ != 0) {
    unsigned int nLen = (unsigned int) nodeMap_->getLength();
    if(i_==0 && nodeTest_!=0 && nodeTest_->getItemType()==0 && 
       !nodeTest_->getNamespaceWildcard() && !nodeTest_->getNameWildcard())
    {
      const XMLCh* uri=nodeTest_->getNodeUri();
      if(uri==0)
        result=nodeMap_->getNamedItem(nodeTest_->getNodeName());
      else
        result=nodeMap_->getNamedItemNS(uri,nodeTest_->getNodeName());
      i_=nLen;
    }
    while(result == 0 && i_ < nLen) {
      result = nodeMap_->item(i_);
      ++i_;
      //Check to see if this attribute starts with "xmlns:" or is equal to "xmlns"
      const XMLCh* name=result->getNodeName();
      const XMLCh* xmlns=XMLUni::fgXMLNSString;
      while(*name && *xmlns)
      {
        if(*name++!=*xmlns++)
          break;
      }
      if(*xmlns==0 && (*name==0 || *name==chColon))
        result = 0;
    }
  }

  return result;
}

std::string AttributeAxis::asString(DynamicContext *context, int indent) const
{
  return "AttributeAxis";
}


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

#ifndef _NAMESPACEAXIS_HPP
#define _NAMESPACEAXIS_HPP

#include <set>

#include "Axis.hpp"
#include <xqilla/utils/XMLChCompare.hpp>

#include <xercesc/dom/DOMNamedNodeMap.hpp>

class XQILLA_API NamespaceAxis : public Axis
{
public:
  NamespaceAxis(const LocationInfo *info, const XERCES_CPP_NAMESPACE_QUALIFIER DOMNode *contextNode, const Node *nodeObj,
       const NodeTest *nodeTest, const AxisNodeFactory &factory);

  const XERCES_CPP_NAMESPACE_QUALIFIER DOMNode *nextNode(DynamicContext *context);
  std::string asString(DynamicContext *context, int indent) const;

private:
  const XERCES_CPP_NAMESPACE_QUALIFIER DOMNode *node_, *originalNode_;
  XERCES_CPP_NAMESPACE_QUALIFIER DOMNamedNodeMap *nodeMap_;
  unsigned int i_;

  enum {
    CHECK_ELEMENT,
    CHECK_ATTR,
    DO_XML,
    DONE
  } state_;

  typedef std::set<const XMLCh*, XMLChSort> DoneSet;
  DoneSet done_;
};

#endif


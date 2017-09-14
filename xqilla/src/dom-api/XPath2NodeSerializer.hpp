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

#ifndef __XPATH2NODESERIALIZER_HPP
#define __XPATH2NODESERIALIZER_HPP

#include <xqilla/framework/XQillaExport.hpp>

#include <xercesc/util/XercesVersion.hpp>
#if _XERCES_VERSION >= 30000
#include <xercesc/dom/impl/DOMLSSerializerImpl.hpp>
#else
#include <xercesc/dom/impl/DOMWriterImpl.hpp>
#endif

XERCES_CPP_NAMESPACE_BEGIN

class DOMNode;
class XMLFormatter;

XERCES_CPP_NAMESPACE_END

class XQILLA_API XPath2NodeSerializer :
#if _XERCES_VERSION >= 30000
  public XERCES_CPP_NAMESPACE_QUALIFIER DOMLSSerializerImpl
#else
  public XERCES_CPP_NAMESPACE_QUALIFIER DOMWriterImpl
#endif
{
public:
  XPath2NodeSerializer(XERCES_CPP_NAMESPACE_QUALIFIER MemoryManager* const manager = XERCES_CPP_NAMESPACE_QUALIFIER XMLPlatformUtils::fgMemoryManager);
  virtual ~XPath2NodeSerializer();

protected:

  virtual bool customNodeSerialize(const XERCES_CPP_NAMESPACE_QUALIFIER DOMNode* const nodeToWrite, int level);

};

#endif


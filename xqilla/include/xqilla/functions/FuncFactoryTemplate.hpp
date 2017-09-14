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

#ifndef _FUNCFACTORYTEMPLATE_HPP
#define _FUNCFACTORYTEMPLATE_HPP

#include <xqilla/functions/FuncFactory.hpp>

template<class TYPE>
class FuncFactoryTemplate : public FuncFactory
{
public:

  FuncFactoryTemplate(XERCES_CPP_NAMESPACE_QUALIFIER MemoryManager *memMgr)
    : FuncFactory(TYPE::XMLChFunctionURI, TYPE::name, TYPE::minArgs, TYPE::maxArgs, memMgr)
  {
  }

  FuncFactoryTemplate(const XMLCh *uri, const XMLCh *name, size_t minArgs, size_t maxArgs,
                      XERCES_CPP_NAMESPACE_QUALIFIER MemoryManager *mm)
    : FuncFactory(uri, name, minArgs, maxArgs, mm)
  {
  }
  
  virtual ASTNode *createInstance(const VectorOfASTNodes &args, XPath2MemoryManager* memMgr) const
  {
    return new (memMgr) TYPE(args, memMgr);
  }
};

#endif

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

#ifndef __XQILLAXMLGRAMMARPOOLIMPL_HPP
#define __XQILLAXMLGRAMMARPOOLIMPL_HPP

#include <xqilla/framework/XQillaExport.hpp>

#include <xercesc/util/XercesVersion.hpp>

#if _XERCES_VERSION >= 30000
#include <xercesc/framework/XMLGrammarPoolImpl.hpp>
#else
#include <xercesc/internal/XMLGrammarPoolImpl.hpp>
#endif


XERCES_CPP_NAMESPACE_BEGIN
class Grammar;
class MemoryManager;
XERCES_CPP_NAMESPACE_END  

class XQillaXMLGrammarPoolImpl : public XERCES_CPP_NAMESPACE_QUALIFIER XMLGrammarPoolImpl
{
public:

    XQillaXMLGrammarPoolImpl(XERCES_CPP_NAMESPACE_QUALIFIER MemoryManager* const memMgr);

    /**
     * This method is overriden to not throw exceptions when a repeated grammar is found, but just ignore them
     **/
    virtual bool cacheGrammar(XERCES_CPP_NAMESPACE_QUALIFIER Grammar* const  gramToCache);
};

#endif

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

#include <xqilla/xerces/XercesConfiguration.hpp>
#include <xqilla/schema/DocumentCacheImpl.hpp>
#include "../schema/FaxppDocumentCacheImpl.hpp"
#include <xqilla/context/impl/ItemFactoryImpl.hpp>
#include <xqilla/context/DynamicContext.hpp>
#include <xqilla/utils/XPath2Utils.hpp>
#include <xqilla/optimizer/ASTToXML.hpp>

#include "XercesUpdateFactory.hpp"
#include "XercesNodeImpl.hpp"
#include "XercesURIResolver.hpp"
#include "XercesSequenceBuilder.hpp"

XERCES_CPP_NAMESPACE_USE;

const XMLCh XercesConfiguration::gXerces[] =   // Points to "Xerces"
{
  chLatin_X, chLatin_e, chLatin_r, chLatin_c, chLatin_e, chLatin_s, chNull
};

DocumentCache *XercesConfiguration::createDocumentCache(MemoryManager *memMgr)
{
#ifdef HAVE_FAXPP
  return new (memMgr) FaxppDocumentCacheImpl(memMgr);
#else
  return new (memMgr) DocumentCacheImpl(memMgr);
#endif
}

SequenceBuilder *XercesConfiguration::createSequenceBuilder(const DynamicContext *context)
{
  return new (context->getMemoryManager()) XercesSequenceBuilder(context);
}

ItemFactory *XercesConfiguration::createItemFactory(DocumentCache *cache,
                                                    MemoryManager *memMgr)
{
  return new (memMgr) ItemFactoryImpl(cache, memMgr);
}

UpdateFactory *XercesConfiguration::createUpdateFactory(MemoryManager *memMgr)
{
  return new (memMgr) XercesUpdateFactory();
}

URIResolver *XercesConfiguration::createDefaultURIResolver(MemoryManager *memMgr)
{
  return new (memMgr) XercesURIResolver(memMgr);
}

ASTToXML *XercesConfiguration::createASTToXML()
{
  return new ASTToXML();
}

Node::Ptr XercesConfiguration::createNode(const DOMNode *node, const DynamicContext *context) const
{
  return new XercesNodeImpl(node, 0);
}

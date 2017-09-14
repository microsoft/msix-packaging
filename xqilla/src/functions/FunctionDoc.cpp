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
#include <xqilla/functions/FunctionDoc.hpp>
#include <xqilla/context/DynamicContext.hpp>
#include <xqilla/exceptions/FunctionException.hpp>
#include <xqilla/exceptions/XMLParseException.hpp>
#include <xqilla/ast/StaticAnalysis.hpp>
#include <xqilla/utils/XPath2Utils.hpp>

#include <xercesc/util/XMLString.hpp>

XERCES_CPP_NAMESPACE_USE;

const XMLCh FunctionDoc::name[] = {
  chLatin_d, chLatin_o, chLatin_c, 
  chNull 
};
const unsigned int FunctionDoc::minArgs = 1;
const unsigned int FunctionDoc::maxArgs = 1;

/**
 * fn:doc($uri as xs:string?) as document?
 **/
FunctionDoc::FunctionDoc(const VectorOfASTNodes &args, XPath2MemoryManager* memMgr)
  : XQFunction(name, "($uri as xs:string?) as document-node()?", args, memMgr),
    queryPathTree_(0)
{
}

ASTNode *FunctionDoc::staticTypingImpl(StaticContext *context)
{
  _src.clearExceptType();
  calculateSRCForArguments(context);

  if(_args[0]->getStaticAnalysis().getStaticType().getMin() == 0)
	  _src.getStaticType() = StaticType(StaticType::DOCUMENT_TYPE, 0, 1);
  else _src.getStaticType() = StaticType(StaticType::DOCUMENT_TYPE, 1, 1);

  _src.setProperties(StaticAnalysis::DOCORDER | StaticAnalysis::GROUPED |
                     StaticAnalysis::PEER | StaticAnalysis::SUBTREE | StaticAnalysis::ONENODE);
  _src.availableDocumentsUsed(true);

  return this;
}

Sequence FunctionDoc::createSequence(DynamicContext* context, int flags) const
{
  Item::Ptr uriArg = getParamNumber(1,context)->next(context);
  
  if(uriArg.isNull()) {
    return Sequence(context->getMemoryManager());
  }
  
  const XMLCh *uri = uriArg->asString(context);

  // on Windows, we can have URIs using \ instead of /; let's normalize them
  if(uri != 0) {
    unsigned int len = XPath2Utils::uintStrlen(uri);
    AutoDeleteArray<XMLCh> newURI(new XMLCh[len + 1]);

    const XMLCh *src = uri;
    XMLCh *dst = newURI;
    while(*src != 0) {
      if(*src == '\\') *dst = '/';
      else *dst = *src;

      ++src; ++dst;
    }
    *dst = 0;

    uri = context->getMemoryManager()->getPooledString(newURI);
  }

  if(!XPath2Utils::isValidURI(uri, context->getMemoryManager()))
    XQThrow(FunctionException, X("FunctionDoc::createSequence"), X("Invalid argument to fn:doc function [err:FODC0005]"));

  return context->resolveDocument(uri, this, context->getProjection() ? queryPathTree_ : 0);
}

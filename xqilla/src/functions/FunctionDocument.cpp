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
#include <xqilla/functions/FunctionDocument.hpp>
#include <xqilla/context/DynamicContext.hpp>
#include <xqilla/exceptions/FunctionException.hpp>
#include <xqilla/ast/StaticAnalysis.hpp>
#include <xqilla/utils/XPath2Utils.hpp>
#include <xercesc/util/XMLUri.hpp>
#include <xercesc/util/XMLURL.hpp>

#include <xercesc/util/XMLString.hpp>

XERCES_CPP_NAMESPACE_USE;

const XMLCh FunctionDocument::name[] = {
  chLatin_d, chLatin_o, chLatin_c, chLatin_u, chLatin_m, chLatin_e, chLatin_n, chLatin_t,
  chNull 
};
const unsigned int FunctionDocument::minArgs = 1;
const unsigned int FunctionDocument::maxArgs = 2;

/**
 * document($uri-sequence as item()*) as node()*
 * document($uri-sequence as item()*, $base-node as node()) as node()* 
 **/
FunctionDocument::FunctionDocument(const VectorOfASTNodes &args, XPath2MemoryManager* memMgr)
  : XQFunction(name, "($uri-sequence as item()*, $base-node as node()) as node()*", args, memMgr),
    queryPathTree_(0)
{
}

ASTNode *FunctionDocument::staticTypingImpl(StaticContext *context)
{
  _src.clearExceptType();
  calculateSRCForArguments(context);
  _src.setProperties(StaticAnalysis::DOCORDER | StaticAnalysis::GROUPED |
                     StaticAnalysis::PEER | StaticAnalysis::SUBTREE | StaticAnalysis::ONENODE);

  // TBD Change if we support fragment identifiers - jpcs
  _src.getStaticType() = StaticType(StaticType::DOCUMENT_TYPE, 0, StaticType::UNLIMITED);
  _src.availableDocumentsUsed(true);

  return this;
}

Sequence FunctionDocument::createSequence(DynamicContext* context, int flags) const
{
  const XMLCh *baseURI;
  if(getNumArgs() == 2) {
    baseURI = ((Node*)getParamNumber(2, context)->next(context).get())->dmBaseURI(context).first()->asString(context);
  }
  else {
    baseURI = context->getBaseURI();
  }

  Sequence result(context->getMemoryManager());
  Result args = getParamNumber(1, context);
  Item::Ptr uriArg;
  while((uriArg = args->next(context)).notNull()) {
    const XMLCh *uri = uriArg->asString(context);
    if(!XPath2Utils::isValidURI(uri, context->getMemoryManager()))
      XQThrow(FunctionException, X("FunctionDocument::createSequence"), X("Invalid argument to fn:document function [err:FODC0005]"));

    try {
      XMLUri base(baseURI);
      XMLUri full(&base, uri);
      uri = context->getMemoryManager()->getPooledString(full.getUriText());
    }
    catch(MalformedURLException &e){
      XQThrow(FunctionException, X("FunctionDocument::createSequence"), X("Invalid argument to resolve-uri [err:FORG0002]"));
    }

    result.joinSequence(context->resolveDocument(uri, this, context->getProjection() ? queryPathTree_ : 0));
  }

  return result;
}

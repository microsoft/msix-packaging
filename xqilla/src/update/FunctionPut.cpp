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
#include <xqilla/update/FunctionPut.hpp>
#include <xqilla/update/PendingUpdateList.hpp>
#include <xqilla/context/DynamicContext.hpp>
#include <xqilla/exceptions/FunctionException.hpp>
#include <xqilla/exceptions/XMLParseException.hpp>
#include <xqilla/ast/StaticAnalysis.hpp>
#include <xqilla/utils/XPath2Utils.hpp>
#include <xercesc/util/XMLString.hpp>

#include <xercesc/util/XMLUri.hpp>

#if defined(XERCES_HAS_CPP_NAMESPACE)
XERCES_CPP_NAMESPACE_USE
#endif

const XMLCh FunctionPut::name[] = {
  XERCES_CPP_NAMESPACE_QUALIFIER chLatin_p, XERCES_CPP_NAMESPACE_QUALIFIER chLatin_u, XERCES_CPP_NAMESPACE_QUALIFIER chLatin_t, 
  XERCES_CPP_NAMESPACE_QUALIFIER chNull 
};
const unsigned int FunctionPut::minArgs = 1;
const unsigned int FunctionPut::maxArgs = 2;

/**
 * fn:put($node as node(), $uri as xs:string?) as empty-sequence()
 */
FunctionPut::FunctionPut(const VectorOfASTNodes &args, XPath2MemoryManager* memMgr)
  : XQFunction(name, "updating ($node as node(), $uri as xs:string?) as empty-sequence()", args, memMgr),
    baseURI_(0)
{
}

FunctionPut::FunctionPut(const XMLCh *baseURI, const VectorOfASTNodes &args, XPath2MemoryManager* memMgr)
  : XQFunction(name, "updating ($node as node(), $uri as xs:string?) as empty-sequence()", args, memMgr),
    baseURI_(baseURI)
{
}

ASTNode* FunctionPut::staticResolution(StaticContext *context)
{
  baseURI_ = context->getBaseURI();
  resolveArguments(context);
  return this;
}

ASTNode *FunctionPut::staticTypingImpl(StaticContext *context)
{
  _src.clear();

  _src.forceNoFolding(true);
  _src.updating(true);
  calculateSRCForArguments(context);
  return this;
}

PendingUpdateList FunctionPut::createUpdateList(DynamicContext *context) const
{
  Node::Ptr node = (Node*)getParamNumber(1,context)->next(context).get();

  if(node->dmNodeKind() != Node::document_string &&
     node->dmNodeKind() != Node::element_string) {
    XQThrow(FunctionException, X("FunctionPut::createSequence"),
            X("The argument to fn:put is not a document or element node [err:FOUP0001]"));
  }

  const XMLCh *uriArg = 0;
  if(getNumArgs() == 2) {
    Item::Ptr uri = getParamNumber(2,context)->next(context);
    if(uri.notNull())
      uriArg = uri->asString(context);
  }
  else {
    Sequence seq = node->dmDocumentURI(context);
    if(!seq.isEmpty())
      uriArg = seq.item(0)->asString(context);
  }

  if(uriArg == 0)
    XQThrow(FunctionException, X("FunctionPut::createSequence"),
            X("The uri argument to fn:put is empty [err:FOUP0002]"));

  if(!baseURI_)
    XQThrow(FunctionException, X("FunctionPut::createSequence"),
            X("Base uri undefined in the static context [err:FONS0005]"));

  Item::Ptr anyURI;
  try {
    XMLUri base(baseURI_);
    XMLUri resolved(&base, uriArg);

    anyURI = context->getItemFactory()->createAnyURI(resolved.getUriText(), context);
  }
  catch(XQException &e) {
    XQThrow(FunctionException, X("FunctionPut::createSequence"),
            X("The argument to fn:put is not a valid xs:anyURI [err:FOUP0002]"));
  }

  return PendingUpdate(PendingUpdate::PUT, node, anyURI, this);
}

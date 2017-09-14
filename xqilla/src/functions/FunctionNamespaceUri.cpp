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
#include <assert.h>
#include <xqilla/functions/FunctionNamespaceUri.hpp>
#include <xqilla/context/DynamicContext.hpp>
#include <xqilla/items/DatatypeFactory.hpp>
#include <xqilla/items/Node.hpp>
#include <xqilla/items/ATStringOrDerived.hpp>
#include <xqilla/ast/StaticAnalysis.hpp>
#include <xqilla/exceptions/FunctionException.hpp>
#include <xqilla/ast/XQContextItem.hpp>

XERCES_CPP_NAMESPACE_USE;

const XMLCh FunctionNamespaceUri::name[] = {
  chLatin_n, chLatin_a, chLatin_m, 
  chLatin_e, chLatin_s, chLatin_p, 
  chLatin_a, chLatin_c, chLatin_e, 
  chDash,    chLatin_u, chLatin_r, 
  chLatin_i, chNull 
};
const unsigned int FunctionNamespaceUri::minArgs = 0;
const unsigned int FunctionNamespaceUri::maxArgs = 1;

/**
 * fn:namespace-uri() as xs:anyURI
 * fn:namespace-uri($arg as node()?) as xs:anyURI
**/

FunctionNamespaceUri::FunctionNamespaceUri(const VectorOfASTNodes &args, XPath2MemoryManager* memMgr)
  : XQFunction(name, "($arg as node()?) as xs:anyURI", args, memMgr)
{
}

ASTNode* FunctionNamespaceUri::staticResolution(StaticContext *context)
{
  XPath2MemoryManager *mm = context->getMemoryManager();

  if(_args.empty()) {
    XQContextItem *ci = new (mm) XQContextItem(mm);
    ci->setLocationInfo(this);
    _args.push_back(ci);
  }

  resolveArguments(context);
  return this;
}

Sequence FunctionNamespaceUri::createSequence(DynamicContext* context, int flags) const
{
  XPath2MemoryManager* memMgr = context->getMemoryManager();

  Node::Ptr ctxNode = (Node*)getParamNumber(1, context)->next(context).get();
  if(ctxNode.notNull()) {
    ATQNameOrDerived::Ptr name = ctxNode->dmNodeName(context);
    if(name.notNull())
      return Sequence(context->getItemFactory()->createAnyURI(((const ATQNameOrDerived*)name.get())->getURI(), context), memMgr);
  }
  return Sequence(context->getItemFactory()->createAnyURI(XMLUni::fgZeroLenString, context), memMgr);
}

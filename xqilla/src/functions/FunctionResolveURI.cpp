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
#include <xqilla/functions/FunctionResolveURI.hpp>
#include <xqilla/items/ATStringOrDerived.hpp>
#include <xqilla/exceptions/FunctionException.hpp>
#include <xqilla/exceptions/StaticErrorException.hpp>
#include "../exceptions/InvalidLexicalSpaceException.hpp"
#include <xqilla/context/DynamicContext.hpp>
#include <xqilla/items/DatatypeFactory.hpp>
#include <xqilla/ast/XQLiteral.hpp>

#include <xercesc/util/XMLString.hpp>
#include <xercesc/util/XMLUri.hpp>

XERCES_CPP_NAMESPACE_USE;

const XMLCh FunctionResolveURI::name[] = {
  chLatin_r, chLatin_e, chLatin_s, 
  chLatin_o, chLatin_l, chLatin_v, 
  chLatin_e, chDash,    chLatin_u, 
  chLatin_r, chLatin_i, chNull 
};
const unsigned int FunctionResolveURI::minArgs = 1;
const unsigned int FunctionResolveURI::maxArgs = 2;

/**
 * fn:resolve-uri($relative as xs:string?) as xs:anyURI?
 * fn:resolve-uri($relative as xs:string?, $base as xs:string) as xs:anyURI?
 **/

FunctionResolveURI::FunctionResolveURI(const VectorOfASTNodes &args, XPath2MemoryManager* memMgr)
  : XQFunction(name, "($relative as xs:string?, $base as xs:string) as xs:anyURI?", args, memMgr)
{
}

ASTNode* FunctionResolveURI::staticResolution(StaticContext *context)
{
  if(getNumArgs() == 1) {
    if(!context->getBaseURI())
      XQThrow(StaticErrorException, X("FunctionResolveURI::staticResolution"),
              X("Base uri undefined in the static context [err:FONS0005]"));

    XPath2MemoryManager* mm = context->getMemoryManager();

    ASTNode *baseURI = new (mm) XQLiteral(SchemaSymbols::fgURI_SCHEMAFORSCHEMA, SchemaSymbols::fgDT_STRING,
                                          context->getBaseURI(), AnyAtomicType::STRING, mm);
    baseURI->setLocationInfo(this);
    _args.push_back(baseURI);
  }

  resolveArguments(context);
  return this;
}

Sequence FunctionResolveURI::createSequence(DynamicContext* context, int flags) const
{
  XPath2MemoryManager *memMgr = context->getMemoryManager();

  Item::Ptr relative = getParamNumber(1, context)->next(context);
  if(relative.isNull()) return Sequence(memMgr);

  const XMLCh *relativeURI = relative->asString(context);
  try {
    if(XMLUri::isValidURI(false, relativeURI))
      return Sequence(context->getItemFactory()->createAnyURI(relativeURI, context), memMgr); 
  }
  catch(InvalidLexicalSpaceException &e){
    XQThrow(FunctionException, X("FunctionResolveURI::createSequence"), X("Invalid argument to resolve-uri [err:FORG0002]"));
  }

  try {
    const XMLCh *baseURI = getParamNumber(2, context)->next(context)->asString(context);

    if(!XMLUri::isValidURI(true, relativeURI))
      XQThrow(FunctionException, X("FunctionResolveURI::createSequence"),
              X("Invalid relative uri argument to resolve-uri [err:FORG0002]"));
    if(!XMLUri::isValidURI(false, baseURI))
      XQThrow(FunctionException, X("FunctionResolveURI::createSequence"),
              X("Invalid base-uri argument to resolve-uri [err:FORG0002]"));
  
    try {
      XMLUri base(baseURI);
      XMLUri full(&base, relativeURI);

      return Sequence(context->getItemFactory()->createAnyURI(full.getUriText(), context), memMgr); 
    }
    catch(InvalidLexicalSpaceException &e){
      XQThrow(FunctionException, X("FunctionResolveURI::createSequence"), X("Invalid argument to resolve-uri [err:FORG0002]"));
    }

  }
  catch(XMLException &e) {
    //if can't build, assume its cause there was a relative URI given
    XQThrow(FunctionException, X("FunctionResolveURI::createSequence"), X("Relative URI base argument to resolve-uri [err:FORG0009]"));
  }
  
  //should not get here
  assert(0);
}

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
#include <xqilla/functions/FunctionDocAvailable.hpp>
#include <xqilla/context/DynamicContext.hpp>
#include <xqilla/exceptions/FunctionException.hpp>
#include <xqilla/items/ATBooleanOrDerived.hpp>
#include <xqilla/items/DatatypeFactory.hpp>
#include <xqilla/ast/StaticAnalysis.hpp>
#include <xqilla/utils/XPath2Utils.hpp>
#include <xercesc/util/XMLString.hpp>

XERCES_CPP_NAMESPACE_USE;

const XMLCh FunctionDocAvailable::name[] = {
  chLatin_d, chLatin_o, chLatin_c, 
  chDash,    chLatin_a, chLatin_v, 
  chLatin_a, chLatin_i, chLatin_l, 
  chLatin_a, chLatin_b, chLatin_l, 
  chLatin_e, chNull 
};
const unsigned int FunctionDocAvailable::minArgs = 1;
const unsigned int FunctionDocAvailable::maxArgs = 1;

/**
 * fn:doc-available($uri as xs:string?) as xs:boolean
 **/
FunctionDocAvailable::FunctionDocAvailable(const VectorOfASTNodes &args, XPath2MemoryManager* memMgr)
  : XQFunction(name, "($uri as xs:string?) as xs:boolean", args, memMgr)
{
}

ASTNode *FunctionDocAvailable::staticTypingImpl(StaticContext *context)
{
  _src.clearExceptType();
  _src.availableDocumentsUsed(true);
  calculateSRCForArguments(context);
  return this;
}

BoolResult FunctionDocAvailable::boolResult(DynamicContext* context) const
{
  Sequence uriArg = getParamNumber(1,context)->toSequence(context);
  
  if (uriArg.isEmpty()) return false;
  
  const XMLCh* uri = uriArg.first()->asString(context);
  // on Windows, we can have URIs using \ instead of /; let's normalize them
  XMLCh backSlash[]={ chBackSlash, chNull };
  if(XMLString::findAny(uri,backSlash))
  {
	  XMLCh* newUri=XMLString::replicate(uri,context->getMemoryManager());
	  for(unsigned int i=0;i<XMLString::stringLen(newUri);i++)
		  if(newUri[i]==chBackSlash)
			  newUri[i]=chForwardSlash;
	  uri=newUri;
  }
  if(!XPath2Utils::isValidURI(uri, context->getMemoryManager()))
    XQThrow(FunctionException, X("FunctionDocAvailable::createSequence"), X("Invalid argument to fn:doc-available function [err:FODC0005]"));

  try {
    return !context->resolveDocument(uri, this).isEmpty();
  } 
  catch(...) {
  }
  return false;
}

Result FunctionDocAvailable::createResult(DynamicContext* context, int flags) const
{
  return (Item::Ptr)context->getItemFactory()->createBoolean(boolResult(context), context);
}

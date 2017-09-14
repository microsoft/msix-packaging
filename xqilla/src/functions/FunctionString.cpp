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
#include <xqilla/functions/FunctionString.hpp>
#include <assert.h>
#include <xqilla/utils/XPath2Utils.hpp>
#include <xqilla/context/DynamicContext.hpp>
#include <xqilla/items/ATStringOrDerived.hpp>
#include <xqilla/items/Node.hpp>
#include <xqilla/items/DatatypeFactory.hpp>
#include <xercesc/util/XMLUni.hpp>
#include <xercesc/validators/schema/SchemaSymbols.hpp>
#include <xqilla/ast/StaticAnalysis.hpp>
#include <xqilla/ast/XQContextItem.hpp>
#include <xqilla/exceptions/FunctionException.hpp>

XERCES_CPP_NAMESPACE_USE

const XMLCh FunctionString::name[] = {
  chLatin_s, chLatin_t, chLatin_r, 
  chLatin_i, chLatin_n, chLatin_g, 
  chNull 
};
const unsigned int FunctionString::minArgs = 0;
const unsigned int FunctionString::maxArgs = 1;

/**
 * fn:string() as xs:string
 * fn:string($arg as item()?) as xs:string
**/

FunctionString::FunctionString(const VectorOfASTNodes &args, XPath2MemoryManager* memMgr)
  : XQFunction(name, "($arg as item()?) as xs:string", args, memMgr)
{
}

ASTNode* FunctionString::staticResolution(StaticContext *context)
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

Sequence FunctionString::createSequence(DynamicContext* context, int flags) const
{
  return Sequence(string_item(getParamNumber(1, context)->next(context), context),
	  context->getMemoryManager());
}

Item::Ptr FunctionString::string_item(const Item::Ptr &item, DynamicContext *context)
{
  return context->getItemFactory()->createString(string(item, context), context);
}

const XMLCh *FunctionString::string(const Item::Ptr &item, DynamicContext *context)
{
  if(item.isNull()) {
    // Do nothing
  }
  else if(item->isNode()) {
    return ((Node*)item.get())->dmStringValue(context);
  }
  else if(item->isAtomicValue()) {
    return ((AnyAtomicType*)item.get())->asString(context);
  }
  return XMLUni::fgZeroLenString;
}

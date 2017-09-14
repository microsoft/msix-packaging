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
#include <xqilla/functions/FunctionNumber.hpp>
#include <xqilla/context/DynamicContext.hpp>
#include <xqilla/items/ATDoubleOrDerived.hpp>
#include <xqilla/items/Node.hpp>
#include <xqilla/items/Numeric.hpp>
#include <xqilla/exceptions/FunctionException.hpp>
#include <xqilla/exceptions/XPath2TypeCastException.hpp>
#include <xqilla/ast/StaticAnalysis.hpp>
#include <xqilla/ast/XQContextItem.hpp>
#include <xqilla/exceptions/XPath2TypeMatchException.hpp>

#include <xercesc/validators/schema/SchemaSymbols.hpp>
#include <assert.h>

XERCES_CPP_NAMESPACE_USE

const XMLCh FunctionNumber::name[] = {
  chLatin_n, chLatin_u, chLatin_m, 
  chLatin_b, chLatin_e, chLatin_r, 
  chNull 
};
const unsigned int FunctionNumber::minArgs = 0;
const unsigned int FunctionNumber::maxArgs = 1;

/**
 * fn:number() as xs:double
 * fn:number($arg as anyAtomicType?) as xs:double
**/

FunctionNumber::FunctionNumber(const VectorOfASTNodes &args, XPath2MemoryManager* memMgr)
  : XQFunction(name, "($arg as xs:anyAtomicType?) as xs:double", args, memMgr)
{
}

ASTNode* FunctionNumber::staticResolution(StaticContext *context)
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

Sequence FunctionNumber::createSequence(DynamicContext* context, int flags) const
{
  return Sequence(number((AnyAtomicType*)getParamNumber(1, context)->next(context).get(), context),
                  context->getMemoryManager());
}

Numeric::Ptr FunctionNumber::number(const AnyAtomicType *item, DynamicContext *context)
{
  if(item == 0) {
    return context->getItemFactory()->createDouble(Numeric::NaN_string, context);
  }
  else {
    try {
      return (Numeric*)item->castAs(AnyAtomicType::DOUBLE, 0, 0, context).get();
    } catch (XPath2TypeCastException &e) {
      return context->getItemFactory()->createDouble(Numeric::NaN_string, context);
    }   
  }
}

Numeric::Ptr FunctionNumber::number(const Item::Ptr &item, DynamicContext *context, const LocationInfo *location)
{
  Item::Ptr tmp = item;

  if(item.isNull()) {
    // Do nothing
  }
  else if(item->isNode()) {
    // Atomize first
    Result atomized = ((Node*)item.get())->dmTypedValue(context);
    tmp = atomized->next(context);

    if(tmp.notNull() && atomized->next(context).notNull()) {
      XQThrow3(XPath2TypeMatchException, X("FunctionNumber::number"),
               X("Sequence does not match type xs:anyAtomicType? - found more than one item [err:XPTY0004]"), location);
    }
  }
  else if(item->isFunction()) {
    XMLBuffer buf;
    buf.set(X("Sequence does not match type (xs:anyAtomicType | node())*"));
    buf.append(X(" - found item of type "));
    item->typeToBuffer(context, buf);
    buf.append(X(" [err:XPTY0004]"));
    XQThrow3(XPath2TypeMatchException, X("FunctionNumber::number"), buf.getRawBuffer(), location);
  }

  return number((AnyAtomicType*)tmp.get(), context);
}

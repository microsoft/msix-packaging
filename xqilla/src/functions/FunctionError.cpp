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
#include <xqilla/functions/FunctionError.hpp>
#include <xqilla/exceptions/XPath2ErrorException.hpp>
#include <xqilla/items/Item.hpp>
#include <xqilla/context/DynamicContext.hpp>
#include <xqilla/ast/StaticAnalysis.hpp>
#include <xqilla/update/PendingUpdateList.hpp>

XERCES_CPP_NAMESPACE_USE;

// http://www.w3.org/2005/xqt-errors
const XMLCh FunctionError::XMLChXQueryErrorURI[] = {
  'h', 't', 't', 'p', ':', '/', '/', 'w', 'w', 'w', '.', 'w', '3', '.', 'o', 'r', 'g', '/', '2', '0', '0', '5', '/', 'x', 'q', 't', '-', 'e', 'r', 'r', 'o', 'r', 's', 0
};

const XMLCh FunctionError::name[] = {
  chLatin_e, chLatin_r, chLatin_r, 
  chLatin_o, chLatin_r, chNull 
};
const unsigned int FunctionError::minArgs = 0;
const unsigned int FunctionError::maxArgs = 3;

/*
 * fn:error() as none
 * fn:error($error as xs:QName) as none
 * fn:error($error as xs:QName?, $description as xs:string) as none
 * fn:error($error as xs:QName?, $description as xs:string, $error-object as item()*) as none
 */

FunctionError::FunctionError(const VectorOfASTNodes &args, XPath2MemoryManager* memMgr)
  : XQFunction(name, args.size() == 1 ?
               "($error as xs:QName) as empty-sequence()" :
               "($error as xs:QName?, $description as xs:string, $error-object as item()*) as empty-sequence()", args, memMgr)
{
}

ASTNode *FunctionError::staticTypingImpl(StaticContext *context)
{
  _src.clearExceptType();
  _src.forceNoFolding(true);
  _src.possiblyUpdating(true);
  calculateSRCForArguments(context);

  // we need to specify item()*, or we get constant folded away all the time
  _src.getStaticType() = StaticType(StaticType::ITEM_TYPE, 0, StaticType::UNLIMITED);

  return this;
}

PendingUpdateList FunctionError::createUpdateList(DynamicContext *context) const
{
  createSequence(context); // doesn't return
  return PendingUpdateList();
}

Sequence FunctionError::createSequence(DynamicContext* context, int flags) const
{
    XMLBuffer exc_name;
    switch(getNumArgs()) {
    case 3: // TODO: extra storage in the exception object for the user object
    case 2: {
      exc_name.set(getParamNumber(2, context)->next(context)->asString(context));
      // Fall through
    }
    case 1: {
      Item::Ptr qname = getParamNumber(1, context)->next(context);
      if(qname.isNull()) {
        exc_name.append(X(" [err:FOER0000]"));
      }
      else {
        exc_name.append(X(" ["));
        exc_name.append(qname->asString(context));
        exc_name.append(X("]"));
      }
      break;
    }
    case 0:
      exc_name.set(X("User-requested error [err:FOER0000]"));
      break;
    }
    XQThrow(XPath2ErrorException, X("FunctionError::createSequence"), exc_name.getRawBuffer());
}



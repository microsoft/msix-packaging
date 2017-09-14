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
#include <xqilla/functions/FunctionEndsWith.hpp>
#include <xqilla/context/Collation.hpp>
#include <xqilla/context/impl/CodepointCollation.hpp>
#include <xqilla/context/DynamicContext.hpp>
#include <xqilla/exceptions/FunctionException.hpp>
#include <xqilla/exceptions/XPath2ErrorException.hpp>
#include <xqilla/items/ATAnyURIOrDerived.hpp>
#include <xqilla/items/ATBooleanOrDerived.hpp>
#include <xqilla/items/ATStringOrDerived.hpp>
#include <xqilla/utils/XPath2Utils.hpp>
#include <xqilla/framework/XPath2MemoryManager.hpp>
#include <xqilla/items/DatatypeFactory.hpp>
#include <xqilla/context/ItemFactory.hpp>

#include <xercesc/util/XMLString.hpp>
#include <xercesc/util/XMLUni.hpp>

XERCES_CPP_NAMESPACE_USE;

const XMLCh FunctionEndsWith::name[] = {
  chLatin_e, chLatin_n, chLatin_d, 
  chLatin_s, chDash,    chLatin_w, 
  chLatin_i, chLatin_t, chLatin_h, 
  chNull 
};
const unsigned int FunctionEndsWith::minArgs = 2;
const unsigned int FunctionEndsWith::maxArgs = 3;

/*
 * fn:ends-with($arg1 as xs:string?, $arg2 as xs:string?) as xs:boolean
 * fn:ends-with($arg1 as xs:string?, $arg2 as xs:string?, $collation as xs:string) as xs:boolean
 */

FunctionEndsWith::FunctionEndsWith(const VectorOfASTNodes &args, XPath2MemoryManager* memMgr)
  : XQFunction(name, "($arg1 as xs:string?, $arg2 as xs:string?, $collation as xs:string) as xs:boolean", args, memMgr)
{
}

BoolResult FunctionEndsWith::boolResult(DynamicContext* context) const
{
    Sequence sourceString=getParamNumber(1,context)->toSequence(context);
    Sequence findString=getParamNumber(2,context)->toSequence(context);

    const XMLCh* source = XMLUni::fgZeroLenString;
    if(!sourceString.isEmpty())
        source=sourceString.first()->asString(context);
    const XMLCh* find = XMLUni::fgZeroLenString;
    if(!findString.isEmpty())
        find=findString.first()->asString(context);

    // If the value of $operand1 is the zero-length string and the value of $operand2 is not the zero-length string,
    // then the function returns false.
    if(XMLString::stringLen(source)==0 && XMLString::stringLen(find)>0)
        return false;
    // If the value of $operand2 is the zero-length string, then the function returns true
    if(XMLString::stringLen(find)==0)
        return true;

    Collation* collation=NULL;
    if(getNumArgs()>2) {
        Sequence collArg = getParamNumber(3,context)->toSequence(context);
        const XMLCh* collName = collArg.first()->asString(context);
        try {
            context->getItemFactory()->createAnyURI(collName, context);
        } catch(XPath2ErrorException &e) {
            XQThrow(FunctionException, X("FunctionEndsWith::createSequence"), X("Invalid collationURI"));  
        }
        collation=context->getCollation(collName, this);
    }
    else
        collation=context->getDefaultCollation(this);

	// Returns a boolean indicating whether or not the value of $operand1 ends with a string that is equal to the value
	// of $operand2 according to the specified collation

    if(XMLString::stringLen(find)>XMLString::stringLen(source))
        return false;
    int i,j;
//    for(i = XMLString::stringLen(source)-1, j=XMLString::stringLen(find)-1; i >=0 && j >=0; i--,j--)

    for(i = XPath2Utils::intStrlen(source)-1, j=XPath2Utils::intStrlen(find)-1; i >=0 && j >=0; i--,j--)
    {
        const XMLCh *string1 = XPath2Utils::subString(source, i,1, context->getMemoryManager());
        const XMLCh *string2 =  XPath2Utils::subString(find, j, 1, context->getMemoryManager());
        bool result = (collation->compare(string1, string2)!=0);

        if(result) {
            return false;
        }
    }

    return true;
}

Result FunctionEndsWith::createResult(DynamicContext* context, int flags) const
{
  return (Item::Ptr)context->getItemFactory()->createBoolean(boolResult(context), context);
}

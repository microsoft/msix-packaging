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
#include <xqilla/functions/FunctionLang.hpp>

#include <xqilla/utils/XPath2Utils.hpp>
#include <xqilla/context/DynamicContext.hpp>
#include <xqilla/exceptions/FunctionException.hpp>
#include <xqilla/items/ATBooleanOrDerived.hpp>
#include <xqilla/items/ATStringOrDerived.hpp>
#include <xqilla/items/Node.hpp>
#include <xqilla/items/DatatypeFactory.hpp>
#include <xqilla/ast/StaticAnalysis.hpp>
#include <xqilla/ast/XQContextItem.hpp>

XERCES_CPP_NAMESPACE_USE;

const XMLCh FunctionLang::name[] = {
  chLatin_l, chLatin_a, chLatin_n, 
  chLatin_g, chNull 
};
const unsigned int FunctionLang::minArgs = 1;
const unsigned int FunctionLang::maxArgs = 2;

/**
 * fn:lang($testlang as xs:string?) as xs:boolean
 * fn:lang($testlang as xs:string?, $node as node()) as xs:boolean
**/

FunctionLang::FunctionLang(const VectorOfASTNodes &args, XPath2MemoryManager* memMgr)
  : XQFunction(name, "($testlang as xs:string?, $node as node()) as xs:boolean", args, memMgr)
{
}

ASTNode* FunctionLang::staticResolution(StaticContext *context)
{
  XPath2MemoryManager *mm = context->getMemoryManager();

  if(_args.size() == 1) {
    XQContextItem *ci = new (mm) XQContextItem(mm);
    ci->setLocationInfo(this);
    _args.push_back(ci);
  }

  resolveArguments(context);
  return this;
}

Sequence FunctionLang::createSequence(DynamicContext* context, int flags) const
{
    XPath2MemoryManager* memMgr = context->getMemoryManager();

    Item::Ptr arg1 = getParamNumber(1, context)->next(context);
    const XMLCh *param1 = arg1.isNull() ? XMLUni::fgZeroLenString : arg1->asString(context);

    const XMLCh* xmlLangValue = XPath2Utils::toLower(param1, context->getMemoryManager());
    bool isSubLang = XMLString::indexOf(xmlLangValue, chDash) > 0;

    static const XMLCh xmlLang[] = { chLatin_l, chLatin_a,
                                     chLatin_n, chLatin_g,
                                     chNull};

    static const XMLCh* xmlLangNamespace = XMLUni::fgXMLURIName;

    Node::Ptr node = (Node*)getParamNumber(2, context)->next(context).get();
    while(node.notNull())
    {
      Result attrs = node->dmAttributes(context, this);
      Node::Ptr att;
      while((att = (Node::Ptr)attrs->next(context)).notNull()) {
        ATQNameOrDerived::Ptr name = att->dmNodeName(context);
        if(name.notNull()) {
          const XMLCh *node_uri = ((const ATQNameOrDerived*)name.get())->getURI();
          const XMLCh *node_name = ((const ATQNameOrDerived*)name.get())->getName();

          if(XPath2Utils::equals(xmlLang, node_name) &&
             XPath2Utils::equals(xmlLangNamespace, node_uri)) {
            const XMLCh *value = att->dmStringValue(context);

            const XMLCh* asLower = XPath2Utils::toLower(value, context->getMemoryManager());
            int dashLocation = XMLString::indexOf(asLower, chDash);

            if(dashLocation!=-1 && !isSubLang) {
              asLower = XPath2Utils::subString(asLower, 0, dashLocation, memMgr);
            }

            return Sequence(context->getItemFactory()->createBoolean(XPath2Utils::equals(asLower, xmlLangValue), context), memMgr);
          }
        }
      }

      node = node->dmParent(context);
    }

    return Sequence(context->getItemFactory()->createBoolean(false, context), memMgr);
}

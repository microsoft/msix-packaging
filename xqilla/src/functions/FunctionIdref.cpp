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
#include <xqilla/functions/FunctionIdref.hpp>

#include <xqilla/context/DynamicContext.hpp>
#include <xqilla/exceptions/FunctionException.hpp>
#include "../exceptions/InvalidLexicalSpaceException.hpp"
#include <xqilla/utils/XPath2Utils.hpp>
#include <xqilla/items/Node.hpp>
#include <xqilla/items/DatatypeFactory.hpp>
#include <xqilla/ast/StaticAnalysis.hpp>
#include <xqilla/framework/XPath2MemoryManagerImpl.hpp>
#include <xqilla/context/ItemFactory.hpp>
#include <xqilla/ast/XQContextItem.hpp>

XERCES_CPP_NAMESPACE_USE;

const XMLCh FunctionIdref::name[] = {
  chLatin_i, chLatin_d, chLatin_r, 
  chLatin_e, chLatin_f, chNull 
};
const unsigned int FunctionIdref::minArgs = 1;
const unsigned int FunctionIdref::maxArgs = 2;

/**
 * fn:idref($arg as xs:string*) as node()*
 * fn:idref($arg as xs:string*, $node as node()) as node()*
 **/

FunctionIdref::FunctionIdref(const VectorOfASTNodes &args, XPath2MemoryManager* memMgr)
  : XQFunction(name, "($arg as xs:string*, $node as node()) as node()*", args, memMgr)
{
}

ASTNode* FunctionIdref::staticResolution(StaticContext *context)
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

Sequence FunctionIdref::createSequence(DynamicContext* context, int flags) const
{
  Node::Ptr root = ((Node*)getParamNumber(2, context)->next(context).get())->root(context);
  if(root->dmNodeKind() != Node::document_string) {
    XQThrow(FunctionException,X("FunctionIdref::createSequence"), X("Current context doesn't belong to a document [err:FODC0001]"));
  }
    
  Sequence strings = getParamNumber(1, context)->toSequence(context);
	if(strings.isEmpty())
		return Sequence(context->getMemoryManager());

  std::vector<const XMLCh*> values;
    
  //get the list of idref values we're looking for by iterating over each string in the sequence
  for (Sequence::iterator stringIt = strings.begin(); stringIt != strings.end(); ++stringIt) {
    const XMLCh *str = (*stringIt)->asString(context);

    //for each string check that it is lexically a xs:ID, if not ignore it
    bool validID = true;
    try {
      context->getItemFactory()->createStringOrDerived(SchemaSymbols::fgURI_SCHEMAFORSCHEMA, XMLUni::fgIDString, str, context);
    } catch (InvalidLexicalSpaceException &e) {
      validID = false;
    }
    if (validID) {
      values.push_back(str);
    }
  }

  Sequence result(context->getMemoryManager());

  std::vector<Result> resultStack;
  resultStack.push_back(root->dmChildren(context, this));
  Node::Ptr child = resultStack.back()->next(context);
  while(child.notNull()) {
    if(child->dmNodeKind() == Node::element_string) {
      if(child->dmIsIdRefs(context)->isTrue()) {
        // child is of type xs:ID
        const XMLCh* id = child->dmStringValue(context);
        std::vector<const XMLCh*> vals = XPath2Utils::getVal(id, context->getMemoryManager());
        for(std::vector<const XMLCh*>::iterator val = vals.begin();
            val != vals.end(); ++val) {
          if(XPath2Utils::containsString(values, *val)) {
            result.addItem(child);
            break;
          }
        }
      }

      Result attrs = child->dmAttributes(context, this);
      Node::Ptr att;
      while((att = (Node::Ptr)attrs->next(context)).notNull()) {
        if(att->dmIsIdRefs(context)->isTrue()) {
          // att is of type xs:ID
          const XMLCh* id = att->dmStringValue(context);
          std::vector<const XMLCh*> vals = XPath2Utils::getVal(id, context->getMemoryManager());
          for(std::vector<const XMLCh*>::iterator val = vals.begin();
              val != vals.end(); ++val) {
            if(XPath2Utils::containsString(values, *val)) {
              result.addItem(att);
              break;
            }
          }
        }
      }
    }

    resultStack.push_back(child->dmChildren(context, this));
    while(!resultStack.empty() && (child = resultStack.back()->next(context)).isNull()) {
      resultStack.pop_back();
    }
  }

  return result;
}

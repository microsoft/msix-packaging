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
#include <xqilla/functions/FunctionNodeName.hpp>
#include <xqilla/items/Node.hpp>
#include <xqilla/exceptions/FunctionException.hpp>
#include <xqilla/context/DynamicContext.hpp>

const XMLCh FunctionNodeName::name[] = {
  XERCES_CPP_NAMESPACE_QUALIFIER chLatin_n, XERCES_CPP_NAMESPACE_QUALIFIER chLatin_o, XERCES_CPP_NAMESPACE_QUALIFIER chLatin_d, 
  XERCES_CPP_NAMESPACE_QUALIFIER chLatin_e, XERCES_CPP_NAMESPACE_QUALIFIER chDash,    XERCES_CPP_NAMESPACE_QUALIFIER chLatin_n, 
  XERCES_CPP_NAMESPACE_QUALIFIER chLatin_a, XERCES_CPP_NAMESPACE_QUALIFIER chLatin_m, XERCES_CPP_NAMESPACE_QUALIFIER chLatin_e, 
  XERCES_CPP_NAMESPACE_QUALIFIER chNull 
};
const unsigned int FunctionNodeName::minArgs = 1;
const unsigned int FunctionNodeName::maxArgs = 1;

/*
 * fn:node-name($arg as node()?) as xs:QName?
 */

FunctionNodeName::FunctionNodeName(const VectorOfASTNodes &args, XPath2MemoryManager* memMgr)
  : XQFunction(name, "($arg as node()?) as xs:QName?", args, memMgr)
{
}

ASTNode *FunctionNodeName::staticTypingImpl(StaticContext *context)
{
  _src.clearExceptType();
  calculateSRCForArguments(context);

  if(_args[0]->getStaticAnalysis().getStaticType().getMin() == 1 &&
     _args[0]->getStaticAnalysis().getStaticType().isType(StaticType::ELEMENT_TYPE | StaticType::ATTRIBUTE_TYPE |
                                                          StaticType::PI_TYPE | StaticType::NAMESPACE_TYPE)) {
    _src.getStaticType() = StaticType::QNAME_TYPE;
  }
  else {
    _src.getStaticType() = StaticType(StaticType::QNAME_TYPE, 0, 1);
  }

  return this;
}

Sequence FunctionNodeName::createSequence(DynamicContext* context, int flags) const
{
  Item::Ptr item = getParamNumber(1,context)->next(context);
  if(item.isNull())
    return Sequence(context->getMemoryManager());
  return Sequence(((Node*)item.get())->dmNodeName(context), context->getMemoryManager());
}

















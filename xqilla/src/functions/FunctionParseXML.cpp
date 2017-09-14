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
#include <xqilla/functions/FunctionParseXML.hpp>
#include <xqilla/context/DynamicContext.hpp>
#include <xqilla/exceptions/FunctionException.hpp>
#include <xqilla/exceptions/XMLParseException.hpp>
#include <xqilla/ast/StaticAnalysis.hpp>

#include <xercesc/util/XMLString.hpp>
#include <xercesc/framework/MemBufInputSource.hpp>

XERCES_CPP_NAMESPACE_USE;

const XMLCh FunctionParseXML::name[] = {
  chLatin_p, chLatin_a, chLatin_r, chLatin_s, chLatin_e, chDash, chLatin_x, chLatin_m, chLatin_l, chNull 
};
const unsigned int FunctionParseXML::minArgs = 1;
const unsigned int FunctionParseXML::maxArgs = 1;

/**
 * xqilla:parse-xml($xml as xs:string?) as document-node()?
 */
FunctionParseXML::FunctionParseXML(const VectorOfASTNodes &args, XPath2MemoryManager* memMgr)
  : XQillaFunction(name, "($xml as xs:string?) as document-node()?", args, memMgr),
    queryPathTree_(0)
{
}

ASTNode *FunctionParseXML::staticTypingImpl(StaticContext *context)
{
  _src.clearExceptType();

  calculateSRCForArguments(context);

  _src.setProperties(StaticAnalysis::DOCORDER | StaticAnalysis::GROUPED |
                     StaticAnalysis::PEER | StaticAnalysis::SUBTREE | StaticAnalysis::ONENODE);
  _src.getStaticType() = StaticType(StaticType::DOCUMENT_TYPE, 0, 1);
  _src.creative(true);

  return this;
}

Sequence FunctionParseXML::createSequence(DynamicContext* context, int flags) const
{
  Item::Ptr item = getParamNumber(1, context)->next(context);

  if(item.isNull()) return Sequence(context->getMemoryManager());

  const XMLCh *xml = item->asString(context);

  MemBufInputSource src((XMLByte*)xml, XMLString::stringLen(xml) * sizeof(XMLCh), name);
  src.setEncoding(XMLUni::fgUTF16EncodingString);

  try {
    return Sequence(context->parseDocument(src, this, context->getProjection() ? queryPathTree_ : 0), context->getMemoryManager());
  }
  catch(XMLParseException &e) {
    XQThrow(FunctionException, X("FunctionParseXML::createSequence"), e.getError());
  }
}

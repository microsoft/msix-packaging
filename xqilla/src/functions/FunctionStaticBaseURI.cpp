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
#include <xqilla/functions/FunctionStaticBaseURI.hpp>
#include <xqilla/context/DynamicContext.hpp>
#include <xqilla/ast/XQLiteral.hpp>

#include <xercesc/util/XMLUni.hpp>
#include <xercesc/validators/schema/SchemaSymbols.hpp>

#if defined(XERCES_HAS_CPP_NAMESPACE)
XERCES_CPP_NAMESPACE_USE
#endif

const XMLCh FunctionStaticBaseURI::name[] = {
  chLatin_s, chLatin_t, chLatin_a, 
  chLatin_t, chLatin_i, chLatin_c, 
  chDash,    chLatin_b, chLatin_a, 
  chLatin_s, chLatin_e, chDash, 
  chLatin_u, chLatin_r, chLatin_i, 
  chNull 
};
const unsigned int FunctionStaticBaseURI::minArgs = 0;
const unsigned int FunctionStaticBaseURI::maxArgs = 0;

/**
 * fn:static-base-uri() as xs:anyURI?
**/

FunctionStaticBaseURI::FunctionStaticBaseURI(const VectorOfASTNodes &args, XPath2MemoryManager* memMgr)
  : XQFunction(name, "() as xs:anyURI?", args, memMgr)
{
}

ASTNode* FunctionStaticBaseURI::staticResolution(StaticContext *context)
{
  XPath2MemoryManager* mm=context->getMemoryManager();

  ASTNode* newBlock = new (mm) XQLiteral(SchemaSymbols::fgURI_SCHEMAFORSCHEMA,
                                         SchemaSymbols::fgDT_ANYURI,
                                         context->getBaseURI() ? context->getBaseURI() : XMLUni::fgZeroLenString,
                                         AnyAtomicType::ANY_URI, mm);
  newBlock->setLocationInfo(this);

  return newBlock->staticResolution(context);
}

ASTNode *FunctionStaticBaseURI::staticTypingImpl(StaticContext *context)
{
  // Should never happen
  return this;
}

Sequence FunctionStaticBaseURI::createSequence(DynamicContext* context, int flags) const
{
  // Always constant folded
  return Sequence(context->getMemoryManager());
}

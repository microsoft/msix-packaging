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
#include <xqilla/functions/FunctionDefaultCollation.hpp>
#include <xqilla/context/DynamicContext.hpp>
#include <xqilla/context/Collation.hpp>
#include <xqilla/context/impl/CodepointCollation.hpp>
#include <xqilla/items/DatatypeFactory.hpp>
#include <xqilla/items/ATStringOrDerived.hpp>
#include <xqilla/context/ItemFactory.hpp>
#include <xqilla/ast/XQLiteral.hpp>

XERCES_CPP_NAMESPACE_USE;

const XMLCh FunctionDefaultCollation::name[] = {
  chLatin_d, chLatin_e, chLatin_f, 
  chLatin_a, chLatin_u, chLatin_l, 
  chLatin_t, chDash,    chLatin_c, 
  chLatin_o, chLatin_l, chLatin_l, 
  chLatin_a, chLatin_t, chLatin_i, 
  chLatin_o, chLatin_n, chNull 
};
const unsigned int FunctionDefaultCollation::minArgs = 0;
const unsigned int FunctionDefaultCollation::maxArgs = 0;

/**
 * fn:default-collation() as xs:string
**/

FunctionDefaultCollation::FunctionDefaultCollation(const VectorOfASTNodes &args, XPath2MemoryManager* memMgr)
  : XQFunction(name, "() as xs:string", args, memMgr)
{
}

ASTNode *FunctionDefaultCollation::staticResolution(StaticContext *context)
{
  XPath2MemoryManager *mm = context->getMemoryManager();

  Collation *collation = context->getDefaultCollation(this);
  ASTNode *newBlock = new (mm) XQLiteral(SchemaSymbols::fgURI_SCHEMAFORSCHEMA, SchemaSymbols::fgDT_STRING,
                                         collation ? collation->getCollationName() : CodepointCollation::getCodepointCollationName(),
                                         AnyAtomicType::STRING, mm);
  newBlock->setLocationInfo(this);

  return newBlock->staticResolution(context);
}

Sequence FunctionDefaultCollation::createSequence(DynamicContext* context, int flags) const
{
  // Always constant folded
  return Sequence(context->getMemoryManager());
}

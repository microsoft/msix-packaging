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
#include <assert.h>
#include <xqilla/operators/Equals.hpp>
#include <xqilla/context/DynamicContext.hpp>
#include <xqilla/items/ATDecimalOrDerived.hpp>
#include <xqilla/items/AnyAtomicType.hpp>
#include <xqilla/items/ATBooleanOrDerived.hpp>
#include <xqilla/items/ATStringOrDerived.hpp>
#include <xqilla/items/DatatypeFactory.hpp>
#include <xqilla/exceptions/XPath2ErrorException.hpp>
#include <xqilla/context/Collation.hpp>
#include <xqilla/context/impl/CodepointCollation.hpp>

/*static*/ const XMLCh Equals::name[]={ XERCES_CPP_NAMESPACE_QUALIFIER chLatin_E, XERCES_CPP_NAMESPACE_QUALIFIER chLatin_Q, XERCES_CPP_NAMESPACE_QUALIFIER chNull };

Equals::Equals(const VectorOfASTNodes &args, XPath2MemoryManager* memMgr)
  : ComparisonOperator(name, args, memMgr)
{
}

/*static*/ bool Equals::equals(const Item::Ptr &arg1, const Item::Ptr &arg2, Collation* collation, DynamicContext* context, const LocationInfo *info)
{
  assert(arg1->isAtomicValue() && arg2->isAtomicValue());
  return equals((const AnyAtomicType::Ptr)arg1, (const AnyAtomicType::Ptr)arg2, collation, context, info);
}

/*static*/ bool Equals::equals(const AnyAtomicType::Ptr &atom1, const AnyAtomicType::Ptr &atom2, Collation* collation, DynamicContext* context, const LocationInfo *info)
{
  try {
    // take care of the special case first
    if(atom1->getPrimitiveTypeIndex() == AnyAtomicType::STRING) {
        if(atom2->getPrimitiveTypeIndex() != AnyAtomicType::STRING &&
           atom2->getPrimitiveTypeIndex() != AnyAtomicType::ANY_URI) {
            XQThrow3(XPath2ErrorException,X("Equals::equals"), X("An attempt to compare a string type to a non string type has occurred [err:XPTY0004]"), info);
        }
        // if the function returns 0, then they are equal
        if(!collation) collation = context->getDefaultCollation(info);
        return (collation->compare(atom1->asString(context),atom2->asString(context))==0);
    } 

    return atom1->equals(atom2, context);
  }
  catch(XQException &e) {
      if(e.getXQueryLine() == 0)
        e.setXQueryPosition(info);
      throw;
  }
}

bool Equals::execute(const AnyAtomicType::Ptr &atom1, const AnyAtomicType::Ptr &atom2, DynamicContext *context) const
{
  return equals(atom1, atom2, 0, context, this);
}


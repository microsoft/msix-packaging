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
#include <xqilla/functions/FunctionDistinctValues.hpp>
#include <xqilla/utils/XPath2Utils.hpp>
#include <xqilla/runtime/Sequence.hpp>
#include <xqilla/operators/Equals.hpp>
#include <xqilla/context/Collation.hpp>
#include <xqilla/context/impl/CodepointCollation.hpp>
#include <xqilla/exceptions/FunctionException.hpp>
#include <xqilla/exceptions/IllegalArgumentException.hpp>
#include <xqilla/exceptions/XPath2ErrorException.hpp>
#include <xqilla/items/ATDoubleOrDerived.hpp>
#include <xqilla/items/ATFloatOrDerived.hpp>
#include <xqilla/items/ATStringOrDerived.hpp>
#include <xqilla/items/ATUntypedAtomic.hpp>
#include <xqilla/context/DynamicContext.hpp>
#include <xqilla/items/DatatypeFactory.hpp>
#include <xqilla/context/ItemFactory.hpp>
#include <xqilla/context/ContextHelpers.hpp>

#include <xercesc/validators/schema/SchemaSymbols.hpp>

#include <set>

#if defined(XERCES_HAS_CPP_NAMESPACE)
XERCES_CPP_NAMESPACE_USE
#endif

const XMLCh FunctionDistinctValues::name[] = {
  chLatin_d, chLatin_i, chLatin_s, 
  chLatin_t, chLatin_i, chLatin_n, 
  chLatin_c, chLatin_t, chDash, 
  chLatin_v, chLatin_a, chLatin_l, 
  chLatin_u, chLatin_e, chLatin_s, 
  chNull 
};
const unsigned int FunctionDistinctValues::minArgs = 1;
const unsigned int FunctionDistinctValues::maxArgs = 2;

/**
 * fn:distinct-values($arg as xs:anyAtomicType*) as xs:anyAtomicType*
 * fn:distinct-values($arg as xs:anyAtomicType*, $collation as xs:string) as xs:anyAtomicType*
**/

FunctionDistinctValues::FunctionDistinctValues(const VectorOfASTNodes &args, XPath2MemoryManager* memMgr)
  : XQFunction(name, "($arg as xs:anyAtomicType*, $collation as xs:string) as xs:anyAtomicType*", args, memMgr)
{
}

ASTNode *FunctionDistinctValues::staticTypingImpl(StaticContext *context)
{
  _src.clearExceptType();
  calculateSRCForArguments(context);
  _src.getStaticType() = _args.front()->getStaticAnalysis().getStaticType();
  _src.getStaticType().setCardinality(_src.getStaticType().getMin() == 0 ? 0 : 1, _src.getStaticType().getMax());
  return this;
}

static inline AnyAtomicType::AtomicObjectType getSortType(const AnyAtomicType::Ptr &a)
{
  switch(a->getPrimitiveTypeIndex()) {
  case AnyAtomicType::ANY_URI:
  case AnyAtomicType::UNTYPED_ATOMIC:
  case AnyAtomicType::STRING: return AnyAtomicType::STRING;

  case AnyAtomicType::DECIMAL:
  case AnyAtomicType::FLOAT:
  case AnyAtomicType::DOUBLE: return AnyAtomicType::DOUBLE;

  case AnyAtomicType::DAY_TIME_DURATION:
  case AnyAtomicType::YEAR_MONTH_DURATION:
  case AnyAtomicType::DURATION: return AnyAtomicType::DURATION;

  case AnyAtomicType::BASE_64_BINARY: return AnyAtomicType::BASE_64_BINARY;
  case AnyAtomicType::BOOLEAN: return AnyAtomicType::BOOLEAN;
  case AnyAtomicType::DATE: return AnyAtomicType::DATE;
  case AnyAtomicType::DATE_TIME: return AnyAtomicType::DATE_TIME;
  case AnyAtomicType::G_DAY: return AnyAtomicType::G_DAY;
  case AnyAtomicType::G_MONTH: return AnyAtomicType::G_MONTH;
  case AnyAtomicType::G_MONTH_DAY: return AnyAtomicType::G_MONTH_DAY;
  case AnyAtomicType::G_YEAR: return AnyAtomicType::G_YEAR;
  case AnyAtomicType::G_YEAR_MONTH: return AnyAtomicType::G_YEAR_MONTH;
  case AnyAtomicType::HEX_BINARY: return AnyAtomicType::HEX_BINARY;
  case AnyAtomicType::NOTATION: return AnyAtomicType::NOTATION;
  case AnyAtomicType::QNAME: return AnyAtomicType::QNAME;
  case AnyAtomicType::TIME: return AnyAtomicType::TIME;

  default: break;
  }

  assert(false); // Not supported
  return AnyAtomicType::STRING;
}

struct dvCompare
{
  dvCompare(const Collation *collation, const DynamicContext *context)
    : collation_(collation), context_(context) {}

  bool operator()(const AnyAtomicType::Ptr &a, const AnyAtomicType::Ptr &b) const
  {
    AnyAtomicType::AtomicObjectType atype = getSortType(a);
    AnyAtomicType::AtomicObjectType btype = getSortType(b);

    if(atype != btype) return atype < btype;

    // Items are comparable
    switch(atype) {
    case AnyAtomicType::STRING:
      return collation_->compare(a->asString(context_), b->asString(context_)) < 0;
    case AnyAtomicType::DOUBLE:
      return ((const Numeric *)a.get())->compare((const Numeric *)b.get(), context_) < 0;
    case AnyAtomicType::DURATION:
      return ((const ATDurationOrDerived *)a.get())->compare((const ATDurationOrDerived *)b.get(), context_) < 0;
    case AnyAtomicType::BASE_64_BINARY:
      return ((const ATBase64BinaryOrDerived *)a.get())->compare((const ATBase64BinaryOrDerived *)b.get(), context_) < 0;
    case AnyAtomicType::BOOLEAN:
      return ((const ATBooleanOrDerived *)a.get())->compare((const ATBooleanOrDerived *)b.get(), context_) < 0;
    case AnyAtomicType::DATE:
      return ((const ATDateOrDerived *)a.get())->compare((const ATDateOrDerived *)b.get(), context_) < 0;
    case AnyAtomicType::DATE_TIME:
      return ((const ATDateTimeOrDerived *)a.get())->compare((const ATDateTimeOrDerived *)b.get(), context_) < 0;
    case AnyAtomicType::G_DAY:
      return ((const ATGDayOrDerived *)a.get())->compare((const ATGDayOrDerived *)b.get(), context_) < 0;
    case AnyAtomicType::G_MONTH:
      return ((const ATGMonthOrDerived *)a.get())->compare((const ATGMonthOrDerived *)b.get(), context_) < 0;
    case AnyAtomicType::G_MONTH_DAY:
      return ((const ATGMonthDayOrDerived *)a.get())->compare((const ATGMonthDayOrDerived *)b.get(), context_) < 0;
    case AnyAtomicType::G_YEAR:
      return ((const ATGYearOrDerived *)a.get())->compare((const ATGYearOrDerived *)b.get(), context_) < 0;
    case AnyAtomicType::G_YEAR_MONTH:
      return ((const ATGYearMonthOrDerived *)a.get())->compare((const ATGYearMonthOrDerived *)b.get(), context_) < 0;
    case AnyAtomicType::HEX_BINARY:
      return ((const ATHexBinaryOrDerived *)a.get())->compare((const ATHexBinaryOrDerived *)b.get(), context_) < 0;
    case AnyAtomicType::NOTATION:
      return ((const ATNotationOrDerived *)a.get())->compare((const ATNotationOrDerived *)b.get(), context_) < 0;
    case AnyAtomicType::QNAME:
      return ((const ATQNameOrDerived *)a.get())->compare((const ATQNameOrDerived *)b.get(), context_) < 0;
    case AnyAtomicType::TIME:
      return ((const ATTimeOrDerived *)a.get())->compare((const ATTimeOrDerived *)b.get(), context_) < 0;
    default: break;
    }

    assert(false);
    return false;
  }

  const Collation *collation_;
  const DynamicContext *context_;
};

typedef std::set<AnyAtomicType::Ptr, dvCompare> DistinctSet;

class DistinctValueResult : public ResultImpl
{
public:
  DistinctValueResult(const FunctionDistinctValues *fdv, const DynamicContext *context);
  ~DistinctValueResult();
  Item::Ptr next(DynamicContext *context);
private:
  const FunctionDistinctValues *fdv_;
  Result parent_;
  bool toDo_;

  DistinctSet *alreadySeen_;
};

DistinctValueResult::DistinctValueResult(const FunctionDistinctValues *fdv, const DynamicContext *context)
  : ResultImpl(fdv),
    fdv_(fdv),
    parent_(0),
    toDo_(true),
    alreadySeen_(0)
{
}

DistinctValueResult::~DistinctValueResult()
{
  delete alreadySeen_;
}

Item::Ptr DistinctValueResult::next(DynamicContext *context)
{
  if(toDo_) {
    toDo_ = false;
    parent_ = fdv_->getParamNumber(1, context);

    Collation *collation;
    if(fdv_->getNumArgs() > 1) {
        const XMLCh* collName = fdv_->getParamNumber(2, context)->next(context)->asString(context);
        try {
            context->getItemFactory()->createAnyURI(collName, context);
        } catch(XPath2ErrorException &e) {
            XQThrow(FunctionException, X("FunctionDistinctValues::DistinctValueResult::next"), X("Invalid collationURI"));  
        }
        collation = context->getCollation(collName, this);
    }
    else
        collation = context->getDefaultCollation(this);

    alreadySeen_ = new DistinctSet(dvCompare(collation, context));
  }

  AnyAtomicType::Ptr item;
  while(true) {
    item = (const AnyAtomicType *)parent_->next(context).get();
    if(item.isNull()) break;

    if(alreadySeen_->insert(item).second)
      return item;
  }

  parent_ = 0;
  return 0;
}

Result FunctionDistinctValues::createResult(DynamicContext* context, int flags) const
{
  return new DistinctValueResult(this, context);
}


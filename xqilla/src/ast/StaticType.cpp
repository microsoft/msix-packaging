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

#include <assert.h>

#include "../config/xqilla_config.h"

#include <xqilla/ast/StaticType.hpp>
#include <xqilla/context/ItemFactory.hpp>
#include <xqilla/context/StaticContext.hpp>
#include <xqilla/utils/XPath2Utils.hpp>
#include <xqilla/utils/XStr.hpp>

#include <xercesc/validators/schema/SchemaSymbols.hpp>

XERCES_CPP_NAMESPACE_USE;

const unsigned int StaticType::UNLIMITED = (unsigned int)-1;

StaticType StaticType::create(const XMLCh *uri, const XMLCh *name, const StaticContext *context,
                              bool &isExact)
{
  if(XPath2Utils::equals(name, AnyAtomicType::fgDT_ANYATOMICTYPE) &&
     XPath2Utils::equals(uri, SchemaSymbols::fgURI_SCHEMAFORSCHEMA)) {
    isExact = true;
    return ANY_ATOMIC_TYPE;
  }
  else {
    return create(context->getItemFactory()->getPrimitiveTypeIndex(uri, name, /*isPrimitive*/isExact));
  }
}

StaticType StaticType::create(AnyAtomicType::AtomicObjectType primitiveType)
{
  switch(primitiveType) {
  case AnyAtomicType::ANY_SIMPLE_TYPE: return ANY_SIMPLE_TYPE;
  case AnyAtomicType::ANY_URI: return ANY_URI_TYPE;
  case AnyAtomicType::BASE_64_BINARY: return BASE_64_BINARY_TYPE;
  case AnyAtomicType::BOOLEAN: return BOOLEAN_TYPE;
  case AnyAtomicType::DATE: return DATE_TYPE;
  case AnyAtomicType::DATE_TIME: return DATE_TIME_TYPE;
  case AnyAtomicType::DAY_TIME_DURATION: return DAY_TIME_DURATION_TYPE;
  case AnyAtomicType::DECIMAL: return DECIMAL_TYPE;
  case AnyAtomicType::DOUBLE: return DOUBLE_TYPE;
  case AnyAtomicType::DURATION: return DURATION_TYPE | DAY_TIME_DURATION_TYPE | YEAR_MONTH_DURATION_TYPE;
  case AnyAtomicType::FLOAT: return FLOAT_TYPE;
  case AnyAtomicType::G_DAY: return G_DAY_TYPE;
  case AnyAtomicType::G_MONTH: return G_MONTH_TYPE;
  case AnyAtomicType::G_MONTH_DAY: return G_MONTH_DAY_TYPE;
  case AnyAtomicType::G_YEAR: return G_YEAR_TYPE;
  case AnyAtomicType::G_YEAR_MONTH: return G_YEAR_MONTH_TYPE;
  case AnyAtomicType::HEX_BINARY: return HEX_BINARY_TYPE;
  case AnyAtomicType::NOTATION: return NOTATION_TYPE;
  case AnyAtomicType::QNAME: return QNAME_TYPE;
  case AnyAtomicType::STRING: return STRING_TYPE;
  case AnyAtomicType::TIME: return TIME_TYPE;
  case AnyAtomicType::UNTYPED_ATOMIC: return UNTYPED_ATOMIC_TYPE;
  case AnyAtomicType::YEAR_MONTH_DURATION: return YEAR_MONTH_DURATION_TYPE;
  default: break;
  }
  return StaticType();
}

StaticType::StaticType()
  : flags_(0),
    min_(0),
    max_(0),
    mm_(0),
    minArgs_(0),
    maxArgs_(0),
    returnType_(0)
{
}

StaticType::StaticType(StaticTypeFlags f, unsigned int min, unsigned int max)
  : flags_(f),
    min_(min),
    max_(max),
    mm_(0),
    minArgs_(0),
    maxArgs_(0),
    returnType_(0)
{
  // max must be 0 if flags is 0
  assert(max != 0 || flags_ == 0);
  assert(min <= max);
  assert(min != UNLIMITED);
}

StaticType::StaticType(XPath2MemoryManager *mm, unsigned int numArgs, const StaticType &returnType, unsigned int min, unsigned int max)
  : flags_(FUNCTION_TYPE),
    min_(min),
    max_(max),
    mm_(mm),
    minArgs_(numArgs),
    maxArgs_(numArgs),
    returnType_(new (mm) StaticType(returnType))
{
  assert(min <= max);
  assert(min != UNLIMITED);
}

StaticType::StaticType(XPath2MemoryManager *mm, unsigned int minArgs, unsigned int maxArgs, const StaticType &returnType,
                       unsigned int min, unsigned int max)
  : flags_(FUNCTION_TYPE),
    min_(min),
    max_(max),
    mm_(mm),
    minArgs_(minArgs),
    maxArgs_(maxArgs),
    returnType_(new (mm) StaticType(returnType))
{
  assert(min <= max);
  assert(min != UNLIMITED);
}

StaticType::StaticType(const StaticType &o)
  : flags_(o.flags_),
    min_(o.min_),
    max_(o.max_),
    mm_(o.mm_),
    minArgs_(o.minArgs_),
    maxArgs_(o.maxArgs_),
    returnType_(o.returnType_ ? new (mm_) StaticType(*o.returnType_) : 0)
{
}

StaticType &StaticType::operator=(const StaticType &o)
{
  if(this != &o) {
    flags_ = o.flags_;
    min_ = o.min_;
    max_ = o.max_;

    delete returnType_;

    if(mm_ == 0) mm_ = o.mm_;
    minArgs_ = o.minArgs_;
    maxArgs_ = o.maxArgs_;
    returnType_ = o.returnType_ ? new (mm_) StaticType(*o.returnType_) : 0;
  }

  return *this;
}

StaticType::~StaticType()
{
  delete returnType_;
}

void StaticType::typeUnion(const StaticType &st)
{
  bool thisFunctions = (flags_ & FUNCTION_TYPE) != 0;
  bool otherFunctions = (st.flags_ & FUNCTION_TYPE) != 0;

  flags_ |= st.flags_;
  min_ = (min_ < st.min_) ? min_ : st.min_;
  max_ = (max_ > st.max_) ? max_ : st.max_;

  if(st.returnType_) {
    if(returnType_) {
      minArgs_ = (minArgs_ < st.minArgs_) ? minArgs_ : st.minArgs_;
      maxArgs_ = (maxArgs_ > st.maxArgs_) ? maxArgs_ : st.maxArgs_;
      returnType_->typeUnion(*st.returnType_);
    }
    else if(!thisFunctions) {
      if(mm_ == 0) mm_ = st.mm_;
      minArgs_ = st.minArgs_;
      maxArgs_ = st.maxArgs_;
      returnType_ = new (mm_) StaticType(*st.returnType_);
    }
    else {
      // This was a wildcard function, the union of which is still a wildcard function
    }
  }
  else if(otherFunctions) {
    // Other was a wildcard function, so this is now a wildcard function
    minArgs_ = 0;
    maxArgs_ = 0;
    delete returnType_;
    returnType_ = 0;
  }
}

void StaticType::typeIntersect(const StaticType &st)
{
  flags_ &= st.flags_;
  min_ = (min_ > st.min_) ? min_ : st.min_;
  max_ = (max_ < st.max_) ? max_ : st.max_;

  if(st.returnType_) {
    if(returnType_) {
      minArgs_ = (minArgs_ > st.minArgs_) ? minArgs_ : st.minArgs_;
      maxArgs_ = (maxArgs_ < st.maxArgs_) ? maxArgs_ : st.maxArgs_;
      returnType_->typeIntersect(*st.returnType_);
    }
    else {
      if(mm_ == 0) mm_ = st.mm_;
      minArgs_ = st.minArgs_;
      maxArgs_ = st.maxArgs_;
      returnType_ = new (mm_) StaticType(*st.returnType_);
    }
  }

  if((flags_ & FUNCTION_TYPE) == 0 || minArgs_ > maxArgs_) {
    // A function is no longer part of this type
    flags_ &= ~FUNCTION_TYPE;
    minArgs_ = 0;
    maxArgs_ = 0;
    delete returnType_;
    returnType_ = 0;
  }

  if(flags_ == 0 || max_ == 0 || min_ > max_) {
    (*this) = StaticType();
  }
}

void StaticType::typeExcept(const StaticType &st)
{
  assert((st.flags_ & FUNCTION_TYPE) == 0);

  flags_ &= ~st.flags_;
}

void StaticType::typeConcat(const StaticType &st)
{
  bool thisFunctions = (flags_ & FUNCTION_TYPE) != 0;
  bool otherFunctions = (st.flags_ & FUNCTION_TYPE) != 0;

  flags_ |= st.flags_;

  min_ += st.min_;
  if(max_ == UNLIMITED || st.max_ == UNLIMITED)
    max_ = UNLIMITED;
  else max_ += st.max_;

  if(st.returnType_) {
    if(returnType_) {
      minArgs_ = (minArgs_ < st.minArgs_) ? minArgs_ : st.minArgs_;
      maxArgs_ = (maxArgs_ > st.maxArgs_) ? maxArgs_ : st.maxArgs_;
      returnType_->typeUnion(*st.returnType_);
    }
    else if(!thisFunctions) {
      if(mm_ == 0) mm_ = st.mm_;
      minArgs_ = st.minArgs_;
      maxArgs_ = st.maxArgs_;
      returnType_ = new (mm_) StaticType(*st.returnType_);
    }
    else {
      // This was a wildcard function, the union of which is still a wildcard function
    }
  }
  else if(otherFunctions) {
    // Other was a wildcard function, so this is now a wildcard function
    minArgs_ = 0;
    maxArgs_ = 0;
    delete returnType_;
    returnType_ = 0;
  }
}

void StaticType::typeNodeIntersect(const StaticType &st)
{
  if(flags_ != st.flags_ && (flags_ & st.flags_) != 0) {
    min_ = 0;
  }

  flags_ &= st.flags_;
  min_ = (min_ < st.min_) ? min_ : st.min_;
  max_ = (max_ < st.max_) ? max_ : st.max_;

  flags_ &= ~FUNCTION_TYPE;
  minArgs_ = 0;
  maxArgs_ = 0;
  delete returnType_;
  returnType_ = 0;

  if(flags_ == 0 || max_ == 0) {
    min_ = 0;
    max_ = 0;
    flags_ = 0;
  }

  assert(min_ <= max_);
}

StaticType StaticType::operator|(const StaticType &st) const
{
  StaticType result(*this);
  result.typeUnion(st);
  return result;
}

StaticType &StaticType::operator|=(const StaticType &st)
{
  typeUnion(st);
  return *this;
}

StaticType::StaticTypeFlags operator|(StaticType::StaticTypeFlags a, StaticType::StaticTypeFlags b)
{
  return (StaticType::StaticTypeFlags)((unsigned int)a | (unsigned int)b);
}

StaticType StaticType::operator&(const StaticType &st) const
{
  StaticType result(*this);
  result.typeIntersect(st);
  return result;
}

StaticType &StaticType::operator&=(const StaticType &st)
{
  typeIntersect(st);
  return *this;
}

StaticType::StaticTypeFlags operator&(StaticType::StaticTypeFlags a, StaticType::StaticTypeFlags b)
{
  return (StaticType::StaticTypeFlags)((unsigned int)a & (unsigned int)b);
}

StaticType &StaticType::substitute(const StaticType &from, const StaticType &to)
{
  assert((from.flags_ & FUNCTION_TYPE) == 0);
  assert((to.flags_ & FUNCTION_TYPE) == 0);

  if(containsType(from)) {
    if(to.max_ > 0) {
      if(max_ == UNLIMITED || to.max_ == UNLIMITED)
        max_ = UNLIMITED;
      else max_ *= to.max_;
    }

    flags_ &= ~from.flags_;
    flags_ |= to.flags_;

    if(flags_ == 0) {
      min_ = 0;
      max_ = 0;
    }
  }

  return *this;
}

StaticType &StaticType::multiply(unsigned int min, unsigned int max)
{
  assert(min <= max);
  assert(min != UNLIMITED);

  min_ *= min;

  if(max == UNLIMITED || max_ == UNLIMITED)
    max_ = UNLIMITED;
  else max_ *= max;

  if(max_ == 0) {
    min_ = 0;
    flags_ = 0;
  }

  return *this;
}

void StaticType::setCardinality(unsigned int min, unsigned int max)
{
  assert(min <= max);
  assert(min != UNLIMITED);

  min_ = min;
  max_ = max;

  if(max_ > 0 && flags_ == 0) flags_ = ITEM_TYPE;
  else if(max_ == 0 && flags_ != 0) flags_ = 0;
}

bool StaticType::containsType(StaticType::StaticTypeFlags flags) const
{
  return (flags_ & flags) != 0;
}

bool StaticType::containsType(const StaticType &type) const
{
  if((flags_ & type.flags_) == 0)
    return false;

  if((type.flags_ & FUNCTION_TYPE) != 0 && type.returnType_ != 0 &&
     returnType_ != 0 &&
     (type.minArgs_ > maxArgs_ || type.maxArgs_ < minArgs_ ||

      returnType_->min_ > type.returnType_->max_ ||
      returnType_->max_ < type.returnType_->min_ ||

      (!type.returnType_->containsType(*returnType_) &&
       (returnType_->min_ > 0 || type.returnType_->min_ > 0))
      ))
    return false;

  return true;
}

bool StaticType::isType(const StaticType &type) const
{
  if(type.flags_ == 0) return flags_ == 0;
  if(flags_ == 0) return type.min_ == 0;

  if((flags_ & type.flags_) == 0 || (flags_ & ~type.flags_) != 0)
    return false;

  if((type.flags_ & FUNCTION_TYPE) != 0 && type.returnType_ != 0 &&
     (type.minArgs_ != minArgs_ || type.maxArgs_ != maxArgs_ ||
      returnType_->min_ > type.returnType_->max_ ||
      returnType_->max_ < type.returnType_->min_ ||
      (!type.returnType_->containsType(*returnType_) &&
       (returnType_->min_ > 0 || type.returnType_->min_ > 0))
      ))
    return false;

  return true;
}

StaticType::TypeMatchEnum StaticType::matchesFunctionType(const StaticType &type) const
{
  TypeMatchEnum result = ALWAYS;

  if(returnType_ && type.returnType_) {
    if(type.minArgs_ > maxArgs_ || type.maxArgs_ < minArgs_)
      return NEVER;
    if(result > MAYBE &&
       type.minArgs_ != minArgs_ && type.maxArgs_ != maxArgs_)
      result = MAYBE;

    // TBD Take function conversion into account here - jpcs

    TypeMatch ret = returnType_->matches(*type.returnType_);
//     if(ret.type < result) result = ret.type;
    if(ret.cardinality < result) result = ret.cardinality;
  }

  return result;
}

StaticType::TypeMatchEnum StaticType::matchesType(const StaticType &type) const
{
  TypeMatchEnum result = ALWAYS;

  if(type.flags_ == 0 && min_ == 0)
    return ALWAYS;

  if((flags_ & type.flags_) == 0) {
    if(min_ > 0 || type.min_ > 0)
      return NEVER;
    result = PROBABLY_NOT;
  }

  if((type.flags_ & ~flags_) != 0)
    result = MAYBE;

  TypeMatchEnum func = matchesFunctionType(type);

  if(flags_ == FUNCTION_TYPE) {
    if(func < result) result = func;
  }
  else {
    if(func == NEVER && result > PROBABLY_NOT) result = PROBABLY_NOT;
    if(func < ALWAYS && result > MAYBE) result = MAYBE;
  }

  return result;
}

StaticType::TypeMatch StaticType::matches(const StaticType &actual) const
{
  TypeMatch result = { ALWAYS, ALWAYS };

  if(min_ > actual.max_ ||
     max_ < actual.min_) {
    result.cardinality = NEVER;
  }
  else if(min_ > actual.min_ ||
          max_ < actual.max_) {
    if(actual.max_ != StaticType::UNLIMITED)
      result.cardinality = PROBABLY_NOT;
    else
      result.cardinality = MAYBE;
  }

  result.type = matchesType(actual);

  return result;
}

static int countBits(unsigned int v)
{
  int result = 0;

  if(v == StaticType::ITEM_TYPE) {
    ++result;
    v &= ~StaticType::ITEM_TYPE;
  }
  if(v == StaticType::ANY_ATOMIC_TYPE) {
    ++result;
    v &= ~StaticType::ANY_ATOMIC_TYPE;
  }
  if(v == StaticType::NODE_TYPE) {
    ++result;
    v &= ~StaticType::NODE_TYPE;
  }

  while(v != 0) {
    result += (v & 0x1);
    v = v >> 1;
  }
  return result;
}

void StaticType::typeToBuf(XMLBuffer &result) const
{
  if(flags_ == 0) {
    result.append(X("empty-sequence()"));
    return;
  }

  unsigned int flags = flags_;

  bool except = false;
  int count = 0;
  XMLBuffer buf;

  if(flags != ANY_ATOMIC_TYPE &&
     (flags & ~ANY_ATOMIC_TYPE) == 0 &&
     countBits(flags) > countBits(ANY_ATOMIC_TYPE & ~flags)) {
    result.append(X("(xs:anyAtomicType - "));
    except = true;
    flags = ANY_ATOMIC_TYPE & ~flags;
  }
  else if(flags != NODE_TYPE &&
          (flags & ~NODE_TYPE) == 0 &&
          countBits(flags) > countBits(NODE_TYPE & ~flags)) {
    result.append(X("(node() - "));
    except = true;
    flags = NODE_TYPE & ~flags;
  }
  else if(flags != ITEM_TYPE &&
          countBits(flags) > countBits(ITEM_TYPE & ~flags)) {
    result.append(X("(item() - "));
    except = true;
    flags = ITEM_TYPE & ~flags;
  }


  if(flags == ITEM_TYPE && returnType_ == 0) {
    if(count++) buf.append(X(" | "));
    buf.append(X("item()"));
    flags = 0;
  }
  if((flags & ANY_ATOMIC_TYPE) == ANY_ATOMIC_TYPE) {
    if(count++) buf.append(X(" | "));
    buf.append(X("xs:anyAtomicType"));
    flags &= ~ANY_ATOMIC_TYPE;
  }
  if((flags & NODE_TYPE) == NODE_TYPE) {
    if(count++) buf.append(X(" | "));
    buf.append(X("node()"));
    flags &= ~NODE_TYPE;
  }

  if((flags & DOCUMENT_TYPE) != 0) {
    if(count++) buf.append(X(" | "));
    buf.append(X("document-node()"));
  }
  if((flags & ELEMENT_TYPE) != 0) {
    if(count++) buf.append(X(" | "));
    buf.append(X("element()"));
  }
  if((flags & ATTRIBUTE_TYPE) != 0) {
    if(count++) buf.append(X(" | "));
    buf.append(X("attribute()"));
  }
  if((flags & TEXT_TYPE) != 0) {
    if(count++) buf.append(X(" | "));
    buf.append(X("text()"));
  }
  if((flags & PI_TYPE) != 0) {
    if(count++) buf.append(X(" | "));
    buf.append(X("processing-instruction()"));
  }
  if((flags & COMMENT_TYPE) != 0) {
    if(count++) buf.append(X(" | "));
    buf.append(X("comment()"));
  }
  if((flags & NAMESPACE_TYPE) != 0) {
    if(count++) buf.append(X(" | "));
    buf.append(X("namespace()"));
  }
  if((flags & FUNCTION_TYPE) != 0) {
    if(count++) buf.append(X(" | "));

    buf.append(X("function("));
    if(returnType_ == 0) {
      buf.append(X("*"));
    }
    else {
      XPath2Utils::numToBuf(minArgs_, buf);
      buf.append(X(","));
      if(maxArgs_ == UNLIMITED)
        buf.append(X("unlimited"));
      else XPath2Utils::numToBuf(maxArgs_, buf);
      buf.append(X(","));
      returnType_->typeToBuf(buf);
    }
    buf.append(X(")"));
  }

  if((flags & ANY_SIMPLE_TYPE) != 0) {
    if(count++) buf.append(X(" | "));
    buf.append(X("xs:anySimpleType"));
  }
  if((flags & ANY_URI_TYPE) != 0) {
    if(count++) buf.append(X(" | "));
    buf.append(X("xs:anyURI"));
  }
  if((flags & BASE_64_BINARY_TYPE) != 0) {
    if(count++) buf.append(X(" | "));
    buf.append(X("xs:base64Binary"));
  }
  if((flags & BOOLEAN_TYPE) != 0) {
    if(count++) buf.append(X(" | "));
    buf.append(X("xs:boolean"));
  }
  if((flags & DATE_TYPE) != 0) {
    if(count++) buf.append(X(" | "));
    buf.append(X("xs:date"));
  }
  if((flags & DATE_TIME_TYPE) != 0) {
    if(count++) buf.append(X(" | "));
    buf.append(X("xs:dateTime"));
  }
  if((flags & DAY_TIME_DURATION_TYPE) != 0) {
    if(count++) buf.append(X(" | "));
    buf.append(X("xs:dayTimeDuration"));
  }
  if((flags & DECIMAL_TYPE) != 0) {
    if(count++) buf.append(X(" | "));
    buf.append(X("xs:decimal"));
  }
  if((flags & DOUBLE_TYPE) != 0) {
    if(count++) buf.append(X(" | "));
    buf.append(X("xs:double"));
  }
  if((flags & DURATION_TYPE) != 0) {
    if(count++) buf.append(X(" | "));
    buf.append(X("xs:duration"));
  }
  if((flags & FLOAT_TYPE) != 0) {
    if(count++) buf.append(X(" | "));
    buf.append(X("xs:float"));
  }
  if((flags & G_DAY_TYPE) != 0) {
    if(count++) buf.append(X(" | "));
    buf.append(X("xs:gDay"));
  }
  if((flags & G_MONTH_TYPE) != 0) {
    if(count++) buf.append(X(" | "));
    buf.append(X("xs:gMonth"));
  }
  if((flags & G_MONTH_DAY_TYPE) != 0) {
    if(count++) buf.append(X(" | "));
    buf.append(X("xs:gMonthDay"));
  }
  if((flags & G_YEAR_TYPE) != 0) {
    if(count++) buf.append(X(" | "));
    buf.append(X("xs:gYear"));
  }
  if((flags & G_YEAR_MONTH_TYPE) != 0) {
    if(count++) buf.append(X(" | "));
    buf.append(X("xs:gYearMonth"));
  }
  if((flags & HEX_BINARY_TYPE) != 0) {
    if(count++) buf.append(X(" | "));
    buf.append(X("xs:hexBinary"));
  }
  if((flags & NOTATION_TYPE) != 0) {
    if(count++) buf.append(X(" | "));
    buf.append(X("xs:NOTATION"));
  }
  if((flags & QNAME_TYPE) != 0) {
    if(count++) buf.append(X(" | "));
    buf.append(X("xs:QName"));
  }
  if((flags & STRING_TYPE) != 0) {
    if(count++) buf.append(X(" | "));
    buf.append(X("xs:string"));
  }
  if((flags & TIME_TYPE) != 0) {
    if(count++) buf.append(X(" | "));
    buf.append(X("xs:time"));
  }
  if((flags & UNTYPED_ATOMIC_TYPE) != 0) {
    if(count++) buf.append(X(" | "));
    buf.append(X("xs:untypedAtomic"));
  }
  if((flags & YEAR_MONTH_DURATION_TYPE) != 0) {
    if(count++) buf.append(X(" | "));
    buf.append(X("xs:yearMonthDuration"));
  }

  if(count > 1) {
    result.append(X("("));
    result.append(buf.getRawBuffer());
    result.append(X(")"));
  }
  else {
    result.append(buf.getRawBuffer());
  }

  if(except) {
    result.append(X(")"));
  }

  if(min_ == 0 && max_ == 1) {
    result.append(X("?"));
  }
  else if(min_ == 0 && max_ == StaticType::UNLIMITED) {
    result.append(X("*"));
  }
  else if(min_ == 1 && max_ == StaticType::UNLIMITED) {
    result.append(X("+"));
  }
  else if(min_ == 1 && max_ == 1) {
    // nothing
  }
  else {
    result.append(X("{"));

    XPath2Utils::numToBuf(min_, result);
    result.append(X(","));
    if(max_ == UNLIMITED)
      result.append(X("unlimited"));
    else XPath2Utils::numToBuf(max_, result);

    result.append(X("}"));
  }
}


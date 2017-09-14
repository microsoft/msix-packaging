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
#include <xqilla/framework/XQillaExport.hpp>
#include <xqilla/ast/XQLiteral.hpp>
#include <xqilla/ast/XQSequence.hpp>
#include <xqilla/context/DynamicContext.hpp>
#include <xqilla/items/Item.hpp>
#include <xqilla/context/ItemFactory.hpp>
#include <xqilla/events/EventHandler.hpp>
#include <xqilla/items/impl/ATQNameOrDerivedImpl.hpp>
#include <xqilla/items/impl/ATDecimalOrDerivedImpl.hpp>
#include <xqilla/items/impl/ATDoubleOrDerivedImpl.hpp>
#include <xqilla/items/impl/ATFloatOrDerivedImpl.hpp>
#include <xqilla/exceptions/IllegalArgumentException.hpp>
#include <xqilla/utils/XPath2Utils.hpp>

#include <xercesc/validators/schema/SchemaSymbols.hpp>

XERCES_CPP_NAMESPACE_USE;

ASTNode *XQLiteral::create(const Item::Ptr &item, DynamicContext *context, XPath2MemoryManager* memMgr,
                           const LocationInfo *location)
{
  if(item->isAtomicValue()) {
    ASTNode *result = 0;

    const AnyAtomicType *atom = (const AnyAtomicType*)item.get();
    switch(atom->getPrimitiveTypeIndex()) {
    case AnyAtomicType::QNAME: {
      const ATQNameOrDerived *qname = (const ATQNameOrDerived*)atom;
      result = new (memMgr) XQQNameLiteral(atom->getTypeURI(), atom->getTypeName(),
                                           qname->getURI(), qname->getPrefix(),
                                           qname->getName(), memMgr);
      break;
    }
    case AnyAtomicType::DECIMAL:
    case AnyAtomicType::DOUBLE:
    case AnyAtomicType::FLOAT: {
      const Numeric *number = (const Numeric*)atom;
      if(number->getState() == Numeric::NUM || (number->getState() == Numeric::NEG_NUM && !number->isZero())) {
        result = new (memMgr) XQNumericLiteral(number->getTypeURI(), number->getTypeName(), number->asMAPM(),
                                               number->getPrimitiveTypeIndex(), memMgr);
        break;
      }
      // Fall through
    }
    default:
      result = new (memMgr) XQLiteral(atom->getTypeURI(), atom->getTypeName(), atom->asString(context),
                                      atom->getPrimitiveTypeIndex(), memMgr);
      break;
    }

    result->setLocationInfo(location);
    return result;
  }
  else {
    XQThrow2(::IllegalArgumentException, X("XQLiteral::create"), X("Cannot create an ASTNode literal for a non atomic item"));
  }
}

ASTNode *XQLiteral::create(bool value, XPath2MemoryManager* mm, const LocationInfo *location)
{
  ASTNode *result = new (mm) XQLiteral(SchemaSymbols::fgURI_SCHEMAFORSCHEMA,
                                       SchemaSymbols::fgDT_BOOLEAN,
                                       value ? SchemaSymbols::fgATTVAL_TRUE : SchemaSymbols::fgATTVAL_FALSE,
                                       AnyAtomicType::BOOLEAN, mm);
  result->setLocationInfo(location);
  return result;
}

XQLiteral::XQLiteral(const XMLCh* typeURI, const XMLCh* typeName, const XMLCh* value,
                     AnyAtomicType::AtomicObjectType primitiveType, XPath2MemoryManager* memMgr)
  : ASTNodeImpl(LITERAL, memMgr),
    typeURI_(typeURI),
    typeName_(typeName),
    primitiveType_(primitiveType),
    value_(value)
{
  _src.getStaticType() = StaticType::create(primitiveType_);
}

bool XQLiteral::isDateOrTimeAndHasNoTimezone(StaticContext *context) const
{
  switch(primitiveType_) {
  case AnyAtomicType::DATE:
  case AnyAtomicType::DATE_TIME:
  case AnyAtomicType::TIME:
  case AnyAtomicType::G_DAY:
  case AnyAtomicType::G_MONTH:
  case AnyAtomicType::G_MONTH_DAY:
  case AnyAtomicType::G_YEAR:
  case AnyAtomicType::G_YEAR_MONTH: {
    if(context == 0) return true;
    AutoDelete<DynamicContext> dContext(context->createDynamicContext());
    dContext->setMemoryManager(context->getMemoryManager());
    Item::Ptr item = dContext->getItemFactory()->createDerivedFromAtomicType(primitiveType_, typeURI_, typeName_, value_, dContext);
    return !((const DateOrTimeType*)item.get())->hasTimezone();
  }
  default: break;
  }

  return false;
}

ASTNode* XQLiteral::staticResolution(StaticContext *context)
{
  switch(primitiveType_) {
  case AnyAtomicType::DECIMAL:
  case AnyAtomicType::FLOAT:
  case AnyAtomicType::DOUBLE: {
    // Constant fold, to parse numeric literals
    XPath2MemoryManager* mm = context->getMemoryManager();
    AutoDelete<DynamicContext> dContext(context->createDynamicContext());
    dContext->setMemoryManager(mm);

    Result result = createResult(dContext);
    ASTNode *newBlock = XQSequence::constantFold(result, dContext, mm, this);
    this->release();
    return newBlock;
  }
  default: break;
  }

  return this;
}

ASTNode *XQLiteral::staticTypingImpl(StaticContext *context)
{
  return this;
}

BoolResult XQLiteral::boolResult(DynamicContext* context) const
{
  assert(primitiveType_ == AnyAtomicType::BOOLEAN);
  return XPath2Utils::equals(SchemaSymbols::fgATTVAL_TRUE, value_);
}

Result XQLiteral::createResult(DynamicContext* context, int flags) const
{
  return (Item::Ptr)context->getItemFactory()->createDerivedFromAtomicType(primitiveType_, typeURI_, typeName_, value_, context).get();
}

EventGenerator::Ptr XQLiteral::generateEvents(EventHandler *events, DynamicContext *context,
                                              bool preserveNS, bool preserveType) const
{
  events->atomicItemEvent(primitiveType_, value_, typeURI_, typeName_);
  return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

XQQNameLiteral::XQQNameLiteral(const XMLCh* typeURI, const XMLCh* typeName, const XMLCh* uri,
                               const XMLCh* prefix, const XMLCh* localname, XPath2MemoryManager* memMgr)
  : ASTNodeImpl(QNAME_LITERAL, memMgr),
    typeURI_(typeURI),
    typeName_(typeName),
    uri_(uri),
    prefix_(prefix),
    localname_(localname)
{
  _src.getStaticType() = StaticType::QNAME_TYPE;
}

bool XQQNameLiteral::isDateOrTimeAndHasNoTimezone(StaticContext *context) const
{
  return false;
}

ASTNode* XQQNameLiteral::staticResolution(StaticContext *context)
{
  return this;
}

ASTNode *XQQNameLiteral::staticTypingImpl(StaticContext *context)
{
  return this;
}

Result XQQNameLiteral::createResult(DynamicContext* context, int flags) const
{
  return Item::Ptr(new ATQNameOrDerivedImpl(typeURI_, typeName_, uri_, prefix_, localname_, context));
}

EventGenerator::Ptr XQQNameLiteral::generateEvents(EventHandler *events, DynamicContext *context,
                                                   bool preserveNS, bool preserveType) const
{
  XMLBuffer buf;
  if(prefix_ && *prefix_) {
    buf.append(prefix_);
    buf.append(':');
  }
  buf.append(localname_);
  events->atomicItemEvent(AnyAtomicType::QNAME, buf.getRawBuffer(), typeURI_, typeName_);
  return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

XQNumericLiteral::XQNumericLiteral(const XMLCh* typeURI, const XMLCh* typeName, const MAPM &value,
                                   AnyAtomicType::AtomicObjectType primitiveType, XPath2MemoryManager* memMgr)
  : ASTNodeImpl(NUMERIC_LITERAL, memMgr),
    typeURI_(typeURI),
    typeName_(typeName),
    primitiveType_(primitiveType)
{
  _src.getStaticType() = StaticType::create(primitiveType_);

  memset(&value_, 0, sizeof(value_));

  const M_APM cval = value.c_struct();

  value_.m_apm_datalength = cval->m_apm_datalength;
  value_.m_apm_exponent = cval->m_apm_exponent;
  value_.m_apm_sign = cval->m_apm_sign;

  int len = (cval->m_apm_datalength + 1) >> 1;
  value_.m_apm_data = (UCHAR*)memMgr->allocate(len);

  memcpy(value_.m_apm_data, cval->m_apm_data, len);
}

bool XQNumericLiteral::isDateOrTimeAndHasNoTimezone(StaticContext *context) const
{
  return false;
}

ASTNode* XQNumericLiteral::staticResolution(StaticContext *context)
{
  return this;
}

ASTNode *XQNumericLiteral::staticTypingImpl(StaticContext *context)
{
  return this;
}

MAPM XQNumericLiteral::getValue() const
{
  // Use the C API to copy our fake MAPM
  MAPM copy;
  m_apm_copy(const_cast<M_APM>(copy.c_struct()), const_cast<M_APM>(&value_));
  return copy;
}

Result XQNumericLiteral::createResult(DynamicContext* context, int flags) const
{
  switch(primitiveType_) {
  case AnyAtomicType::DECIMAL:
    return (Item::Ptr)new ATDecimalOrDerivedImpl(typeURI_, typeName_, getValue(), context);
  case AnyAtomicType::FLOAT:
    return (Item::Ptr)new ATFloatOrDerivedImpl(typeURI_, typeName_, getValue(), context);
  case AnyAtomicType::DOUBLE:
    return (Item::Ptr)new ATDoubleOrDerivedImpl(typeURI_, typeName_, getValue(), context);
  default: break;
  }
  return 0;
}

EventGenerator::Ptr XQNumericLiteral::generateEvents(EventHandler *events, DynamicContext *context,
                                              bool preserveNS, bool preserveType) const
{
  switch(primitiveType_) {
  case AnyAtomicType::DECIMAL:
    events->atomicItemEvent(primitiveType_, Numeric::asDecimalString(getValue(), ATDecimalOrDerivedImpl::g_nSignificantDigits,
                                                                     context), typeURI_, typeName_);
    break;
  case AnyAtomicType::FLOAT:
    events->atomicItemEvent(primitiveType_, Numeric::asDoubleString(Numeric::NUM, getValue(),
                                                                    ATFloatOrDerivedImpl::g_nSignificantDigits, context),
                            typeURI_, typeName_);
    break;
  case AnyAtomicType::DOUBLE:
    events->atomicItemEvent(primitiveType_, Numeric::asDoubleString(Numeric::NUM, getValue(),
                                                                    ATDoubleOrDerivedImpl::g_nSignificantDigits, context),
                            typeURI_, typeName_);
    break;
  default: break;
  }
  return 0;
}


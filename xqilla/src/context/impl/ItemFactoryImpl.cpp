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

#include <xqilla/context/impl/ItemFactoryImpl.hpp>
#include <xqilla/context/DynamicContext.hpp>
#include <xqilla/items/DatatypeLookup.hpp>
#include <xqilla/items/DatatypeFactory.hpp>
#include <xqilla/items/impl/ATBooleanOrDerivedImpl.hpp>
#include <xqilla/items/impl/ATDecimalOrDerivedImpl.hpp>
#include <xqilla/items/impl/ATDoubleOrDerivedImpl.hpp>
#include <xqilla/items/impl/ATFloatOrDerivedImpl.hpp>
#include <xqilla/items/impl/ATQNameOrDerivedImpl.hpp>
#include <xqilla/items/impl/ATDurationOrDerivedImpl.hpp>
#include <xqilla/items/ATDurationOrDerived.hpp>
#include <xqilla/items/ATUntypedAtomic.hpp>
#include <xqilla/utils/XPath2Utils.hpp>
#include <xqilla/utils/XStr.hpp>
#include <xqilla/exceptions/TypeNotFoundException.hpp>
#include "../../exceptions/InvalidLexicalSpaceException.hpp"

#include <xercesc/validators/schema/SchemaSymbols.hpp>

#if defined(XERCES_HAS_CPP_NAMESPACE)
XERCES_CPP_NAMESPACE_USE
#endif

ItemFactoryImpl::ItemFactoryImpl(const DocumentCache* dc, MemoryManager* memMgr)
  : datatypeLookup_(new (memMgr) DatatypeLookup(dc, memMgr))
{
}

ItemFactoryImpl::ItemFactoryImpl(DatatypeLookup *datatypeLookup)
  : datatypeLookup_(datatypeLookup)
{
}

ItemFactoryImpl::~ItemFactoryImpl()
{
  delete datatypeLookup_;
}

AnyAtomicType::AtomicObjectType ItemFactoryImpl::getPrimitiveTypeIndex(const XMLCh* typeURI, const XMLCh* typeName, bool &isPrimitive) const
{
  const DatatypeFactory* dtf = datatypeLookup_->lookupDatatype(typeURI, typeName, isPrimitive);
  return dtf->getPrimitiveTypeIndex();
}

void ItemFactoryImpl::getPrimitiveTypeName(AnyAtomicType::AtomicObjectType type, const XMLCh *&typeURI, const XMLCh *&typeName) const
{
  const DatatypeFactory *dtf = datatypeLookup_->lookupDatatype(type);
  typeURI = dtf->getPrimitiveTypeURI();
  typeName = dtf->getPrimitiveTypeName();
}

ATQNameOrDerived::Ptr ItemFactoryImpl::createQName(const XMLCh* uri,
	const XMLCh *prefix,
	const XMLCh* name, 
	const DynamicContext* context
	) {
  return createQNameOrDerived(
    SchemaSymbols::fgURI_SCHEMAFORSCHEMA,
    SchemaSymbols::fgDT_QNAME,
    uri,
    prefix,
    name,
    context
    );
}

ATDoubleOrDerived::Ptr ItemFactoryImpl::createDouble(const MAPM value, const DynamicContext* context)
{
  return new ATDoubleOrDerivedImpl(SchemaSymbols::fgURI_SCHEMAFORSCHEMA,
                                   SchemaSymbols::fgDT_DOUBLE, value, context);
}

ATDoubleOrDerived::Ptr ItemFactoryImpl::createDouble(const XMLCh* value, const DynamicContext* context) {
  return (ATDoubleOrDerived*)datatypeLookup_->getDoubleFactory()->createInstance(value, context).get();
}

ATFloatOrDerived::Ptr ItemFactoryImpl::createFloat(const MAPM value, const DynamicContext* context)
{
  return new ATFloatOrDerivedImpl(SchemaSymbols::fgURI_SCHEMAFORSCHEMA,
                                  SchemaSymbols::fgDT_FLOAT, value, context);
}

ATFloatOrDerived::Ptr ItemFactoryImpl::createFloat(const XMLCh* value, const DynamicContext* context) {
  return (ATFloatOrDerived*)datatypeLookup_->getFloatFactory()->createInstance(value, context).get();
}

ATDecimalOrDerived::Ptr ItemFactoryImpl::createDecimal(const MAPM value, const DynamicContext* context) {
  return new ATDecimalOrDerivedImpl(SchemaSymbols::fgURI_SCHEMAFORSCHEMA,
                                    SchemaSymbols::fgDT_DECIMAL, value, context);
}

ATDecimalOrDerived::Ptr ItemFactoryImpl::createDecimal(const XMLCh* value, const DynamicContext* context) {
  return (ATDecimalOrDerived*)datatypeLookup_->getDecimalFactory()->createInstance(value, context).get();
}

ATDecimalOrDerived::Ptr ItemFactoryImpl::createInteger(const int value, const DynamicContext* context) {
  return context->getMemoryManager()->createInteger(value);
}

ATDecimalOrDerived::Ptr ItemFactoryImpl::createInteger(const MAPM value, const DynamicContext* context)
{
  if(value.is_integer()) {
    return new ATDecimalOrDerivedImpl(SchemaSymbols::fgURI_SCHEMAFORSCHEMA,
                                      SchemaSymbols::fgDT_INTEGER, value, context);
  }

  return createDecimalOrDerived(
    SchemaSymbols::fgURI_SCHEMAFORSCHEMA,
    SchemaSymbols::fgDT_INTEGER,
    value, context);
}

ATDecimalOrDerived::Ptr ItemFactoryImpl::createInteger(const XMLCh* value, const DynamicContext* context) {
  return createDecimalOrDerived(
    SchemaSymbols::fgURI_SCHEMAFORSCHEMA,
    SchemaSymbols::fgDT_INTEGER,
    value, context);
}

ATBooleanOrDerived::Ptr ItemFactoryImpl::createBoolean(bool value, const DynamicContext* context) {
  return createBooleanOrDerived(
    SchemaSymbols::fgURI_SCHEMAFORSCHEMA,
    SchemaSymbols::fgDT_BOOLEAN,
    value, context);  
}

ATBooleanOrDerived::Ptr ItemFactoryImpl::createBoolean(BoolResult value, const DynamicContext* context)
{
  if(value == BoolResult::Null) return 0;
  return createBooleanOrDerived(
    SchemaSymbols::fgURI_SCHEMAFORSCHEMA,
    SchemaSymbols::fgDT_BOOLEAN,
    value, context);  
}

ATBooleanOrDerived::Ptr ItemFactoryImpl::createBoolean(const XMLCh* value, const DynamicContext* context) {
  return (ATBooleanOrDerived*)datatypeLookup_->getBooleanFactory()->createInstance(value, context).get();
}

ATDecimalOrDerived::Ptr ItemFactoryImpl::createNonNegativeInteger(const MAPM value, const DynamicContext* context)
{
  if(value.is_integer() && value.sign() >= 0) {
    return new ATDecimalOrDerivedImpl(SchemaSymbols::fgURI_SCHEMAFORSCHEMA,
                                      SchemaSymbols::fgDT_NONNEGATIVEINTEGER, value, context);
  }

  return createDecimalOrDerived(
    SchemaSymbols::fgURI_SCHEMAFORSCHEMA,
    SchemaSymbols::fgDT_NONNEGATIVEINTEGER,
    value, context);
}
      
ATDurationOrDerived::Ptr ItemFactoryImpl::createDayTimeDuration(const XMLCh* value, const DynamicContext* context) {
  return (ATDurationOrDerived*)datatypeLookup_->getDurationFactory()->
    createInstance(SchemaSymbols::fgURI_SCHEMAFORSCHEMA, 
                   ATDurationOrDerived::fgDT_DAYTIMEDURATION, value, context).get();
}

ATDurationOrDerived::Ptr ItemFactoryImpl::createDayTimeDuration(const MAPM &seconds, const DynamicContext* context)
{
  return new ATDurationOrDerivedImpl(SchemaSymbols::fgURI_SCHEMAFORSCHEMA,
                                     ATDurationOrDerived::fgDT_DAYTIMEDURATION,
                                     0, seconds, context);
}

ATDurationOrDerived::Ptr ItemFactoryImpl::createYearMonthDuration(const XMLCh* value, const DynamicContext* context) {
  return (ATDurationOrDerived*)datatypeLookup_->getDurationFactory()->
    createInstance(SchemaSymbols::fgURI_SCHEMAFORSCHEMA, 
                   ATDurationOrDerived::fgDT_YEARMONTHDURATION, value, context).get();
}

ATDurationOrDerived::Ptr ItemFactoryImpl::createYearMonthDuration(const MAPM &months, const DynamicContext* context)
{
  return new ATDurationOrDerivedImpl(SchemaSymbols::fgURI_SCHEMAFORSCHEMA,
                                     ATDurationOrDerived::fgDT_YEARMONTHDURATION,
                                     months, 0, context);
}

ATDateOrDerived::Ptr ItemFactoryImpl::createDate(const XMLCh* value, const DynamicContext* context) {
  return (ATDateOrDerived*)datatypeLookup_->getDateFactory()->createInstance(value, context).get();
}

ATDateTimeOrDerived::Ptr ItemFactoryImpl::createDateTime(const XMLCh* value, const DynamicContext* context) {
  return (ATDateTimeOrDerived*)datatypeLookup_->getDateTimeFactory()->createInstance(value, context).get();
}

ATTimeOrDerived::Ptr ItemFactoryImpl::createTime(const XMLCh* value, const DynamicContext* context) {
  return (ATTimeOrDerived*)datatypeLookup_->getTimeFactory()->createInstance(value, context).get();
}

ATAnyURIOrDerived::Ptr ItemFactoryImpl::createAnyURI(const XMLCh* value, const DynamicContext* context) {
  return datatypeLookup_->getAnyURIFactory()->createInstance(value, context);
}

ATStringOrDerived::Ptr ItemFactoryImpl::createString(const XMLCh* value, const DynamicContext* context) {
  return (ATStringOrDerived*)datatypeLookup_->getStringFactory()->createInstance(value, context).get();
}

//////////////////////////
// All creation methods //
//////////////////////////


AnyAtomicType::Ptr ItemFactoryImpl::createDerivedFromAtomicType(AnyAtomicType::AtomicObjectType typeIndex, const XMLCh* value,
                                                                const DynamicContext* context)
{
  return datatypeLookup_->lookupDatatype(typeIndex)->createInstance(value, context);
}

AnyAtomicType::Ptr ItemFactoryImpl::createDerivedFromAtomicType(AnyAtomicType::AtomicObjectType typeIndex, const XMLCh* typeURI,
                                                                const XMLCh* typeName, const XMLCh* value, const DynamicContext* context)
{
  return datatypeLookup_->lookupDatatype(typeIndex)->createInstance(typeURI, typeName, value, context);
}

AnyAtomicType::Ptr ItemFactoryImpl::createDerivedFromAtomicType(const XMLCh* typeURI,
                                                                const XMLCh* typeName, 
                                                                const XMLCh* value, const DynamicContext* context) {
  bool isPrimitive;
  const DatatypeFactory* dtf = datatypeLookup_->lookupDatatype(typeURI, typeName, isPrimitive);
  if(isPrimitive) {
    return dtf->createInstance(value, context);
  }
  else {
    return dtf->createInstance(typeURI, typeName, value, context);
  }
}

/** create a xs:boolean */
ATBooleanOrDerived::Ptr ItemFactoryImpl::createBooleanOrDerived(const XMLCh* typeURI, 
                                                                  const XMLCh* typeName,
                                                                  const XMLCh* value, 
                                                                  const DynamicContext* context) {
  return (const ATBooleanOrDerived*)datatypeLookup_->getBooleanFactory()->createInstance(typeURI, typeName, value, context).get();
}

/** create a xs:boolean with a bool value */
ATBooleanOrDerived::Ptr ItemFactoryImpl::createBooleanOrDerived(const XMLCh* typeURI, 
                                                                  const XMLCh* typeName,
                                                                  bool value, 
                                                                  const DynamicContext* context) {
  // No need to validate
  return new ATBooleanOrDerivedImpl(typeURI, typeName, value, context);
}

/** create a xs:date */
ATDateOrDerived::Ptr ItemFactoryImpl::createDateOrDerived(const XMLCh* typeURI, 
                                                            const XMLCh* typeName,
                                                            const XMLCh* value, 
                                                            const DynamicContext* context){
  return (const ATDateOrDerived*)datatypeLookup_->getDateFactory()->createInstance(typeURI, typeName, value, context).get();
}


/** create a xs:dateTime */
ATDateTimeOrDerived::Ptr ItemFactoryImpl::createDateTimeOrDerived(const XMLCh* typeURI, 
                                                                    const XMLCh* typeName,
                                                                    const XMLCh* value,
                                                                    const DynamicContext* context){
  return (const ATDateTimeOrDerived*)datatypeLookup_->getDateTimeFactory()->createInstance(typeURI, typeName, value, context).get();
}

/** create a xs:decimal */
ATDecimalOrDerived::Ptr ItemFactoryImpl::createDecimalOrDerived(const XMLCh* typeURI, 
                                                                  const XMLCh* typeName,
                                                                  const XMLCh* value,
                                                                  const DynamicContext* context){
  return (const ATDecimalOrDerived*)datatypeLookup_->getDecimalFactory()->createInstance(typeURI, typeName, value, context).get();
}

/** create a xs:decimal with a MAPM */
ATDecimalOrDerived::Ptr ItemFactoryImpl::createDecimalOrDerived(const XMLCh* typeURI, 
                                                                  const XMLCh* typeName,
                                                                  const MAPM value,
                                                                  const DynamicContext* context)
{
  ATDecimalOrDerived::Ptr retVal = new ATDecimalOrDerivedImpl(typeURI, typeName, value, context);

  // check if it's a valid instance
  DatatypeValidator* validator = context->getDocumentCache()->getDatatypeValidator(typeURI, typeName);
  if(!validator) {
    XMLBuffer buf(1023, context->getMemoryManager());
    buf.append(X("Type "));
    buf.append(typeURI);
    buf.append(chColon);
    buf.append(typeName);
    buf.append(X(" not found"));
    XQThrow2(TypeNotFoundException, X("ItemFactoryImpl::createDecimalOrDerived"), buf.getRawBuffer());
  }
  try {
    validator->validate(retVal->asString(context), 0, context->getMemoryManager());
  } catch (XMLException &e) {
    XMLBuffer buf(1023, context->getMemoryManager());
    buf.append(e.getMessage());
    buf.append(X(" [err:FORG0001]"));
    XQThrow2(InvalidLexicalSpaceException, X("ItemFactoryImpl::createDecimalOrDerived"), buf.getRawBuffer());
  }

  return retVal;
}


/** create a xs:double */  
ATDoubleOrDerived::Ptr ItemFactoryImpl::createDoubleOrDerived(const XMLCh* typeURI, 
                                                                const XMLCh* typeName,
                                                                const XMLCh* value, 
                                                                const DynamicContext* context){
  if(XPath2Utils::equals(value, Numeric::NAN_string)) {
    value= Numeric::NaN_string;
  }
  return (const ATDoubleOrDerived*)datatypeLookup_->getDoubleFactory()->createInstance(typeURI, typeName, value, context).get();
}

/** create a xs:double with a MAPM */
ATDoubleOrDerived::Ptr ItemFactoryImpl::createDoubleOrDerived(const XMLCh* typeURI, 
                                                                const XMLCh* typeName,
                                                                const MAPM value, 
                                                                const DynamicContext* context)
{
  ATDoubleOrDerived::Ptr retVal = new ATDoubleOrDerivedImpl(typeURI, typeName, value, context);

  // check if it's a valid instance
  DatatypeValidator* validator = context->getDocumentCache()->getDatatypeValidator(typeURI, typeName);
  if(!validator) {
    XMLBuffer buf(1023, context->getMemoryManager());
    buf.append(X("Type "));
    buf.append(typeURI);
    buf.append(chColon);
    buf.append(typeName);
    buf.append(X(" not found"));
    XQThrow2(TypeNotFoundException, X("ItemFactoryImpl::createDoubleOrDerived"), buf.getRawBuffer());
  }
  try {
    validator->validate(retVal->asString(context), 0, context->getMemoryManager());
  } catch (XMLException &e) {
    XMLBuffer buf(1023, context->getMemoryManager());
    buf.append(e.getMessage());
    buf.append(X(" [err:FORG0001]"));
    XQThrow2(InvalidLexicalSpaceException, X("ItemFactoryImpl::createDoubleOrDerived"), buf.getRawBuffer());
  }

  return retVal;
}


/** create a xs:float */
ATFloatOrDerived::Ptr ItemFactoryImpl::createFloatOrDerived(const XMLCh* typeURI, 
                                                              const XMLCh* typeName,
                                                              const XMLCh* value, 
                                                              const DynamicContext* context){
  if(XPath2Utils::equals(value, Numeric::NAN_string)) {
    value= Numeric::NaN_string;
  }
  return (const ATFloatOrDerived::Ptr)datatypeLookup_->getFloatFactory()->createInstance(typeURI, typeName, value, context);

}

/** create a xs:float with a MAPM */
ATFloatOrDerived::Ptr ItemFactoryImpl::createFloatOrDerived(const XMLCh* typeURI, 
                                                              const XMLCh* typeName,
                                                              const MAPM value, 
                                                              const DynamicContext* context)
{ 
  ATFloatOrDerived::Ptr retVal = new ATFloatOrDerivedImpl(typeURI, typeName, value, context);

  // check if it's a valid instance
  DatatypeValidator* validator = context->getDocumentCache()->getDatatypeValidator(typeURI, typeName);
  if(!validator) {
    XMLBuffer buf(1023, context->getMemoryManager());
    buf.append(X("Type "));
    buf.append(typeURI);
    buf.append(chColon);
    buf.append(typeName);
    buf.append(X(" not found"));
    XQThrow2(TypeNotFoundException, X("ItemFactoryImpl::createDoubleOrDerived"), buf.getRawBuffer());
  }
  try {
    validator->validate(retVal->asString(context), 0, context->getMemoryManager());
  } catch (XMLException &e) {
    XMLBuffer buf(1023, context->getMemoryManager());
    buf.append(e.getMessage());
    buf.append(X(" [err:FORG0001]"));
    XQThrow2(InvalidLexicalSpaceException, X("ItemFactoryImpl::createDoubleOrDerived"), buf.getRawBuffer());
  }

  return retVal;
}

/** create a xs:gDay */
ATGDayOrDerived::Ptr ItemFactoryImpl::createGDayOrDerived(const XMLCh* typeURI, 
                                                            const XMLCh* typeName,
                                                            const XMLCh* value, 
                                                            const DynamicContext* context) {
  return (const ATGDayOrDerived::Ptr)datatypeLookup_->getGDayFactory()->createInstance(typeURI, typeName, value, context);
}

/** create a xs:gMonth */
ATGMonthOrDerived::Ptr ItemFactoryImpl::createGMonthOrDerived(const XMLCh* typeURI, 
                                                                const XMLCh* typeName,
                                                                const XMLCh* value, 
                                                                const DynamicContext* context) {
  return (const ATGMonthOrDerived::Ptr)datatypeLookup_->getGMonthFactory()->createInstance(typeURI, typeName, value, context);

}


/** create a xs:gMonthDay */
ATGMonthDayOrDerived::Ptr ItemFactoryImpl::createGMonthDayOrDerived(const XMLCh* typeURI, 
                                                                      const XMLCh* typeName,
                                                                      const XMLCh* value, 
                                                                      const DynamicContext* context) {
  return (const ATGMonthDayOrDerived::Ptr)datatypeLookup_->getGMonthDayFactory()->createInstance(typeURI, typeName, value, context);
}

/** create a xs:gYear */
ATGYearOrDerived::Ptr ItemFactoryImpl::createGYearOrDerived(const XMLCh* typeURI, 
                                                              const XMLCh* typeName,
                                                              const XMLCh* value, 
                                                              const DynamicContext* context) {
  return (const ATGYearOrDerived::Ptr)datatypeLookup_->getGYearFactory()->createInstance(typeURI, typeName, value, context);
}


/** create a xs:gYearMonth */
ATGYearMonthOrDerived::Ptr ItemFactoryImpl::createGYearMonthOrDerived(const XMLCh* typeURI, 
                                                                        const XMLCh* typeName,
                                                                        const XMLCh* value, 
                                                                        const DynamicContext* context) {
  return (const ATGYearMonthOrDerived::Ptr)datatypeLookup_->getGYearMonthFactory()->createInstance(typeURI, typeName, value, context);
}

/** create a xs:QName with two parameters */
ATQNameOrDerived::Ptr ItemFactoryImpl::createQNameOrDerived(const XMLCh* typeURI, 
	const XMLCh* typeName,
	const XMLCh* uri,
	const XMLCh* prefix,
	const XMLCh* name, 
	const DynamicContext* context) {

  ATQNameOrDerivedImpl* tmp =  new ATQNameOrDerivedImpl(typeURI, typeName, uri, prefix, name, context);
  
  const DatatypeFactory* dtf_anyURI = datatypeLookup_->getAnyURIFactory();
  if(dtf_anyURI->checkInstance(uri, context->getMemoryManager())) {
    const DatatypeFactory* dtf_NCName = datatypeLookup_->getStringFactory();
    if (dtf_NCName->checkInstance(SchemaSymbols::fgURI_SCHEMAFORSCHEMA, SchemaSymbols::fgDT_NCNAME, name, context->getMemoryManager())) {
      return tmp;
    } else {
      // this call will obviously fail, but it is better for error reporting, 
      // since we actually get the XMLException's error message 
      return (const ATQNameOrDerived::Ptr )dtf_NCName->
        createInstance(SchemaSymbols::fgURI_SCHEMAFORSCHEMA, SchemaSymbols::fgDT_NCNAME, name, context);
    }
  } else {
    // this call will obviously fail, but it is better for error reporting, 
    // since we actually get the XMLException's error message 
    return (const ATQNameOrDerived::Ptr )dtf_anyURI->createInstance(uri, context);
  }
}


/** create a xs:string */
ATStringOrDerived::Ptr ItemFactoryImpl::createStringOrDerived(const XMLCh* typeURI, 
                                                                const XMLCh* typeName,
                                                                const XMLCh* value, 
                                                                const DynamicContext* context) {
  return (const ATStringOrDerived::Ptr)datatypeLookup_->getStringFactory()->createInstance(typeURI, typeName, value, context);
}

/** create a xs:time */
ATTimeOrDerived::Ptr ItemFactoryImpl::createTimeOrDerived(const XMLCh* typeURI, 
                                                            const XMLCh* typeName,
                                                            const XMLCh* value, 
                                                            const DynamicContext* context){
  return (const ATTimeOrDerived::Ptr)datatypeLookup_->getTimeFactory()->createInstance(typeURI, typeName, value, context);
}

/** create an xdt:untypedAtomic */
ATUntypedAtomic::Ptr ItemFactoryImpl::createUntypedAtomic(const XMLCh* value, const DynamicContext* context) {
  return (const ATUntypedAtomic::Ptr)datatypeLookup_->getUntypedAtomicFactory()->createInstance(value, context);
}

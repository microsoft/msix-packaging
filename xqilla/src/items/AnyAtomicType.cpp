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
#include <xqilla/items/AnyAtomicType.hpp>
#include <xqilla/utils/XPath2Utils.hpp>
#include <xqilla/context/DynamicContext.hpp>
#include <xqilla/items/DatatypeFactory.hpp>
#include <xqilla/framework/XPath2MemoryManager.hpp>
#include "../exceptions/InvalidLexicalSpaceException.hpp"
#include <xqilla/exceptions/TypeNotFoundException.hpp>
#include <xqilla/exceptions/XPath2TypeCastException.hpp>
#include <xqilla/exceptions/NamespaceLookupException.hpp>
#include <xqilla/exceptions/IllegalArgumentException.hpp>
#include <xqilla/context/ItemFactory.hpp>
#include <xqilla/events/EventHandler.hpp>

#include <xercesc/util/XMLString.hpp>
#include <xercesc/util/XMLUniDefs.hpp>
#include <xercesc/validators/schema/SchemaSymbols.hpp>
#include <xercesc/framework/XMLBuffer.hpp>

XERCES_CPP_NAMESPACE_USE

/* anyAtomicType */
const XMLCh AnyAtomicType::fgDT_ANYATOMICTYPE[]=
{ 
	chLatin_a, chLatin_n, chLatin_y, 
	chLatin_A, chLatin_t, chLatin_o, 
	chLatin_m, chLatin_i, chLatin_c, 
	chLatin_T, chLatin_y, chLatin_p, 
	chLatin_e, chNull 
};

const AnyAtomicType::CastTable AnyAtomicType::staticCastTable;

/* isAtomicValue from Item */
bool AnyAtomicType::isAtomicValue() const {
  return true;
}

/* isNode from Item */
bool AnyAtomicType::isNode() const {
  return false;
}

bool AnyAtomicType::isFunction() const {
  return false;
}

/* is this type numeric?  Return false by default */
bool AnyAtomicType::isNumericValue() const {
  return false;
}

/* is this type date or time based?  Return false by default */
bool AnyAtomicType::isDateOrTimeTypeValue() const {
  return false;
}

void AnyAtomicType::generateEvents(EventHandler *events, const DynamicContext *context,
                                   bool preserveNS, bool preserveType) const
{
  events->atomicItemEvent(getPrimitiveTypeIndex(), asString(context),
                          getTypeURI(), getTypeName());
}

/* Get the namespace uri of the primitive type (basic type) of this type */
const XMLCh* AnyAtomicType::getPrimitiveTypeURI() const{
  return SchemaSymbols::fgURI_SCHEMAFORSCHEMA;
}

void AnyAtomicType::typeToBuffer(DynamicContext *context, XMLBuffer &buffer) const
{
  if(getTypeURI()) {
    buffer.append('{');
    buffer.append(getTypeURI());
    buffer.append('}');
  }
  buffer.append(getTypeName());
}

AnyAtomicType::Ptr AnyAtomicType::castAs(AtomicObjectType targetIndex, const DynamicContext* context) const
{
  // We assume this cast is supported, since it's an internal cast,
  // not one specified by the user
  return castAsNoCheck(targetIndex, 0, 0, context);
}

AnyAtomicType::Ptr AnyAtomicType::castAs(AtomicObjectType targetIndex, const XMLCh* targetTypeURI,
                                         const XMLCh* targetTypeName, const DynamicContext* context) const
{
  if(!castIsSupported(targetIndex, context)) {
    if(targetTypeName == 0) {
      context->getItemFactory()->getPrimitiveTypeName(targetIndex, targetTypeURI, targetTypeName);
    }

    XMLBuffer buffer(1023, context->getMemoryManager());
    buffer.set(X("Casting from {"));
    buffer.append(getTypeURI());
    buffer.append(X("}"));
    buffer.append(getTypeName());
    buffer.append(X(" to {"));
    buffer.append(targetTypeURI);
    buffer.append(X("}"));
    buffer.append(targetTypeName);
    buffer.append(X(" is not supported [err:XPTY0004]"));

    XQThrow2(XPath2TypeCastException, X("AnyAtomicType::castAs"), buffer.getRawBuffer());
  }

  return castAsNoCheck(targetIndex, targetTypeURI, targetTypeName, context);
}
  
AnyAtomicType::Ptr AnyAtomicType::castAsNoCheck(AtomicObjectType targetIndex, const XMLCh* targetURI, const XMLCh* targetType,
                                                const DynamicContext* context) const
{
  try {
    return castAsInternal(targetIndex, targetURI, targetType, context);
  } catch (TypeNotFoundException &e) {
    XQThrow2(XPath2TypeCastException, X("AnyAtomicType::castAs"), e.getError());
  } catch (InvalidLexicalSpaceException &e) {
    if(getPrimitiveTypeIndex() == UNTYPED_ATOMIC ||
       getPrimitiveTypeIndex() == ANY_SIMPLE_TYPE || 
       getPrimitiveTypeIndex() == STRING) {
      XQThrow2(XPath2TypeCastException, X("AnyAtomicType::castAs"), X("Invalid lexical value [err:FORG0001]"));
    } else if(getPrimitiveTypeIndex() == targetIndex) {
      XQThrow2(XPath2TypeCastException, X("AnyAtomicType::castAs"), X("Value does not conform to facets [err:FORG0001]"));
    } else {
      XQThrow2(XPath2TypeCastException, X("AnyAtomicType::castAs"), e.getError());  // should never be here, in theory
    }
  } catch (NamespaceLookupException &e) {
    XQThrow2(XPath2TypeCastException, X("AnyAtomicType::castAs"), e.getError());
  }
}

AnyAtomicType::Ptr AnyAtomicType::castAsInternal(AtomicObjectType targetIndex, const XMLCh* targetTypeURI,
                                                 const XMLCh* targetTypeName, const DynamicContext* context) const
{
  if(targetTypeName == 0) {
    return context->getItemFactory()->createDerivedFromAtomicType(targetIndex, asString(context), context);
  }
  else {
    return context->getItemFactory()->createDerivedFromAtomicType(targetIndex, targetTypeURI, targetTypeName,
                                                                  asString(context), context);
  }
}

/* Test if this type can be cast to the target type */
bool AnyAtomicType::castable(AtomicObjectType targetIndex, const XMLCh* targetTypeURI, const XMLCh* targetTypeName,
                             const DynamicContext* context) const
{
  if(!castIsSupported(targetIndex, context)) {
          return false;
  }
  // validate the data by calling castAs (can't use checkInstance)
  try {
    castAsInternal(targetIndex, targetTypeURI, targetTypeName, context);
  } catch (::IllegalArgumentException &e) {
    return false;
  } catch (XPath2TypeCastException &e) {
    return false;
  } catch (TypeNotFoundException &e) {
    return false;
  } catch (InvalidLexicalSpaceException &e) {
    return false;
  } catch (NamespaceLookupException &e) {
    return false;
  }
  return true;
}

/* Returns true if this typeName and uri match the given typeName and uri */
bool AnyAtomicType::isOfType(const XMLCh* targetTypeURI, const XMLCh* targetTypeName, const DynamicContext* context) const {
  return (XPath2Utils::equals(targetTypeName, this->getTypeName())) &&
         (XPath2Utils::equals(targetTypeURI,  this->getTypeURI()));
}

/* Returns true if this typeName and uri match the given type name and uri,
 * or if any of this type's parents match the given type name and uri */
bool AnyAtomicType::isInstanceOfType(const XMLCh* targetTypeURI, const XMLCh* targetTypeName, const StaticContext* context) const {
  return context->isTypeOrDerivedFromType(this->getTypeURI(), this->getTypeName(), targetTypeURI, targetTypeName);
}


/**
  * Returns true if 
  * (a) both the input type and the target type are built-in schema types and 
  *     cast is supported for the combination.
  * (b) cast is supported if the input type is a derived atomic type and the 
  *     target type is a supertype of the input type
  * (c) cast is supported if the target type is a derived atomic type and the 
  *     input type is xs:string, xs:anySimpleType, or a supertype of the 
  *     target type.
  * (d) If a primitive type P1 can be cast into a primitive type P2, then any 
  *     subtype of P1 can be cast into any subtype of P2
 **/
bool AnyAtomicType::castIsSupported(AtomicObjectType targetIndex, const DynamicContext* context) const {
  return staticCastTable.getCell(getPrimitiveTypeIndex(), targetIndex);
}


///////////////////
// methods from  //
//   CastTable   //
///////////////////

/* This is the table we will mimic, but we will make it enum safe :-)
 * 
       uA str flt dbl dec int dur yMD dTD dT tim dat gYM gYr gMD gDay gMon bol b64 hxB aURI QN NOT
uA  { { 1, 1,  1,  1,  1,  1,  1,  1,  1,  1, 1,  1,  1,  1,  1,   1,   1,  1,  1,  1,  1,  0,  0 },
str   { 1, 1,  1,  1,  1,  1,  1,  1,  1,  1, 1,  1,  1,  1,  1,   1,   1,  1,  1,  1,  1,  1,  1 },
flt   { 1, 1,  1,  1,  1,  1,  0,  0,  0,  0, 0,  0,  0,  0,  0,   0,   0,  1,  0,  0,  0,  0,  0 },
dbl   { 1, 1,  1,  1,  1,  1,  0,  0,  0,  0, 0,  0,  0,  0,  0,   0,   0,  1,  0,  0,  0,  0,  0 },
dec   { 1, 1,  1,  1,  1,  1,  0,  0,  0,  0, 0,  0,  0,  0,  0,   0,   0,  1,  0,  0,  0,  0,  0 },
int   { 1, 1,  1,  1,  1,  1,  0,  0,  0,  0, 0,  0,  0,  0,  0,   0,   0,  1,  0,  0,  0,  0,  0 },
dur   { 1, 1,  0,  0,  0,  0,  1,  1,  1,  0, 0,  0,  0,  0,  0,   0,   0,  0,  0,  0,  0,  0,  0 },
yMD   { 1, 1,  0,  0,  0,  0,  1,  1,  1,  0, 0,  0,  0,  0,  0,   0,   0,  0,  0,  0,  0,  0,  0 },
dTD   { 1, 1,  0,  0,  0,  0,  1,  1,  1,  0, 0,  0,  0,  0,  0,   0,   0,  0,  0,  0,  0,  0,  0 },
dT    { 1, 1,  0,  0,  0,  0,  0,  0,  0,  1, 1,  1,  1,  1,  1,   1,   1,  0,  0,  0,  0,  0,  0 }, 
tim   { 1, 1,  0,  0,  0,  0,  0,  0,  0,  0, 1,  0,  0,  0,  0,   0,   0,  0,  0,  0,  0,  0,  0 }, 
dat   { 1, 1,  0,  0,  0,  0,  0,  0,  0,  1, 0,  1,  1,  1,  1,   1,   1,  0,  0,  0,  0,  0,  0 }, 
gYM   { 1, 1,  0,  0,  0,  0,  0,  0,  0,  0, 0,  0,  1,  0,  0,   0,   0,  0,  0,  0,  0,  0,  0 }, 
gYr   { 1, 1,  0,  0,  0,  0,  0,  0,  0,  0, 0,  0,  0,  1,  0,   0,   0,  0,  0,  0,  0,  0,  0 }, 
gMD   { 1, 1,  0,  0,  0,  0,  0,  0,  0,  0, 0,  0,  0,  0,  1,   0,   0,  0,  0,  0,  0,  0,  0 }, 
gDay  { 1, 1,  0,  0,  0,  0,  0,  0,  0,  0, 0,  0,  0,  0,  0,   1,   0,  0,  0,  0,  0,  0,  0 }, 
gMon  { 1, 1,  0,  0,  0,  0,  0,  0,  0,  0, 0,  0,  0,  0,  0,   0,   1,  0,  0,  0,  0,  0,  0 }, 
bool  { 1, 1,  1,  1,  1,  1,  0,  0,  0,  0, 0,  0,  0,  0,  0,   0,   0,  1,  0,  0,  0,  0,  0 }, 
b64   { 1, 1,  0,  0,  0,  0,  0,  0,  0,  0, 0,  0,  0,  0,  0,   0,   0,  0,  1,  1,  0,  0,  0 }, 
hxB   { 1, 1,  0,  0,  0,  0,  0,  0,  0,  0, 0,  0,  0,  0,  0,   0,   0,  0,  1,  1,  0,  0,  0 }, 
aURI  { 1, 1,  0,  0,  0,  0,  0,  0,  0,  0, 0,  0,  0,  0,  0,   0,   0,  0,  0,  0,  1,  0,  0 }, 
QN,   { 1, 1,  0,  0,  0,  0,  0,  0,  0,  0, 0,  0,  0,  0,  0,   0,   0,  0,  0,  0,  0,  1,  0 }, 
NOT   { 1, 1,  0,  0,  0,  0,  0,  0,  0,  0, 0,  0,  0,  0,  0,   0,   0,  0,  0,  0,  0,  0,  1 } };
*/  


AnyAtomicType::CastTable::CastTable() {
  // initialize table to allow casting only between same types
  int j;
  for(j = 0; j<(int)AnyAtomicType::NumAtomicObjectTypes; ++j) {
    for(int k = 0; k<(int)AnyAtomicType::NumAtomicObjectTypes; ++k) {
      staticCastTable[j][k] = false;
    }
  }

  for(j = 0; j<(int)AnyAtomicType::NumAtomicObjectTypes; ++j) {
    staticCastTable[j][j] = true; // you can always cast something to itself
    staticCastTable[j][AnyAtomicType::STRING] = true;  // anything can be cast to string
    staticCastTable[j][AnyAtomicType::UNTYPED_ATOMIC] = true;  // anything can be cast to untypedAtomic

    // xs:string and xs:untypedAtomic can be cast to anything, except an xs:QName or xs:NOTATION
    if(j != AnyAtomicType::NOTATION && j != AnyAtomicType::QNAME) {
      staticCastTable[AnyAtomicType::STRING][j] = true;  // strings can be cast to anything
      staticCastTable[AnyAtomicType::UNTYPED_ATOMIC][j] = true;  // untypedAtomic can be cast to anything
    }
  }

  // finally, add special casting rules
  staticCastTable[AnyAtomicType::FLOAT][AnyAtomicType::DOUBLE] = true;
  staticCastTable[AnyAtomicType::DOUBLE][AnyAtomicType::FLOAT] = true;
  
  staticCastTable[AnyAtomicType::FLOAT][AnyAtomicType::DECIMAL] = true;
  staticCastTable[AnyAtomicType::DECIMAL][AnyAtomicType::FLOAT] = true;
  
  staticCastTable[AnyAtomicType::FLOAT][AnyAtomicType::BOOLEAN] = true;
  staticCastTable[AnyAtomicType::BOOLEAN][AnyAtomicType::FLOAT] = true;
  
  staticCastTable[AnyAtomicType::DOUBLE][AnyAtomicType::DECIMAL] = true;
  staticCastTable[AnyAtomicType::DECIMAL][AnyAtomicType::DOUBLE] = true;
  
  staticCastTable[AnyAtomicType::DOUBLE][AnyAtomicType::BOOLEAN] = true;
  staticCastTable[AnyAtomicType::BOOLEAN][AnyAtomicType::DOUBLE] = true;
  
  staticCastTable[AnyAtomicType::DECIMAL][AnyAtomicType::BOOLEAN] = true;
  staticCastTable[AnyAtomicType::BOOLEAN][AnyAtomicType::DECIMAL] = true;
  
  staticCastTable[AnyAtomicType::DURATION][AnyAtomicType::YEAR_MONTH_DURATION] = true;
  staticCastTable[AnyAtomicType::YEAR_MONTH_DURATION][AnyAtomicType::DURATION] = true;
  
  staticCastTable[AnyAtomicType::DURATION][AnyAtomicType::DAY_TIME_DURATION] = true;
  staticCastTable[AnyAtomicType::DAY_TIME_DURATION][AnyAtomicType::DURATION] = true;
  
  staticCastTable[AnyAtomicType::DAY_TIME_DURATION][AnyAtomicType::YEAR_MONTH_DURATION] = true;
  staticCastTable[AnyAtomicType::YEAR_MONTH_DURATION][AnyAtomicType::DAY_TIME_DURATION] = true;
  
  staticCastTable[AnyAtomicType::DATE_TIME][AnyAtomicType::DATE] = true;
  staticCastTable[AnyAtomicType::DATE][AnyAtomicType::DATE_TIME] = true;
  
  staticCastTable[AnyAtomicType::DATE_TIME][AnyAtomicType::TIME] = true;
  
  staticCastTable[AnyAtomicType::DATE_TIME][AnyAtomicType::G_YEAR_MONTH] = true;
  staticCastTable[AnyAtomicType::DATE_TIME][AnyAtomicType::G_YEAR]       = true;
  staticCastTable[AnyAtomicType::DATE_TIME][AnyAtomicType::G_MONTH_DAY]  = true;
  staticCastTable[AnyAtomicType::DATE_TIME][AnyAtomicType::G_DAY]        = true;
  staticCastTable[AnyAtomicType::DATE_TIME][AnyAtomicType::G_MONTH]      = true;
  
  staticCastTable[AnyAtomicType::DATE][AnyAtomicType::G_YEAR_MONTH] = true;
  staticCastTable[AnyAtomicType::DATE][AnyAtomicType::G_YEAR]       = true;
  staticCastTable[AnyAtomicType::DATE][AnyAtomicType::G_MONTH_DAY]  = true;
  staticCastTable[AnyAtomicType::DATE][AnyAtomicType::G_DAY]        = true;
  staticCastTable[AnyAtomicType::DATE][AnyAtomicType::G_MONTH]      = true;

  staticCastTable[AnyAtomicType::BASE_64_BINARY][AnyAtomicType::HEX_BINARY]      = true;
  staticCastTable[AnyAtomicType::HEX_BINARY][AnyAtomicType::BASE_64_BINARY]      = true;

/*  debug output
 *  for(int i = 0; i<(int)AnyAtomicType::NumAtomicObjectTypes; i++) {
    for(int j = 0; j<(int)AnyAtomicType::NumAtomicObjectTypes; j++) {
       cerr << " " << staticCastTable[i][j] << ","<<flush; 
    }
    cerr << endl;
  }
*/      
  
}

bool AnyAtomicType::CastTable::getCell(AnyAtomicType::AtomicObjectType source, 
                AnyAtomicType::AtomicObjectType target) const {
  return staticCastTable[source][target];
  
}

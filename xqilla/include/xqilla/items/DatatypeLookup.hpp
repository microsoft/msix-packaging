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

#ifndef _DATATYPELOOKUP_HPP
#define _DATATYPELOOKUP_HPP

#include <xqilla/framework/XQillaExport.hpp>
#include <xqilla/items/AnyAtomicType.hpp>

#include <xercesc/util/XercesDefs.hpp>
#include <xercesc/util/RefHashTableOf.hpp>
#include <xercesc/util/XMemory.hpp>

class DatatypeFactory;
class DocumentCache;

class XQILLA_API DatatypeLookup : public XERCES_CPP_NAMESPACE_QUALIFIER XMemory
{
public:
  DatatypeLookup(const DocumentCache* dc, XERCES_CPP_NAMESPACE_QUALIFIER MemoryManager* memMgr);
  virtual ~DatatypeLookup();

  ///returns the appropriate Datatype object
  const DatatypeFactory* lookupDatatype(AnyAtomicType::AtomicObjectType typeIndex) const;

  ///returns the appropriate Datatype object
  const DatatypeFactory* lookupDatatype(const XMLCh* typeURI, const XMLCh* typeName, bool &isPrimitive) const;

  DatatypeFactory *getAnySimpleTypeFactory() const;
  DatatypeFactory *getAnyURIFactory() const;
  DatatypeFactory *getBase64BinaryFactory() const;
  DatatypeFactory *getBooleanFactory() const;
  DatatypeFactory *getDateFactory() const;
  DatatypeFactory *getDateTimeFactory() const;
  DatatypeFactory *getDayTimeDurationFactory() const;
  DatatypeFactory *getDecimalFactory() const;
  DatatypeFactory *getDoubleFactory() const;
  DatatypeFactory *getDurationFactory() const;
  DatatypeFactory *getFloatFactory() const;
  DatatypeFactory *getGDayFactory() const;
  DatatypeFactory *getGMonthFactory() const;
  DatatypeFactory *getGMonthDayFactory() const;
  DatatypeFactory *getGYearFactory() const;
  DatatypeFactory *getGYearMonthFactory() const;
  DatatypeFactory *getHexBinaryFactory() const;
  DatatypeFactory *getNotationFactory() const;
  DatatypeFactory *getQNameFactory() const;
  DatatypeFactory *getStringFactory() const;
  DatatypeFactory *getTimeFactory() const;
  DatatypeFactory *getYearMonthDurationFactory() const;
  DatatypeFactory *getUntypedAtomicFactory() const;

private:
  DatatypeLookup & operator= (const DatatypeLookup& r);
  DatatypeLookup(const DatatypeLookup &other);

  // add a primitive datatype to the table
  void insertDatatype(DatatypeFactory *datatype);
    
private:
  DatatypeFactory *anySimpleType_;
  DatatypeFactory *anyURI_;
  DatatypeFactory *base64Binary_;
  DatatypeFactory *boolean_;
  DatatypeFactory *date_;
  DatatypeFactory *dateTime_;
  DatatypeFactory *dayTimeDuration_;
  DatatypeFactory *decimal_;
  DatatypeFactory *double_;
  DatatypeFactory *duration_;
  DatatypeFactory *float_;
  DatatypeFactory *gDay_;
  DatatypeFactory *gMonth_;
  DatatypeFactory *gMonthDay_;
  DatatypeFactory *gYear_;
  DatatypeFactory *gYearMonth_;
  DatatypeFactory *hexBinary_;
  DatatypeFactory *notation_;
  DatatypeFactory *qname_;
  DatatypeFactory *string_;
  DatatypeFactory *time_;
  DatatypeFactory *yearMonthDuration_;
  DatatypeFactory *untypedAtomic_;

  XERCES_CPP_NAMESPACE_QUALIFIER RefHashTableOf< DatatypeFactory > fDatatypeTable;
    
  const DocumentCache *fDocumentCache;

  XERCES_CPP_NAMESPACE_QUALIFIER MemoryManager* fMemMgr;
};

#endif 

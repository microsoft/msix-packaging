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

#ifndef _ATBOOLEANORDERIVED_HPP
#define _ATBOOLEANORDERIVED_HPP

#include <xercesc/util/XercesDefs.hpp>
#include <xqilla/items/AnyAtomicType.hpp>

#include <xqilla/framework/XQillaExport.hpp>

class XPath2MemoryManager;

struct XQILLA_API BoolResult
{
  static const BoolResult True;
  static const BoolResult False;
  static const BoolResult Null;

  inline BoolResult(bool b) : value(b ? BOOL_TRUE : BOOL_FALSE) {}
  inline operator bool() const { return value == BOOL_TRUE; }
  inline bool operator==(BoolResult o) const { return value == o.value; }
  inline bool operator!=(BoolResult o) const { return value != o.value; }

private:
  inline BoolResult() : value(BOOL_NULL) {}
  enum {
    BOOL_NULL,
    BOOL_FALSE,
    BOOL_TRUE
  } value;
};

class XQILLA_API ATBooleanOrDerived : public AnyAtomicType
{
public:
  typedef RefCountPointer<const ATBooleanOrDerived> Ptr;

  /* Get the name of the primitive type (basic type) of this type 
   * (ie "decimal" for xs:decimal) */
  virtual const XMLCh* getPrimitiveTypeName() const = 0;

  /* Get the namespace URI for this type */
  virtual const XMLCh* getTypeURI() const = 0;

  /* Get the name of this type  (ie "integer" for xs:integer) */
  virtual const XMLCh* getTypeName() const = 0;

  /* returns the XMLCh* (canonical) representation of this type */
  virtual const XMLCh* asString(const DynamicContext* context) const = 0;

  /* returns true if the two objects have the same boolean value
   * false otherwise */
  virtual bool equals(const AnyAtomicType::Ptr &target, const DynamicContext* context) const = 0;

  /** Returns less than 0 if this is less that other,
      0 if they are the same, and greater than 0 otherwise */
  virtual int compare(const ATBooleanOrDerived::Ptr &other, const DynamicContext *context) const = 0;
  
  /* returns true if boolean value evaluates to true
   * false otherwise */
  virtual bool isTrue() const = 0;

  /* returns false if boolean value evaluates to true
   * true otherwise */
  virtual bool isFalse() const = 0;

  virtual AnyAtomicType::AtomicObjectType getPrimitiveTypeIndex() const = 0;
};

#endif

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

#ifndef _ANYATOMICTYPE_HPP
#define _ANYATOMICTYPE_HPP

#include <xqilla/framework/XQillaExport.hpp>
#include <xqilla/items/Item.hpp>
#include <xercesc/util/XercesDefs.hpp>

class DynamicContext;
class StaticContext;

class XQILLA_API AnyAtomicType: public Item
{

public:
  enum AtomicObjectType {
    ANY_SIMPLE_TYPE    = 0,
    ANY_URI            = 1,
    BASE_64_BINARY     = 2,
    BOOLEAN            = 3,
    DATE               = 4,
    DATE_TIME          = 5,
    DAY_TIME_DURATION  = 6,
    DECIMAL            = 7,
    DOUBLE             = 8,
    DURATION           = 9,
    FLOAT              = 10,
    G_DAY              = 11,
    G_MONTH            = 12,
    G_MONTH_DAY        = 13,
    G_YEAR             = 14,
    G_YEAR_MONTH       = 15,
    HEX_BINARY         = 16,
    NOTATION           = 17,
    QNAME              = 18,
    STRING             = 19,
    TIME               = 20,
    UNTYPED_ATOMIC     = 21,
    YEAR_MONTH_DURATION= 22,
    NumAtomicObjectTypes= 23
  };

  typedef RefCountPointer<const AnyAtomicType> Ptr;

  /* isAtomicValue from Item */
  virtual bool isAtomicValue() const;

  /* isNode from Item */
  virtual bool isNode() const;

  virtual bool isFunction() const;

  /* is this type numeric?  Return false by default */
  virtual bool isNumericValue() const;

  /* is this type date or time based?  Return false by default */
  virtual bool isDateOrTimeTypeValue() const;

  /* Get the namespace uri of the primitive type (basic type) of this type */
  virtual const XMLCh* getPrimitiveTypeURI() const;

  /* Get the name of the primitive type (basic type) of this type (ie "decimal" for xs:decimal) */
  virtual const XMLCh* getPrimitiveTypeName() const = 0;

  /* Get the namespace URI for this type */
  virtual const XMLCh* getTypeURI() const = 0;

  /* Get the name of this type  (ie "integer" for xs:integer) */
  virtual const XMLCh* getTypeName() const = 0;

  virtual void generateEvents(EventHandler *events, const DynamicContext *context,
                              bool preserveNS = true, bool preserveType = true) const;

  /* If possible, cast this type to the target type  -- no casting table lookup made */
  AnyAtomicType::Ptr castAs(AtomicObjectType targetIndex, const DynamicContext* context) const;

  /* If possible, cast this type to the target type  -- no casting table lookup made */
  AnyAtomicType::Ptr castAsNoCheck(AtomicObjectType targetIndex, const XMLCh* targetURI, const XMLCh* targetType,
                                   const DynamicContext* context) const;

  /* If possible, cast this type to the target type  -- not virtual, this is the single entry point for casting */
  AnyAtomicType::Ptr castAs(AtomicObjectType targetIndex, const XMLCh* targetURI, const XMLCh* targetType,
                            const DynamicContext* context) const;

  /* Test if this type can be cast to the target type */
  virtual bool castable(AtomicObjectType targetIndex, const XMLCh* targetURI, const XMLCh* targetType,
                        const DynamicContext* context) const;

  /* returns the XMLCh* (canonical) representation of this type */
  virtual const XMLCh* asString(const DynamicContext* context) const = 0;

  /* returns true if the two objects are equal (whatever that means 
   * in the context of the datatype), false otherwise */
  virtual bool equals(const AnyAtomicType::Ptr &target, const DynamicContext* context) const = 0;

  /* Returns true if this typeName and uri match the given typeName and uri */
  virtual bool isOfType(const XMLCh* targetURI, const XMLCh* targetType, const DynamicContext* context) const;

  /* Returns true if this typeName and uri match the given typeName and uri,
   * or if any of this type's parents match the given typeName and uri */
  virtual bool isInstanceOfType(const XMLCh* targetURI, const XMLCh* targetType, const StaticContext* context) const;

  virtual void typeToBuffer(DynamicContext *context, XERCES_CPP_NAMESPACE_QUALIFIER XMLBuffer &buffer) const;
  
  /**
   * Returns true if 
   * (a) both the input type and the target type are built-in schema types and 
   *     cast is supported for the combination.
   * (b) the input type is a derived atomic type and the 
   *     target type is a supertype of the input type
   * (c) the target type is a derived atomic type and the 
   *     input type is xs:string, xs:anySimpleType, or a supertype of the 
   *     target type.
   * (d) If a primitive type P1 can be cast into a primitive type P2, then any 
   *     subtype of P1 can be cast into any subtype of P2
  **/
  bool castIsSupported(AtomicObjectType targetIndex, const DynamicContext* context) const;

  virtual AtomicObjectType getPrimitiveTypeIndex() const = 0;
 
  static const XMLCh fgDT_ANYATOMICTYPE[];

protected:
  /* internal castAs method.  This one is virtual and does the real work */
  virtual AnyAtomicType::Ptr castAsInternal(AtomicObjectType targetIndex, const XMLCh* targetURI,
                                            const XMLCh* targetType, const DynamicContext* context) const;

private:

  //friend class CastTable;
  //template <class TYPE> friend class DatatypeFactoryTemplate;

  // class that implements the Casting Table
  class CastTable {
    public: 
        CastTable();
        bool getCell(AtomicObjectType source, 
                     AtomicObjectType target) const;
    private:
        bool staticCastTable[NumAtomicObjectTypes][NumAtomicObjectTypes];
  };


  // reproduction of the XPath2 casting table: http://www.w3.org/TR/xquery-operators/#casting
  static const CastTable staticCastTable;  
};

#endif

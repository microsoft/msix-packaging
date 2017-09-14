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

#ifndef _STATICTYPE_HPP
#define _STATICTYPE_HPP

#include <xqilla/framework/XQillaExport.hpp>
#include <xqilla/items/AnyAtomicType.hpp>
#include <xqilla/framework/XPath2MemoryManager.hpp>

#include <xercesc/framework/XMLBuffer.hpp>
#include <xercesc/util/XMemory.hpp>

/**
 * Class that represents the static type of an expression
 */
class XQILLA_API StaticType : public XERCES_CPP_NAMESPACE_QUALIFIER XMemory {
public:
  /**
   * Flags that determine what item types are returned from an expression
   */
  enum StaticTypeFlags {
    DOCUMENT_TYPE            = 0x00000001,
    ELEMENT_TYPE             = 0x00000002,
    ATTRIBUTE_TYPE           = 0x00000004,
    TEXT_TYPE                = 0x00000008,
    PI_TYPE                  = 0x00000010,
    COMMENT_TYPE             = 0x00000020,
    NAMESPACE_TYPE           = 0x00000040,

    ANY_SIMPLE_TYPE          = 0x00000080,
    ANY_URI_TYPE             = 0x00000100,
    BASE_64_BINARY_TYPE      = 0x00000200,
    BOOLEAN_TYPE             = 0x00000400,
    DATE_TYPE                = 0x00000800,
    DATE_TIME_TYPE           = 0x00001000,
    DAY_TIME_DURATION_TYPE   = 0x00002000,
    DECIMAL_TYPE             = 0x00004000,
    DOUBLE_TYPE              = 0x00008000,
    DURATION_TYPE            = 0x00010000,
    FLOAT_TYPE               = 0x00020000,
    G_DAY_TYPE               = 0x00040000,
    G_MONTH_TYPE             = 0x00080000,
    G_MONTH_DAY_TYPE         = 0x00100000,
    G_YEAR_TYPE              = 0x00200000,
    G_YEAR_MONTH_TYPE        = 0x00400000,
    HEX_BINARY_TYPE          = 0x00800000,
    NOTATION_TYPE            = 0x01000000,
    QNAME_TYPE               = 0x02000000,
    STRING_TYPE              = 0x04000000,
    TIME_TYPE                = 0x08000000,
    UNTYPED_ATOMIC_TYPE      = 0x10000000,
    YEAR_MONTH_DURATION_TYPE = 0x20000000,

    FUNCTION_TYPE            = 0x40000000,

    NODE_TYPE                = (DOCUMENT_TYPE | ELEMENT_TYPE | ATTRIBUTE_TYPE | TEXT_TYPE | PI_TYPE | COMMENT_TYPE |
                                NAMESPACE_TYPE),

    NUMERIC_TYPE             = (DECIMAL_TYPE | FLOAT_TYPE | DOUBLE_TYPE),

    TIMEZONE_TYPE            = (DATE_TYPE | DATE_TIME_TYPE | G_DAY_TYPE | G_MONTH_TYPE | G_MONTH_DAY_TYPE |
                                G_YEAR_TYPE | G_YEAR_MONTH_TYPE | TIME_TYPE),

    TYPED_ATOMIC_TYPE        = (NUMERIC_TYPE | TIMEZONE_TYPE | ANY_SIMPLE_TYPE | ANY_URI_TYPE | BASE_64_BINARY_TYPE |
                                BOOLEAN_TYPE | DAY_TIME_DURATION_TYPE | DURATION_TYPE | HEX_BINARY_TYPE |
                                NOTATION_TYPE | QNAME_TYPE | STRING_TYPE | YEAR_MONTH_DURATION_TYPE),

    ANY_ATOMIC_TYPE          = (TYPED_ATOMIC_TYPE | UNTYPED_ATOMIC_TYPE),

    ITEM_TYPE                = (NODE_TYPE | ANY_ATOMIC_TYPE | FUNCTION_TYPE),

    EMPTY_TYPE               = 0
  };

  static const unsigned int UNLIMITED;

  /// Constructor for an empty type
  StaticType();
  // Constructor for normal types
  StaticType(StaticTypeFlags f, unsigned int min = 1, unsigned int max = 1);
  /// Constructor for a function type
  StaticType(XPath2MemoryManager *mm, unsigned int numArgs, const StaticType &returnType, unsigned int min = 1, unsigned int max = 1);
  /// Constructor for a function type
  StaticType(XPath2MemoryManager *mm, unsigned int minArgs, unsigned int maxArgs, const StaticType &returnType,
             unsigned int min = 1, unsigned int max = 1);

  StaticType(const StaticType &o);
  StaticType &operator=(const StaticType &o);
  ~StaticType();

  static StaticType create(const XMLCh *uri, const XMLCh *name, const StaticContext *context,
                                  bool &isExact);
  static StaticType create(AnyAtomicType::AtomicObjectType primitiveType);

  void typeUnion(const StaticType &st);
  void typeIntersect(const StaticType &st);
  void typeExcept(const StaticType &st);

  void typeConcat(const StaticType &st);
  void typeNodeIntersect(const StaticType &st);

  StaticType operator|(const StaticType &st) const;
  StaticType &operator|=(const StaticType &st);

  StaticType operator&(const StaticType &st) const;
  StaticType &operator&=(const StaticType &st);

  StaticType &substitute(const StaticType &from, const StaticType &to);
  StaticType &multiply(unsigned int min, unsigned int max);
  void setCardinality(unsigned int min, unsigned int max);

  bool containsType(const StaticType &type) const;
  bool containsType(StaticType::StaticTypeFlags flags) const;
  bool isType(const StaticType &type) const;

  enum TypeMatchEnum {
    NEVER = 0,
    PROBABLY_NOT = 1,
    MAYBE = 2,
    ALWAYS = 3
  };

  struct TypeMatch
  {
    TypeMatchEnum type, cardinality;
  };

  TypeMatch matches(const StaticType &actual) const;

  unsigned int getMin() const { return min_; }
  unsigned int getMax() const { return max_; }

  unsigned int getMinArgs() const { return minArgs_; }
  unsigned int getMaxArgs() const { return maxArgs_; }
  const StaticType *getReturnType() const { return returnType_; }

  void typeToBuf(XERCES_CPP_NAMESPACE_QUALIFIER XMLBuffer &buf) const;

private:
  TypeMatchEnum matchesFunctionType(const StaticType &type) const;
  TypeMatchEnum matchesType(const StaticType &type) const;

  unsigned int flags_;
  unsigned int min_;
  unsigned int max_;

  XPath2MemoryManager *mm_;
  unsigned int minArgs_;
  unsigned int maxArgs_;
  StaticType *returnType_;
};

XQILLA_API StaticType::StaticTypeFlags operator|(StaticType::StaticTypeFlags a, StaticType::StaticTypeFlags b);
XQILLA_API StaticType::StaticTypeFlags operator&(StaticType::StaticTypeFlags a, StaticType::StaticTypeFlags b);

#endif

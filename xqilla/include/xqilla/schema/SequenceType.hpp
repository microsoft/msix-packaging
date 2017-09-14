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

#ifndef _SEQUENCETYPE_HPP
#define _SEQUENCETYPE_HPP

#include <xqilla/framework/XQillaExport.hpp>

#include <xqilla/items/Node.hpp>
#include <xqilla/items/FunctionRef.hpp>
#include <xqilla/parser/QName.hpp>
#include <xqilla/runtime/Result.hpp>
#include <xqilla/runtime/ResultImpl.hpp>

class Item;
class XPath2MemoryManager;
class SequenceType;
class FunctionSignature;

typedef std::vector<SequenceType*, XQillaAllocator<SequenceType*> > VectorOfSequenceTypes;

class XQILLA_API SequenceType : public LocationInfo
{
public:

  class XQILLA_API ItemType
  {
  public:

    /**
     * The type of item that this sequence can hold.
     */
    typedef enum {
      TEST_ELEMENT,      ///< element node
      TEST_ATTRIBUTE,    ///< attribute node
      TEST_SCHEMA_ELEMENT,      ///< element node
      TEST_SCHEMA_ATTRIBUTE,    ///< attribute node
      TEST_SCHEMA_DOCUMENT,     ///< document node
      TEST_NODE,         ///< node
      TEST_PI,           ///< processing instruction node
      TEST_COMMENT,      ///< comment node
      TEST_TEXT,         ///< text node
      TEST_DOCUMENT,     ///< document node
      TEST_NAMESPACE,    ///< namespace node
      TEST_ANYTHING,     ///< any item
      TEST_ATOMIC_TYPE,  ///< the named atomic type
      TEST_FUNCTION      ///< function
    } ItemTestType;

    // Normal constructor
    ItemType(ItemTestType test, QualifiedName* name=NULL, QualifiedName* type=NULL);
    // Constructor for an atomic type
    ItemType(const XMLCh *typeURI,const XMLCh *typeName, XPath2MemoryManager *mm);
    // Constructor for a function
    ItemType(VectorOfSequenceTypes *argTypes, SequenceType *returnType);

    /**
     * Destructor.
     */
    ~ItemType();

    /**
     * Getter for m_nTestType
     */
    ItemTestType getItemTestType() const;
    void setItemTestType(ItemTestType t);

    void setAllowNilled(bool value);
    bool getAllowNilled() const;

    QualifiedName *getName() const;
    QualifiedName *getType() const;

    void setName(QualifiedName * name);
    void setType(QualifiedName * type);

    const XMLCh* getTypeURI() const;
    const XMLCh* getNameURI() const;

    VectorOfSequenceTypes *getArgumentTypes() const { return argTypes_; }
    SequenceType *getReturnType() const { return returnType_; }

    void getStaticType(StaticType &st, const StaticContext *context,
                       bool &isExact, const LocationInfo *location) const;

    bool matches(const Item::Ptr &toBeTested, DynamicContext* context) const;
    bool matches(const Node::Ptr &toBeTested, DynamicContext* context) const;
    bool matches(const FunctionRef::Ptr &toBeTested, DynamicContext* context) const;
    bool matches(const FunctionSignature *sig, DynamicContext* context) const;
    bool matchesNameType(const Item::Ptr &toBeTested, const DynamicContext* context) const;
    bool matchesSchemaElement(const Node::Ptr &toBeTested, const DynamicContext* context) const;

    bool isSubtypeOf(const ItemType *toBeTested, const StaticContext* context) const;

    void staticResolution(StaticContext *context, const LocationInfo *location);

    void toBuffer(XERCES_CPP_NAMESPACE_QUALIFIER XMLBuffer &buffer, bool addBrackets = false) const;

  protected:
    bool isSubtypeOfNameType(const ItemType *toBeTested, const StaticContext* context) const;

    // The ItemTestType of this ItemType
    ItemTestType m_nTestType;

    // The name and type to match
    QualifiedName *m_pName, *m_pType;

    // The forced URIs for name and type
    const XMLCh *m_NameURI, *m_TypeURI;

    // allow elements having the xsi:nil="true" attribute
    bool m_bAllowNil;

    // The number of arguments the function should take
    VectorOfSequenceTypes *argTypes_;
    // The return type of the function
    SequenceType *returnType_;

    bool staticallyResolved_;
  };



  /**
   * Number of occurrences of the ItemType.
   * STAR specifies zero or more occurrences.
   * PLUS specifies one or more occurrences.
   * QUESTION_MARK specifies zero or one occurrence.
   */
  typedef enum 
  {
    EXACTLY_ONE=0,
    STAR=1,
    PLUS=2,
    QUESTION_MARK=3
  } OccurrenceIndicator;

  /**
   * Constructor for atomic types
   */
  SequenceType(const XMLCh* typeURI,const XMLCh* typeName, OccurrenceIndicator occur, XPath2MemoryManager *mm);

  /**
   * Generic constructor.
   */
  SequenceType(ItemType* test, OccurrenceIndicator occur = EXACTLY_ONE);

  /**
   * Default constructor.
   */
  SequenceType();

  /**
   * Destructor.
   */
  virtual ~SequenceType();

  /** 
   * Setter for m_pItemType.
   */ 
  void setItemType(ItemType* itemType);

  /**
   * Setter for m_nOccurrencer.
   */
  void setOccurrence(OccurrenceIndicator nOccurrence);

  //Return a URI, firstly the overriding URI string, if not, the URI bond to the QName prefix
  const XMLCh *getTypeURI() const;
  const XMLCh *getNameURI() const;

  /**
   * Returns a Result that will throw an XPath2TypeMatchException if
   * the toBeTested Result doesn't match this SequenceType.
   */
  Result matches(const Result &toBeTested, const LocationInfo *location, const XMLCh *errorCode) const;
  Result occurrenceMatches(const Result &toBeTested, const LocationInfo *location, const XMLCh *errorCode) const;
  Result typeMatches(const Result &toBeTested, const LocationInfo *location, const XMLCh *errorCode) const;

  Result convertFunctionArg(const Result &input, DynamicContext *context, bool xpath1Compat,
                            const LocationInfo *location, const XMLCh *errorCode);
  ASTNode *convertFunctionArg(ASTNode *arg, StaticContext *context, bool numericFunction,
                              const LocationInfo *location);

  QualifiedName *getConstrainingType() const;
  QualifiedName *getConstrainingName() const;

  /**
   * Getter for m_pItemType.
   */
  ItemType::ItemTestType getItemTestType() const;

  /**
   * Getter for m_nOccurrence.
   */
  OccurrenceIndicator getOccurrenceIndicator() const;

  const ItemType *getItemType() const;

  bool isSubtypeOf(const SequenceType *toBeTested, const StaticContext* context) const;

  void staticResolution(StaticContext* context);
  void getStaticType(StaticType &st, const StaticContext *context,
                     bool &isExact, const LocationInfo *location) const;

  void toBuffer(XERCES_CPP_NAMESPACE_QUALIFIER XMLBuffer &buffer) const;

protected:

  // The ItemType of this SequenceType
  ItemType* m_pItemType;
  
  // The OccurrenceIndicator of this SequenceType
  OccurrenceIndicator m_nOccurrence;

  class OccurrenceMatchesResult : public ResultImpl
  {
  public:
    OccurrenceMatchesResult(const Result &parent, const SequenceType *seqType, const LocationInfo *location, const XMLCh *errorCode);

    Item::Ptr next(DynamicContext *context);
  private:
    const SequenceType *_seqType;
    Result _parent;
    const XMLCh *_errorCode;
    bool _toDo;
  };

  class TypeMatchesResult : public ResultImpl
  {
  public:
    TypeMatchesResult(const Result &parent, const SequenceType *seqType, const LocationInfo *location, const XMLCh *errorCode);

    Item::Ptr next(DynamicContext *context);
  private:
    const SequenceType *_seqType;
    Result _parent;
    const XMLCh *_errorCode;
  };
};

#endif

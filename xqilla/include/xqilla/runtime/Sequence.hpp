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

#ifndef _SEQUENCE_HPP
#define _SEQUENCE_HPP

#include <vector>

#include <xqilla/framework/XQillaExport.hpp>
#include <xqilla/items/ATDecimalOrDerived.hpp>
#include <xqilla/framework/XPath2MemoryManager.hpp>

#include <xercesc/util/XMemory.hpp>
#include <xercesc/util/PlatformUtils.hpp>

class DynamicContext;

typedef std::vector<Item::Ptr, XQillaAllocator<Item::Ptr> > VectorOfItems;

/**
 * An eagerly evaluated result of a query execution.
 */
class XQILLA_API Sequence : public XERCES_CPP_NAMESPACE_QUALIFIER XMemory
{

public:
  typedef VectorOfItems::iterator iterator;
  typedef VectorOfItems::const_iterator const_iterator;
  typedef VectorOfItems::reverse_iterator reverse_iterator;
  typedef VectorOfItems::const_reverse_iterator const_reverse_iterator;

  // constructor that takes one Item
  Sequence(const Item::Ptr &item, XERCES_CPP_NAMESPACE_QUALIFIER MemoryManager* memMgr =
           XERCES_CPP_NAMESPACE_QUALIFIER XMLPlatformUtils::fgMemoryManager);
  // constructor that creates a empty sequence
  Sequence(XERCES_CPP_NAMESPACE_QUALIFIER MemoryManager* memMgr =
           XERCES_CPP_NAMESPACE_QUALIFIER XMLPlatformUtils::fgMemoryManager);
  /// construct and reserve space for n elements
  Sequence(size_t n, XERCES_CPP_NAMESPACE_QUALIFIER MemoryManager* memMgr =
           XERCES_CPP_NAMESPACE_QUALIFIER XMLPlatformUtils::fgMemoryManager);
  Sequence(const Sequence&, size_t start, XERCES_CPP_NAMESPACE_QUALIFIER MemoryManager* memMgr =
           XERCES_CPP_NAMESPACE_QUALIFIER XMLPlatformUtils::fgMemoryManager);
  // copy constructor
  Sequence(const Sequence&, XERCES_CPP_NAMESPACE_QUALIFIER MemoryManager* memMgr);
  // copy constructor
  Sequence(const Sequence&);
  Sequence &operator=(const Sequence &);

  ~Sequence();

  void clear();

  const Item::Ptr &first() const;
  const Item::Ptr &second() const;

  ///Begin an iterator
  Sequence::iterator begin(void);
  Sequence::const_iterator begin(void) const;

  ///Find the end of the iterator
  Sequence::iterator end(void);
  Sequence::const_iterator end(void) const;

  ///Reverse iterator methods
  Sequence::reverse_iterator rbegin(void);
  Sequence::const_reverse_iterator rbegin(void) const;

  Sequence::reverse_iterator rend(void);
  Sequence::const_reverse_iterator rend(void) const;

  ///Returns the number of nodes in the list                        
  size_t getLength(void) const;

  ///Return the indexth item in the collection 
  const Item::Ptr &item(size_t index) const;
  ///Return the indexth item in the collection - takes ATDecimalOrDerived
  const Item::Ptr &item(const ATDecimalOrDerived::Ptr &index) const;

  /// add an item to this sequence
  void addItem(const Item::Ptr &item);

  /// add an item to this sequence
  void addItemFront(const Item::Ptr &item);

  /// combine another sequence onto this sequence
  void joinSequence(const Sequence & s);

  ///Returns true if the list is empty
  bool isEmpty() const;

  /// sort into document order (only works for Sequences containing only Nodes)
  void sortIntoDocumentOrder(const DynamicContext *context);

  /// sort as strings, using the given collation
  void sortWithCollation(const Collation *collation, const DynamicContext *context);

private:
  VectorOfItems _itemList;

  friend class SequenceResult;
};

#endif

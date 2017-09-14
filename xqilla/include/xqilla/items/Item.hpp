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

#ifndef _ITEM_HPP
#define _ITEM_HPP

#include <xqilla/framework/XQillaExport.hpp>
#include <xqilla/framework/ReferenceCounted.hpp>
#include <xercesc/util/XercesDefs.hpp>

XERCES_CPP_NAMESPACE_BEGIN
class XMLBuffer;
XERCES_CPP_NAMESPACE_END

// forward declare when we can
class XPath2MemoryManager;
class DynamicContext;
class EventHandler;

class XQILLA_API Item : public ReferenceCounted
{
public:
  typedef RefCountPointer<const Item> Ptr;

  /** The "XQilla" item interface */
  static const XMLCh gXQilla[];

  virtual bool isNode() const = 0;

  virtual bool isAtomicValue() const = 0;

  virtual bool isFunction() const = 0;

  virtual const XMLCh* asString(const DynamicContext* context) const = 0;

  virtual const XMLCh* getTypeURI() const = 0;

  virtual const XMLCh* getTypeName() const = 0;

  /** Generate events for this Item to the given EventHandler */
  virtual void generateEvents(EventHandler *events, const DynamicContext *context,
                              bool preserveNS = true, bool preserveType = true) const = 0;

  /** Method for returning arbitrary interfaces from the implementations */
  virtual void *getInterface(const XMLCh *name) const = 0;

  /** Debug method to output the type of the item in SequenceType notation */
  virtual void typeToBuffer(DynamicContext *context, XERCES_CPP_NAMESPACE_QUALIFIER XMLBuffer &buffer) const = 0;

protected:
  Item() {}

private:
  // copy constructor
  Item (const Item & other);            
  // assignment operation
  Item & operator=(const Item & item) ;
};

#endif


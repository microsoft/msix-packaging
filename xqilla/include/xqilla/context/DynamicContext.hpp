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

#ifndef _DYNAMICCONTEXT_HPP
#define _DYNAMICCONTEXT_HPP

#include <xqilla/context/StaticContext.hpp>
#include <xqilla/items/ATDurationOrDerived.hpp>

class Sequence;
class Result;
class Item;
class URIResolver;
class VariableStore;
class SequenceBuilder;
class UpdateFactory;
class RegexGroupStore;
class DebugListener;
class StackFrame;

/// The execution time dynamic context interface
class XQILLA_API DynamicContext : public StaticContext
{
public:
  virtual ~DynamicContext() {};

  virtual DynamicContext *createModuleDynamicContext(const DynamicContext* moduleCtx, XERCES_CPP_NAMESPACE_QUALIFIER MemoryManager *memMgr =
                                                     XERCES_CPP_NAMESPACE_QUALIFIER XMLPlatformUtils::fgMemoryManager) const = 0;

  /** Resets the dynamic context, as if it had never been used */
  virtual void clearDynamicContext() = 0;

  /** Get the context Item */
  virtual Item::Ptr getContextItem() const = 0;
  /** Set the context item to item */
  virtual void setContextItem(const Item::Ptr &item) = 0;

  /** Get the context position */
  virtual size_t getContextPosition() const = 0;
  /** Set the context position */ 
  virtual void setContextPosition(size_t pos) = 0;

  /** Get the context size */
  virtual size_t getContextSize() const = 0;
  /** Set the context size */
  virtual void setContextSize(size_t size) = 0;
  
  /** get the variable store */
  virtual const VariableStore* getVariableStore() const = 0;
  /** set the variable store */
  virtual void setVariableStore(const VariableStore *store) = 0;
  /** get the variable store for globally scoped variables */
  virtual const VariableStore* getGlobalVariableStore() const = 0;
  /** set the variable store for globally scoped variables */
  virtual void setGlobalVariableStore(const VariableStore *store) = 0;
  /** set the value of an external global variable with the given uri/localname pair */
  virtual void setExternalVariable(const XMLCh *namespaceURI, const XMLCh *name, const Result &value) = 0;
  /** set the value of an external global variable with the given QName */
  virtual void setExternalVariable(const XMLCh *qname, const Result &value) = 0;

  /** get the regex group store */
  virtual const RegexGroupStore* getRegexGroupStore() const = 0;
  /** set the regex group store */
  virtual void setRegexGroupStore(const RegexGroupStore *store) = 0;

  /** Return the current time */
  virtual time_t getCurrentTime() const = 0;
  /** Set the current time */
  virtual void setCurrentTime(time_t newTime) = 0;

  /** Return the implicit timezone for this system.*/
  virtual ATDurationOrDerived::Ptr getImplicitTimezone() const = 0;
  /** Set the implicit timezone */
  virtual void setImplicitTimezone(const ATDurationOrDerived::Ptr &timezoneAsDuration) = 0;

  /////////////////////////////////////////
  //  XQilla context specific accessors  //
  /////////////////////////////////////////

  /**
     Register a callback object for resolving URIs. Multiple URIResolver
     objects can be registered with the DynamicContext at the same time.
     The most recently added URIResolver will be called first, and if it
     returns false, subsequent ones will be called - ending in a call to
     the fallback routines.

     If "adopt" is false, the DynamicContext will not
     adopt this object, making the user responsible for making sure that the
     URIResolver object's lifespan matches or exceeds the life of the
     DynamicContext.
  */
  virtual void registerURIResolver(URIResolver *resolver, bool adopt) = 0;
  /** Returns the default URIResolver */
  virtual URIResolver *getDefaultURIResolver() const = 0;
  /** Sets the default URIResolver */
  virtual void setDefaultURIResolver(URIResolver *resolver, bool adopt) = 0;

  /** Set the listener for debug messages */
  virtual void setDebugListener(DebugListener *listener) = 0;
  /** Gets the listener for debug messages */
  virtual DebugListener *getDebugListener() const = 0;

  /** Sets the current stack frame whilst debugging */
  virtual void setStackFrame(const StackFrame *frame) = 0;
  /** Gets the listener for debug messages */
  virtual const StackFrame *getStackFrame() const = 0;

  /** Parse an XML document from the provided InputSource */
  virtual Node::Ptr parseDocument(XERCES_CPP_NAMESPACE_QUALIFIER InputSource &srcToUse,
                                  const LocationInfo *location = 0, const QueryPathNode *projection = 0) = 0;
  /** Resolve the given uri (and baseUri) to an XML document. If the uri
      is relative, the base uri is obtained from the context. */
  virtual Sequence resolveDocument(const XMLCh* uri, const LocationInfo *location = 0,
                                   const QueryPathNode *projection = 0) = 0;
  /** Resolve the given uri (and baseUri) to a sequence of Node objects. If the uri
      is relative, the base uri is obtained from the context. */
  virtual Sequence resolveCollection(const XMLCh* uri, const LocationInfo *location = 0,
                                     const QueryPathNode *projection = 0) = 0;
  /** Resolve the default collection to a sequence of Node objects. */
  virtual Sequence resolveDefaultCollection(const QueryPathNode *projection = 0) = 0;

  /** Attempts to put the document specified to the given URI by calling the registered
      URIResolver objects. Returns whether the operation was successful. */
  virtual bool putDocument(const Node::Ptr &document, const XMLCh *uri) = 0;

  /** Create a new SequenceBuilder, which is used to turn a
      stream of events into a Sequence. */
  virtual SequenceBuilder *createSequenceBuilder() const = 0;

  /** Creates a new UpdateFactory, used for performing updates.
      Caller owns the returned object, and should delete it */
  virtual UpdateFactory *createUpdateFactory() const = 0;

  /** Test if the query should be interrupted, and throw if so. */
  virtual void testInterrupt() const = 0;
};

#endif

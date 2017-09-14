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

#ifndef _URIRESOLVER_HPP
#define _URIRESOLVER_HPP

#include <xqilla/framework/XQillaExport.hpp>
#include <xqilla/items/Node.hpp>

#include <xercesc/util/XMemory.hpp>

class DynamicContext;
class Sequence;
class QueryPathNode;

/** 
 * This is an abstract class used to resolve URIs in different ways. The user should
 * derive a concrete class from URIResolver, and use the DynamicContext::registerURIResolver()
 * method to add this resolver to the list used by XQilla.
 */
class XQILLA_API URIResolver : public XERCES_CPP_NAMESPACE_QUALIFIER XMemory
{
public:
  /** virtual destructor, does nothing */
  virtual ~URIResolver() {};      

  /**
   * Resolve the given uri (and baseUri) to a Sequence (reference parameter). If the uri
   * is relative, the base uri can be obtained from the context. If the uri is not handled
   * by this URIResolver, returns false, otherwise returns true.
   */
  virtual bool resolveDocument(Sequence &result, const XMLCh* uri, DynamicContext *context, const QueryPathNode *projection) = 0;

  /**
   * Resolve the given uri (and baseUri) to a Sequence (reference parameter). If the uri
   * is relative, the base uri can be obtained from the context. If the uri is not handled
   * by this URIResolver, returns false, otherwise returns true.
   */
  virtual bool resolveCollection(Sequence &result, const XMLCh* uri, DynamicContext *context, const QueryPathNode *projection) = 0;

  /**
   * Resolve the default collection. If it is not defined, returns false, otherwise returns true.
   */
  virtual bool resolveDefaultCollection(Sequence &result, DynamicContext *context, const QueryPathNode *projection) = 0;

  /**
   * Called with any document that has been updated, to allow the user to save the document in any manner applicable.
   * The uri parameter will be the argument to the fn:put() function if that was called with the document - otherwise it
   * will be the document URI of the document. If this URIResolver successfully handled the putDocument() request, the method
   * should return true, otherwise it should return false and subsequent URIResolver objects will be called to handle the
   * document.
   */
  virtual bool putDocument(const Node::Ptr &document, const XMLCh *uri, DynamicContext *context) = 0;
};
#endif

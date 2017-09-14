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

#ifndef XERCESURIRESOLVER_H
#define XERCESURIRESOLVER_H

#include <xqilla/framework/XQillaExport.hpp>
#include <xqilla/context/URIResolver.hpp>

#include <xercesc/util/RefHashTableOf.hpp>

XERCES_CPP_NAMESPACE_BEGIN
class DOMDocument;
XERCES_CPP_NAMESPACE_END

class XQILLA_API XercesURIResolver : public URIResolver {
public:
  XercesURIResolver(XERCES_CPP_NAMESPACE_QUALIFIER MemoryManager *mm);
  virtual ~XercesURIResolver();

  virtual bool resolveDocument(Sequence &result, const XMLCh* uri, DynamicContext* context,
                               const QueryPathNode *projection);
  virtual bool resolveCollection(Sequence &result, const XMLCh* uri, DynamicContext* context,
                                 const QueryPathNode *projection);
  virtual bool resolveDefaultCollection(Sequence &result, DynamicContext* context,
                                        const QueryPathNode *projection);
  virtual bool putDocument(const Node::Ptr &document, const XMLCh *uri, DynamicContext *context);

  void incrementDocumentRefCount(const XERCES_CPP_NAMESPACE_QUALIFIER DOMDocument* document);
  void decrementDocumentRefCount(const XERCES_CPP_NAMESPACE_QUALIFIER DOMDocument* document);

private:
  class DocRefCount {
  public:
    DocRefCount() : doc(0), ref_count(1), next(0) {}

    const XERCES_CPP_NAMESPACE_QUALIFIER DOMDocument *doc;
    unsigned int ref_count;
    DocRefCount *next;
  };

  DocRefCount *_firstDocRefCount;
  XERCES_CPP_NAMESPACE_QUALIFIER RefHashTableOf< XERCES_CPP_NAMESPACE_QUALIFIER DOMDocument > _documentMap;
#if _XERCES_VERSION >= 30000
  XERCES_CPP_NAMESPACE_QUALIFIER RefHashTableOf< XMLCh, XERCES_CPP_NAMESPACE_QUALIFIER PtrHasher > _uriMap;
#else
  XERCES_CPP_NAMESPACE_QUALIFIER RefHashTableOf< XMLCh > _uriMap;
#endif
  XERCES_CPP_NAMESPACE_QUALIFIER MemoryManager *_mm;
};

#endif

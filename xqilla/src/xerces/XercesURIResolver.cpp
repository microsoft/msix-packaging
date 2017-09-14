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

#include "../config/xqilla_config.h"

#include "XercesURIResolver.hpp"
#include "XercesNodeImpl.hpp"

#include <xqilla/context/DynamicContext.hpp>
#include <xqilla/exceptions/XMLParseException.hpp>
#include <xqilla/utils/XPath2Utils.hpp>
#include <xqilla/utils/UTF8Str.hpp>
#include <xqilla/exceptions/ASTException.hpp>
#include <xqilla/xerces/XercesConfiguration.hpp>
#include <xqilla/runtime/Sequence.hpp>

#include "../dom-api/XQillaImplementation.hpp"

#include <xercesc/util/XMLURL.hpp>
#include <xercesc/util/XMLUri.hpp>
#if _XERCES_VERSION < 30000
#include <xercesc/util/HashPtr.hpp>
#endif

#include <xercesc/dom/DOM.hpp>
#include <xercesc/framework/LocalFileFormatTarget.hpp>
#include <xercesc/util/XMLUniDefs.hpp>
#include <xercesc/framework/XMLBuffer.hpp>

XERCES_CPP_NAMESPACE_USE;
using namespace std;

XercesURIResolver::XercesURIResolver(MemoryManager *mm)
  : _firstDocRefCount(new (mm) DocRefCount()),
    _documentMap(3,false,mm),
#if _XERCES_VERSION >= 30000
    _uriMap(3,false, mm),
#else
    _uriMap(3,false, new (mm) HashPtr(), mm),
#endif
    _mm(mm)
{
}

XercesURIResolver::~XercesURIResolver()
{
  DocRefCount *drc;
  while(_firstDocRefCount != 0) {
    drc = _firstDocRefCount;
    _firstDocRefCount = _firstDocRefCount->next;
    _mm->deallocate(drc);
  }
}

bool XercesURIResolver::resolveDocument(Sequence &result, const XMLCh* uri, DynamicContext* context, const QueryPathNode *projection)
{
  Node::Ptr doc;

  // Resolve the uri against the base uri
  const XMLCh *systemId = uri;
  XMLURL urlTmp(context->getMemoryManager());
  if(urlTmp.setURL(context->getBaseURI(), uri, urlTmp)) {
    systemId = context->getMemoryManager()->getPooledString(urlTmp.getURLText());
  } else {
	systemId = context->getMemoryManager()->getPooledString(uri);
  }

  // Check in the cache
  DOMDocument *found = _documentMap.get((void*)systemId);

  // Check to see if we can locate and parse the document
  if(found == 0) {
    try {
      doc = const_cast<DocumentCache*>(context->getDocumentCache())->loadDocument(uri, context, projection);

      found = (DOMDocument*)((DOMNode*)doc->getInterface(XercesConfiguration::gXerces));

      _documentMap.put((void*)systemId, found);
      _uriMap.put((void*)found, const_cast<XMLCh*>(systemId));
    }
    catch(const XMLParseException& e) {
      XMLBuffer errMsg;
      errMsg.set(X("Error parsing resource: "));
      errMsg.append(uri);
      errMsg.append(X(". Error message: "));
      errMsg.append(e.getError());
      errMsg.append(X(" [err:FODC0002]"));
      XQThrow2(XMLParseException,X("XercesContextImpl::resolveDocument"), errMsg.getRawBuffer());
    }
  }
  else {
    doc = new XercesNodeImpl(found, (XercesURIResolver*)context->getDefaultURIResolver());
  }

  if(doc.notNull()) {
    result.addItem(doc);
    return true;
  }

  XMLBuffer errMsg;
  errMsg.set(X("Error retrieving resource: "));
  errMsg.append(uri);
  errMsg.append(X(" [err:FODC0002]"));
  XQThrow2(XMLParseException,X("XercesContextImpl::resolveDocument"), errMsg.getRawBuffer());

  return false;
}

bool XercesURIResolver::resolveCollection(Sequence &result, const XMLCh* uri, DynamicContext* context, const QueryPathNode *projection)
{
  Node::Ptr doc;

  // Resolve the uri against the base uri
  const XMLCh *systemId = uri;
  XMLURL urlTmp(context->getMemoryManager());
  if(urlTmp.setURL(context->getBaseURI(), uri, urlTmp)) {
    systemId = context->getMemoryManager()->getPooledString(urlTmp.getURLText());
  } else {
	systemId = context->getMemoryManager()->getPooledString(uri);
  }

  // Check in the cache
  DOMDocument *found = _documentMap.get((void*)systemId);

  // Check to see if we can locate and parse the document
  if(found == 0) {
    try {
      doc = const_cast<DocumentCache*>(context->getDocumentCache())->loadDocument(uri, context, projection);

      found = (DOMDocument*)((DOMNode*)doc->getInterface(XercesConfiguration::gXerces));

      _documentMap.put((void*)systemId, found);
      _uriMap.put((void*)found, const_cast<XMLCh*>(systemId));
    }
    catch(const XMLParseException& e) {
      XMLBuffer errMsg;
      errMsg.set(X("Error parsing resource: "));
      errMsg.append(uri);
      errMsg.append(X(". Error message: "));
      errMsg.append(e.getError());
      errMsg.append(X(" [err:FODC0004]"));
      XQThrow2(XMLParseException,X("XercesURIResolver::resolveCollection"), errMsg.getRawBuffer());
    }
  }
  else {
    doc = new XercesNodeImpl(found, (XercesURIResolver*)context->getDefaultURIResolver());
  }

  if(doc.notNull()) {
    result.addItem(doc);
    return true;
  }

  XMLBuffer errMsg;
  errMsg.set(X("Error retrieving resource: "));
  errMsg.append(uri);
  errMsg.append(X(" [err:FODC0004]"));
  XQThrow2(XMLParseException,X("XercesURIResolver::resolveCollection"), errMsg.getRawBuffer());

  return false;
}

bool XercesURIResolver::resolveDefaultCollection(Sequence &result, DynamicContext* context, const QueryPathNode *projection)
{
  return false;
}

static const XMLCh ls_string[] = { chLatin_L, chLatin_S, chNull };
static const XMLCh file_scheme[] = { chLatin_f, chLatin_i, chLatin_l, chLatin_e, 0 };
static const XMLCh utf8_str[] = { chLatin_u, chLatin_t, chLatin_f, chDash, chDigit_8, 0 };

#define char2hexdigit(ch) (((ch) >= 'a') ? (ch) - 'a' : (((ch) >= 'A') ? (ch) - 'A' : (ch) - '0'))

bool XercesURIResolver::putDocument(const Node::Ptr &document, const XMLCh *uri, DynamicContext *context)
{
  // Ignore nodes with no URI
  if(uri == 0) return true;

  try {
    XMLUri uri_obj(uri);

    // Check for a "file" scheme
    if(!XPath2Utils::equals(uri_obj.getScheme(), file_scheme))
      return false;

    // Check this is a Xerces data model node
    const DOMNode *domnode = (const DOMNode*)document->getInterface(XercesConfiguration::gXerces);
    if(!domnode) return false;

    // Write the document to disk
    DOMImplementation *impl = XQillaImplementation::getDOMImplementationImpl();

    const XMLCh *path = uri_obj.getPath();
    //Get rid of the leading / if it is a Windows path.
    int colonIdx = XMLString::indexOf(path, chColon);
    if(path && colonIdx == 2 && XMLString::isAlpha(path[1])){
      path++;
    }

    // Unescape the URI
    // Since URI escaping encodes UTF-8 char sequences, it's easier to do the unescaping with a UTF-8 string.
    UTF8Str path8(path);
    string unencode8;
    const char *ptr = path8.str();
    while(*ptr) {
      if(*ptr == '%') {
        if(ptr[1] == 0) throw MalformedURLException(__FILE__, __LINE__, XMLExcepts::URL_MalformedURL);
        if(ptr[2] == 0) throw MalformedURLException(__FILE__, __LINE__, XMLExcepts::URL_MalformedURL);
        unencode8.append(1, (char)(char2hexdigit(ptr[1]) * 0x10 + char2hexdigit(ptr[2])));
        ptr += 3;
      }
      else {
        unencode8.append(1, *ptr);
        ptr += 1;
      }
    }

    LocalFileFormatTarget target(X(unencode8.c_str()));

#if _XERCES_VERSION >= 30000
    AutoRelease<DOMLSSerializer> writer(impl->createLSSerializer());
    AutoRelease<DOMLSOutput> output(impl->createLSOutput());
    output->setByteStream(&target);
#else
    // Find the encoding to use
    const XMLCh *encoding = 0;
    if(domnode->getNodeType() == DOMNode::DOCUMENT_NODE) {
      // Use the document's encoding, if this is a document node
      encoding = ((DOMDocument*)domnode)->getEncoding();
    }
    if(encoding == 0 || *encoding == 0) {
      // Otherwise, just use UTF-8
      encoding = utf8_str;
    }

    AutoRelease<DOMWriter> writer(((DOMImplementationLS*)impl)->createDOMWriter());
    writer->setEncoding(encoding);
#endif

    try {
#if _XERCES_VERSION >= 30000
      if(!writer->write(domnode, output)) {
#else
      if(!writer->writeNode(&target, *domnode)) {
#endif
        XMLBuffer buf;
        buf.append(X("Writing to URI \""));
        buf.append(uri_obj.getUriText());
        buf.append(X("\" failed."));

        XQThrow2(ASTException, X("XercesURIResolver::putDocument"), buf.getRawBuffer());
      }
    }
    catch(DOMException &ex) {
      XMLBuffer buf;
      buf.append(X("Writing to URI \""));
      buf.append(uri_obj.getUriText());
      buf.append(X("\" failed: "));
      buf.append(ex.msg);

      XQThrow2(ASTException, X("XercesURIResolver::putDocument"), buf.getRawBuffer());
    }
  }
  catch(const MalformedURLException &ex) {
    XMLBuffer buf;
    buf.append(X("Unable to re-write document - bad document URI \""));
    buf.append(uri);
    buf.append(X("\""));

    XQThrow2(ASTException, X("XercesURIResolver::putDocument"), buf.getRawBuffer());
  }

  return true;
}

void XercesURIResolver::incrementDocumentRefCount(const DOMDocument* document)
{
  assert(document != 0);

  DocRefCount *found = _firstDocRefCount;
  while(found->doc != 0 && found->doc != document) {
    found = found->next;
  }

  if(found->doc == 0) {
    found->doc = document;
    found->next = new (_mm) DocRefCount();
  }
  else {
    ++found->ref_count;
  }
}

void XercesURIResolver::decrementDocumentRefCount(const DOMDocument* document)
{
  assert(document != 0);

  DocRefCount *prev = 0;
  DocRefCount *found = _firstDocRefCount;
  while(found->doc != 0 && found->doc != document) {
    prev = found;
    found = found->next;
  }

  if(found->doc != 0) {
    if(--found->ref_count == 0) {
      if(prev == 0) {
        _firstDocRefCount = found->next;
      }
      else {
        prev->next = found->next;
      }
      _mm->deallocate(found);
      XMLCh *uri = _uriMap.get((void*)document);
      if(uri != 0) {
        _uriMap.removeKey((void*)document);
        _documentMap.removeKey((void*)uri);
      }
      const_cast<DOMDocument*>(document)->release();
    }
  }
}


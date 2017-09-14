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
#include <xqilla/functions/FunctionUnparsedText.hpp>
#include <xqilla/functions/FunctionMatches.hpp>
#include <xqilla/context/DynamicContext.hpp>
#include <xqilla/exceptions/FunctionException.hpp>
#include <xqilla/ast/StaticAnalysis.hpp>
#include <xqilla/utils/XPath2Utils.hpp>
#include <xqilla/events/EventHandler.hpp>

#include <xercesc/util/XMLResourceIdentifier.hpp>
#include <xercesc/util/XMLEntityResolver.hpp>
#include <xercesc/util/XMLURL.hpp>
#include <xercesc/framework/URLInputSource.hpp>
#include <xercesc/framework/LocalFileInputSource.hpp>
#include <xercesc/util/BinInputStream.hpp>
#include <xercesc/util/Janitor.hpp>

XERCES_CPP_NAMESPACE_USE;
using namespace std;

const XMLCh FunctionUnparsedText::name[] = {
  chLatin_u, chLatin_n, chLatin_p, chLatin_a, chLatin_r, chLatin_s, chLatin_e, chLatin_d, chDash, chLatin_t, chLatin_e, chLatin_x, chLatin_t, chNull 
};
const unsigned int FunctionUnparsedText::minArgs = 1;
const unsigned int FunctionUnparsedText::maxArgs = 2;

/**
 * fn:unparsed-text($href as xs:string?) as xs:string?
 * fn:unparsed-text($href as xs:string?, $encoding as xs:string) as xs:string? 
 **/
FunctionUnparsedText::FunctionUnparsedText(const VectorOfASTNodes &args, XPath2MemoryManager* memMgr)
  : XQFunction(name, "($href as xs:string?, $encoding as xs:string) as xs:string?", args, memMgr)
{
}

ASTNode *FunctionUnparsedText::staticTypingImpl(StaticContext *context)
{
  _src.clearExceptType();
  _src.availableDocumentsUsed(true);
  calculateSRCForArguments(context);
  return this;
}

class FindXMLEncoding : private EventHandler
{
public:
  const XMLCh *start(InputSource &src, DynamicContext *context)
  {
    encoding_ = 0;
    mm_ = context->getMemoryManager();
    try {
      const_cast<DocumentCache*>(context->getDocumentCache())->parseDocument(src, this, context);
    }
    catch(...) {
    }
    return encoding_;
  }

private:
  const XMLCh *encoding_;
  XPath2MemoryManager *mm_;

  virtual void startDocumentEvent(const XMLCh *documentURI, const XMLCh *encoding)
  {
    encoding_ = mm_->getPooledString(encoding);
    throw "Done";
  }

  virtual void endDocumentEvent() {}
  virtual void namespaceEvent(const XMLCh *prefix, const XMLCh *uri) {}
  virtual void endEvent() {}
  virtual void piEvent(const XMLCh *target, const XMLCh *value) {}
  virtual void commentEvent(const XMLCh *value) {}
  virtual void atomicItemEvent(AnyAtomicType::AtomicObjectType type, const XMLCh *value, const XMLCh *typeURI,
                               const XMLCh *typeName) {}

  virtual void startElementEvent(const XMLCh *prefix, const XMLCh *uri, const XMLCh *localname){}
  virtual void endElementEvent(const XMLCh *prefix, const XMLCh *uri, const XMLCh *localname,
        const XMLCh *typeURI, const XMLCh *typeName) {}
  virtual void attributeEvent(const XMLCh *prefix, const XMLCh *uri, const XMLCh *localname, const XMLCh *value,
        const XMLCh *typeURI, const XMLCh *typeName) {}
  virtual void textEvent(const XMLCh *value) {}
  virtual void textEvent(const XMLCh *chars, unsigned int length) {}
};

Sequence FunctionUnparsedText::createSequence(DynamicContext* context, int flags) const
{
  Item::Ptr uriArg = getParamNumber(1, context)->next(context);
  
  if(uriArg.isNull()) {
    return Sequence(context->getMemoryManager());
  }
  
  const XMLCh *uri = uriArg->asString(context);

  if(!XPath2Utils::isValidURI(uri, context->getMemoryManager()))
    XQThrow(FunctionException, X("FunctionUnparsedText::createSequence"), X("The argument to fn:unparsed-text() is not a valid URI [err:XTDE1170]"));

  // TBD Implement a URIResolver method for resolving unparsed text - jpcs

  const XMLCh *baseUri = context->getBaseURI();

  InputSource *srcToUse = 0;
  if(context->getXMLEntityResolver()){
    XMLResourceIdentifier resourceIdentifier(XMLResourceIdentifier::UnKnown, uri, 0, XMLUni::fgZeroLenString, baseUri);
    srcToUse = context->getXMLEntityResolver()->resolveEntity(&resourceIdentifier);
  }

  if(srcToUse == 0) {
    try {
      // Resolve the uri against the base uri
      XMLURL urlTmp;

      if(baseUri && *baseUri) {
        urlTmp.setURL(baseUri, uri);
      }
      else {
        urlTmp.setURL(uri);
      }

      srcToUse = new URLInputSource(urlTmp);
    }
    catch(const MalformedURLException &e) {
    }
  }

  if(srcToUse == 0) {
    // It's not a URL, so let's assume it's a local file name.
    if(baseUri && *baseUri) {
      AutoDeallocate<XMLCh> tmpBuf(XMLPlatformUtils::weavePaths(baseUri, uri), XMLPlatformUtils::fgMemoryManager);
      srcToUse = new LocalFileInputSource(tmpBuf);
    }
    else {
      srcToUse = new LocalFileInputSource(uri);
    }
  }

  Janitor<InputSource> janIS(srcToUse);

  if(getNumArgs() == 2) {
    const XMLCh *encoding = getParamNumber(2, context)->next(context)->asString(context);
    srcToUse->setEncoding(encoding);
  }

  XMLBuffer result;
  try {
    BinInputStream *stream = srcToUse->makeStream();
    if(stream == NULL) {
      XMLBuffer buf;
      buf.set(X("Cannot read unparsed content from "));
      buf.append(uri);
      buf.append(X(" [err:XTDE1170]"));
      XQThrow2(FunctionException,X("FunctionUnparsedText::createSequence"), buf.getRawBuffer());
    }
    Janitor<BinInputStream> janStream(stream);

#ifdef HAVE_GETCONTENTTYPE
    if(FunctionMatches::matches(stream->getContentType(), X("(text|application)/(xml|[^ +;]+\\+xml)"), X("i"))) {
      srcToUse->setEncoding(0);
      srcToUse->setEncoding(FindXMLEncoding().start(*srcToUse, context));
    }
#endif

    XPath2Utils::readSource(stream, context->getMemoryManager(), result, srcToUse->getEncoding());
  }
  catch(XMLException &e) {
    XMLBuffer buf;
    buf.set(X("Exception reading unparsed content: "));
    buf.append(e.getMessage());
    buf.append(X(" [err:XTDE1190]"));
    XQThrow2(FunctionException,X("FunctionUnparsedText::createSequence"), buf.getRawBuffer());
  }

  return Sequence(context->getItemFactory()->createString(result.getRawBuffer(), context), context->getMemoryManager());
}

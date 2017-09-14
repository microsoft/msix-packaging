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

#ifndef XSLT2LEXER_HPP
#define XSLT2LEXER_HPP

#include "../config/xqilla_config.h"

#include "XQLexer.hpp"
#include "../schema/FaxppDocumentCacheImpl.hpp"

#include <xercesc/framework/XMLBuffer.hpp>
#include <xercesc/framework/XMLDocumentHandler.hpp>
#include <xercesc/framework/XMLEntityHandler.hpp>
#include <xercesc/framework/XMLPScanToken.hpp>

XERCES_CPP_NAMESPACE_BEGIN
class GrammarResolver;
class XMLScanner;
class Locator;
XERCES_CPP_NAMESPACE_END

struct InstructionInfo;
class DynamicContext;
class XQillaNSResolverImpl;

class XSLT2Lexer : public Lexer
{
public:
  enum EventType {
    START_DOCUMENT,
    END_DOCUMENT,
    START_ELEMENT,
    END_ELEMENT,
    NAMESPACE,
    ATTRIBUTE,      
    TEXT
  };

  XSLT2Lexer(DynamicContext *context, const XERCES_CPP_NAMESPACE_QUALIFIER InputSource &srcToUse, XQilla::Language lang = XQilla::XSLT2);
  virtual ~XSLT2Lexer();

  virtual int yylex(YYSTYPE* pYYLVAL, YYLTYPE* pYYLOC);

  virtual XERCES_CPP_NAMESPACE_QUALIFIER DOMXPathNSResolver *getNSResolver() const;
  virtual void undoLessThan();

protected:
  enum State {
    LANG_TOKEN,
    NEXT_EVENT,
    CURRENT_EVENT,
    POP_STACK
  };

  struct ElementStackEntry
  {
    ElementStackEntry(ElementStackEntry *p);

    bool reportWhitespace() const;

    const InstructionInfo *info;
    bool xmlSpacePreserve;

    XQillaNSResolverImpl *nsResolver;

    ElementStackEntry *prev;
  };

  int lang_token_state(YYSTYPE* pYYLVAL, YYLTYPE* pYYLOC);
  int attrs_state(YYSTYPE* pYYLVAL, YYLTYPE* pYYLOC);
  int next_event_state(YYSTYPE* pYYLVAL, YYLTYPE* pYYLOC);
  int current_event_state(YYSTYPE* pYYLVAL, YYLTYPE* pYYLOC);

  ASTNode *wrap(ASTNode *result) const;
  ASTNode *makeDirectName(const XMLCh *prefix, const XMLCh *name) const;
  ASTNode *makeQNameLiteral(const XMLCh *uri, const XMLCh *prefix, const XMLCh *name) const;
  ASTNode *makeStringLiteral(const XMLCh *value) const;

  void pushElementStack();
  void popElementStack();
  void setNamespace(const XMLCh *prefix, const XMLCh *uri);

  virtual void nextEvent(YYLTYPE* pYYLOC) = 0;
  virtual EventType getEventType() = 0;
  virtual void getEventName(const XMLCh *&prefix, const XMLCh *&uri, const XMLCh *&localname) = 0;
  virtual void getEventValue(const XMLCh *&value, unsigned int &length, std::vector<XQLexer::ValueOffset> &offsets) = 0;

  virtual void getEventLocation(YYLTYPE* pYYLOC) = 0;
  virtual void getValueLocation(YYLTYPE* pYYLOC) = 0;

  DynamicContext *context_;

  AutoDelete<Lexer> childLexer_;
  std::vector<XQLexer::ValueOffset> offsets_;

  ElementStackEntry *elementStack_;

  State state_;

  XERCES_CPP_NAMESPACE_QUALIFIER XMLBuffer textBuffer_;
  bool textToCreate_;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

class XercesXSLT2Lexer : public XSLT2Lexer,
                         private XERCES_CPP_NAMESPACE_QUALIFIER XMLDocumentHandler,
                         private XERCES_CPP_NAMESPACE_QUALIFIER XMLEntityHandler
{
public:
  XercesXSLT2Lexer(DynamicContext *context, const XERCES_CPP_NAMESPACE_QUALIFIER InputSource &srcToUse, XQilla::Language lang = XQilla::XSLT2);
  virtual ~XercesXSLT2Lexer();

  struct CachedEvent {
    CachedEvent(EventType t, const XERCES_CPP_NAMESPACE_QUALIFIER Locator *locator, CachedEvent *pv);
    CachedEvent(const XMLCh *p, const XMLCh *u, const XMLCh *l, const XERCES_CPP_NAMESPACE_QUALIFIER Locator *locator, CachedEvent *pv);
    CachedEvent(EventType t, const XMLCh *p, const XMLCh *u, const XMLCh *l, const XMLCh *v, const XERCES_CPP_NAMESPACE_QUALIFIER Locator *locator, CachedEvent *pv);
    CachedEvent(const XMLCh *value, size_t length, const XERCES_CPP_NAMESPACE_QUALIFIER Locator *locator, CachedEvent *pv);

    EventType type;

    const XMLCh *prefix, *uri, *localname;
    XERCES_CPP_NAMESPACE_QUALIFIER XMLBuffer value;

    XercesFileLoc line, column;

    CachedEvent *prev, *next;
  };

protected:
  // From XSLT2Lexer
  virtual void nextEvent(YYLTYPE* pYYLOC);
  virtual EventType getEventType();
  virtual void getEventName(const XMLCh *&prefix, const XMLCh *&uri, const XMLCh *&localname);
  virtual void getEventValue(const XMLCh *&value, unsigned int &length, std::vector<XQLexer::ValueOffset> &offsets);

  virtual void getEventLocation(YYLTYPE* pYYLOC);
  virtual void getValueLocation(YYLTYPE* pYYLOC);

  // From XMLEntityHandler
  virtual void startInputSource(const XERCES_CPP_NAMESPACE_QUALIFIER InputSource& inputSource) {}
  virtual void endInputSource(const XERCES_CPP_NAMESPACE_QUALIFIER InputSource& inputSource) {}
  virtual bool expandSystemId(const XMLCh* const systemId, XERCES_CPP_NAMESPACE_QUALIFIER XMLBuffer &toFill) { return false; }
  virtual void resetEntities() {}
  virtual XERCES_CPP_NAMESPACE_QUALIFIER InputSource* resolveEntity(const XMLCh* const publicId, const XMLCh* const systemId, const XMLCh* const baseURI = 0) { return 0; }
  virtual XERCES_CPP_NAMESPACE_QUALIFIER InputSource* resolveEntity(XERCES_CPP_NAMESPACE_QUALIFIER XMLResourceIdentifier* resourceIdentifier);

  // From XMLDocumentHandler
  virtual void startDocument();
  virtual void endDocument();
  virtual void resetDocument() {}
#if _XERCES_VERSION >= 30000
  virtual void startElement(const XERCES_CPP_NAMESPACE_QUALIFIER XMLElementDecl& elemDecl, const unsigned int urlId,
                            const XMLCh* const elemPrefix,
                            const XERCES_CPP_NAMESPACE_QUALIFIER RefVectorOf<XERCES_CPP_NAMESPACE_QUALIFIER XMLAttr>& attrList,
                            const XMLSize_t attrCount, const bool isEmpty, const bool isRoot);
  virtual void docCharacters(const XMLCh* const chars, const XMLSize_t length, const bool cdataSection);
  virtual void ignorableWhitespace(const XMLCh* const chars, const XMLSize_t length, const bool cdataSection) {}
#else
  virtual void startElement(const XERCES_CPP_NAMESPACE_QUALIFIER XMLElementDecl& elemDecl, const unsigned int urlId,
                            const XMLCh* const elemPrefix,
                            const XERCES_CPP_NAMESPACE_QUALIFIER RefVectorOf<XERCES_CPP_NAMESPACE_QUALIFIER XMLAttr>& attrList,
                            const unsigned int attrCount, const bool isEmpty, const bool isRoot);
  virtual void docCharacters(const XMLCh* const chars, const unsigned int length, const bool cdataSection);
  virtual void ignorableWhitespace(const XMLCh* const chars, const unsigned int length, const bool cdataSection) {}
#endif
  virtual void endElement(const XERCES_CPP_NAMESPACE_QUALIFIER XMLElementDecl& elemDecl, const unsigned int urlId,
                          const bool isRoot, const XMLCh* const elemPrefix=0);
  virtual void docComment(const XMLCh* const comment) {}
  virtual void docPI(const XMLCh* const target, const XMLCh* const data) {}
  virtual void startEntityReference(const XERCES_CPP_NAMESPACE_QUALIFIER XMLEntityDecl& entDecl) {}
  virtual void endEntityReference(const XERCES_CPP_NAMESPACE_QUALIFIER XMLEntityDecl& entDecl) {}
  virtual void XMLDecl(const XMLCh* const versionStr, const XMLCh* const encodingStr, const XMLCh* const standaloneStr,
                       const XMLCh* const actualEncodingStr) {}

  XERCES_CPP_NAMESPACE_QUALIFIER GrammarResolver *grammarResolver_;
  XERCES_CPP_NAMESPACE_QUALIFIER XMLScanner *scanner_;
  XERCES_CPP_NAMESPACE_QUALIFIER XMLPScanToken pptoken_;
  CachedEvent *currentEvent_, *firstEvent_, *lastEvent_;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef HAVE_FAXPP

extern "C" {
#include <faxpp/parser.h>
}

class FAXPPXSLT2Lexer : public XSLT2Lexer
{
public:
  FAXPPXSLT2Lexer(DynamicContext *context, const XERCES_CPP_NAMESPACE_QUALIFIER InputSource &srcToUse, XQilla::Language lang = XQilla::XSLT2);
  virtual ~FAXPPXSLT2Lexer();

protected:
  virtual void nextEvent(YYLTYPE* pYYLOC);
  virtual EventType getEventType();
  virtual void getEventName(const XMLCh *&prefix, const XMLCh *&uri, const XMLCh *&localname);
  virtual void getEventValue(const XMLCh *&value, unsigned int &length, std::vector<XQLexer::ValueOffset> &offsets);

  virtual void getEventLocation(YYLTYPE* pYYLOC);
  virtual void getValueLocation(YYLTYPE* pYYLOC);

  bool nextNamespace();
  bool nextAttribute();

  void setLocation(YYLTYPE* pYYLOC, const FAXPP_Event *event);
  void setLocation(YYLTYPE* pYYLOC, const FAXPP_Attribute *attr);
  void setLocation(YYLTYPE* pYYLOC, const FAXPP_AttrValue *attrval);
  void setErrorLocation(YYLTYPE* pYYLOC);

  FaxppParserWrapper wrapper_;
  EventType eventType_;
  unsigned int attrIndex_;
  XERCES_CPP_NAMESPACE_QUALIFIER XMLBuffer value_;
};

#endif

#endif

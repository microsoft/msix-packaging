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

#ifndef XQLEXER_H
#define XQLEXER_H

// #define XQ_DEBUG_LEXER 1

#define YY_CHAR unsigned short

#include <xercesc/util/XMLUniDefs.hpp>
#include <xercesc/util/XMLChar.hpp>

#include <xqilla/framework/XQillaExport.hpp>
#include <xqilla/simple-api/XQilla.hpp>

typedef struct yyltype
{
  int first_line;
  int first_column;

  int last_line;
  int last_column;

  int first_offset;
  int last_offset;
} yyltype;
#define YYLTYPE yyltype
#define YYLTYPE_IS_DECLARED 1
#define YYLTYPE_IS_TRIVIAL 1

#define YY_USER_ACTION  userAction(yytext, yyleng);

#define YYLLOC_DEFAULT(Current, Rhs, N)         \
  do                  \
    if (N)                \
    {                \
      (Current).first_line   = (Rhs)[1].first_line;  \
      (Current).first_column = (Rhs)[1].first_column;  \
      (Current).last_line    = (Rhs)[N].last_line;    \
      (Current).last_column  = (Rhs)[N].last_column;  \
      (Current).first_offset = (Rhs)[1].first_offset;  \
      (Current).last_offset  = (Rhs)[N].last_offset;   \
    }                \
    else                \
    {                \
      (Current).first_line   = (Current).last_line   =    \
        (Rhs)[0].last_line;        \
      (Current).first_column = (Current).last_column =    \
        (Rhs)[0].last_column;        \
      (Current).first_offset = (Current).last_offset =    \
        (Rhs)[0].last_offset;        \
    }                \
  while (0)

#ifndef yyFlexLexer
#define yyFlexLexer xqFlexLexer
#include "FlexLexer.h"
#endif

#include <xercesc/util/BitSet.hpp>
#include <xercesc/util/RefHashTableOf.hpp>
#include <xqilla/simple-api/XQQuery.hpp>
#include <xqilla/context/DynamicContext.hpp>

#ifndef YYSTYPE
#include <xqilla/ast/ASTNode.hpp>
#include <xqilla/ast/XQSequence.hpp>
#include <xqilla/ast/TupleNode.hpp>
#include <xqilla/ast/OrderByTuple.hpp>
#include <xqilla/ast/XQNav.hpp>
#include <xqilla/ast/XQCallTemplate.hpp>
#include <xqilla/schema/SequenceType.hpp>
#include <xqilla/parser/QName.hpp>
#include <xqilla/functions/XQUserFunction.hpp>
#include <xqilla/functions/FunctionSignature.hpp>
#include <xqilla/ast/XQTypeswitch.hpp>
#include <xqilla/ast/XQPredicate.hpp>
#include <xqilla/fulltext/FTSelection.hpp>
#include <xqilla/fulltext/FTWords.hpp>
#include <xqilla/fulltext/FTOption.hpp>
#include <xqilla/fulltext/FTRange.hpp>
#include <xqilla/update/UTransform.hpp>

class LetTuple;
class XQGlobalVariable;

typedef union {
  XMLCh* str;
  ASTNode* astNode;
  XQSequence *parenExpr;
  XQUserFunction* functDecl;
  ArgumentSpec* argSpec;
  ArgumentSpecs* argSpecs;
  FunctionSignature *signature;
  XQUserFunction::Mode* mode;
  XQUserFunction::ModeList* modeList;
  XQGlobalVariable *globalVar;
  NodeTest *nodeTest;
  XQStep::Axis axis;
  QualifiedName *qName;
  SequenceType* sequenceType;
  SequenceType::OccurrenceIndicator occurrence;
  SequenceType::ItemType* itemType;
  VectorOfSequenceTypes *sequenceTypes;
  VectorOfASTNodes* itemList;
  VectorOfPredicates* predicates;
  TupleNode *tupleNode;
  LetTuple *letTuple;
  CopyBinding* copyBinding;
  VectorOfCopyBinding* copyBindingList;
  XQTemplateArgument *templateArg;
  TemplateArguments *templateArgs;
  XQTypeswitch::Case *caseClause;
  XQTypeswitch::Cases *caseClauses;
  OrderByTuple::Modifiers orderByModifier;
  VectorOfStrings* stringList;
  FTSelection *ftselection;
  FTWords::FTAnyallOption ftanyalloption;
  FTOption *ftoption;
  VectorOfFTOptions *ftoptionlist;
  FTRange ftrange;
  FTOption::FTUnit ftunit;
  bool boolean;
  int integer;
} yystype;
#define YYSTYPE yystype
#define YYSTYPE_IS_DECLARED 1
#define YYSTYPE_IS_TRIVIAL 1

#endif  // #ifndef YYSTYPE

#include "../parser/XQParser.hpp"  // to be included *after* defining YYSTYPE and YYLTYPE

class XQILLA_API Lexer
{
public:
  virtual ~Lexer() {}

  virtual int yylex(YYSTYPE* pYYLVAL, YYLTYPE* pYYLOC) = 0;

  int error(const char* message)
  {
    Error(message, m_lineno, m_columnno);
    return 0;
  }

  int error(const yyltype &pos, const char* message)
  {
    Error(message, pos.first_line, pos.first_column);
    return 0;
  }

  int error(XQilla::Language lang, const char* where, unsigned int line, unsigned int column)
  {
    Error(lang, where, line, column);
    return 0;
  }

  XQilla::Language getLanguage() const { return m_language; }
  bool isXQuery() const { return (m_language & (XQilla::XPATH2 | XQilla::XSLT2)) == 0; }
  bool isXPath() const { return (m_language & XQilla::XPATH2) != 0; }
  bool isXSLT() const { return (m_language & XQilla::XSLT2) != 0; }
  bool isFullText() const { return (m_language & XQilla::FULLTEXT) != 0; }
  bool isUpdate() const { return (m_language & XQilla::UPDATE) != 0; }
  bool isVersion3() const { return (m_language & XQilla::VERSION3) != 0; }
  bool isExtensions() const { return (m_language & XQilla::EXTENSIONS) != 0; }

  void setVersion3(bool value)
  {
    m_language = (XQilla::Language)((m_language & ~(XQilla::VERSION3)) |
                                    (value ? XQilla::VERSION3 : 0));
  }

  void setGenerateErrorException(bool bEnable) { m_bGenerateErrorException=bEnable; }

  virtual void undoLessThan() = 0;
  virtual XERCES_CPP_NAMESPACE_QUALIFIER DOMXPathNSResolver *getNSResolver() const { return 0; }

  const XMLCh *getFile() const { return m_szQueryFile; }
  XPath2MemoryManager *getMemoryManager() const { return mm_; }

protected:
  Lexer(XPath2MemoryManager *mm, XQilla::Language lang, const XMLCh *queryFile, int line, int column)
    : mm_(mm),
      m_language(lang),
      m_szQueryFile(queryFile),
      m_lineno(line),
      m_columnno(column),
      m_bGenerateErrorException(true)
  {
  }

  void Error(const char* msg, int line, int col);
  void Error(XQilla::Language lang, const char *where, unsigned int line, unsigned int col);

  XPath2MemoryManager *mm_;
  XQilla::Language m_language;

  const XMLCh* m_szQueryFile;
  int m_lineno,m_columnno;

  bool m_bGenerateErrorException;
};

class XQILLA_API XQLexer : public Lexer, public yyFlexLexer
{
public:
  enum StartMode {
    MODE_NORMAL,
    MODE_ATTR_VALUE_TEMPLATE,
    MODE_TEMPLATE_MODES,
    MODE_OUTPUT_METHOD,
    MODE_QNAMES
  };

  struct ValueOffset {
    ValueOffset(int i, int l, int c) : index(i), lineOffset(l), columnOffset(c) {}
    int index, lineOffset, columnOffset;
  };

  XQLexer(XPath2MemoryManager* memMgr, const XMLCh *queryFile, const XMLCh* query, XQilla::Language lang);
  XQLexer(XPath2MemoryManager* memMgr, int firstToken, const XMLCh *queryFile, int line, int column, const XMLCh *query);
  XQLexer(XPath2MemoryManager* memMgr, const XMLCh *queryFile, int line, int column, const XMLCh* query,
          unsigned int length, const std::vector<ValueOffset> &offsets, XQilla::Language lang, StartMode mode = MODE_NORMAL);
  ~XQLexer();

  // Implemented in XQLexer.cpp, output of XQLexer.l
  int yylex();
  virtual int yylex(YYSTYPE* pYYLVAL, YYLTYPE* pYYLOC);

  const XMLCh* getQueryString() { return m_szQuery; }

  virtual void undoLessThan();

protected:
  // For look ahead
  XQLexer(const XQLexer *other);

  virtual int LexerInput(YY_CHAR* buf, int max_size);
  virtual void LexerOutput(const YY_CHAR* buf, int size);
  virtual void LexerError(const char* msg);
  virtual void yy_pop_state();

  void userAction(YY_CHAR* text, int length);
  void undoUserAction();

  int lookAhead();
    
  void beginComment();
  void endComment();
  bool isCommentClosed();

  XMLCh *allocate_string(const XMLCh* src);
  XMLCh *allocate_string(const XMLCh* src, int len);
  XMLCh *allocate_string_and_unescape(XMLCh toBeUnescaped, const XMLCh* src, int len);
  XMLCh *allocate_string_and_unescape(XMLCh *src, int len, XMLCh quoteChar, bool unescapeBrace,
                                      bool unescapeEntities, bool unescapeCDATA);

  YYSTYPE yylval;
  YYLTYPE yyloc;

  int firstToken_;
  StartMode mode_;

  const XMLCh* m_szQuery;
  bool m_ownQuery;
  unsigned int m_nLength;
  std::vector<ValueOffset> m_offsets;

  int m_position,m_index;
  unsigned int m_currentOffset;

  int m_nOpenComments;
};

class XQILLA_API XQParserArgs
{
public:
  XQParserArgs(Lexer *lexer, XQQuery *query)
    : _lexer(lexer),
      _context((DynamicContext*)query->getStaticContext()),
      _query(query),
      _function(0),
      _moduleName(0),
      _flags(32),
      _namespaceDecls(13)
  {
  }

  XQParserArgs(Lexer *lexer)
    : _lexer(lexer),
      _context(0),
      _query(0),
      _function(0),
      _moduleName(0),
      _flags(32),
      _namespaceDecls(13)
  {
  }

  Lexer* _lexer;
  DynamicContext* _context;
  XQQuery* _query;
  XQUserFunction *_function;
  FunctionSignature *_signature;
  const XMLCh *_moduleName;
  XERCES_CPP_NAMESPACE_QUALIFIER BitSet _flags;
  XERCES_CPP_NAMESPACE_QUALIFIER RefHashTableOf<XMLCh> _namespaceDecls;
};

namespace XQParser {
  extern XQILLA_API int yyparse(void *);
  extern XQILLA_API int yydebug;
}


#endif


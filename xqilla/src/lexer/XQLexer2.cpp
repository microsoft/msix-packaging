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

#include "XQLexer.hpp"

#include "../config/xqilla_config.h"
#include <xqilla/utils/XPath2Utils.hpp>
#include <xqilla/framework/XPath2MemoryManager.hpp>
#include <xqilla/exceptions/XQException.hpp>

XERCES_CPP_NAMESPACE_USE;

XQLexer::XQLexer(XPath2MemoryManager* memMgr, const XMLCh *queryFile, const XMLCh* query, XQilla::Language lang)
  : Lexer(memMgr, lang, queryFile, 1, 1),
    firstToken_(_LANG_XQUERY_),
    mode_(MODE_NORMAL),
    m_szQuery(XPath2Utils::normalizeEOL(query, memMgr)),
    m_ownQuery(true),
    m_nLength(XPath2Utils::uintStrlen(m_szQuery)),
    m_position(0),
    m_index(0),
    m_currentOffset(0),
    m_nOpenComments(0)
{
  if((m_language & XQilla::XPATH2) != 0) {
    firstToken_ = _LANG_XPATH2_;
  } else {
    firstToken_ = _LANG_XQUERY_;
  }
}

XQLexer::XQLexer(XPath2MemoryManager* memMgr, int firstToken, const XMLCh *queryFile, int line, int column, const XMLCh *query)
  : Lexer(memMgr, (XQilla::Language)(XQilla::XQUERY3_FULLTEXT_UPDATE | XQilla::EXTENSIONS), queryFile, line, column),
    firstToken_(firstToken),
    mode_(MODE_NORMAL),
    m_szQuery(XPath2Utils::normalizeEOL(query, memMgr)),
    m_ownQuery(true),
    m_nLength(XPath2Utils::uintStrlen(m_szQuery)),
    m_position(0),
    m_index(0),
    m_currentOffset(0),
    m_nOpenComments(0)
{
}

XQLexer::XQLexer(XPath2MemoryManager* memMgr, const XMLCh *queryFile, int line, int column, const XMLCh* query,
                 unsigned int length, const std::vector<ValueOffset> &offsets, XQilla::Language lang, StartMode mode)
  : Lexer(memMgr, lang, queryFile, line, column),
    firstToken_(MYEOF),
    mode_(mode),
    m_szQuery(query),
    m_ownQuery(false),
    m_nLength(length),
    m_offsets(offsets),
    m_position(0),
    m_index(0),
    m_currentOffset(0),
    m_nOpenComments(0)
{
}

XQLexer::XQLexer(const XQLexer *other)
  : Lexer(other->mm_, other->m_language, other->m_szQueryFile, 1, 1),
    firstToken_(MYEOF),
    mode_(MODE_NORMAL),
    m_szQuery(other->m_szQuery + other->m_index),
    m_ownQuery(false),
    m_nLength(other->m_nLength - other->m_index),
    m_offsets(other->m_offsets),
    m_position(0),
    m_index(0),
    m_currentOffset(other->m_currentOffset),
    m_nOpenComments(0)
{
  m_bGenerateErrorException = false;
  yy_start = other->yy_start;
}

XQLexer::~XQLexer()
{
  if(m_szQuery && m_ownQuery)
    mm_->deallocate((void*)m_szQuery);
}

int XQLexer::yylex(YYSTYPE* pYYLVAL, YYLTYPE* pYYLOC)
{
  int nextToken=yylex();
  memcpy(pYYLVAL,&yylval,sizeof(YYSTYPE));
  memcpy(pYYLOC,&yyloc,sizeof(YYLTYPE));
  if(nextToken==0 && !isCommentClosed())
    LexerError("Unterminated comment");
  return nextToken;
}

void XQLexer::beginComment()
{
  m_nOpenComments++;
}
void XQLexer::endComment()
{
  if(m_nOpenComments==0)
    {
      error("Unbalanced comment found");
      return;
    }
  m_nOpenComments--;
}
bool XQLexer::isCommentClosed()
{
  return m_nOpenComments==0;
}

XMLCh* XQLexer::allocate_string(const XMLCh* src)
{
  return (XMLCh*)mm_->getPooledString(src);
}

XMLCh* XQLexer::allocate_string(const XMLCh* src, int len)
{
  return (XMLCh*)XPath2Utils::subString(src,0,len,mm_);
}

XMLCh *XQLexer::allocate_string_and_unescape(XMLCh *src, int len, XMLCh quoteChar, bool unescapeBrace,
                                             bool unescapeEntities, bool unescapeCDATA)
{
  // The input to this method has already been validated by the lexer,
  // so we don't need to do error checking

  *(src + len) = 0; // Null terminate the source

  XMLCh *result = (XMLCh*)mm_->allocate((len + 1) * sizeof(XMLCh));
  XMLCh *dst = result;

  for(; *src; ++src) {
    switch(*src) {
    case '&':
      if(!unescapeEntities) {
        *dst = *src;
        ++dst;
        break;
      }

      ++src;
      switch(*src) {
      case 'a':
        ++src;
        if(*src == 'm') {
          // &amp;
          *dst = '&';
          ++dst;
          src += 2;
        }
        else {
          // &apos;
          *dst = '\'';
          ++dst;
          src += 3;
        }
        break;
      case 'g':
        // &gt;
        *dst = '>';
        ++dst;
        src += 2;
        break;
      case 'l':
        // &lt;
        *dst = '<';
        ++dst;
        src += 2;
        break;
      case 'q':
        // &quot;
        *dst = '"';
        ++dst;
        src += 4;
        break;
      case '#': {
        // character reference
        unsigned int value = 0;
        unsigned int radix = 10;
        ++src;
        if(*src == 'x') {
          ++src;
          radix=16;
        }

        for(; *src != ';'; ++src) {
          if(*src >= 'a') value = (value * radix) + (*src - 'a' + 10);
          else if(*src >= 'A') value = (value * radix) + (*src - 'A' + 10);
          else value = (value * radix) + (*src - '0');
        }

        if(!XMLChar1_0::isXMLChar(value))
          error("Entity reference is not a valid XML character [err:XQST0090]");

        if(value <= 0xFFFD)
          *dst = value;
        else if (value >= 0x10000 && value <= 0x10FFFF) {
          value -= 0x10000;
          *dst = (value >> 10) + 0xD800;
          ++dst;
          *dst = (value & 0x3FF) + 0xDC00;
        }
        ++dst;
        break;
      }
      default:
        error("Should never happen");
        break;
      }
      break;
    case '\n':
    case '\r':
    case '\t':
      if(quoteChar != 0 && unescapeBrace) {
        // Normalize whitespace in attribute contents
        *dst = chSpace;
      }
      else {
        *dst = *src;
      }
      ++dst;
      break;
    case '<': {
      if(!unescapeCDATA) {
        *dst = *src;
        ++dst;
        break;
      }

      src += 9; // Length of "<![CDATA["

      // Eat the CDATA section
      int endSeen = 0;
      for(;; ++src) {
        if(*src == ']') {
          endSeen += 1;
        }
        else if(endSeen >= 2 && *src == '>') {
          // Don't include the terminator "]]>"
          dst -= 2;
          break;
        }
        else {
          endSeen = 0;
        }
        *dst = *src;
        ++dst;
      }
      break;
    }
    case '\'':
    case '"':
      *dst = *src;
      ++dst;
      if(*src == quoteChar && *(src + 1) == quoteChar)
        ++src;
      break;
    case '{':
    case '}':
      *dst = *src;
      ++dst;
      if(unescapeBrace && *(src + 1) == *src)
        ++src;
      break;
    default:
      *dst = *src;
      ++dst;
      break;
    }
  }

  *dst = 0;
  return result;
}

int XQLexer::LexerInput( YY_CHAR* buf, int max_size )
{
  int nToRead=m_nLength-m_position;
  if(nToRead==0)
    return 0;

  if(nToRead>max_size)
    nToRead=max_size;
  memcpy(buf, m_szQuery + m_position, nToRead * sizeof(XMLCh));
  m_position+=nToRead;
  return nToRead;
}

void XQLexer::LexerOutput( const YY_CHAR* buf, int size )
{
  // do nothing
}

void XQLexer::LexerError( const char* msg )
{
  error(msg);
}

void Lexer::Error( const char* msg, int line, int col )
{
  if(!m_bGenerateErrorException)
    return;
  if(strstr(msg, "[err:")!=NULL)
    XQSimpleThrow(X(msg), m_szQueryFile, line, col);
  const XMLCh* szMsg=XPath2Utils::concatStrings(X(msg), X(" [err:XPST0003]"), mm_);
  XQSimpleThrow(szMsg, m_szQueryFile, line, col);
}

void Lexer::Error(XQilla::Language lang, const char *where, unsigned int line, unsigned int col)
{
  if(!m_bGenerateErrorException)
    return;

  XMLBuffer buf;
  buf.append(X("Invalid "));

  if((lang & XQilla::XPATH2) != 0) {
    buf.append(X("XPath 2.0"));
  } else {
    buf.append(X("XQuery"));
  }
  if((lang & XQilla::FULLTEXT) != 0) {
    buf.append(X(" Full-Text"));
  }
  if((lang & XQilla::UPDATE) != 0) {
    buf.append(X(" Update"));
  }
  if((lang & XQilla::EXTENSIONS) != 0) {
    buf.append(X(" with extensions"));
  }

  buf.append(X(" syntax: "));
  buf.append(X(where));
  buf.append(X(" [err:XPST0003]"));

  XQSimpleThrow(buf.getRawBuffer(), m_szQueryFile, line, col);
}

void XQLexer::userAction(YY_CHAR* text, int length)
{
  yyloc.first_line = m_lineno;
  yyloc.first_column = m_columnno;
  yyloc.first_offset = m_index;

  m_index += length;
  for(int i = 0; i < length; ++i) {
    if(text[i] == '\n') {
      ++m_lineno;
      m_columnno = 1;
    }
    else if(text[i] != '\r')
      ++m_columnno;
  }

  // Make adjustments for the offsets we've been given
  while(m_currentOffset < m_offsets.size()) {
    ValueOffset &off = m_offsets[m_currentOffset];
    if(off.index >= m_index) break;

    m_lineno += off.lineOffset;
    m_columnno += off.columnOffset;
    ++m_currentOffset;
  }

  yyloc.last_line = m_lineno;
  yyloc.last_column = m_columnno;
  yyloc.last_offset = m_index - 1;
}

void XQLexer::undoUserAction()
{
  // undo the changes done in the user action, or there will be a hole in the (first,last) sequence
  m_lineno=yyloc.last_line=yyloc.first_line;
  m_columnno=yyloc.last_column=yyloc.first_column;
  m_index=yyloc.last_offset=yyloc.first_offset;
}

void XQLexer::yy_pop_state()
{
  if(yy_start_stack_ptr == 0) {
    char szMsg[256], text[256];
    XMLString::copyString(szMsg, "Unbalanced '");
    XMLString::transcode((XMLCh*)yytext, text, 256);
    XMLString::catString(szMsg, text);
    XMLString::catString(szMsg, "' token");
    LexerError(szMsg);
  }
  else
    yyFlexLexer::yy_pop_state();
}

int XQLexer::lookAhead()
{
  return XQLexer(this).yylex();
}


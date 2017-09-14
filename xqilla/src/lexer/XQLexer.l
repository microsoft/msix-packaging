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

%{

#include <cstdio>
#include <sstream>
#include <xqilla/utils/UTF8Str.hpp>
#include "XQLexer.hpp"
#include "../parser/XQParser.hpp"

#ifdef XQ_DEBUG_LEXER

#include <iostream>

#define CHANGE_STATE(state) BEGIN((state)); std::cerr << "Begin: " << xqGetStateName((state)) << ", "
#define RECOGNIZE(token) { std::cerr << "Recognize: "#token << std::endl; return token; }
#define RECOGNIZE_VALUE(token, value) { \
  yylval.str = allocate_string((XMLCh*)(value)); \
  std::cerr << "Recognize: \"" << UTF8(yylval.str) << "\" ("#token << ")" << std::endl; \
  return token; \
}
#define RECOGNIZE_VALUE_LEN(token, value, length) { \
  yylval.str = allocate_string((XMLCh*)(value), (length)); \
  std::cerr << "Recognize: \"" << UTF8(yylval.str) << "\" ("#token << ")" << std::endl; \
  return token; \
}
#define RECOGNIZE_VALUE_UNESCAPE(token, value, length, quot_char, unescape_brace, unescape_entities, unescape_cdata) { \
  yylval.str = allocate_string_and_unescape((XMLCh*)(value), (length), (quot_char), (unescape_brace), (unescape_entities), (unescape_cdata)); \
  std::cerr << "Recognize: \"" << UTF8(yylval.str) << "\" ("#token << ")" << std::endl; \
  return token; \
}
// yy_push_state pushes the current state and does a transition to the specified one;
#define PUSH_AND_CHANGE_STATE(state) std::cerr << "Push: " << xqGetStateName(YYSTATE) << ", "; yy_push_state((state)); std::cerr << "Begin: " << xqGetStateName((state)) << ", "
#define POP() yy_pop_state(); std::cerr << "Pop: " << xqGetStateName(YYSTATE) << ", "

#else

#define CHANGE_STATE(state) BEGIN((state))
#define RECOGNIZE(token) return token
#define RECOGNIZE_VALUE(token, value) yylval.str = allocate_string((XMLCh*)(value)); return token
#define RECOGNIZE_VALUE_LEN(token, value, length) yylval.str = allocate_string((XMLCh*)(value), (length)); return token
#define RECOGNIZE_VALUE_UNESCAPE(token, value, length, quot_char, unescape_brace, unescape_entities, unescape_cdata) \
  yylval.str = allocate_string_and_unescape((XMLCh*)(value), (length), (quot_char), (unescape_brace), (unescape_entities), (unescape_cdata)); return token
// yy_push_state pushes the current state and does a transition to the specified one;
#define PUSH_AND_CHANGE_STATE(state) yy_push_state((state))
#define POP() yy_pop_state()

#endif

%}

/* UTF16 Surrogates, for chars above 0xFFFF */
HighSurrogates ([\xD800-\xDB7F])
HighPrivateUseSurrogates ([\xDB80-\xDBFF])
LowSurrogates ([\xDC00-\xDFFF])
SurrogatePairs {HighSurrogates}|{HighPrivateUseSurrogates}|{LowSurrogates}
NonPrivateUseSurrogates {HighSurrogates}|{LowSurrogates}

Digits [0-9]+

Char ([\x0009]|[\x000D]|[\x000A]|[\x0020-\xD7FF]|[\xE000-\xFFFD]|{SurrogatePairs})

WhitespaceChar ([\x0009]|[\x000D]|[\x000A]|[\x0020]) 

Letter {BaseChar}|{Ideographic}

BaseChar1 ([\x0041-\x005A]|[\x0061-\x007A]|[\x00C0-\x00D6]|[\x00D8-\x00F6]|[\x00F8-\x00FF]|[\x0100-\x0131]|[\x0134-\x013E]|[\x0141-\x0148]|[\x014A-\x017E]|[\x0180-\x01C3]|[\x01CD-\x01F0]|[\x01F4-\x01F5]|[\x01FA-\x0217]|[\x0250-\x02A8]|[\x02BB-\x02C1]|[\x0386]|[\x0388-\x038A]|[\x038C]|[\x038E-\x03A1]|[\x03A3-\x03CE]|[\x03D0-\x03D6]|[\x03DA]|[\x03DC]|[\x03DE]|[\x03E0]|[\x03E2-\x03F3]|[\x0401-\x040C]|[\x040E-\x044F]|[\x0451-\x045C]|[\x045E-\x0481]|[\x0490-\x04C4]|[\x04C7-\x04C8]|[\x04CB-\x04CC]|[\x04D0-\x04EB]|[\x04EE-\x04F5]|[\x04F8-\x04F9]|[\x0531-\x0556]|[\x0559]|[\x0561-\x0586]|[\x05D0-\x05EA]|[\x05F0-\x05F2]|[\x0621-\x063A]|[\x0641-\x064A]|[\x0671-\x06B7]|[\x06BA-\x06BE]|[\x06C0-\x06CE]|[\x06D0-\x06D3]|[\x06D5]|[\x06E5-\x06E6]|[\x0905-\x0939]|[\x093D]|[\x0958-\x0961]|[\x0985-\x098C]|[\x098F-\x0990]|[\x0993-\x09A8]|[\x09AA-\x09B0]|[\x09B2]|[\x09B6-\x09B9]|[\x09DC-\x09DD]|[\x09DF-\x09E1]|[\x09F0-\x09F1]|[\x0A05-\x0A0A]|[\x0A0F-\x0A10]|[\x0A13-\x0A28]|[\x0A2A-\x0A30]|[\x0A32-\x0A33]|[\x0A35-\x0A36]|[\x0A38-\x0A39]|[\x0A59-\x0A5C]|[\x0A5E]|[\x0A72-\x0A74]|[\x0A85-\x0A8B]|[\x0A8D]|[\x0A8F-\x0A91]|[\x0A93-\x0AA8]|[\x0AAA-\x0AB0]|[\x0AB2-\x0AB3]|[\x0AB5-\x0AB9]|[\x0ABD]|[\x0AE0]|[\x0B05-\x0B0C]|[\x0B0F-\x0B10]|[\x0B13-\x0B28]|[\x0B2A-\x0B30]|[\x0B32-\x0B33]|[\x0B36-\x0B39]|[\x0B3D]|[\x0B5C-\x0B5D]|[\x0B5F-\x0B61]|[\x0B85-\x0B8A]|[\x0B8E-\x0B90]|[\x0B92-\x0B95]|[\x0B99-\x0B9A]|[\x0B9C]|[\x0B9E-\x0B9F]|[\x0BA3-\x0BA4]|[\x0BA8-\x0BAA]|[\x0BAE-\x0BB5]|[\x0BB7-\x0BB9]|[\x0C05-\x0C0C]|[\x0C0E-\x0C10]|[\x0C12-\x0C28]|[\x0C2A-\x0C33]|[\x0C35-\x0C39]|[\x0C60-\x0C61]|[\x0C85-\x0C8C]|[\x0C8E-\x0C90]|[\x0C92-\x0CA8]|[\x0CAA-\x0CB3]|[\x0CB5-\x0CB9]|[\x0CDE]|[\x0CE0-\x0CE1]|[\x0D05-\x0D0C])
BaseChar2 ([\x0D0E-\x0D10]|[\x0D12-\x0D28]|[\x0D2A-\x0D39]|[\x0D60-\x0D61]|[\x0E01-\x0E2E]|[\x0E30]|[\x0E32-\x0E33]|[\x0E40-\x0E45]|[\x0E81-\x0E82]|[\x0E84]|[\x0E87-\x0E88]|[\x0E8A]|[\x0E8D]|[\x0E94-\x0E97]|[\x0E99-\x0E9F]|[\x0EA1-\x0EA3]|[\x0EA5]|[\x0EA7]|[\x0EAA-\x0EAB]|[\x0EAD-\x0EAE]|[\x0EB0]|[\x0EB2-\x0EB3]|[\x0EBD]|[\x0EC0-\x0EC4]|[\x0F40-\x0F47]|[\x0F49-\x0F69]|[\x10A0-\x10C5]|[\x10D0-\x10F6]|[\x1100]|[\x1102-\x1103]|[\x1105-\x1107]|[\x1109]|[\x110B-\x110C]|[\x110E-\x1112]|[\x113C]|[\x113E]|[\x1140]|[\x114C]|[\x114E]|[\x1150]|[\x1154-\x1155]|[\x1159]|[\x115F-\x1161]|[\x1163]|[\x1165]|[\x1167]|[\x1169]|[\x116D-\x116E]|[\x1172-\x1173]|[\x1175]|[\x119E]|[\x11A8]|[\x11AB]|[\x11AE-\x11AF]|[\x11B7-\x11B8]|[\x11BA]|[\x11BC-\x11C2]|[\x11EB]|[\x11F0]|[\x11F9]|[\x1E00-\x1E9B]|[\x1EA0-\x1EF9]|[\x1F00-\x1F15]|[\x1F18-\x1F1D]|[\x1F20-\x1F45]|[\x1F48-\x1F4D]|[\x1F50-\x1F57]|[\x1F59]|[\x1F5B]|[\x1F5D]|[\x1F5F-\x1F7D]|[\x1F80-\x1FB4]|[\x1FB6-\x1FBC]|[\x1FBE]|[\x1FC2-\x1FC4]|[\x1FC6-\x1FCC]|[\x1FD0-\x1FD3]|[\x1FD6-\x1FDB]|[\x1FE0-\x1FEC]|[\x1FF2-\x1FF4]|[\x1FF6-\x1FFC]|[\x2126]|[\x212A-\x212B]|[\x212E]|[\x2180-\x2182]|[\x3041-\x3094]|[\x30A1-\x30FA]|[\x3105-\x312C]|[\xAC00-\xD7A3])
BaseChar {BaseChar1}|{BaseChar2}

Ideographic ([\x4E00-\x9FA5]|[\x3007]|[\x3021-\x3029])
Digit ([\x0030-\x0039]|[\x0660-\x0669]|[\x06F0-\x06F9]|[\x0966-\x096F]|[\x09E6-\x09EF]|[\x0A66-\x0A6F]|[\x0AE6-\x0AEF]|[\x0B66-\x0B6F]|[\x0BE7-\x0BEF]|[\x0C66-\x0C6F]|[\x0CE6-\x0CEF]|[\x0D66-\x0D6F]|[\x0E50-\x0E59]|[\x0ED0-\x0ED9]|[\x0F20-\x0F29]) 
CombiningChar ([\x0300-\x0345]|[\x0360-\x0361]|[\x0483-\x0486]|[\x0591-\x05A1]|[\x05A3-\x05B9]|[\x05BB-\x05BD]|[\x05BF]|[\x05C1-\x05C2]|[\x05C4]|[\x064B-\x0652]|[\x0670]|[\x06D6-\x06DC]|[\x06DD-\x06DF]|[\x06E0-\x06E4]|[\x06E7-\x06E8]|[\x06EA-\x06ED]|[\x0901-\x0903]|[\x093C]|[\x093E-\x094C]|[\x094D]|[\x0951-\x0954]|[\x0962-\x0963]|[\x0981-\x0983]|[\x09BC]|[\x09BE]|[\x09BF]|[\x09C0-\x09C4]|[\x09C7-\x09C8]|[\x09CB-\x09CD]|[\x09D7]|[\x09E2-\x09E3]|[\x0A02]|[\x0A3C]|[\x0A3E]|[\x0A3F]|[\x0A40-\x0A42]|[\x0A47-\x0A48]|[\x0A4B-\x0A4D]|[\x0A70-\x0A71]|[\x0A81-\x0A83]|[\x0ABC]|[\x0ABE-\x0AC5]|[\x0AC7-\x0AC9]|[\x0ACB-\x0ACD]|[\x0B01-\x0B03]|[\x0B3C]|[\x0B3E-\x0B43]|[\x0B47-\x0B48]|[\x0B4B-\x0B4D]|[\x0B56-\x0B57]|[\x0B82-\x0B83]|[\x0BBE-\x0BC2]|[\x0BC6-\x0BC8]|[\x0BCA-\x0BCD]|[\x0BD7]|[\x0C01-\x0C03]|[\x0C3E-\x0C44]|[\x0C46-\x0C48]|[\x0C4A-\x0C4D]|[\x0C55-\x0C56]|[\x0C82-\x0C83]|[\x0CBE-\x0CC4]|[\x0CC6-\x0CC8]|[\x0CCA-\x0CCD]|[\x0CD5-\x0CD6]|[\x0D02-\x0D03]|[\x0D3E-\x0D43]|[\x0D46-\x0D48]|[\x0D4A-\x0D4D]|[\x0D57]|[\x0E31]|[\x0E34-\x0E3A]|[\x0E47-\x0E4E]|[\x0EB1]|[\x0EB4-\x0EB9]|[\x0EBB-\x0EBC]|[\x0EC8-\x0ECD]|[\x0F18-\x0F19]|[\x0F35]|[\x0F37]|[\x0F39]|[\x0F3E]|[\x0F3F]|[\x0F71-\x0F84]|[\x0F86-\x0F8B]|[\x0F90-\x0F95]|[\x0F97]|[\x0F99-\x0FAD]|[\x0FB1-\x0FB7]|[\x0FB9]|[\x20D0-\x20DC]|[\x20E1]|[\x302A-\x302F]|[\x3099]|[\x309A])
Extender ([\x00B7]|[\x02D0]|[\x02D1]|[\x0387]|[\x0640]|[\x0E46]|[\x0EC6]|[\x3005]|[\x3031-\x3035]|[\x309D-\x309E]|[\x30FC-\x30FE])

/* [20]    CData    ::=    (Char* - (Char* ']]>' Char*))  */
CharMinusBrack ([\x0009]|[\x000D]|[\x000A]|[\x0020-\x005C]|[\x005E-\xD7FF]|[\xE000-\xFFFD]|{SurrogatePairs})
CharMinusGT ([\x0009]|[\x000D]|[\x000A]|[\x0020-\x003D]|[\x003F-\xD7FF]|[\xE000-\xFFFD]|{SurrogatePairs})
CDATASection ("<![CDATA["({CharMinusBrack}|("]"{CharMinusBrack})|("]""]"+{CharMinusGT}))*"]""]"+">")

/* [16]    PI    ::=    '<?' PITarget (S (Char* - (Char* '?>' Char*)))? '?>' */
PITarget {NCName}
CharMinusQuestion ([\x0009]|[\x000D]|[\x000A]|[\x0020-\x003E]|[\x0040-\xD7FF]|[\xE000-\xFFFD]|{SurrogatePairs})
PIContentAndEnd (({CharMinusQuestion}|("?"+{CharMinusGT}))*"?"+">")

/* [66]    PragmaContents    ::=    (Char* - (Char* '#)' Char*)) */
CharMinusSharp ([\x0009]|[\x000D]|[\x000A]|[\x0020-\x0022]|[\x0024-\xD7FF]|[\xE000-\xFFFD]|{SurrogatePairs})
CharMinusClosePar ([\x0009]|[\x000D]|[\x000A]|[\x0020-\x0028]|[\x002A-\xD7FF]|[\xE000-\xFFFD]|{SurrogatePairs})
PragmaContentsAndEnd (({CharMinusSharp}|("#"+{CharMinusClosePar}))*"#"+")")

/* [104]    	DirCommentContents 	   ::=    	((Char - '-') | ('-' (Char - '-')))* */
CharMinusDash ([\x0009]|[\x000D]|[\x000A]|[\x0020-\x002C]|[\x002E-\xD7FF]|[\xE000-\xFFFD]|{SurrogatePairs})
DirCommentContents (({CharMinusDash}|("-"{CharMinusDash}))*)

/* from Namespace spec */
NCNameChar {Letter}|{Digit}|"."|"-"|"_"|{CombiningChar}|{Extender}
NCName ({Letter}|"_"){NCNameChar}*

Prefix {NCName}
LocalPart {NCName}
QName            ({Prefix}":")?{LocalPart}

XMLNSQName       "xmlns"(":"{LocalPart})?

/* from XQuery spec */
IntegerLiteral       {Digits}
DecimalLiteral   ("."{Digits})|({Digits}"."[0-9]*)
DoubleLiteral   (("."{Digits})|({Digits}("."[0-9]*)?))("e"|"E")("+"|"-")?{Digits}
NotNumber       (("."{Digits})|({Digits}("."[0-9]*)?))(("e"|"E")("+"|"-")?{Digits})?[a-zA-Z][0-9a-zA-Z]*

StringLiteral      ("\""({PredefinedEntityRef}|{CharRef}|("\"\"")|[^"&])*"\"")|("'"({PredefinedEntityRef}|{CharRef}|("''")|[^'&])*"'")
XPathStringLiteral ("\""(("\"\"")|[^"])*"\"")|("'"(("''")|[^'])*"'")

/* [148]    	ElementContentChar 	   ::=    	Char - [{}<&] */
ElementContentChar ([\x0009]|[\x000D]|[\x000A]|[\x0020-\x0025]|[\x0027-\x003B]|[\x003D-\x007A]|[\x007C]|[\x007E-\xD7FF]|[\xE000-\xFFFD]|{SurrogatePairs})
ElementContent (({PredefinedEntityRef}|{CharRef}|"{{"|"}}"|{ElementContentChar}|{CDATASection})*)

/* [149]    	QuotAttrContentChar 	   ::=    	Char - ["{}<&] */
QuotAttrContentChar ([\x0009]|[\x000D]|[\x000A]|[\x0020-\x0021]|[\x0023-\x0025]|[\x0027-\x003B]|[\x003D-\x007A]|[\x007C]|[\x007E-\xD7FF]|[\xE000-\xFFFD]|{SurrogatePairs})
QuotAttrContent (({PredefinedEntityRef}|{CharRef}|"{{"|"}}"|"\"\""|{QuotAttrContentChar})*)

/* [150]    	AposAttrContentChar 	   ::=    	Char - ['{}<&] */
AposAttrContentChar ([\x0009]|[\x000D]|[\x000A]|[\x0020-\x0025]|[\x0028-\x003B]|[\x003D-\x007A]|[\x007C]|[\x007E-\xD7FF]|[\xE000-\xFFFD]|{SurrogatePairs})
AposAttrContent (({PredefinedEntityRef}|{CharRef}|"{{"|"}}"|"''"|{AposAttrContentChar})*)

/* AttrValueTemplateChar 	   ::=    	Char - [{}] */
AttrValueTemplateChar ([\x0009]|[\x000D]|[\x000A]|[\x0020-\x0021]|[\x0023-\x007A]|[\x007C]|\x007E-\xD7FF]|[\xE000-\xFFFD]|{SurrogatePairs})
AttrValueTemplateContent (("{{"|"}}"|{AttrValueTemplateChar})*)

PredefinedEntityRef "&"("lt"|"gt"|"amp"|"quot"|"apos")";"
HexDigits ([0-9]|[a-f]|[A-F])+ 
CharRef "&#"({Digits}|("x"{HexDigits}))";"

%option never-interactive
%option noyywrap
%option yyclass="XQLexer"
%option c++
%option 16bit
%option stack

%x EXPR_COMMENT
%x PRAGMA
%x PRAGMAWS
%x PRAGMACONTENTS
%x START_TAG
%x END_TAG
%x ELEMENT_CONTENT
%x QUOT_ATTRIBUTE_CONTENT
%x APOS_ATTRIBUTE_CONTENT
%x PROCESSING_INSTRUCTION
%x PROCESSING_INSTRUCTION_CONTENT
%x XML_COMMENT
%x COMPUTED_CONSTRUCTOR
%x ATTR_VALUE_TEMPLATE
%x TEMPLATE_MODES
%x OUTPUT_METHOD
%x QNAMES

%{
const char *xqGetStateName(int state)
{
  switch(state) {
  case INITIAL: return "INITIAL";
  case EXPR_COMMENT: return "EXPR_COMMENT";
  case PRAGMA: return "PRAGMA";
  case PRAGMAWS: return "PRAGMAWS";
  case PRAGMACONTENTS: return "PRAGMACONTENTS";
  case START_TAG: return "START_TAG";
  case END_TAG: return "END_TAG";
  case ELEMENT_CONTENT: return "ELEMENT_CONTENT";
  case QUOT_ATTRIBUTE_CONTENT: return "QUOT_ATTRIBUTE_CONTENT";
  case APOS_ATTRIBUTE_CONTENT: return "APOS_ATTRIBUTE_CONTENT";
  case PROCESSING_INSTRUCTION: return "PROCESSING_INSTRUCTION";
  case PROCESSING_INSTRUCTION_CONTENT: return "PROCESSING_INSTRUCTION_CONTENT";
  case XML_COMMENT: return "XML_COMMENT";
  case COMPUTED_CONSTRUCTOR: return "COMPUTED_CONSTRUCTOR";
  case ATTR_VALUE_TEMPLATE: return "ATTR_VALUE_TEMPLATE";
  case TEMPLATE_MODES: return "TEMPLATE_MODES";
  case OUTPUT_METHOD: return "OUTPUT_METHOD";
  case QNAMES: return "QNAMES";
  }
  return "unknown";
}

void XQLexer::undoLessThan()
{
  CHANGE_STATE(INITIAL);
  POP();
}
%}

%%

%{
  if(firstToken_ != MYEOF) {
    int token = firstToken_;
    firstToken_ = MYEOF;
    RECOGNIZE(token);
  }

  switch(mode_) {
  case MODE_ATTR_VALUE_TEMPLATE:
    mode_ = MODE_NORMAL;
    CHANGE_STATE(ATTR_VALUE_TEMPLATE);
    break;
  case MODE_TEMPLATE_MODES:
    mode_ = MODE_NORMAL;
    CHANGE_STATE(TEMPLATE_MODES);
    break;
  case MODE_OUTPUT_METHOD:
    mode_ = MODE_NORMAL;
    CHANGE_STATE(OUTPUT_METHOD);
    break;
  case MODE_QNAMES:
    mode_ = MODE_NORMAL;
    CHANGE_STATE(QNAMES);
    break;
  case MODE_NORMAL:
    break;
  }
%}

<INITIAL>{IntegerLiteral} RECOGNIZE_VALUE(_INTEGER_LITERAL_, yytext);
<INITIAL>{DecimalLiteral} RECOGNIZE_VALUE(_DECIMAL_LITERAL_, yytext);
<INITIAL>{DoubleLiteral}  RECOGNIZE_VALUE(_DOUBLE_LITERAL_, yytext);

<INITIAL>{StringLiteral} RECOGNIZE_VALUE_UNESCAPE(_STRING_LITERAL_, yytext + 1, yyleng - 2, *yytext, false, !isXPath(), false);
<INITIAL>{XPathStringLiteral} {
  if(isXPath()) {
    RECOGNIZE_VALUE_UNESCAPE(_STRING_LITERAL_, yytext + 1, yyleng - 2, *yytext, false, false, false);
  } else {
    std::ostringstream oss;
    oss << "Bad escaping in string literal: " << UTF8((XMLCh*)yytext);
    return error(oss.str().c_str());
  }
}

<INITIAL,COMPUTED_CONSTRUCTOR>{WhitespaceChar}+ /* Do nothing */

<INITIAL,PRAGMA,EXPR_COMMENT,COMPUTED_CONSTRUCTOR>"(:" beginComment(); PUSH_AND_CHANGE_STATE(EXPR_COMMENT);
<EXPR_COMMENT>{Char}       /* Do nothing */
<EXPR_COMMENT>":)"         endComment(); POP();

<INITIAL>"-"                      RECOGNIZE(_MINUS_);
<INITIAL>"+"                      RECOGNIZE(_PLUS_);
<INITIAL>"="                      RECOGNIZE(_EQUALS_);
<INITIAL>";"                      RECOGNIZE(_SEMICOLON_);
<INITIAL>","                      RECOGNIZE(_COMMA_);
<INITIAL>"$"                      RECOGNIZE(_DOLLAR_);
<INITIAL>":="                     RECOGNIZE(_COLON_EQUALS_);
<INITIAL>"("                      RECOGNIZE(_LPAR_);
<INITIAL>")"                      RECOGNIZE(_RPAR_);
<INITIAL>"*"                      RECOGNIZE(_ASTERISK_);
<INITIAL>"|"                      RECOGNIZE(_BAR_);
<INITIAL>"!="                     RECOGNIZE(_BANG_EQUALS_);
<INITIAL>"<="                     RECOGNIZE(_LESS_THAN_EQUALS_);
<INITIAL>">"                      RECOGNIZE(_GREATER_THAN_);
<INITIAL>">="                     RECOGNIZE(_GREATER_THAN_EQUALS_);
<INITIAL>"<<"                     RECOGNIZE(_LESS_THAN_LESS_THAN_);
<INITIAL>">>"                     RECOGNIZE(_GREATER_THAN_GREATER_THAN_);
<INITIAL>"::"                     RECOGNIZE(_COLON_COLON_);
<INITIAL>"@"                      RECOGNIZE(_AT_SIGN_);
<INITIAL>".."                     RECOGNIZE(_DOT_DOT_);
<INITIAL>"."                      RECOGNIZE(_DOT_);
<INITIAL>"["                      RECOGNIZE(_LSQUARE_);
<INITIAL>"]"                      RECOGNIZE(_RSQUARE_);
<INITIAL>"?"                      RECOGNIZE(_QUESTION_MARK_);

<INITIAL>"external"               RECOGNIZE_VALUE(_EXTERNAL_, yytext);
<INITIAL>"ascending"              RECOGNIZE_VALUE(_ASCENDING_, yytext);
<INITIAL>"descending"             RECOGNIZE_VALUE(_DESCENDING_, yytext);
<INITIAL>"greatest"               RECOGNIZE_VALUE(_GREATEST_, yytext);
<INITIAL>"least"                  RECOGNIZE_VALUE(_LEAST_, yytext);
<INITIAL>"comment"                RECOGNIZE_VALUE(_COMMENT_, yytext);
<INITIAL>"document-node"          RECOGNIZE_VALUE(_DOCUMENT_NODE_, yytext);
<INITIAL>"if"                     RECOGNIZE_VALUE(_IF_, yytext);
<INITIAL>"item"                   RECOGNIZE_VALUE(_ITEM_, yytext);
<INITIAL>"node"                   RECOGNIZE_VALUE(_NODE_, yytext);
<INITIAL>"schema-attribute"       RECOGNIZE_VALUE(_SCHEMA_ATTRIBUTE_, yytext);
<INITIAL>"schema-element"         RECOGNIZE_VALUE(_SCHEMA_ELEMENT_, yytext);
<INITIAL>"text"                   RECOGNIZE_VALUE(_TEXT_, yytext);
<INITIAL>"empty-sequence"         RECOGNIZE_VALUE(_EMPTY_SEQUENCE_, yytext);
<INITIAL>"version"                RECOGNIZE_VALUE(_VERSION_, yytext);
<INITIAL>"encoding"               RECOGNIZE_VALUE(_ENCODING_, yytext);
<INITIAL>"declare"                RECOGNIZE_VALUE(_DECLARE_, yytext);
<INITIAL>"boundary-space"         RECOGNIZE_VALUE(_BOUNDARY_SPACE_, yytext);
<INITIAL>"preserve"               RECOGNIZE_VALUE(_PRESERVE_, yytext);
<INITIAL>"strip"                  RECOGNIZE_VALUE(_STRIP_, yytext);
<INITIAL>"default"                RECOGNIZE_VALUE(_DEFAULT_, yytext);
<INITIAL>"option"                 RECOGNIZE_VALUE(_OPTION_, yytext);
<INITIAL>"ordering"               RECOGNIZE_VALUE(_ORDERING_, yytext);
<INITIAL>"ordered"                RECOGNIZE_VALUE(_ORDERED_, yytext);
<INITIAL>"unordered"              RECOGNIZE_VALUE(_UNORDERED_, yytext);
<INITIAL>"empty"                  RECOGNIZE_VALUE(_EMPTY_, yytext);
<INITIAL>"copy-namespaces"        RECOGNIZE_VALUE(_COPY_NAMESPACES_, yytext);
<INITIAL>"no-preserve"            RECOGNIZE_VALUE(_NO_PRESERVE_, yytext);
<INITIAL>"inherit"                RECOGNIZE_VALUE(_INHERIT_, yytext);
<INITIAL>"no-inherit"             RECOGNIZE_VALUE(_NO_INHERIT_, yytext);
<INITIAL>"collation"              RECOGNIZE_VALUE(_COLLATION_, yytext);
<INITIAL>"base-uri"               RECOGNIZE_VALUE(_BASE_URI_, yytext);
<INITIAL>"import"                 RECOGNIZE_VALUE(_IMPORT_, yytext);
<INITIAL>"schema"                 RECOGNIZE_VALUE(_SCHEMA_, yytext);
<INITIAL>"variable"               RECOGNIZE_VALUE(_VARIABLE_, yytext);
<INITIAL>"construction"           RECOGNIZE_VALUE(_CONSTRUCTION_, yytext);
<INITIAL>"as"                     RECOGNIZE_VALUE(_AS_, yytext);
<INITIAL>"lax"                    RECOGNIZE_VALUE(_LAX_, yytext);
<INITIAL>"strict"                 RECOGNIZE_VALUE(_STRICT_, yytext);
<INITIAL>"return"                 RECOGNIZE_VALUE(_RETURN_, yytext);
<INITIAL>"for"                    RECOGNIZE_VALUE(_FOR_, yytext);
<INITIAL>"in"                     RECOGNIZE_VALUE(_IN_, yytext);
<INITIAL>"by"                     RECOGNIZE_VALUE(_BY_, yytext);
<INITIAL>"some"                   RECOGNIZE_VALUE(_SOME_, yytext);
<INITIAL>"every"                  RECOGNIZE_VALUE(_EVERY_, yytext);
<INITIAL>"satisfies"              RECOGNIZE_VALUE(_SATISFIES_, yytext);
<INITIAL>"then"                   RECOGNIZE_VALUE(_THEN_, yytext);
<INITIAL>"else"                   RECOGNIZE_VALUE(_ELSE_, yytext);
<INITIAL>"or"                     RECOGNIZE_VALUE(_OR_, yytext);
<INITIAL>"and"                    RECOGNIZE_VALUE(_AND_, yytext);
<INITIAL>"to"                     RECOGNIZE_VALUE(_TO_, yytext);
<INITIAL>"div"                    RECOGNIZE_VALUE(_DIV_, yytext);
<INITIAL>"idiv"                   RECOGNIZE_VALUE(_IDIV_, yytext);
<INITIAL>"mod"                    RECOGNIZE_VALUE(_MOD_, yytext);
<INITIAL>"union"                  RECOGNIZE_VALUE(_UNION_, yytext);
<INITIAL>"intersect"              RECOGNIZE_VALUE(_INTERSECT_, yytext);
<INITIAL>"except"                 RECOGNIZE_VALUE(_EXCEPT_, yytext);
<INITIAL>"instance"               RECOGNIZE_VALUE(_INSTANCE_, yytext);
<INITIAL>"of"                     RECOGNIZE_VALUE(_OF_, yytext);
<INITIAL>"treat"                  RECOGNIZE_VALUE(_TREAT_, yytext);
<INITIAL>"castable"               RECOGNIZE_VALUE(_CASTABLE_, yytext);
<INITIAL>"cast"                   RECOGNIZE_VALUE(_CAST_, yytext);
<INITIAL>"eq"                     RECOGNIZE_VALUE(_EQ_, yytext);
<INITIAL>"ne"                     RECOGNIZE_VALUE(_NE_, yytext);
<INITIAL>"lt"                     RECOGNIZE_VALUE(_LT_, yytext);
<INITIAL>"le"                     RECOGNIZE_VALUE(_LE_, yytext);
<INITIAL>"gt"                     RECOGNIZE_VALUE(_GT_, yytext);
<INITIAL>"ge"                     RECOGNIZE_VALUE(_GE_, yytext);
<INITIAL>"is"                     RECOGNIZE_VALUE(_IS_, yytext);
<INITIAL>"validate"               RECOGNIZE_VALUE(_VALIDATE_, yytext);
<INITIAL>"child"                  RECOGNIZE_VALUE(_CHILD_, yytext);
<INITIAL>"descendant"             RECOGNIZE_VALUE(_DESCENDANT_, yytext);
<INITIAL>"self"                   RECOGNIZE_VALUE(_SELF_, yytext);
<INITIAL>"descendant-or-self"     RECOGNIZE_VALUE(_DESCENDANT_OR_SELF_, yytext);
<INITIAL>"following-sibling"      RECOGNIZE_VALUE(_FOLLOWING_SIBLING_, yytext);
<INITIAL>"following"              RECOGNIZE_VALUE(_FOLLOWING_, yytext);
<INITIAL>"parent"                 RECOGNIZE_VALUE(_PARENT_, yytext);
<INITIAL>"ancestor"               RECOGNIZE_VALUE(_ANCESTOR_, yytext);
<INITIAL>"preceding-sibling"      RECOGNIZE_VALUE(_PRECEDING_SIBLING_, yytext);
<INITIAL>"preceding"              RECOGNIZE_VALUE(_PRECEDING_, yytext);
<INITIAL>"ancestor-or-self"       RECOGNIZE_VALUE(_ANCESTOR_OR_SELF_, yytext);
<INITIAL>"document"               RECOGNIZE_VALUE(_DOCUMENT_, yytext);
<INITIAL>"weight"                 RECOGNIZE_VALUE(_WEIGHT_, yytext);
<INITIAL>"not"                    RECOGNIZE_VALUE(_NOT_, yytext);
<INITIAL>"lowercase"              RECOGNIZE_VALUE(_LOWERCASE_, yytext);
<INITIAL>"uppercase"              RECOGNIZE_VALUE(_UPPERCASE_, yytext);
<INITIAL>"sensitive"              RECOGNIZE_VALUE(_SENSITIVE_, yytext);
<INITIAL>"insensitive"            RECOGNIZE_VALUE(_INSENSITIVE_, yytext);
<INITIAL>"diacritics"             RECOGNIZE_VALUE(_DIACRITICS_, yytext);
<INITIAL>"stemming"               RECOGNIZE_VALUE(_STEMMING_, yytext);
<INITIAL>"thesaurus"              RECOGNIZE_VALUE(_THESAURUS_, yytext);
<INITIAL>"relationship"           RECOGNIZE_VALUE(_RELATIONSHIP_, yytext);
<INITIAL>"levels"                 RECOGNIZE_VALUE(_LEVELS_, yytext);
<INITIAL>"stop"                   RECOGNIZE_VALUE(_STOP_, yytext);
<INITIAL>"words"                  RECOGNIZE_VALUE(_WORDS_, yytext);
<INITIAL>"language"               RECOGNIZE_VALUE(_LANGUAGE_, yytext);
<INITIAL>"wildcards"              RECOGNIZE_VALUE(_WILDCARDS_, yytext);
<INITIAL>"entire"                 RECOGNIZE_VALUE(_ENTIRE_, yytext);
<INITIAL>"content"                RECOGNIZE_VALUE(_CONTENT_, yytext);
<INITIAL>"any"                    RECOGNIZE_VALUE(_ANY_, yytext);
<INITIAL>"all"                    RECOGNIZE_VALUE(_ALL_, yytext);
<INITIAL>"phrase"                 RECOGNIZE_VALUE(_PHRASE_, yytext);
<INITIAL>"word"                   RECOGNIZE_VALUE(_WORD_, yytext);
<INITIAL>"exactly"                RECOGNIZE_VALUE(_EXACTLY_, yytext);
<INITIAL>"from"                   RECOGNIZE_VALUE(_FROM_, yytext);
<INITIAL>"distance"               RECOGNIZE_VALUE(_DISTANCE_, yytext);
<INITIAL>"window"                 RECOGNIZE_VALUE(_WINDOW_, yytext);
<INITIAL>"occurs"                 RECOGNIZE_VALUE(_OCCURS_, yytext);
<INITIAL>"times"                  RECOGNIZE_VALUE(_TIMES_, yytext);
<INITIAL>"same"                   RECOGNIZE_VALUE(_SAME_, yytext);
<INITIAL>"different"              RECOGNIZE_VALUE(_DIFFERENT_, yytext);
<INITIAL>"sentences"              RECOGNIZE_VALUE(_SENTENCES_, yytext);
<INITIAL>"paragraphs"             RECOGNIZE_VALUE(_PARAGRAPHS_, yytext);
<INITIAL>"sentence"               RECOGNIZE_VALUE(_SENTENCE_, yytext);
<INITIAL>"paragraph"              RECOGNIZE_VALUE(_PARAGRAPH_, yytext);
<INITIAL>"type"                   RECOGNIZE_VALUE(_TYPE_, yytext);
<INITIAL>"start"                  RECOGNIZE_VALUE(_START_, yytext);
<INITIAL>"end"                    RECOGNIZE_VALUE(_END_, yytext);
<INITIAL>"most"                   RECOGNIZE_VALUE(_MOST_, yytext);
<INITIAL>"skip"                   RECOGNIZE_VALUE(_SKIP_, yytext);
<INITIAL>"value"                  RECOGNIZE_VALUE(_VALUE_, yytext);
<INITIAL>"first"                  RECOGNIZE_VALUE(_FIRST_, yytext);
<INITIAL>"last"                   RECOGNIZE_VALUE(_LAST_, yytext);
<INITIAL>"into"                   RECOGNIZE_VALUE(_INTO_, yytext);
<INITIAL>"after"                  RECOGNIZE_VALUE(_AFTER_, yytext);
<INITIAL>"before"                 RECOGNIZE_VALUE(_BEFORE_, yytext);
<INITIAL>"modify"                 RECOGNIZE_VALUE(_MODIFY_, yytext);
<INITIAL>"nodes"                  RECOGNIZE_VALUE(_NODES_, yytext);
<INITIAL>"id"                     RECOGNIZE_VALUE(_ID_, yytext);
<INITIAL>"key"                    RECOGNIZE_VALUE(_KEY_, yytext);
<INITIAL>"no"                     RECOGNIZE_VALUE(_NO_, yytext);
<INITIAL>"with"                   RECOGNIZE_VALUE(_WITH_, yytext);

<INITIAL>"namespace" {
  if(!isXPath() && !isExtensions() && lookAhead() == _COLON_COLON_) {
    RECOGNIZE_VALUE(_QNAME_, yytext);
  } else {
    CHANGE_STATE(COMPUTED_CONSTRUCTOR);
    RECOGNIZE_VALUE(_NAMESPACE_, yytext);
  }
}

<INITIAL>"template" {
  if(isExtensions()) {
    RECOGNIZE_VALUE(_TEMPLATE_, yytext);
  } else {
    RECOGNIZE_VALUE(_QNAME_, yytext);
  }
}
<INITIAL>"matches" {
  if(isExtensions()) {
    RECOGNIZE_VALUE(_MATCHES_, yytext);
  } else {
    RECOGNIZE_VALUE(_QNAME_, yytext);
  }
}
<INITIAL>"name" {
  if(isExtensions()) {
    RECOGNIZE_VALUE(_NAME_, yytext);
  } else {
    RECOGNIZE_VALUE(_QNAME_, yytext);
  }
}
<INITIAL>"call" {
  if(isExtensions()) {
    RECOGNIZE_VALUE(_CALL_, yytext);
  } else {
    RECOGNIZE_VALUE(_QNAME_, yytext);
  }
}
<INITIAL>"apply" {
  if(isExtensions()) {
    RECOGNIZE_VALUE(_APPLY_, yytext);
  } else {
    RECOGNIZE_VALUE(_QNAME_, yytext);
  }
}
<INITIAL>"templates" {
  if(isExtensions()) {
    RECOGNIZE_VALUE(_TEMPLATES_, yytext);
  } else {
    RECOGNIZE_VALUE(_QNAME_, yytext);
  }
}
<INITIAL>"mode" {
  if(isExtensions()) {
    RECOGNIZE_VALUE(_MODE_, yytext);
  } else {
    RECOGNIZE_VALUE(_QNAME_, yytext);
  }
}
<INITIAL>"function" {
  if(isVersion3()) {
    RECOGNIZE_VALUE(_FUNCTION_EXT_, yytext);
  } else {
    RECOGNIZE_VALUE(_FUNCTION_, yytext);
  }
}
<INITIAL>"#" {
  if(isVersion3()) {
    RECOGNIZE(_HASH_);
  } else {
    return error("Unrecognized token '#'");
  }
}
<INITIAL>"count" {
  if(isVersion3() && lookAhead() == _DOLLAR_) {
    RECOGNIZE_VALUE(_COUNT_, yytext);
  } else {
    RECOGNIZE_VALUE(_QNAME_, yytext);
  }
}
<INITIAL>"private" {
  if(isVersion3()) {
    RECOGNIZE_VALUE(_PRIVATE_, yytext);
  } else {
    RECOGNIZE_VALUE(_QNAME_, yytext);
  }
}
<INITIAL>"public" {
  if(isVersion3()) {
    RECOGNIZE_VALUE(_PUBLIC_, yytext);
  } else {
    RECOGNIZE_VALUE(_QNAME_, yytext);
  }
}
<INITIAL>"deterministic" {
  if(isVersion3()) {
    RECOGNIZE_VALUE(_DETERMINISTIC_, yytext);
  } else {
    RECOGNIZE_VALUE(_QNAME_, yytext);
  }
}
<INITIAL>"nondeterministic" {
  if(isVersion3()) {
    RECOGNIZE_VALUE(_NONDETERMINISTIC_, yytext);
  } else {
    RECOGNIZE_VALUE(_QNAME_, yytext);
  }
}
<INITIAL>"namespace-node" {
  if(isVersion3()) {
    RECOGNIZE_VALUE(_NAMESPACE_NODE_, yytext);
  } else {
    RECOGNIZE_VALUE(_QNAME_, yytext);
  }
}

<INITIAL>"let" {
  if(isXQuery()) {
    RECOGNIZE_VALUE(_LET_, yytext);
  } else {
    RECOGNIZE_VALUE(_QNAME_, yytext);
  }
}
<INITIAL>"where" {
  if(isXQuery()) {
    RECOGNIZE_VALUE(_WHERE_, yytext);
  } else {
    RECOGNIZE_VALUE(_QNAME_, yytext);
  }
}
<INITIAL>"order" {
  if(isXQuery()) {
    RECOGNIZE_VALUE(_ORDER_, yytext);
  } else {
    RECOGNIZE_VALUE(_QNAME_, yytext);
  }
}
<INITIAL>"stable" {
  if(isXQuery()) {
    RECOGNIZE_VALUE(_STABLE_, yytext);
  } else {
    RECOGNIZE_VALUE(_QNAME_, yytext);
  }
}
<INITIAL>"typeswitch" {
  if(isXQuery()) {
    RECOGNIZE_VALUE(_TYPESWITCH_, yytext);
  } else {
    RECOGNIZE_VALUE(_QNAME_, yytext);
  }
}

<INITIAL>"ft-option" {
  if(isFullText()) {
    RECOGNIZE_VALUE(_FT_OPTION_, yytext);
  } else {
    RECOGNIZE_VALUE(_QNAME_, yytext);
  }
}
<INITIAL>"score" {
  if(isFullText()) {
    RECOGNIZE_VALUE(_SCORE_, yytext);
  } else {
    RECOGNIZE_VALUE(_QNAME_, yytext);
  }
}
<INITIAL>"contains" {
  if(isFullText()) {
    RECOGNIZE_VALUE(_CONTAINS_, yytext);
  } else {
    RECOGNIZE_VALUE(_QNAME_, yytext);
  }
}
<INITIAL>"ftor" {
  if(isFullText()) {
    RECOGNIZE_VALUE(_FTOR_, yytext);
  } else {
    RECOGNIZE_VALUE(_QNAME_, yytext);
  }
}
<INITIAL>"ftand" {
  if(isFullText()) {
    RECOGNIZE_VALUE(_FTAND_, yytext);
  } else {
    RECOGNIZE_VALUE(_QNAME_, yytext);
  }
}
<INITIAL>"ftnot" {
  if(isFullText()) {
    RECOGNIZE_VALUE(_FTNOT_, yytext);
  } else {
    RECOGNIZE_VALUE(_QNAME_, yytext);
  }
}

<INITIAL>"revalidation" {
  if(isUpdate()) {
    RECOGNIZE_VALUE(_REVALIDATION_, yytext);
  } else {
    RECOGNIZE_VALUE(_QNAME_, yytext);
  }
}
<INITIAL>"updating" {
  if(isUpdate()) {
    RECOGNIZE_VALUE(_UPDATING_, yytext);
  } else {
    RECOGNIZE_VALUE(_QNAME_, yytext);
  }
}
<INITIAL>"insert" {
  if(isUpdate()) {
    RECOGNIZE_VALUE(_INSERT_, yytext);
  } else {
    RECOGNIZE_VALUE(_QNAME_, yytext);
  }
}
<INITIAL>"delete" {
  if(isUpdate()) {
    RECOGNIZE_VALUE(_DELETE_, yytext);
  } else {
    RECOGNIZE_VALUE(_QNAME_, yytext);
  }
}
<INITIAL>"replace" {
  if(isUpdate()) {
    RECOGNIZE_VALUE(_REPLACE_, yytext);
  } else {
    RECOGNIZE_VALUE(_QNAME_, yytext);
  }
}
<INITIAL>"rename" {
  if(isUpdate()) {
    RECOGNIZE_VALUE(_RENAME_, yytext);
  } else {
    RECOGNIZE_VALUE(_QNAME_, yytext);
  }
}
<INITIAL>"copy" {
  if(isUpdate()) {
    RECOGNIZE_VALUE(_COPY_, yytext);
  } else {
    RECOGNIZE_VALUE(_QNAME_, yytext);
  }
}

<INITIAL>"xquery" {
  int tok = lookAhead();
  if(tok == _VERSION_ || (isVersion3() && tok == _ENCODING_)) {
    RECOGNIZE_VALUE(_XQUERY_, yytext);
  } else {
    RECOGNIZE_VALUE(_QNAME_, yytext);
  }
}
<INITIAL>"module" {
  int tok = lookAhead();
  if(tok == _NAMESPACE_ || tok == _STRING_LITERAL_) {
    RECOGNIZE_VALUE(_MODULE_, yytext);
  } else {
    RECOGNIZE_VALUE(_QNAME_, yytext);
  }
}
<INITIAL>"without" {
  if(lookAhead() == _CONTENT_) {
    RECOGNIZE_VALUE(_WITHOUT_C_, yytext);
  } else {
    RECOGNIZE_VALUE(_WITHOUT_, yytext);
  }
}
<INITIAL>"case" {
  int tok = lookAhead();
  if(tok == _SENSITIVE_ || tok == _INSENSITIVE_) {
    RECOGNIZE_VALUE(_CASE_S_, yytext);
  } else {
    RECOGNIZE_VALUE(_CASE_, yytext);
  }
}
<INITIAL>"using" {
  if(isFullText()) {
    RECOGNIZE_VALUE(_USING_, yytext);
  } else {
    RECOGNIZE_VALUE(_QNAME_, yytext);
  }
}
<INITIAL>"at" {
  int tok = lookAhead();
  if(tok == _LEAST_ || tok == _MOST_) {
    RECOGNIZE_VALUE(_AT_LM_, yytext);
  } else {
    RECOGNIZE_VALUE(_AT_, yytext);
  }
}

<INITIAL>"element"                CHANGE_STATE(COMPUTED_CONSTRUCTOR); RECOGNIZE_VALUE(_ELEMENT_, yytext);
<INITIAL>"attribute"              CHANGE_STATE(COMPUTED_CONSTRUCTOR); RECOGNIZE_VALUE(_ATTRIBUTE_, yytext);
<INITIAL>"processing-instruction" CHANGE_STATE(COMPUTED_CONSTRUCTOR); RECOGNIZE_VALUE(_PROCESSING_INSTRUCTION_, yytext);
<COMPUTED_CONSTRUCTOR>{QName} {
  CHANGE_STATE(INITIAL);
  if(lookAhead() == _LBRACE_) {
    RECOGNIZE_VALUE(_CONSTR_QNAME_, yytext);
  } else {
    undoUserAction();
    yyless(0);
  }
}
<COMPUTED_CONSTRUCTOR>[\x0000-\xFFFF] {
  CHANGE_STATE(INITIAL);
  undoUserAction();
  yyless(0);
}


<INITIAL,ELEMENT_CONTENT,QUOT_ATTRIBUTE_CONTENT,APOS_ATTRIBUTE_CONTENT,ATTR_VALUE_TEMPLATE>"{" PUSH_AND_CHANGE_STATE(INITIAL); RECOGNIZE(_LBRACE_);

<ELEMENT_CONTENT,QUOT_ATTRIBUTE_CONTENT,APOS_ATTRIBUTE_CONTENT,ATTR_VALUE_TEMPLATE>"}" return error("It is an error for a right brace to appear in element or attribute content");
<INITIAL>"}"                                                       POP(); RECOGNIZE(_RBRACE_);

<ELEMENT_CONTENT,QUOT_ATTRIBUTE_CONTENT,APOS_ATTRIBUTE_CONTENT>"&"  return error("It is an error for an ampersand to appear in element or attribute content");

<INITIAL,ELEMENT_CONTENT>"<"                       PUSH_AND_CHANGE_STATE(START_TAG); RECOGNIZE(_LESS_THAN_OP_OR_TAG_);
<QUOT_ATTRIBUTE_CONTENT,APOS_ATTRIBUTE_CONTENT>"<" return error("It is an error for a left angle bracket to appear in attribute content");

<START_TAG>">" CHANGE_STATE(ELEMENT_CONTENT); RECOGNIZE(_START_TAG_CLOSE_);
<END_TAG>">"   POP(); RECOGNIZE(_END_TAG_CLOSE_);

<ATTR_VALUE_TEMPLATE>{AttrValueTemplateContent} RECOGNIZE_VALUE_UNESCAPE(_QUOT_ATTR_CONTENT_, yytext, yyleng, 0, true, false, false);

<START_TAG>"\""                           CHANGE_STATE(QUOT_ATTRIBUTE_CONTENT); RECOGNIZE(_OPEN_QUOT_);
<QUOT_ATTRIBUTE_CONTENT>{QuotAttrContent} RECOGNIZE_VALUE_UNESCAPE(_QUOT_ATTR_CONTENT_, yytext, yyleng, '"', true, true, false);
<QUOT_ATTRIBUTE_CONTENT>"\""              CHANGE_STATE(START_TAG); RECOGNIZE(_CLOSE_QUOT_);

<START_TAG>"'"                            CHANGE_STATE(APOS_ATTRIBUTE_CONTENT); RECOGNIZE(_OPEN_APOS_);
<APOS_ATTRIBUTE_CONTENT>{AposAttrContent} RECOGNIZE_VALUE_UNESCAPE(_APOS_ATTR_CONTENT_, yytext, yyleng, '\'', true, true, false);
<APOS_ATTRIBUTE_CONTENT>"'"               CHANGE_STATE(START_TAG); RECOGNIZE(_CLOSE_APOS_);

<ELEMENT_CONTENT>{WhitespaceChar}+ RECOGNIZE_VALUE(_WHITESPACE_ELEMENT_CONTENT_, yytext);
<ELEMENT_CONTENT>{ElementContent} RECOGNIZE_VALUE_UNESCAPE(_ELEMENT_CONTENT_, yytext, yyleng, 0, true, true, true);

<START_TAG>"/>" POP(); RECOGNIZE(_EMPTY_TAG_CLOSE_);

<ELEMENT_CONTENT>"</" CHANGE_STATE(END_TAG); RECOGNIZE(_END_TAG_OPEN_);

<START_TAG>"=" RECOGNIZE(_VALUE_INDICATOR_);

<INITIAL>"(#"                          CHANGE_STATE(PRAGMA); RECOGNIZE(_PRAGMA_OPEN_);
<PRAGMA>{WhitespaceChar}+              RECOGNIZE(_WHITESPACE_);
<PRAGMA>{QName}                        CHANGE_STATE(PRAGMAWS); RECOGNIZE_VALUE(_QNAME_, yytext);
<PRAGMAWS>"#)"                         CHANGE_STATE(INITIAL); RECOGNIZE_VALUE_LEN(_PRAGMA_CONTENT_, yytext, yyleng - 2);
<PRAGMAWS>{WhitespaceChar}+            CHANGE_STATE(PRAGMACONTENTS); RECOGNIZE(_WHITESPACE_);
<PRAGMACONTENTS>{PragmaContentsAndEnd} CHANGE_STATE(INITIAL); RECOGNIZE_VALUE_LEN(_PRAGMA_CONTENT_, yytext, yyleng - 2);

<INITIAL,ELEMENT_CONTENT>"<?"                                             PUSH_AND_CHANGE_STATE(PROCESSING_INSTRUCTION); RECOGNIZE(_PI_START_);
<PROCESSING_INSTRUCTION>("xml"|"Xml"|"xMl"|"xmL"|"XMl"|"xML"|"XML"|"XmL") return error("The target of a processing instruction must not be 'XML'");
<PROCESSING_INSTRUCTION>{PITarget}                                        RECOGNIZE_VALUE(_PI_TARGET_, yytext);
<PROCESSING_INSTRUCTION>{WhitespaceChar}+                                 CHANGE_STATE(PROCESSING_INSTRUCTION_CONTENT); RECOGNIZE(_WHITESPACE_);
<PROCESSING_INSTRUCTION>"?>"                                              POP(); RECOGNIZE_VALUE_LEN(_PI_CONTENT_, yytext, yyleng - 2);
<PROCESSING_INSTRUCTION_CONTENT>{PIContentAndEnd}                         POP(); RECOGNIZE_VALUE_LEN(_PI_CONTENT_, yytext, yyleng - 2);

<INITIAL,ELEMENT_CONTENT>"<!--"    PUSH_AND_CHANGE_STATE(XML_COMMENT); RECOGNIZE(_XML_COMMENT_START_);
<XML_COMMENT>{DirCommentContents}  RECOGNIZE_VALUE(_XML_COMMENT_CONTENT_, yytext);
<XML_COMMENT>"--"                  return error("It is an error for a double dash to appear in an XML comment");
<XML_COMMENT>"-->"                 POP(); RECOGNIZE(_XML_COMMENT_END_);

<INITIAL>"/"  RECOGNIZE(_SLASH_);
<INITIAL>"//" RECOGNIZE(_SLASH_SLASH_);

<START_TAG>{XMLNSQName}            RECOGNIZE_VALUE(_XMLNS_QNAME_, yytext);
<INITIAL,START_TAG,END_TAG>{QName} RECOGNIZE_VALUE(_QNAME_, yytext);

<INITIAL>{NCName}":*" RECOGNIZE_VALUE_LEN(_NCNAME_COLON_STAR_, yytext, yyleng - 2);
<INITIAL>"*:"{NCName} RECOGNIZE_VALUE(_STAR_COLON_NCNAME_, yytext + 2);

<START_TAG,END_TAG>{WhitespaceChar}+ RECOGNIZE(_WHITESPACE_);

<TEMPLATE_MODES>{QName} RECOGNIZE_VALUE(_QNAME_, yytext);
<INITIAL,TEMPLATE_MODES>"#default" RECOGNIZE(_HASH_DEFAULT_);
<INITIAL,TEMPLATE_MODES>"#all" RECOGNIZE(_HASH_ALL_);
<INITIAL,TEMPLATE_MODES>"#current" RECOGNIZE(_HASH_CURRENT_);
<TEMPLATE_MODES>{WhitespaceChar}+ /* Do nothing */

<OUTPUT_METHOD>"xml" RECOGNIZE_VALUE(_XML_, yytext);
<OUTPUT_METHOD>"html" RECOGNIZE_VALUE(_HTML_, yytext);
<OUTPUT_METHOD>"xhtml" RECOGNIZE_VALUE(_XHTML_, yytext);
<OUTPUT_METHOD>"text" RECOGNIZE_VALUE(_TEXT_, yytext);
<OUTPUT_METHOD>{QName} RECOGNIZE_VALUE(_QNAME_, yytext);
<OUTPUT_METHOD>{WhitespaceChar}+ /* Do nothing */

<QNAMES>{QName} RECOGNIZE_VALUE(_QNAME_, yytext);
<QNAMES>{WhitespaceChar}+ /* Do nothing */

<INITIAL>{NotNumber} {
  std::ostringstream oss;
  oss << "Invalid literal found: " << UTF8((XMLCh*)yytext);
  return error(oss.str().c_str());
}

<*>[\x0000-\xFFFF] { 
  std::ostringstream oss;
  oss << "Unrecognized character '" << UTF8((XMLCh*)yytext) << "' (" << yytext[0] << ")";
  return error(oss.str().c_str());
}

%%

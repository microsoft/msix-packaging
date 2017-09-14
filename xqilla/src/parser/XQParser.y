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

#if defined(WIN32) && !defined(__CYGWIN__)
// turn off the warnings "switch statement contains 'default' but no 'case' labels"
//                       "'yyerrlab1' : unreferenced label"
#pragma warning(disable : 4065 4102)
#endif

#include <cstdio>
#include <iostream>
#include <sstream>

#include <xqilla/utils/UTF8Str.hpp>

#include "../lexer/XQLexer.hpp"

#include <xqilla/simple-api/XQQuery.hpp>
#include <xqilla/ast/XQFunction.hpp>
#include <xqilla/ast/XQDocumentConstructor.hpp>
#include <xqilla/ast/XQElementConstructor.hpp>
#include <xqilla/ast/XQAttributeConstructor.hpp>
#include <xqilla/ast/XQNamespaceConstructor.hpp>
#include <xqilla/ast/XQPIConstructor.hpp>
#include <xqilla/ast/XQCommentConstructor.hpp>
#include <xqilla/ast/XQTextConstructor.hpp>
#include <xqilla/ast/XQQuantified.hpp>
#include <xqilla/ast/XQTypeswitch.hpp>
#include <xqilla/ast/XQValidate.hpp>
#include <xqilla/ast/XQGlobalVariable.hpp>
#include <xqilla/ast/XQFunctionCall.hpp>
#include <xqilla/ast/XQOrderingChange.hpp>
#include <xqilla/ast/XQDocumentOrder.hpp>
#include <xqilla/ast/XQReturn.hpp>
#include <xqilla/ast/XQNamespaceBinding.hpp>

#include <xercesc/validators/schema/SchemaSymbols.hpp>
#include <xercesc/dom/DOMXPathNSResolver.hpp>

#include <xqilla/ast/XQSequence.hpp>
#include <xqilla/ast/XQNav.hpp>
#include <xqilla/ast/XQStep.hpp>
#include <xqilla/ast/XQLiteral.hpp>
#include <xqilla/ast/XQVariable.hpp>
#include <xqilla/ast/XQCastAs.hpp>
#include <xqilla/ast/XQCastableAs.hpp>
#include <xqilla/ast/XQTreatAs.hpp>
#include <xqilla/ast/XQFunctionConversion.hpp>
#include <xqilla/ast/XQAnalyzeString.hpp>
#include <xqilla/ast/XQCopyOf.hpp>
#include <xqilla/ast/XQCopy.hpp>
#include <xqilla/ast/XQAtomize.hpp>
#include <xqilla/ast/ConvertFunctionArg.hpp>
#include <xqilla/ast/XQIf.hpp>
#include <xqilla/ast/XQContextItem.hpp>
#include <xqilla/ast/XQPredicate.hpp>
#include <xqilla/ast/XQApplyTemplates.hpp>
#include <xqilla/ast/XQInlineFunction.hpp>
#include <xqilla/ast/XQFunctionDeref.hpp>
#include <xqilla/ast/XQFunctionRef.hpp>
#include <xqilla/ast/XQPartialApply.hpp>
#include <xqilla/ast/XQMap.hpp>

#include <xqilla/fulltext/FTContains.hpp>
#include <xqilla/fulltext/FTOr.hpp>
#include <xqilla/fulltext/FTAnd.hpp>
#include <xqilla/fulltext/FTMildnot.hpp>
#include <xqilla/fulltext/FTUnaryNot.hpp>
#include <xqilla/fulltext/FTOrder.hpp>
#include <xqilla/fulltext/FTDistance.hpp>
#include <xqilla/fulltext/FTScope.hpp>
#include <xqilla/fulltext/FTContent.hpp>
#include <xqilla/fulltext/FTWindow.hpp>

#include <xqilla/update/UDelete.hpp>
#include <xqilla/update/URename.hpp>
#include <xqilla/update/UReplace.hpp>
#include <xqilla/update/UReplaceValueOf.hpp>
#include <xqilla/update/UInsertAsFirst.hpp>
#include <xqilla/update/UInsertAsLast.hpp>
#include <xqilla/update/UInsertInto.hpp>
#include <xqilla/update/UInsertAfter.hpp>
#include <xqilla/update/UInsertBefore.hpp>
#include <xqilla/update/UTransform.hpp>
#include <xqilla/update/UApplyUpdates.hpp>

#include <xqilla/ast/ContextTuple.hpp>
#include <xqilla/ast/ForTuple.hpp>
#include <xqilla/ast/LetTuple.hpp>
#include <xqilla/ast/WhereTuple.hpp>
#include <xqilla/ast/OrderByTuple.hpp>
#include <xqilla/ast/CountTuple.hpp>

#include <xqilla/parser/QName.hpp>

#include <xqilla/operators/Equals.hpp>
#include <xqilla/operators/GeneralComp.hpp>
#include <xqilla/operators/NotEquals.hpp>
#include <xqilla/operators/LessThan.hpp>
#include <xqilla/operators/GreaterThan.hpp>
#include <xqilla/operators/LessThanEqual.hpp>
#include <xqilla/operators/GreaterThanEqual.hpp>
#include <xqilla/operators/Plus.hpp>
#include <xqilla/operators/Minus.hpp>
#include <xqilla/operators/Multiply.hpp>
#include <xqilla/operators/Divide.hpp>
#include <xqilla/operators/IntegerDivide.hpp>
#include <xqilla/operators/Mod.hpp>
#include <xqilla/operators/UnaryMinus.hpp>
#include <xqilla/operators/NodeComparison.hpp>
#include <xqilla/operators/OrderComparison.hpp>
#include <xqilla/operators/Intersect.hpp>
#include <xqilla/operators/Except.hpp>
#include <xqilla/operators/Union.hpp>
#include <xqilla/operators/Or.hpp>
#include <xqilla/operators/And.hpp>

#include <xqilla/functions/FunctionRoot.hpp>
#include <xqilla/functions/FunctionQName.hpp>
#include <xqilla/functions/FunctionId.hpp>
#include <xqilla/functions/FunctionError.hpp>
#include <xqilla/functions/XQillaFunction.hpp>
#include <xqilla/functions/BuiltInModules.hpp>

#include <xqilla/axis/NodeTest.hpp>

#include <xqilla/schema/SequenceType.hpp>
#include <xqilla/context/DynamicContext.hpp>
#include <xqilla/exceptions/NamespaceLookupException.hpp>
#include <xqilla/exceptions/ContextException.hpp>
#include <xqilla/exceptions/StaticErrorException.hpp>

#include <xqilla/utils/XPath2Utils.hpp>
#include <xqilla/utils/XPath2NSUtils.hpp>
#include <xqilla/utils/UTF8Str.hpp>
#include "../config/xqilla_config.h"

#define YYPARSE_PARAM qp
#define YYDEBUG 1
#define YYERROR_VERBOSE

// this removes a memory leak occurring when an error is found in the query (we throw an exception from inside
// yyerror, preventing the bison-generated code from cleaning up the memory); by defining this macro we use 
// stack-based memory instead of the heap
#define YYSTACK_USE_ALLOCA  1
#if HAVE_ALLOCA_H
#include <alloca.h>
#elif defined __GNUC__
#undef alloca
#define alloca __builtin_alloca
#elif defined _AIX
#define alloca __alloca
#elif defined _MSC_VER
#include <malloc.h>
#else
#include <stddef.h>
#ifdef __cplusplus
extern "C"
#endif
void *alloca (size_t);
#endif

#define QP       ((XQParserArgs*)qp)
#define CONTEXT  (QP->_context)
#define LANGUAGE (QP->_lexer->getLanguage())
#define MEMMGR   (QP->_lexer->getMemoryManager())

#define REJECT_NOT_XQUERY(where,pos)      if(!QP->_lexer->isXQuery()) { yyerror(LANGUAGE, #where, (pos).first_line, (pos).first_column); }
#define REJECT_NOT_VERSION3(where,pos)   if(!QP->_lexer->isVersion3()) { yyerror(LANGUAGE, #where, (pos).first_line, (pos).first_column); }

#define WRAP(pos,object)        (wrapForDebug((QP), (object), (pos).first_line, (pos).first_column))

#define LOCATION(pos,name) LocationInfo name(QP->_lexer->getFile(), (pos).first_line, (pos).first_column)

#define GET_NAVIGATION(pos, object) (((object)->getType() == ASTNode::NAVIGATION) ? (XQNav*)(object) : WRAP(pos, new (MEMMGR) XQNav((object), MEMMGR)))

#define PRESERVE_NS(pos, object) (QP->_lexer->getNSResolver() == 0 ? (object) : WRAP((pos), new (MEMMGR) XQNamespaceBinding(QP->_lexer->getNSResolver(), (object), MEMMGR)))

#define BIT_ORDERING_SPECIFIED                  0
#define BIT_BOUNDARY_SPECIFIED                  1
#define BIT_COLLATION_SPECIFIED                 2
#define BIT_BASEURI_SPECIFIED                   3
#define BIT_CONSTRUCTION_SPECIFIED              4
#define BIT_EMPTYORDERING_SPECIFIED             5
#define BIT_COPYNAMESPACE_SPECIFIED             6
#define BIT_DEFAULTELEMENTNAMESPACE_SPECIFIED   7
#define BIT_DEFAULTFUNCTIONNAMESPACE_SPECIFIED  8
#define BIT_DECLARE_SECOND_STEP                 9
#define BIT_REVALIDATION_SPECIFIED              10

#define CHECK_SECOND_STEP(pos, name) { \
  if(QP->_flags.get(BIT_DECLARE_SECOND_STEP)) \
    yyerror((pos), "Prolog contains " name " declaration after a variable, function or option declaration"); \
}

#define CHECK_SPECIFIED(pos, bit, name, error) { \
  if(QP->_flags.get(bit)) \
    yyerror((pos), "Prolog contains more than one " name " declaration [err:" error "]"); \
  QP->_flags.set(bit); \
}

#define SET_NAMESPACE(pos, prefix, uri) { \
  if(QP->_namespaceDecls.containsKey((prefix))) \
    yyerror((pos), "Namespace prefix has already been bound to a namespace [err:XQST0033]"); \
  QP->_namespaceDecls.put((prefix), NULL); \
  CONTEXT->setNamespaceBinding((prefix), (uri)); \
}

#define SET_BUILT_IN_NAMESPACE(prefix, uri) { \
  if(!QP->_namespaceDecls.containsKey((prefix))) \
    CONTEXT->setNamespaceBinding((prefix), (uri)); \
}

#undef yylex
#define yylex QP->_lexer->yylex
#undef yyerror
#define yyerror QP->_lexer->error

XERCES_CPP_NAMESPACE_USE;
using namespace std;

static const XMLCh sz1_0[] = { chDigit_1, chPeriod, chDigit_0, chNull };
static const XMLCh sz3_0[] = { chDigit_3, chPeriod, chDigit_0, chNull };
static const XMLCh option_projection[] = { 'p', 'r', 'o', 'j', 'e', 'c', 't', 'i', 'o', 'n', 0 };
static const XMLCh option_psvi[] = { 'p', 's', 'v', 'i', 0 };
static const XMLCh option_lint[] = { 'l', 'i', 'n', 't', 0 };
static const XMLCh var_name[] = { 'n', 'a', 'm', 'e', 0 };

static const XMLCh XMLChXS[]    = { chLatin_x, chLatin_s, chNull };
static const XMLCh XMLChXSI[]   = { chLatin_x, chLatin_s, chLatin_i, chNull };
static const XMLCh XMLChFN[]    = { chLatin_f, chLatin_n, chNull };
static const XMLCh XMLChLOCAL[] = { chLatin_l, chLatin_o, chLatin_c, chLatin_a, chLatin_l, chNull };
static const XMLCh XMLChERR[]   = { chLatin_e, chLatin_r, chLatin_r, chNull };

static inline VectorOfASTNodes packageArgs(ASTNode *arg1Impl, ASTNode *arg2Impl, XPath2MemoryManager* memMgr)
{
  VectorOfASTNodes args=VectorOfASTNodes(2,(ASTNode*)NULL,XQillaAllocator<ASTNode*>(memMgr));
  args[0]=arg1Impl;
  args[1]=arg2Impl;

  return args;
}

static inline TupleNode *getLastAncestor(TupleNode *node)
{
  while(node->getParent()) node = const_cast<TupleNode*>(node->getParent());
  return node;
}

static inline TupleNode *setLastAncestor(TupleNode *node, TupleNode *ancestor)
{
  getLastAncestor(node)->setParent(ancestor);
  return node;
}

template<typename TYPE>
TYPE *wrapForDebug(XQParserArgs *qp, TYPE* pObjToWrap, unsigned int line, unsigned int column)
{
  if(pObjToWrap->getLine() == 0)
    pObjToWrap->setLocationInfo(QP->_lexer->getFile(), line, column);
  return pObjToWrap;
}

#define RESOLVE_QNAME(pos, qname) const XMLCh *uri, *name; resolveQName((pos), QP, (qname), uri, name)

static void resolveQName(const yyltype &pos, XQParserArgs *qp, const XMLCh *qname, const XMLCh *&uri, const XMLCh *&name)
{
  const XMLCh *prefix = XPath2NSUtils::getPrefix(qname, MEMMGR);
  name = XPath2NSUtils::getLocalName(qname);

  if(prefix == 0 || *prefix == 0) {
    uri = 0;
  }
  else {
    if(QP->_lexer->getNSResolver() == 0) {
      if(prefix == 0 || *prefix == 0)
        uri = CONTEXT->getDefaultElementAndTypeNS();
      else uri = CONTEXT->getNSResolver()->lookupNamespaceURI(prefix);
    }
    else {
      uri = QP->_lexer->getNSResolver()->lookupNamespaceURI(prefix);
    }

    if(uri == 0 || *uri == 0) {
      ostringstream oss;
      oss << "No namespace binding for prefix '" << UTF8(prefix) << "' [err:XTSE0280]";
      yyerror(pos, oss.str().c_str());
    }
  }
}

#define XSLT_VARIABLE_VALUE(pos, select, seqConstruct, seqType) variableValueXSLT((pos), QP, (select), (seqConstruct), (seqType))

static ASTNode *variableValueXSLT(const yyltype &pos, XQParserArgs *qp, ASTNode *select, XQSequence *seqConstruct, SequenceType *seqType)
{
  if(!seqConstruct->getChildren().empty()) {
    if(select != 0) return 0; // signifies an error
    
    if(seqType == 0) {
      return WRAP(pos, new (MEMMGR) XQDocumentConstructor(seqConstruct, MEMMGR));
    }

    return WRAP(pos, new (MEMMGR) XQFunctionConversion(seqConstruct, seqType, MEMMGR));
  }

  if(select != 0) {
    if(seqType == 0) return select;

    return WRAP(pos, new (MEMMGR) XQFunctionConversion(select, seqType, MEMMGR));
  }

  if(seqType == 0) {
    return WRAP(pos, new (MEMMGR) XQLiteral(
                  SchemaSymbols::fgURI_SCHEMAFORSCHEMA,
                  SchemaSymbols::fgDT_STRING,
                  XMLUni::fgZeroLenString, AnyAtomicType::STRING,
                  MEMMGR));
  }

  return WRAP(pos, new (MEMMGR) XQFunctionConversion(WRAP(pos, new (MEMMGR) XQSequence(MEMMGR)), seqType, MEMMGR));
}

namespace XQParser {

%}

/* this gives a nice name to the $end token */
%token MYEOF 0                    "<end of file>"

/* "Fake" tokens that decide which language we're parsing */
%token _LANG_XPATH2_                   "<Language: XPath2>"
%token _LANG_XQUERY_                   "<Language: XQuery>"
%token _LANG_XSLT2_                    "<Language: XSLT 2.0>"
%token _LANG_FUNCDECL_                 "<Language: Function Declaration>"
%token _LANG_DELAYEDMODULE_            "<Language: Delayed Module>"
%token _LANG_FUNCTION_SIGNATURE_       "<Language: Function Signature>"

%token _DOLLAR_ "$"
%token _COLON_EQUALS_ ":="
%token _ASTERISK_ "*"
%token _BAR_ "|"
%token _BANG_EQUALS_ "!="
%token _LESS_THAN_EQUALS_ "<="
%token _GREATER_THAN_ "> (comparison operator)"
%token _GREATER_THAN_EQUALS_ ">="
%token _LESS_THAN_LESS_THAN_ "<<"
%token _GREATER_THAN_GREATER_THAN_ ">>"
%token _COLON_COLON_ "::"
%token _AT_SIGN_ "@"
%token _LSQUARE_ "["
%token _RSQUARE_ "]"
%token _QUESTION_MARK_ "?"
%token _LESS_THAN_OP_OR_TAG_ "<"
%token _START_TAG_CLOSE_ "> (start tag close)"
%token _END_TAG_CLOSE_ "> (end tag close)"
%token _PI_START_ "<?"
%token _XML_COMMENT_START_ "<!--"
%token _XML_COMMENT_END_ "-->"
%token _SLASH_SLASH_ "//"
%token _END_TAG_OPEN_ "</"
%token _PRAGMA_OPEN_           "(#"
%token _COMMA_                 ","
%token _PLUS_                  "+"
%token _MINUS_                 "-"
%token _SLASH_                 "/"
%token _EQUALS_                "= (comparison operator)"
%token _DOT_                   "."
%token _DOT_DOT_               ".."
%token _LPAR_                  "("
%token _RPAR_                  ")"
%token _EMPTY_TAG_CLOSE_       "/>"
%token _VALUE_INDICATOR_       "="
%token _OPEN_QUOT_             "\" (open)"
%token _CLOSE_QUOT_            "\" (close)"
%token _OPEN_APOS_             "' (open)"
%token _CLOSE_APOS_            "' (close)"
%token _LBRACE_                "{"
%token _RBRACE_                "}"
%token _SEMICOLON_             ";"
%token _HASH_                  "#"

%token <str> _INTEGER_LITERAL_ "<integer literal>"
%token <str> _DECIMAL_LITERAL_ "<decimal literal>"
%token <str> _DOUBLE_LITERAL_ "<double literal>"
%token <str> _ATTRIBUTE_ "attribute"
%token <str> _COMMENT_ "comment"
%token <str> _DOCUMENT_NODE_ "document-node"
%token <str> _NAMESPACE_NODE_ "namespace-node"
%token <str> _NODE_ "node"
%token <str> _PROCESSING_INSTRUCTION_ "processing-instruction"
%token <str> _SCHEMA_ATTRIBUTE_ "schema-attribute"
%token <str> _SCHEMA_ELEMENT_ "schema-element"
%token <str> _TEXT_ "text"
%token <str> _EMPTY_SEQUENCE_ "empty-sequence"
%token <str> _BOUNDARY_SPACE_ "boundary-space"
%token <str> _FT_OPTION_ "ft-option"
%token <str> _BASE_URI_ "base-uri"
%token <str> _LAX_ "lax"
%token <str> _STRICT_ "strict"
%token <str> _IDIV_ "idiv"
%token <str> _CHILD_ "child"
%token <str> _DESCENDANT_ "descendant"
%token <str> _SELF_ "self"
%token <str> _DESCENDANT_OR_SELF_ "descendant-or-self"
%token <str> _FOLLOWING_SIBLING_ "following-sibling"
%token <str> _FOLLOWING_ "following"
%token <str> _PARENT_ "parent"
%token <str> _ANCESTOR_ "ancestor"
%token <str> _PRECEDING_SIBLING_ "preceding-sibling"
%token <str> _PRECEDING_ "preceding"
%token <str> _ANCESTOR_OR_SELF_ "ancestor-of-self"
%token <str> _DOCUMENT_ "document"
%token <str> _NOT_ "not"
%token <str> _USING_ "using"
%token <str> _NO_ "no"
%token <str> _SENSITIVE_ "sensitive"
%token <str> _INSENSITIVE_ "insensitive"
%token <str> _DIACRITICS_ "diacritics"
%token <str> _WITHOUT_ "without"
%token <str> _WITHOUT_C_ "without (followed by content)"
%token <str> _STEMMING_ "stemming"
%token <str> _THESAURUS_ "thesaurus"
%token <str> _STOP_ "stop"
%token <str> _WILDCARDS_ "wildcards"
%token <str> _ENTIRE_ "entire"
%token <str> _CONTENT_ "content"
%token <str> _WORD_ "word"
%token <str> _TYPE_ "type"
%token <str> _START_ "start"
%token <str> _END_ "end"
%token <str> _MOST_ "most"
%token <str> _SKIP_ "skip"
%token <str> _COPY_ "copy"
%token <str> _VALUE_ "value"
%token <str> _WHITESPACE_ "<whitespace>"
%token <str> _PI_CONTENT_ "<processing instruction content>"
%token <str> _XML_COMMENT_CONTENT_ "<xml comment content>"
%token <str> _EQ_  "eq"
%token <str> _NE_  "ne"
%token <str> _LT_  "lt"
%token <str> _LE_  "le"
%token <str> _GT_  "gt"
%token <str> _GE_  "ge"
%token <str> _AT_  "at"
%token <str> _QUOT_ATTR_CONTENT_ "<quot attribute content>"
%token <str> _APOS_ATTR_CONTENT_ "<apos attribute content>"
%token <str> _WHITESPACE_ELEMENT_CONTENT_ "<whitespace element content>"
%token <str> _ELEMENT_CONTENT_ "<element content>"
%token <str> _AT_LM_  "at (followed by least or most)"
%token <str> _ORDERED_  "ordered"
%token <str> _UNORDERED_  "unordered"
%token <str> _QNAME_                "<qualified name>"
%token <str> _XMLNS_QNAME_                "<xmlns qualified name>"
%token <str> _CONSTR_QNAME_                "<computed constructor qualified name>"
%token <str> _STRING_LITERAL_            "'...'"
%token <str> _VARIABLE_                "variable"
%token <str> _NCNAME_COLON_STAR_          "<NCName>:*"
%token <str> _STAR_COLON_NCNAME_          "*:<NCName>"
%token <str> _PI_TARGET_              "<processing instruction target>"
%token <str> _PRAGMA_CONTENT_            "<pragma content>"
%token <str> _RETURN_              "return"
%token <str> _FOR_              "for"
%token <str> _IN_                "in"
%token <str> _LET_              "let"
%token <str> _WHERE_              "where"
%token <str> _COUNT_              "count"
%token <str> _BY_                    "by"
%token <str> _ORDER_              "order"
%token <str> _STABLE_                 "stable"
%token <str> _ASCENDING_            "ascending"
%token <str> _DESCENDING_            "descending"
%token <str> _EMPTY_                  "empty"
%token <str> _GREATEST_              "greatest"
%token <str> _LEAST_                  "least"
%token <str> _COLLATION_            "collation"
%token <str> _SOME_              "some"
%token <str> _EVERY_              "every"
%token <str> _SATISFIES_            "satisfies"
%token <str> _TYPESWITCH_            "typeswitch"
%token <str> _CASE_              "case"
%token <str> _CASE_S_              "case (followed by (in)sensitive)"
%token <str> _AS_                "as"
%token <str> _IF_                "if"
%token <str> _THEN_              "then"
%token <str> _ELSE_              "else"
%token <str> _OR_                "or"
%token <str> _AND_              "and"
%token <str> _INSTANCE_             "instance"
%token <str> _OF_                      "of"
%token <str> _CASTABLE_              "castable"
%token <str> _TO_                "to"
%token <str> _DIV_              "div"
%token <str> _MOD_              "mod"
%token <str> _UNION_              "union"
%token <str> _INTERSECT_            "intersect"
%token <str> _EXCEPT_              "except"
%token <str> _VALIDATE_               "validate"
%token <str> _CAST_                "cast"
%token <str> _TREAT_              "treat"
%token <str> _IS_                "is"
%token <str> _PRESERVE_            "preserve"
%token <str> _STRIP_              "strip"
%token <str> _NAMESPACE_            "namespace"
%token <str> _ITEM_              "item"
%token <str> _EXTERNAL_            "external"
%token <str> _ENCODING_            "encoding"
%token <str> _NO_PRESERVE_          "no-preserve"
%token <str> _INHERIT_            "inherit"
%token <str> _NO_INHERIT_            "no-inherit"
%token <str> _DECLARE_            "declare"
%token <str> _CONSTRUCTION_          "construction"
%token <str> _ORDERING_            "ordering"
%token <str> _DEFAULT_          "default"
%token <str> _COPY_NAMESPACES_        "copy-namespaces"
%token <str> _OPTION_                  "option"
%token <str> _XQUERY_              "xquery"
%token <str> _VERSION_            "version"
%token <str> _IMPORT_                "import"
%token <str> _SCHEMA_                  "schema"
%token <str> _MODULE_                  "module"
%token <str> _ELEMENT_                "element"
%token <str> _FUNCTION_                "function"
%token <str> _FUNCTION_EXT_                "function (ext)"
%token <str> _SCORE_                                          "score"
%token <str> _CONTAINS_                                       "contains"
%token <str> _WEIGHT_                                         "weight"
%token <str> _WINDOW_                                         "window"
%token <str> _DISTANCE_                                       "distance"
%token <str> _OCCURS_                                         "occurs"
%token <str> _TIMES_                                          "times"
%token <str> _SAME_                                           "same"
%token <str> _DIFFERENT_                                      "different"
%token <str> _LOWERCASE_                                      "lowercase"
%token <str> _UPPERCASE_                                      "uppercase"
%token <str> _RELATIONSHIP_                                   "relationship"
%token <str> _LEVELS_                                         "levels"
%token <str> _LANGUAGE_                                       "language"
%token <str> _ANY_                                            "any"
%token <str> _ALL_                                            "all"
%token <str> _PHRASE_                                         "phrase"
%token <str> _EXACTLY_                                        "exactly"
%token <str> _FROM_                                           "from"
%token <str> _WORDS_                                          "words"
%token <str> _SENTENCES_                                      "sentences"
%token <str> _PARAGRAPHS_                                     "paragraphs"
%token <str> _SENTENCE_                                       "sentence"
%token <str> _PARAGRAPH_                                      "paragraph"
%token <str> _REPLACE_                                        "replace"
%token <str> _MODIFY_                                         "modify"
%token <str> _FIRST_                                          "first"
%token <str> _INSERT_                                         "insert"
%token <str> _BEFORE_                                         "before"
%token <str> _AFTER_                                          "after"
%token <str> _REVALIDATION_                                   "revalidation"
%token <str> _WITH_                                           "with"
%token <str> _NODES_                                          "nodes"
%token <str> _RENAME_                                         "rename"
%token <str> _LAST_                                           "last"
%token <str> _DELETE_                                         "delete"
%token <str> _INTO_                                           "into"
%token <str> _UPDATING_                                       "updating"
%token <str> _ID_                                             "id"
%token <str> _KEY_                                            "key"
%token <str> _TEMPLATE_                                       "template"
%token <str> _MATCHES_                                        "matches"
%token <str> _NAME_                                           "name"
%token <str> _CALL_                                           "call"
%token <str> _APPLY_                                          "apply"
%token <str> _TEMPLATES_                                      "templates"
%token <str> _MODE_                                           "mode"
%token <str> _FTOR_                                           "ftor"
%token <str> _FTAND_                                          "ftand"
%token <str> _FTNOT_                                          "ftnot"
%token <str> _PRIVATE_                                        "private"
%token <str> _PUBLIC_                                         "public"
%token <str> _DETERMINISTIC_                                  "deterministic"
%token <str> _NONDETERMINISTIC_                               "nondeterministic"

/* XSLT 2.0 tokens */
%token _XSLT_END_ELEMENT_                                     "<XSLT end element>"
%token _XSLT_STYLESHEET_                                      "<xsl:stylesheet..."
%token _XSLT_TEMPLATE_                                        "<xsl:template..."
%token _XSLT_VALUE_OF_                                        "<xsl:value-of..."
%token _XSLT_TEXT_                                            "<xsl:text..."
%token _XSLT_APPLY_TEMPLATES_                                 "<xsl:apply-templates..."
%token _XSLT_CALL_TEMPLATE_                                   "<xsl:call-template..."
%token _XSLT_WITH_PARAM_                                      "<xsl:with-param..."
%token _XSLT_SEQUENCE_                                        "<xsl:sequence..."
%token _XSLT_PARAM_                                           "<xsl:param..."
%token _XSLT_FUNCTION_                                        "<xsl:function..."
%token _XSLT_CHOOSE_                                          "<xsl:choose..."
%token _XSLT_WHEN_                                            "<xsl:when..."
%token _XSLT_OTHERWISE_                                       "<xsl:otherwise..."
%token _XSLT_IF_                                              "<xsl:if..."
%token _XSLT_VARIABLE_                                        "<xsl:variable..."
%token _XSLT_COMMENT_                                         "<xsl:comment..."
%token _XSLT_PI_                                              "<xsl:processing-instruction..."
%token _XSLT_DOCUMENT_                                        "<xsl:document..."
%token _XSLT_ATTRIBUTE_                                       "<xsl:attribute..."
%token _XSLT_NAMESPACE_                                       "<xsl:namespace..."
%token _XSLT_ELEMENT_                                         "<xsl:element..."
%token _XSLT_ANALYZE_STRING_                                  "<xsl:analyze-string..."
%token _XSLT_MATCHING_SUBSTRING_                              "<xsl:matching-substring..."
%token _XSLT_NON_MATCHING_SUBSTRING_                          "<xsl:non-matching-substring..."
%token _XSLT_COPY_OF_                                         "<xsl:copy-of..."
%token _XSLT_COPY_                                            "<xsl:copy..."
%token _XSLT_FOR_EACH_                                        "<xsl:for-each..."
%token _XSLT_OUTPUT_                                          "<xsl:output..."
%token _XSLT_IMPORT_SCHEMA_                                   "<xsl:import-schema..."

%token <str> _XSLT_VERSION_                                   "version='...'"
%token <str> _XSLT_MODE_                                      "mode='...'"
%token <str> _XSLT_NAME_                                      "name='...'"
%token <str> _XSLT_DOCTYPE_PUBLIC_                            "doctype-public='...'"
%token <str> _XSLT_DOCTYPE_SYSTEM_                            "doctype-system='...'"
%token <str> _XSLT_ENCODING_                                  "encoding='...'"
%token <str> _XSLT_MEDIA_TYPE_                                "media-type='...'"
%token <str> _XSLT_NORMALIZATION_FORM_                        "normalization-form='...'"
%token <str> _XSLT_STANDALONE_                                "standalone='...'"
%token <str> _XSLT_EXCLUDE_RESULT_PREFIXES_                   "exclude-result-prefixes='...'"
%token <str> _XSLT_NAMESPACE_STR_                             "namespace='...' (string)"
%token <str> _XSLT_SCHEMA_LOCATION_                           "schema-location='...'"
%token <boolean> _XSLT_TUNNEL_                                "tunnel='...'"
%token <boolean> _XSLT_REQUIRED_                              "required='...'"
%token <boolean> _XSLT_OVERRIDE_                              "override='...'"
%token <boolean> _XSLT_COPY_NAMESPACES_                       "copy-namespaces='...'"
%token <boolean> _XSLT_INHERIT_NAMESPACES_                    "inherit-namespaces='...'"
%token <boolean> _XSLT_BYTE_ORDER_MARK_                       "byte-order-mark='...'"
%token <boolean> _XSLT_ESCAPE_URI_ATTRIBUTES_                 "escape-uri-attributes='...'"
%token <boolean> _XSLT_INCLUDE_CONTENT_TYPE_                  "include-content-type='...'"
%token <boolean> _XSLT_INDENT_                                "indent='...'"
%token <boolean> _XSLT_OMIT_XML_DECLARATION_                  "omit-xml-declaration='...'"
%token <boolean> _XSLT_UNDECLARE_PREFIXES_                    "undeclare-prefixes='...'"
%token _XSLT_MATCH_                                           "match='...'"
%token _XSLT_AS_                                              "as='...'"
%token _XSLT_SELECT_                                          "select='...'"
%token _XSLT_PRIORITY_                                        "priority='...'"
%token _XSLT_TEST_                                            "test='...'"
%token _XSLT_SEPARATOR_                                       "separator='...'"
%token _XSLT_NAMESPACE_A_                                     "namespace='...' (attribute value template)"
%token _XSLT_REGEX_                                           "regex='...'"
%token _XSLT_FLAGS_                                           "flags='...'"
%token _XSLT_METHOD_                                          "method='...'"
%token _XSLT_CDATA_SECTION_ELEMENTS_                          "cdata-section-elements='...'"
%token _XSLT_USE_CHARACTER_MAPS_                              "use-character-maps='...'"

%token <astNode> _XSLT_ELEMENT_NAME_                          "<XSLT element name>"
%token <astNode> _XSLT_XMLNS_ATTR_                            "<XSLT XMLNS attr>"
%token <astNode> _XSLT_ATTR_NAME_                             "<XSLT attr name>"
%token <astNode> _XSLT_TEXT_NODE_                             "<XSLT text node>"
%token <astNode> _XSLT_WS_TEXT_NODE_                          "<XSLT whitespace text node>"

%token _HASH_DEFAULT_                                         "#default"
%token _HASH_ALL_                                             "#all"
%token _HASH_CURRENT_                                         "#current"
%token _XML_                                                  "xml"
%token _HTML_                                                 "html"
%token _XHTML_                                                "xhtml"


%type <functDecl>    FunctionDecl TemplateDecl FunctionAttrs_XSLT TemplateAttrs_XSLT
%type <globalVar>    GlobalVariableAttrs_XSLT GlobalParamAttrs_XSLT
%type <argSpec>      Param Param_XSLT ParamAttrs_XSLT
%type <argSpecs>     ParamList FunctionParamList TemplateParamList ParamList_XSLT
%type <astNode>      Expr ExprSingle OrExpr AndExpr EnclosedExpr FLWORExpr IfExpr ComparisonExpr DecimalLiteral VarRef
%type <astNode>      RangeExpr AdditiveExpr MultiplicativeExpr UnionExpr QuantifiedExpr StringLiteral Literal ContextItemExpr
%type <astNode>      UnaryExpr ValidateExpr CastExpr TreatExpr IntersectExceptExpr ParenthesizedExpr PrimaryExpr FunctionCall
%type <astNode>      RelativePathExpr StepExpr AxisStep PostfixExpr TypeswitchExpr ValueExpr PathExpr NumericLiteral IntegerLiteral 
%type <astNode>      CastableExpr Constructor ComputedConstructor DirElemConstructor DirCommentConstructor DirPIConstructor  
%type <astNode>      CompElemConstructor CompTextConstructor CompPIConstructor CompCommentConstructor OrderedExpr UnorderedExpr
%type <astNode>      CompAttrConstructor CompDocConstructor DoubleLiteral InstanceofExpr DirectConstructor 
%type <astNode>      ExtensionExpr FTContainsExpr FTIgnoreOption VarDeclValue LeadingSlash CompPINCName
%type <astNode>      InsertExpr DeleteExpr RenameExpr ReplaceExpr TransformExpr CompElementName CompAttrName
%type <astNode>      ForwardStep ReverseStep AbbrevForwardStep AbbrevReverseStep OrderExpr CompPIConstructorContent
%type <astNode>      PathPattern_XSLT IdValue_XSLT KeyValue_XSLT CallTemplateExpr ApplyTemplatesExpr
%type <astNode>      DynamicFunctionInvocation InlineFunction LiteralFunctionItem FunctionItemExpr
%type <astNode>      ForwardStepPredicateList ReverseStepPredicateList Argument FunctionBody CompNamespaceConstructor
%type <astNode>      LiteralResultElement_XSLT ValueOf_XSLT ValueOfAttrs_XSLT Text_XSLT TextNode_XSLT ApplyTemplates_XSLT
%type <astNode>      ApplyTemplatesAttrs_XSLT CallTemplate_XSLT CallTemplateAttrs_XSLT Sequence_XSLT Choose_XSLT If_XSLT
%type <astNode>      WhenList_XSLT When_XSLT Otherwise_XSLT Variable_XSLT Comment_XSLT CommentAttrs_XSLT
%type <astNode>      PI_XSLT PIAttrs_XSLT Document_XSLT DocumentAttrs_XSLT Attribute_XSLT AttributeAttrs_XSLT
%type <astNode>      AnalyzeString_XSLT AnalyzeStringAttrs_XSLT MatchingSubstring_XSLT NonMatchingSubstring_XSLT
%type <astNode>      CopyOf_XSLT CopyOfAttrs_XSLT Copy_XSLT CopyAttrs_XSLT ForEach_XSLT ForEachAttrs_XSLT Instruction_XSLT
%type <astNode>      SequenceAttrs_XSLT IfAttrs_XSLT WhenAttrs_XSLT Element_XSLT ElementAttrs_XSLT Namespace_XSLT NamespaceAttrs_XSLT
%type <astNode>      RelativePathPattern_XSLT PatternStep_XSLT IdKeyPattern_XSLT PathPatternStart_XSLT

%type <parenExpr>    SequenceConstructor_XSLT

%type <ftselection>     FTSelection FTWords FTUnaryNot FTMildnot FTAnd FTOr FTPrimaryWithOptions FTPrimary FTExtensionSelection
%type <ftoption>        FTPosFilter
%type <ftoptionlist>    FTPosFilters
%type <ftanyalloption>  FTAnyallOption
%type <ftrange>         FTRange
%type <ftunit>          FTUnit FTBigUnit

%type <itemList>        DirElementContent DirAttributeList QuotAttrValueContent AposAttrValueContent DirAttributeValue FunctionCallArgumentList
%type <itemList>        ContentExpr LiteralDirAttributeValue LiteralQuotAttrValueContent LiteralAposAttrValueContent AttrValueTemplate_XSLT Pattern_XSLT
%type <itemList>        LiteralResultElementAttrs_XSLT
%type <predicates>      PatternStepPredicateList_XSLT
%type <axis>            ForwardAxis ReverseAxis
%type <nodeTest>        NodeTest NameTest Wildcard PatternAxis_XSLT
%type <qName>           QName AtomicType TypeName ElementName AttributeName ElementNameOrWildcard AttribNameOrWildcard AttributeDeclaration ElementDeclaration
%type <sequenceType>    SequenceType TypeDeclaration SingleType TemplateSequenceType FunctionDeclReturnType
%type <sequenceTypes>   FunctionTypeArguments
%type <occurrence>      OccurrenceIndicator SingleTypeOccurrence
%type <itemType>        ItemType KindTest AttributeTest SchemaAttributeTest PITest CommentTest TextTest AnyKindTest ElementTest DocumentTest SchemaElementTest
%type <itemType>        FunctionTest AnyFunctionTest TypedFunctionTest ParenthesizedItemType NamespaceNodeTest
%type <copyBinding>     TransformBinding
%type <copyBindingList> TransformBindingList
%type <templateArg>     TemplateArgument WithParamAttrs_XSLT WithParam_XSLT
%type <templateArgs>    TemplateArgumentList ApplyTemplatesContent_XSLT CallTemplateContent_XSLT
%type <tupleNode>       ForBinding LetBinding WhereClause FLWORTuples OrderByClause OrderSpec OrderSpecList CountClause
%type <tupleNode>       ForClause LetClause ForBindingList LetBindingList QuantifyBinding QuantifyBindingList InitialClause IntermediateClause
%type <letTuple>        VariableAttrs_XSLT
%type <caseClause>      DefaultCase
%type <caseClauses>     CaseClauseList CaseClause CaseSequenceTypeUnion
%type <orderByModifier> OrderDirection EmptyHandling
%type <stringList>      ResourceLocations
%type <str>             PositionalVar SchemaPrefix URILiteral FTScoreVar DirCommentContents DirElemConstructorQName
%type <str>             FunctionName QNameValue VarName NCName DirPIContents PragmaContents Number_XSLT CaseClauseVariable

%type <modeList>        TemplateModes_XSLT TemplateDeclModesSection TemplateDeclModes
%type <mode>            ApplyTemplatesMode_XSLT ApplyTemplatesMode TemplateDeclMode
%type <signature>       FunctionOptions
%type <boolean>         PreserveMode InheritMode PrivateOption DeterministicOption

%start SelectLanguage

%pure_parser

// We're expecting 50 shift/reduce conflicts. These have been checked and are harmless.
// 48 arise from the xgs:leading-lone-slash grammar constraint (http://www.w3.org/TR/xquery/#parse-note-leading-lone-slash)
// 2 arise from the xgs:occurrence-indicator grammar constriant (http://www.w3.org/TR/xquery/#parse-note-occurence-indicators)
//%expect 50

// We're expecting 90 shift/reduce conflicts. These have been checked and are harmless.
// 49 arise from the xgs:leading-lone-slash grammar constraint (http://www.w3.org/TR/xquery/#parse-note-leading-lone-slash)
// 3 arise from the xgs:occurrence-indicator grammar constriant (http://www.w3.org/TR/xquery/#parse-note-occurence-indicators)
// 17 are due to template extensions
// 21 are due to Variable_XSLT
%expect 90

%%

// Select the language we parse, based on the (fake) first token from the lexer
SelectLanguage:
    _LANG_XPATH2_ XPathBuiltinNamespaces QueryBody
  | _LANG_XQUERY_ XQueryBuiltinNamespaces Module
  | _LANG_FUNCDECL_ XQueryBuiltinNamespaces Start_FunctionDecl
  | _LANG_XSLT2_ Start_XSLT
  | _LANG_DELAYEDMODULE_ Start_DelayedModule
  | _LANG_FUNCTION_SIGNATURE_ Start_FunctionSignature
  ;

XPathBuiltinNamespaces:
  /* empty */
  {
    SET_BUILT_IN_NAMESPACE(XQillaFunction::XQillaPrefix, XQillaFunction::XMLChFunctionURI);
  }
  ;

XQueryBuiltinNamespaces:
  /* empty */
  {
    SET_BUILT_IN_NAMESPACE(XMLChXS, SchemaSymbols::fgURI_SCHEMAFORSCHEMA);
    SET_BUILT_IN_NAMESPACE(XMLChXSI, SchemaSymbols::fgURI_XSI);
    SET_BUILT_IN_NAMESPACE(XMLChFN, XQFunction::XMLChFunctionURI);
    SET_BUILT_IN_NAMESPACE(XMLChLOCAL, XQUserFunction::XMLChXQueryLocalFunctionsURI);
    SET_BUILT_IN_NAMESPACE(XMLChERR, FunctionError::XMLChXQueryErrorURI);
    SET_BUILT_IN_NAMESPACE(XQillaFunction::XQillaPrefix, XQillaFunction::XMLChFunctionURI);
  }
  ;

Start_FunctionDecl:
    _DECLARE_ FunctionOptions FunctionKeyword FunctionName FunctionParamList FunctionDeclReturnType EnclosedExpr FunctionDecl_MaybeSemicolon
  {
    $2->argSpecs = $5;
    $2->returnType = $6;
    QP->_function = WRAP(@1, new (MEMMGR) XQUserFunction($4, $2, $7, true, MEMMGR));
  }
  ;

FunctionDecl_MaybeSemicolon: /* empty */ | Separator;

Start_FunctionSignature:
    FunctionOptions FunctionParamList FunctionDeclReturnType
  {
    $1->argSpecs = $2;
    $1->returnType = $3;
    QP->_signature = $1;
  }
  ;

Start_DelayedModule:
    DM_ModuleDecl DM_Prolog
  {
    printf("  { 0, 0, 0, 0, 0 }\n};\n\n");

    UTF8Str module(QP->_moduleName);
    printf("static const DelayedModule %s_module = { %s_file, %s_prefix, %s_uri, %s_functions };\n",
           module.str(), module.str(), module.str(), module.str(), module.str());
  }
  ;

DM_ModuleDecl:
    _MODULE_ _NAMESPACE_ NCName _EQUALS_ URILiteral Separator
  {
    QP->_moduleName = $3;

    UTF8Str module(QP->_moduleName);
    UTF8Str file(QP->_lexer->getFile());

    printf("// Auto-generated by compile-delayed-module from %s\n", file.str());
    printf("// DO NOT EDIT\n\n");

    printf("// %s\n", file.str());
    printf("static const XMLCh %s_file[] = { ", module.str());
    const XMLCh *ptr = QP->_lexer->getFile();
    for(; *ptr != 0; ++ptr) {
      printf("%d, ", *ptr);
    }
    printf("0 };\n\n");

    printf("// %s\n", UTF8($3));
    printf("static const XMLCh %s_prefix[] = { ", module.str());
    ptr = $3;
    for(; *ptr != 0; ++ptr) {
      printf("%d, ", *ptr);
    }
    printf("0 };\n\n");

    printf("// %s\n", UTF8($5));
    printf("static const XMLCh %s_uri[] = { ", module.str());
    ptr = $5;
    for(; *ptr != 0; ++ptr) {
      printf("%d, ", *ptr);
    }
    printf("0 };\n\n");
  }
  ;

DM_Prolog:
    /* empty */
  {
    printf("static const DelayedModule::FuncDef %s_functions[] = {\n", UTF8(QP->_moduleName));
  }
  | DM_Prolog DM_FunctionDecl
  {
  }
  ;

DM_FunctionDecl:
    _DECLARE_ FunctionOptions FunctionKeyword FunctionName FunctionParamList FunctionDeclReturnType EnclosedExpr Separator
  {
    XMLBuffer buf;

    const XMLCh *localname = XPath2NSUtils::getLocalName($4);

    printf("  {\n    \"%s\", %d, %s, %d, %d,\n", UTF8(localname), (int)($5 ? $5->size() : 0),
           $2->privateOption == FunctionSignature::OP_TRUE ? "true" : "false", @1.first_line, @1.first_column);
    printf("    \"");
    const XMLCh *ptr = ((XQLexer*)QP->_lexer)->getQueryString() + @1.first_offset;
    const XMLCh *start = ptr;
    const XMLCh *end = ((XQLexer*)QP->_lexer)->getQueryString() + @8.last_offset;
    for(;ptr < end; ++ptr) {
      if(*ptr == '"') {
        if((ptr - start) == 0) buf.reset();
        else buf.set(start, ptr - start);
        printf("%s\\\"", UTF8(buf.getRawBuffer()));
        start = ptr + 1;
      }
      else if(*ptr == '\n') {
        if((ptr - start) == 0) buf.reset();
        else buf.set(start, ptr - start);
        printf("%s\\n\"\n    \"", UTF8(buf.getRawBuffer()));
        start = ptr + 1;
      }
    }
    buf.set(start, ptr - start);
    printf("%s\\n\"\n", UTF8(buf.getRawBuffer()));
    printf("  },\n");
  }
  ;

////////////////////////////////////////////////////////////////////////////////////////////////////
// XSLT rules

Start_XSLT:
    Stylesheet_XSLT
  {
    SequenceType *optionalString =
      WRAP(@1, new (MEMMGR) SequenceType(SchemaSymbols::fgURI_SCHEMAFORSCHEMA,
                                         AnyAtomicType::fgDT_ANYATOMICTYPE,
                                         SequenceType::QUESTION_MARK, MEMMGR));

    XQGlobalVariable *nameVar =
      WRAP(@1, new (MEMMGR) XQGlobalVariable(0, optionalString,
                                             WRAP(@1, new (MEMMGR) XQSequence(MEMMGR)), MEMMGR, /*isParam*/true));
    nameVar->setVariableURI(XQillaFunction::XMLChFunctionURI);
    nameVar->setVariableLocalName(var_name);

    QP->_query->addVariable(nameVar);

    ASTNode *nameVarRef1 = WRAP(@1, new (MEMMGR) XQVariable(XQillaFunction::XMLChFunctionURI, var_name, MEMMGR));
    XQCallTemplate *call = WRAP(@1, new (MEMMGR) XQCallTemplate(nameVarRef1, 0, MEMMGR));

    ASTNode *ci = WRAP(@1, new (MEMMGR) XQContextItem(MEMMGR));
    ASTNode *apply = WRAP(@1, new (MEMMGR) XQApplyTemplates(ci, 0, 0, MEMMGR));

    VectorOfASTNodes *args = new (MEMMGR) VectorOfASTNodes(XQillaAllocator<ASTNode*>(MEMMGR));
    args->push_back(WRAP(@1, new (MEMMGR) XQVariable(XQillaFunction::XMLChFunctionURI, var_name, MEMMGR)));
    ASTNode *exists = WRAP(@1, new (MEMMGR) XQFunctionCall(0, XQFunction::XMLChFunctionURI,
                                                           MEMMGR->getPooledString("exists"), args, MEMMGR));

    QP->_query->setQueryBody(WRAP(@1, new (MEMMGR) XQIf(exists, call, apply, MEMMGR)));
  }
  ;

Stylesheet_XSLT:
    _XSLT_STYLESHEET_ StylesheetAttrs_XSLT StylesheetContent_XSLT _XSLT_END_ELEMENT_
  {
  }
  | LiteralResultElement_XSLT
  {
    // TBD Check for xsl:version attr - jpcs
    NodeTest *nt = new (MEMMGR) NodeTest();
    nt->setNodeType(Node::document_string);
    nt->setNameWildcard();
    nt->setNamespaceWildcard();

    VectorOfASTNodes *pattern = new (MEMMGR) VectorOfASTNodes(XQillaAllocator<ASTNode*>(MEMMGR));
    pattern->push_back(WRAP(@1, new (MEMMGR) XQStep(XQStep::SELF, nt, MEMMGR)));

    XQUserFunction::ModeList *modelist = new (MEMMGR) XQUserFunction::ModeList(XQillaAllocator<XQUserFunction::Mode*>(MEMMGR));
    modelist->push_back(WRAP(@1, new (MEMMGR) XQUserFunction::Mode(XQUserFunction::Mode::DEFAULT)));

    FunctionSignature *signature = new (MEMMGR) FunctionSignature(MEMMGR);
    XQUserFunction *func = WRAP(@1, new (MEMMGR) XQUserFunction(0, pattern, signature, $1, MEMMGR));
    func->setModeList(modelist);

    QP->_query->addFunction(func);
  }
  ;

StylesheetAttrs_XSLT:
    /* empty */
  | StylesheetAttrs_XSLT _XSLT_VERSION_
  {
    // TBD Check the value - jpcs
  }
  | StylesheetAttrs_XSLT _XSLT_EXCLUDE_RESULT_PREFIXES_
  {
    // TBD implement exclude-result-prefixes - jpcs
  }
  // TBD the rest of the attrs - jpcs
  ;

StylesheetContent_XSLT:
    /* empty */
  | StylesheetContent_XSLT Template_XSLT
  | StylesheetContent_XSLT Function_XSLT
  | StylesheetContent_XSLT GlobalParam_XSLT
  | StylesheetContent_XSLT GlobalVariable_XSLT
  | StylesheetContent_XSLT Output_XSLT
  | StylesheetContent_XSLT ImportSchema_XSLT
  ;

Template_XSLT:
    TemplateAttrs_XSLT ParamList_XSLT SequenceConstructor_XSLT _XSLT_END_ELEMENT_
  {
    if($1->getName() == 0 && $1->getPattern() == 0) {
      yyerror(@1, "The xsl:template declaration does not have either a {}name or {}match attribute, or both [err:XTSE0500]");
    }

    if($1->getPattern() != 0 && $1->getModeList() == 0) {
      XQUserFunction::ModeList *modelist = new (MEMMGR) XQUserFunction::ModeList(XQillaAllocator<XQUserFunction::Mode*>(MEMMGR));
      modelist->push_back(WRAP(@1, new (MEMMGR) XQUserFunction::Mode(XQUserFunction::Mode::DEFAULT)));
      $1->setModeList(modelist);
    }

    $1->getSignature()->argSpecs = $2;
    $1->setFunctionBody($3);
    QP->_query->addFunction($1);
  }
  ;

TemplateAttrs_XSLT:
    _XSLT_TEMPLATE_
  {
    FunctionSignature *signature = new (MEMMGR) FunctionSignature(MEMMGR);
    $$ = WRAP(@1, new (MEMMGR) XQUserFunction(0, 0, signature, 0, MEMMGR));
  }
  | TemplateAttrs_XSLT _XSLT_MATCH_ Pattern_XSLT
  {
    $$ = $1;
    $$->setPattern($3);
  }
  | TemplateAttrs_XSLT _XSLT_NAME_
  {
    $$ = $1;
    RESOLVE_QNAME(@2, $2);
    $$->setURI(uri);
    $$->setName(name);
  }
  | TemplateAttrs_XSLT _XSLT_PRIORITY_ Number_XSLT
  {
    $$ = $1;
    // TBD priority - jpcs
  }
  | TemplateAttrs_XSLT _XSLT_MODE_ TemplateModes_XSLT
  {
    $$ = $1;
    $$->setModeList($3);
  }
  | TemplateAttrs_XSLT _XSLT_AS_ SequenceType
  {
    $$ = $1;
    $$->getSignature()->returnType = $3;
  }
  ;

TemplateModes_XSLT:
    /* empty */
  {
    $$ = new (MEMMGR) XQUserFunction::ModeList(XQillaAllocator<XQUserFunction::Mode*>(MEMMGR));
  }
  | TemplateModes_XSLT _QNAME_
  {
    RESOLVE_QNAME(@2, $2);
    $1->push_back(WRAP(@2, new (MEMMGR) XQUserFunction::Mode(uri, name)));
    $$ = $1;
  }
  | TemplateModes_XSLT _HASH_DEFAULT_
  {
    $1->push_back(WRAP(@2, new (MEMMGR) XQUserFunction::Mode(XQUserFunction::Mode::DEFAULT)));
    $$ = $1;
  }
  | TemplateModes_XSLT _HASH_ALL_
  {
    $1->push_back(WRAP(@2, new (MEMMGR) XQUserFunction::Mode(XQUserFunction::Mode::ALL)));
    $$ = $1;
  }
  ;

Number_XSLT: _INTEGER_LITERAL_ | _DECIMAL_LITERAL_ | _DOUBLE_LITERAL_;

Function_XSLT:
    FunctionAttrs_XSLT ParamList_XSLT SequenceConstructor_XSLT _XSLT_END_ELEMENT_
  {
    if($1->getName() == 0) {
      yyerror(@1, "The xsl:function declaration does not have a {}name attribute");
    }

    $1->getSignature()->argSpecs = $2;
    $1->setFunctionBody($3);
    QP->_query->addFunction($1);
  }
  ;

FunctionAttrs_XSLT:
    _XSLT_FUNCTION_
  {    
    FunctionSignature *signature = new (MEMMGR) FunctionSignature(MEMMGR);
    $$ = WRAP(@1, new (MEMMGR) XQUserFunction(0, signature, 0, true, MEMMGR));
  }
  | FunctionAttrs_XSLT _XSLT_NAME_
  {
    $$ = $1;
    RESOLVE_QNAME(@2, $2);
    $$->setURI(uri);
    $$->setName(name);
  }
  | FunctionAttrs_XSLT _XSLT_AS_ SequenceType
  {
    $$ = $1;
    $$->getSignature()->returnType = $3;
  }
  | FunctionAttrs_XSLT _XSLT_OVERRIDE_
  {
    $$ = $1;
    // TBD override - jpcs
  }
  ;

ParamList_XSLT:
    /* empty */
  {
    $$ = new (MEMMGR) ArgumentSpecs(XQillaAllocator<ArgumentSpec*>(MEMMGR));
  }
  | ParamList_XSLT Param_XSLT
  {
    $$ = $1;
    $$->push_back($2);
  }
  ;

Param_XSLT:
    ParamAttrs_XSLT SequenceConstructor_XSLT _XSLT_END_ELEMENT_
  {
    $$ = $1;

    if($$->getName() == 0) {
      yyerror(@1, "The xsl:param instruction does not have a {}name attribute");
    }

    if($$->getType() == 0) {
      $$->setType(WRAP(@1, new (MEMMGR) SequenceType(new (MEMMGR) SequenceType::ItemType(SequenceType::ItemType::TEST_ANYTHING), SequenceType::STAR)));
    }

    // TBD default parameter values - jpcs

/*     if(!$2->getChildren().empty()) { */
/*       if($$->value != 0) { */
/*         yyerror(@1, "The xsl:with-param instruction has both a select attribute and a sequence constructor [err:XTSE0870]"); */
/*       } */
/*       $$->value = $2; */
/*     } */
/*     else if($$->value == 0) { */
/*       yyerror(@1, "The xsl:with-param instruction has neither a select attribute nor a sequence constructor [err:XTSE0870]"); */
/*     } */
  }
  ;

ParamAttrs_XSLT:
    _XSLT_PARAM_
  {
    $$ = WRAP(@1, new (MEMMGR) ArgumentSpec(0, 0, MEMMGR));
  }
  | ParamAttrs_XSLT _XSLT_NAME_
  {
    $$ = $1;
    RESOLVE_QNAME(@2, $2);
    $$->setURI(uri);
    $$->setName(name);
  }
  | ParamAttrs_XSLT _XSLT_SELECT_ Expr
  {
    $$ = $1;
    // TBD default parameter values - jpcs
/*     $$->value = PRESERVE_NS(@2, $3); */
  }
  | ParamAttrs_XSLT _XSLT_AS_ SequenceType
  {
    $$ = $1;
    $$->setType($3);
  }
  | ParamAttrs_XSLT _XSLT_REQUIRED_
  {
    $$ = $1;
    // TBD required - jpcs
  }
  | ParamAttrs_XSLT _XSLT_TUNNEL_
  {
    $$ = $1;
    // TBD tunnel parameters - jpcs
  }
  ;


GlobalParam_XSLT:
    GlobalParamAttrs_XSLT SequenceConstructor_XSLT _XSLT_END_ELEMENT_
  {
    if($1->isRequired()) {
      if($1->getVariableExpr() != 0 || !$2->getChildren().empty()) {
        yyerror(@1, "A required xsl:param declaration must be empty and must not specify a {}select attribute");
      }
    }
    else {
      $1->setVariableExpr(XSLT_VARIABLE_VALUE(@1, (ASTNode*)$1->getVariableExpr(), $2, (SequenceType*)$1->getSequenceType()));

      if($1->getVariableExpr() == 0) {
        yyerror(@1, "The xsl:param declaration has both a select attribute and a sequence constructor [err:XTSE0620]");
      }
    }

    if($1->getVariableLocalName() == 0) {
      yyerror(@1, "The xsl:param declaration does not have a {}name attribute");
    }

    QP->_query->addVariable($1);
  }
  ;

GlobalParamAttrs_XSLT:
    _XSLT_PARAM_
  {
    $$ = WRAP(@1, new (MEMMGR) XQGlobalVariable(0, 0, 0, MEMMGR, /*isParam*/true));
  }
  | GlobalParamAttrs_XSLT _XSLT_NAME_
  {
    RESOLVE_QNAME(@2, $2);
    $1->setVariableURI(uri);
    $1->setVariableLocalName(name);
    $$ = $1;
  }
  | GlobalParamAttrs_XSLT _XSLT_SELECT_ Expr
  {
    $1->setVariableExpr(PRESERVE_NS(@2, $3));
    $$ = $1;
  }
  | GlobalParamAttrs_XSLT _XSLT_AS_ SequenceType
  {
    $1->setSequenceType($3);
    $$ = $1;
  }
  | GlobalParamAttrs_XSLT _XSLT_REQUIRED_
  {
    $$ = $1;
    $$->setRequired($2);
  }
  | GlobalParamAttrs_XSLT _XSLT_TUNNEL_
  {
    $$ = $1;
    if($2) {
      yyerror(@2, "An xsl:param declaration cannot have a {}tunnel attribute with a value of \"yes\"");
    }
  }
  ;


GlobalVariable_XSLT:
    GlobalVariableAttrs_XSLT SequenceConstructor_XSLT _XSLT_END_ELEMENT_
  {
    $1->setVariableExpr(XSLT_VARIABLE_VALUE(@1, (ASTNode*)$1->getVariableExpr(), $2, (SequenceType*)$1->getSequenceType()));
    $1->setSequenceType(0);

    if($1->getVariableExpr() == 0) {
      yyerror(@1, "The xsl:variable declaration has both a select attribute and a sequence constructor [err:XTSE0620]");
    }

    if($1->getVariableLocalName() == 0) {
      yyerror(@1, "The xsl:variable declaration does not have a {}name attribute");
    }

    QP->_query->addVariable($1);
  }
  ;

GlobalVariableAttrs_XSLT:
    _XSLT_VARIABLE_
  {
    $$ = WRAP(@1, new (MEMMGR) XQGlobalVariable(0, 0, 0, MEMMGR));
  }
  | GlobalVariableAttrs_XSLT _XSLT_NAME_
  {
    RESOLVE_QNAME(@2, $2);
    $1->setVariableURI(uri);
    $1->setVariableLocalName(name);
    $$ = $1;
  }
  | GlobalVariableAttrs_XSLT _XSLT_SELECT_ Expr
  {
    $1->setVariableExpr(PRESERVE_NS(@2, $3));
    $$ = $1;
  }
  | GlobalVariableAttrs_XSLT _XSLT_AS_ SequenceType
  {
    $1->setSequenceType($3);
    $$ = $1;
  }
  ;

Output_XSLT:
    OutputAttrs_XSLT _XSLT_END_ELEMENT_
  {
    // TBD Add the output to the static context - jpcs
  }
  ;

// <xsl:output
//   name? = qname
//   method? = "xml" | "html" | "xhtml" | "text" | qname-but-not-ncname
//   byte-order-mark? = "yes" | "no"
//   cdata-section-elements? = qnames
//   doctype-public? = string
//   doctype-system? = string
//   encoding? = string
//   escape-uri-attributes? = "yes" | "no"
//   include-content-type? = "yes" | "no"
//   indent? = "yes" | "no"
//   media-type? = string
//   normalization-form? = "NFC" | "NFD" | "NFKC" | "NFKD" | "fully-normalized" | "none" | nmtoken
//   omit-xml-declaration? = "yes" | "no"
//   standalone? = "yes" | "no" | "omit"
//   undeclare-prefixes? = "yes" | "no"
//   use-character-maps? = qnames
//   version? = nmtoken />
OutputAttrs_XSLT:
    _XSLT_OUTPUT_
  {
  }
  | OutputAttrs_XSLT _XSLT_NAME_
  {
  }
  | OutputAttrs_XSLT _XSLT_METHOD_ OutputMethod_XSLT
  {
  }
  | OutputAttrs_XSLT _XSLT_BYTE_ORDER_MARK_
  {
  }
  | OutputAttrs_XSLT _XSLT_CDATA_SECTION_ELEMENTS_ QNames_XSLT
  {
  }
  | OutputAttrs_XSLT _XSLT_DOCTYPE_PUBLIC_
  {
  }
  | OutputAttrs_XSLT _XSLT_DOCTYPE_SYSTEM_
  {
  }
  | OutputAttrs_XSLT _XSLT_ENCODING_
  {
  }
  | OutputAttrs_XSLT _XSLT_ESCAPE_URI_ATTRIBUTES_
  {
  }
  | OutputAttrs_XSLT _XSLT_INCLUDE_CONTENT_TYPE_
  {
  }
  | OutputAttrs_XSLT _XSLT_INDENT_
  {
  }
  | OutputAttrs_XSLT _XSLT_MEDIA_TYPE_
  {
  }
  | OutputAttrs_XSLT _XSLT_NORMALIZATION_FORM_
  {
  }
  | OutputAttrs_XSLT _XSLT_OMIT_XML_DECLARATION_
  {
  }
  | OutputAttrs_XSLT _XSLT_STANDALONE_
  {
  }
  | OutputAttrs_XSLT _XSLT_UNDECLARE_PREFIXES_
  {
  }
  | OutputAttrs_XSLT _XSLT_USE_CHARACTER_MAPS_ QNames_XSLT
  {
  }
  | OutputAttrs_XSLT _XSLT_VERSION_
  {
  }
  ;

OutputMethod_XSLT:
    _XML_
  | _HTML_
  | _XHTML_
  | _TEXT_
  | _QNAME_
  {
//     $$ = $1;
    const XMLCh *p = $1;
    while(*p && *p != ':') ++p;
    if(*p == 0) {
      yyerror(@1, "The method for the xsl:output declaration does not have a prefix");
    }
  }
  ;

QNames_XSLT:
    _QNAME_
  {
  }
  | QNames_XSLT _QNAME_
  {
  }
  ;

ImportSchema_XSLT: ImportSchemaAttrs_XSLT _XSLT_END_ELEMENT_;

ImportSchemaAttrs_XSLT:
    _XSLT_IMPORT_SCHEMA_
  {
    LOCATION(@1, loc);
    CONTEXT->addSchemaLocation(XMLUni::fgZeroLenString, 0, &loc);
  }
  | _XSLT_IMPORT_SCHEMA_ _XSLT_NAMESPACE_STR_
  {
    LOCATION(@1, loc);
    CONTEXT->addSchemaLocation($2, 0, &loc);
  }
  | _XSLT_IMPORT_SCHEMA_ _XSLT_SCHEMA_LOCATION_
  {
    VectorOfStrings schemaLoc(XQillaAllocator<const XMLCh*>(MEMMGR));
    schemaLoc.push_back($2);

    LOCATION(@1, loc);
    CONTEXT->addSchemaLocation(XMLUni::fgZeroLenString, &schemaLoc, &loc);
  }
  | _XSLT_IMPORT_SCHEMA_ _XSLT_NAMESPACE_STR_ _XSLT_SCHEMA_LOCATION_
  {
    VectorOfStrings schemaLoc(XQillaAllocator<const XMLCh*>(MEMMGR));
    schemaLoc.push_back($3);

    LOCATION(@1, loc);
    CONTEXT->addSchemaLocation($2, &schemaLoc, &loc);
  }
  | _XSLT_IMPORT_SCHEMA_ _XSLT_SCHEMA_LOCATION_ _XSLT_NAMESPACE_STR_
  {
    VectorOfStrings schemaLoc(XQillaAllocator<const XMLCh*>(MEMMGR));
    schemaLoc.push_back($2);

    LOCATION(@1, loc);
    CONTEXT->addSchemaLocation($3, &schemaLoc, &loc);
  }
  ;

LiteralResultElement_XSLT:
    _XSLT_ELEMENT_NAME_ LiteralResultElementAttrs_XSLT SequenceConstructor_XSLT _XSLT_END_ELEMENT_
  {
    VectorOfASTNodes *children = new (MEMMGR) VectorOfASTNodes(XQillaAllocator<ASTNode*>(MEMMGR));
    *children = $3->getChildren();

    $$ = WRAP(@1, new (MEMMGR) XQElementConstructor($1, $2, children, MEMMGR));
  }
  ;

LiteralResultElementAttrs_XSLT:
    /* empty */
  {
    $$ = new (MEMMGR) VectorOfASTNodes(XQillaAllocator<ASTNode*>(MEMMGR)); 
  }
  | LiteralResultElementAttrs_XSLT _XSLT_ATTR_NAME_ AttrValueTemplate_XSLT
  {
    $$ = $1;

    ASTNode *attrItem = WRAP(@2, new (MEMMGR) XQAttributeConstructor($2, $3, MEMMGR));
    $$->push_back(attrItem);
  }
  | LiteralResultElementAttrs_XSLT _XSLT_XMLNS_ATTR_
  {
    $$ = $1;
    $$->insert($$->begin(), $2);
  }
  /* TBD xsl:use-attribute-sets - jpcs */
  ;

AttrValueTemplate_XSLT:
  /* empty */
  { 
    $$ = new (MEMMGR) VectorOfASTNodes(XQillaAllocator<ASTNode*>(MEMMGR));
  }
  | AttrValueTemplate_XSLT EnclosedExpr
  {
    $$ = $1;
    $$->push_back(PRESERVE_NS(@2, $2));
  }
  | AttrValueTemplate_XSLT _QUOT_ATTR_CONTENT_
  {
    $$ = $1;
    $$->push_back(WRAP(@2, new (MEMMGR) XQLiteral(SchemaSymbols::fgURI_SCHEMAFORSCHEMA,
                               SchemaSymbols::fgDT_STRING,
                               $2, AnyAtomicType::STRING, MEMMGR)));
  }
  ;

SequenceConstructor_XSLT:
    /* empty */
  {
    $$ = WRAP(@$, new (MEMMGR) XQSequence(MEMMGR));
  }
  | SequenceConstructor_XSLT TextNode_XSLT
  {
    $$ = $1;
    $$->addItem(WRAP(@1, new (MEMMGR) XQTextConstructor($2, MEMMGR)));
  }
  | SequenceConstructor_XSLT LiteralResultElement_XSLT
  {
    $$ = $1;
    $$->addItem($2);
  }
  | SequenceConstructor_XSLT Instruction_XSLT
  {
    $$ = $1;
    $$->addItem($2);
  }
  ;

Instruction_XSLT:
    ValueOf_XSLT
  | Text_XSLT
  | ApplyTemplates_XSLT
  | CallTemplate_XSLT
  | Sequence_XSLT
  | Choose_XSLT
  | If_XSLT
  | Variable_XSLT
  | Comment_XSLT
  | PI_XSLT
  | Document_XSLT
  | Attribute_XSLT
  | Namespace_XSLT
  | Element_XSLT
  | AnalyzeString_XSLT
  | CopyOf_XSLT
  | Copy_XSLT
  | ForEach_XSLT
  ;

ValueOf_XSLT:
    ValueOfAttrs_XSLT SequenceConstructor_XSLT _XSLT_END_ELEMENT_
  {
    XQTextConstructor *text = (XQTextConstructor*)$1;
    $$ = text;

    if(!$2->getChildren().empty()) {
      if(text->getValue() != 0) {
        yyerror(@1, "The xsl:value-of instruction has both a select attribute and a sequence constructor [err:XTSE0870]");
      }
      text->setValue($2);
    }
    else if(text->getValue() == 0) {
      yyerror(@1, "The xsl:value-of instruction has neither a select attribute nor a sequence constructor [err:XTSE0870]");
    }
  }
  ;

ValueOfAttrs_XSLT:
    _XSLT_VALUE_OF_
  {
    $$ = WRAP(@1, new (MEMMGR) XQTextConstructor(0, MEMMGR));
  }
  | ValueOfAttrs_XSLT _XSLT_SELECT_ Expr
  {
    $$ = $1;
    ((XQTextConstructor*)$$)->setValue(PRESERVE_NS(@2, $3));
  }
  | ValueOfAttrs_XSLT _XSLT_SEPARATOR_ AttrValueTemplate_XSLT
  {
    $$ = $1;
    // TBD separator - jpcs
/*     ((XQTextConstructor*)$$)->setValue(PRESERVE_NS(@2, $3)); */
  }
  ;

Text_XSLT:
    _XSLT_TEXT_ TextNode_XSLT _XSLT_END_ELEMENT_
  {
    $$ = WRAP(@1, new (MEMMGR) XQTextConstructor($2, MEMMGR));
  }
  ;

TextNode_XSLT: _XSLT_TEXT_NODE_ | _XSLT_WS_TEXT_NODE_;

ApplyTemplates_XSLT:
    ApplyTemplatesAttrs_XSLT ApplyTemplatesContent_XSLT _XSLT_END_ELEMENT_
  {
    // TBD xsl:sort - jpcs
    XQApplyTemplates *apply = (XQApplyTemplates*)$1;
    $$ = apply;

    apply->setArguments($2);

    if(apply->getExpression() == 0) {
      NodeTest *nt = new (MEMMGR) NodeTest();
      nt->setTypeWildcard();
      nt->setNameWildcard();
      nt->setNamespaceWildcard();

      apply->setExpression(WRAP(@1, new (MEMMGR) XQStep(XQStep::CHILD, nt, MEMMGR)));
    }
  }
  ;

ApplyTemplatesAttrs_XSLT:
    _XSLT_APPLY_TEMPLATES_
  {
    $$ = WRAP(@1, new (MEMMGR) XQApplyTemplates(0, 0, 0, MEMMGR));
  }
  | ApplyTemplatesAttrs_XSLT _XSLT_SELECT_ Expr
  {
    $$ = $1;
    ((XQApplyTemplates*)$$)->setExpression(PRESERVE_NS(@2, $3));
  }
  | ApplyTemplatesAttrs_XSLT _XSLT_MODE_ ApplyTemplatesMode_XSLT
  {
    $$ = $1;
    ((XQApplyTemplates*)$$)->setMode($3);
  }
  ;

ApplyTemplatesMode_XSLT:
    _QNAME_
  {
    RESOLVE_QNAME(@1, $1);
    $$ = WRAP(@1, new (MEMMGR) XQUserFunction::Mode(uri, name));
  }
  | _HASH_DEFAULT_
  {
    $$ = WRAP(@1, new (MEMMGR) XQUserFunction::Mode(XQUserFunction::Mode::DEFAULT));
  }
  | _HASH_CURRENT_
  {
    $$ = WRAP(@1, new (MEMMGR) XQUserFunction::Mode(XQUserFunction::Mode::CURRENT));
  }
  ;

ApplyTemplatesContent_XSLT:
    /* empty */
  {
    $$ = new (MEMMGR) TemplateArguments(XQillaAllocator<XQTemplateArgument*>(MEMMGR));
  }
  | ApplyTemplatesContent_XSLT WithParam_XSLT
  {
    $$ = $1;
    $$->push_back($2);
  }
  ;

CallTemplate_XSLT:
    CallTemplateAttrs_XSLT CallTemplateContent_XSLT _XSLT_END_ELEMENT_
  {
    XQCallTemplate *call = (XQCallTemplate*)$1;
    call->setArguments($2);
    $$ = call;

    if(call->getName() == 0) {
      yyerror(@1, "The xsl:call-template instruction does not have a {}name attribute");
    }
  }
  ;

CallTemplateAttrs_XSLT:
    _XSLT_CALL_TEMPLATE_
  {
    $$ = WRAP(@1, new (MEMMGR) XQCallTemplate((const XMLCh*)0, 0, MEMMGR));
  }
  | CallTemplateAttrs_XSLT _XSLT_NAME_
  {
    $$ = $1;
    RESOLVE_QNAME(@2, $2);
    ((XQCallTemplate*)$$)->setURI(uri);
    ((XQCallTemplate*)$$)->setName(name);
  }
  ;

CallTemplateContent_XSLT:
    /* empty */
  {
    $$ = new (MEMMGR) TemplateArguments(XQillaAllocator<XQTemplateArgument*>(MEMMGR));
  }
  | CallTemplateContent_XSLT WithParam_XSLT
  {
    $$ = $1;
    $$->push_back($2);
  }
  ;

WithParam_XSLT:
    WithParamAttrs_XSLT SequenceConstructor_XSLT _XSLT_END_ELEMENT_
  {
    $$ = $1;

    if($$->name == 0) {
      yyerror(@1, "The xsl:with-param instruction does not have a {}name attribute");
    }

    $1->value = XSLT_VARIABLE_VALUE(@1, $1->value, $2, $1->seqType);
    $1->seqType = 0;

    if($1->value == 0) {
      yyerror(@1, "The xsl:with-param instruction has both a select attribute and a sequence constructor [err:XTSE0870]");
    }
  }
  ;

WithParamAttrs_XSLT:
    _XSLT_WITH_PARAM_
  {
    $$ = WRAP(@1, new (MEMMGR) XQTemplateArgument(0, 0, MEMMGR));
  }
  | WithParamAttrs_XSLT _XSLT_NAME_
  {
    $$ = $1;
    RESOLVE_QNAME(@2, $2);
    $$->uri = uri;
    $$->name = name;
  }
  | WithParamAttrs_XSLT _XSLT_SELECT_ Expr
  {
    $$ = $1;
    $$->value = PRESERVE_NS(@2, $3);
  }
  | WithParamAttrs_XSLT _XSLT_AS_ SequenceType
  {
    $$ = $1;
    $$->seqType = $3;
  }
  | WithParamAttrs_XSLT _XSLT_TUNNEL_
  {
    $$ = $1;
    // TBD tunnel parameters - jpcs
  }
  ;

Sequence_XSLT:
    SequenceAttrs_XSLT _XSLT_END_ELEMENT_
  {
    $$ = $1;
  }

SequenceAttrs_XSLT:
    _XSLT_SEQUENCE_ _XSLT_SELECT_ Expr
  {
    // TBD xsl:fallback - jpcs
    $$ = PRESERVE_NS(@2, $3);
  }
  ;

If_XSLT:
    IfAttrs_XSLT SequenceConstructor_XSLT _XSLT_END_ELEMENT_
  {
    ASTNode *empty = WRAP(@1, new (MEMMGR) XQSequence(MEMMGR));
    $$ = WRAP(@1, new (MEMMGR) XQIf($1, $2, empty, MEMMGR));
  }
  ;

IfAttrs_XSLT:
    _XSLT_IF_ _XSLT_TEST_ Expr
  {
    $$ = PRESERVE_NS(@2, $3);
  }
  ;

Choose_XSLT:
    _XSLT_CHOOSE_ WhenList_XSLT Otherwise_XSLT _XSLT_END_ELEMENT_
  {
    XQIf *iff = (XQIf*)$2;
    while(iff->getWhenFalse() != 0) {
      iff = (XQIf*)iff->getWhenFalse();
    }

    iff->setWhenFalse($3);

    $$ = $2;
  }
  ;

WhenList_XSLT:
    When_XSLT
  {
    $$ = $1;
  }
  | WhenList_XSLT When_XSLT
  {
    XQIf *iff = (XQIf*)$1;
    while(iff->getWhenFalse() != 0) {
      iff = (XQIf*)iff->getWhenFalse();
    }

    iff->setWhenFalse($2);

    $$ = $1;
  }
  ;

When_XSLT:
    WhenAttrs_XSLT SequenceConstructor_XSLT _XSLT_END_ELEMENT_
  {
    XQIf *iff = (XQIf*)$1;
    iff->setWhenTrue($2);
    $$ = $1;
  }
  ;

WhenAttrs_XSLT:
    _XSLT_WHEN_ _XSLT_TEST_ Expr
  {
    $$ = WRAP(@1, new (MEMMGR) XQIf(PRESERVE_NS(@2, $3), 0, 0, MEMMGR));
  }
  ;

Otherwise_XSLT:
    /* empty */
  {
    $$ = WRAP(@$, new (MEMMGR) XQSequence(MEMMGR));
  }
  | _XSLT_OTHERWISE_ SequenceConstructor_XSLT _XSLT_END_ELEMENT_
  {
    $$ = $2;
  }
  ;

AnalyzeString_XSLT:
    AnalyzeStringAttrs_XSLT MatchingSubstring_XSLT NonMatchingSubstring_XSLT _XSLT_END_ELEMENT_
  {
    // TBD xsl:fallback - jpcs

    XQAnalyzeString *as = (XQAnalyzeString*)$$;
    $$ = $1;

    if(as->getExpression() == 0) {
      yyerror(@1, "The xsl:analyze-string instruction does not have a {}select attribute");
    }

    if(as->getRegex() == 0) {
      yyerror(@1, "The xsl:analyze-string instruction does not have a {}regex attribute");
    }

    if($2 == 0) {
      if($3 == 0) {
        yyerror(@1, "The xsl:analyze-string instruction doesn't contain an xsl:matching-substring or xsl:non-matching-substring child [err:XTSE1130]");
      }

      as->setMatch(WRAP(@1, new (MEMMGR) XQSequence(MEMMGR)));
    }
    else {
      as->setMatch($2);
    }
    if($3 == 0) {
      as->setNonMatch(WRAP(@1, new (MEMMGR) XQSequence(MEMMGR)));
    }
    else {
      as->setNonMatch($3);
    }
  }
  ;

AnalyzeStringAttrs_XSLT:
    _XSLT_ANALYZE_STRING_
  {
    $$ = WRAP(@$, new (MEMMGR) XQAnalyzeString(MEMMGR));
  }
  | AnalyzeStringAttrs_XSLT _XSLT_SELECT_ Expr
  {
    $$ = $1;
    ((XQAnalyzeString*)$$)->setExpression(PRESERVE_NS(@2, $3));
  }
  | AnalyzeStringAttrs_XSLT _XSLT_REGEX_ AttrValueTemplate_XSLT
  {
    $$ = $1;
    ASTNode *content = WRAP(@3, new (MEMMGR) XQSimpleContent($3, MEMMGR));
    ((XQAnalyzeString*)$$)->setRegex(PRESERVE_NS(@2, content));
  }
  | AnalyzeStringAttrs_XSLT _XSLT_FLAGS_ AttrValueTemplate_XSLT
  {
    $$ = $1;
    ASTNode *content = WRAP(@3, new (MEMMGR) XQSimpleContent($3, MEMMGR));
    ((XQAnalyzeString*)$$)->setFlags(PRESERVE_NS(@2, content));
  }
  ;

MatchingSubstring_XSLT:
    /* empty */
  {
    $$ = 0;
  }
  | _XSLT_MATCHING_SUBSTRING_ SequenceConstructor_XSLT _XSLT_END_ELEMENT_
  {
    $$ = $2;
  }
  ;

NonMatchingSubstring_XSLT:
    /* empty */
  {
    $$ = 0;
  }
  | _XSLT_NON_MATCHING_SUBSTRING_ SequenceConstructor_XSLT _XSLT_END_ELEMENT_
  {
    $$ = $2;
  }
  ;

Variable_XSLT:
    VariableAttrs_XSLT SequenceConstructor_XSLT _XSLT_END_ELEMENT_ SequenceConstructor_XSLT
  {
    $1->setExpression(XSLT_VARIABLE_VALUE(@1, $1->getExpression(), $2, $1->seqType));
    $1->seqType = 0;

    if($1->getExpression() == 0) {
      yyerror(@1, "The xsl:variable instruction has both a select attribute and a sequence constructor [err:XTSE0620]");
    }

    if($1->getVarName() == 0) {
      yyerror(@1, "The xsl:variable instruction does not have a {}name attribute");
    }

    $$ = WRAP(@1, new (MEMMGR) XQReturn($1, $4, MEMMGR));
  }
  ;

VariableAttrs_XSLT:
    _XSLT_VARIABLE_
  {
    $$ = WRAP(@1, new (MEMMGR) LetTuple(WRAP(@1, new (MEMMGR) ContextTuple(MEMMGR)), 0, 0, MEMMGR));
  }
  | VariableAttrs_XSLT _XSLT_NAME_
  {
    RESOLVE_QNAME(@2, $2);
    $1->setVarURI(uri);
    $1->setVarName(name);
    $$ = $1;
  }
  | VariableAttrs_XSLT _XSLT_SELECT_ Expr
  {
    $1->setExpression($3);
    $$ = $1;
  }
  | VariableAttrs_XSLT _XSLT_AS_ SequenceType
  {
    $1->seqType = $3;
    $$ = $1;
  }
  ;

Comment_XSLT:
    CommentAttrs_XSLT SequenceConstructor_XSLT _XSLT_END_ELEMENT_
  {
    XQCommentConstructor *comment = (XQCommentConstructor*)$1;
    $$ = comment;

    if(!$2->getChildren().empty()) {
      if(comment->getValue() != 0) {
        yyerror(@1, "The xsl:comment instruction has both a select attribute and a sequence constructor [err:XTSE0940]");
      }
      comment->setValue($2);
    }
    else if(comment->getValue() == 0) {
      comment->setValue(WRAP(@1, new (MEMMGR) XQSequence(MEMMGR)));
    }
  }
  ;

CommentAttrs_XSLT:
    _XSLT_COMMENT_
  {
    $$ = WRAP(@1, new (MEMMGR) XQCommentConstructor(0, MEMMGR, /*xslt*/true));
  }
  | CommentAttrs_XSLT _XSLT_SELECT_ Expr
  {
    $$ = $1;
    ((XQCommentConstructor*)$$)->setValue(PRESERVE_NS(@2, $3));
  }
  ;

PI_XSLT:
    PIAttrs_XSLT SequenceConstructor_XSLT _XSLT_END_ELEMENT_
  {
    XQPIConstructor *pi = (XQPIConstructor*)$1;
    $$ = pi;

    if(pi->getName() == 0) {
      yyerror(@1, "The xsl:processing-instruction instruction does not have a {}name attribute");
    }

    if(!$2->getChildren().empty()) {
      if(pi->getValue() != 0) {
        yyerror(@1, "The xsl:processing-instruction instruction has both a select attribute and a sequence constructor [err:XTSE0880]");
      }
      pi->setValue($2);
    }
    else if(pi->getValue() == 0) {
      pi->setValue(WRAP(@1, new (MEMMGR) XQSequence(MEMMGR)));
    }
  }
  ;

PIAttrs_XSLT:
    _XSLT_PI_
  {
    $$ = WRAP(@1, new (MEMMGR) XQPIConstructor(0, 0, MEMMGR, /*xslt*/true));
  }
  | PIAttrs_XSLT _XSLT_NAME_ AttrValueTemplate_XSLT
  {
    $$ = $1;

    ASTNode *content = WRAP(@3, new (MEMMGR) XQSimpleContent($3, MEMMGR));
    ((XQPIConstructor*)$$)->setName(PRESERVE_NS(@2, content));
  }
  | PIAttrs_XSLT _XSLT_SELECT_ Expr
  {
    $$ = $1;
    ((XQPIConstructor*)$$)->setValue(PRESERVE_NS(@2, $3));
  }
  ;

Document_XSLT:
    DocumentAttrs_XSLT SequenceConstructor_XSLT _XSLT_END_ELEMENT_
  {
    XQDocumentConstructor *doc = (XQDocumentConstructor*)$1;
    doc->setValue($2);
    $$ = doc;
  }
  ;

DocumentAttrs_XSLT:
    _XSLT_DOCUMENT_
  {
    $$ = WRAP(@1, new (MEMMGR) XQDocumentConstructor(0, MEMMGR));
  }
  // TBD validation attrs - jpcs
  ;

Attribute_XSLT:
    AttributeAttrs_XSLT SequenceConstructor_XSLT _XSLT_END_ELEMENT_
  {
    XQAttributeConstructor *attr = (XQAttributeConstructor*)$1;
    $$ = attr;

    if(attr->getName() == 0) {
      yyerror(@1, "The xsl:attribute instruction does not have a {}name attribute");
    }

    if(attr->namespaceExpr != 0) {
      // Use fn:QName() to assign the correct URI
      VectorOfASTNodes args(XQillaAllocator<ASTNode*>(MEMMGR));
      args.push_back(attr->namespaceExpr);
      args.push_back(const_cast<ASTNode*>(attr->getName()));
      FunctionQName *name = WRAP(@1, new (MEMMGR) FunctionQName(args, MEMMGR));
      attr->setName(name);
      attr->namespaceExpr = 0;
    }

    if(!$2->getChildren().empty()) {
      if(attr->getChildren() != 0 && !attr->getChildren()->empty()) {
        yyerror(@1, "The xsl:attribute instruction has both a select attribute and a sequence constructor [err:XTSE0840]");
      }

      VectorOfASTNodes *children = new (MEMMGR) VectorOfASTNodes(XQillaAllocator<ASTNode*>(MEMMGR));
      *children = $2->getChildren();
      attr->setChildren(children);
    }
    else if(attr->getChildren() == 0) {
      VectorOfASTNodes *children = new (MEMMGR) VectorOfASTNodes(XQillaAllocator<ASTNode*>(MEMMGR));
      children->push_back(WRAP(@1, new (MEMMGR) XQSequence(MEMMGR)));
      attr->setChildren(children);
    }
  }
  ;

AttributeAttrs_XSLT:
    _XSLT_ATTRIBUTE_
  {
    $$ = WRAP(@1, new (MEMMGR) XQAttributeConstructor(0, 0, MEMMGR));
  }
  | AttributeAttrs_XSLT _XSLT_NAME_ AttrValueTemplate_XSLT
  {
    $$ = $1;

    ASTNode *content = WRAP(@3, new (MEMMGR) XQSimpleContent($3, MEMMGR));
    ((XQAttributeConstructor*)$$)->setName(PRESERVE_NS(@2, content));
  }
  | AttributeAttrs_XSLT _XSLT_NAMESPACE_A_ AttrValueTemplate_XSLT
  {
    $$ = $1;

    ASTNode *content = WRAP(@3, new (MEMMGR) XQSimpleContent($3, MEMMGR));
    ((XQAttributeConstructor*)$$)->namespaceExpr = PRESERVE_NS(@2, content);
  }
  | AttributeAttrs_XSLT _XSLT_SELECT_ Expr
  {
    $$ = $1;

    VectorOfASTNodes *children = new (MEMMGR) VectorOfASTNodes(XQillaAllocator<ASTNode*>(MEMMGR));
    children->push_back(PRESERVE_NS(@2, $3));

    ((XQAttributeConstructor*)$$)->setChildren(children);
  }
  | AttributeAttrs_XSLT _XSLT_SEPARATOR_ AttrValueTemplate_XSLT
  {
    $$ = $1;
    // TBD separator - jpcs
/*     ((XQAttributeConstructor*)$$)->setChildren(children); */
  }
  ;

Namespace_XSLT:
    NamespaceAttrs_XSLT SequenceConstructor_XSLT _XSLT_END_ELEMENT_
  {
    XQNamespaceConstructor *ns = (XQNamespaceConstructor*)$1;
    $$ = ns;

    if(ns->getName() == 0) {
      yyerror(@1, "The xsl:namespace instruction does not have a {}name attribute");
    }

    if(!$2->getChildren().empty()) {
      if(ns->getChildren() != 0 && !ns->getChildren()->empty()) {
        yyerror(@1, "The xsl:namespace instruction has both a select attribute and a sequence constructor [err:XTSE0840]");
      }

      VectorOfASTNodes *children = new (MEMMGR) VectorOfASTNodes(XQillaAllocator<ASTNode*>(MEMMGR));
      *children = $2->getChildren();
      ns->setChildren(children);
    }
    else if(ns->getChildren() == 0) {
      VectorOfASTNodes *children = new (MEMMGR) VectorOfASTNodes(XQillaAllocator<ASTNode*>(MEMMGR));
      children->push_back(WRAP(@1, new (MEMMGR) XQSequence(MEMMGR)));
      ns->setChildren(children);
    }
  }
  ;

NamespaceAttrs_XSLT:
    _XSLT_NAMESPACE_
  {
    $$ = WRAP(@1, new (MEMMGR) XQNamespaceConstructor(0, 0, MEMMGR));
  }
  | NamespaceAttrs_XSLT _XSLT_NAME_ AttrValueTemplate_XSLT
  {
    $$ = $1;

    ASTNode *content = WRAP(@3, new (MEMMGR) XQSimpleContent($3, MEMMGR));
    ((XQNamespaceConstructor*)$$)->setName(PRESERVE_NS(@2, content));
  }
  | NamespaceAttrs_XSLT _XSLT_SELECT_ Expr
  {
    $$ = $1;

    VectorOfASTNodes *children = new (MEMMGR) VectorOfASTNodes(XQillaAllocator<ASTNode*>(MEMMGR));
    children->push_back(PRESERVE_NS(@2, $3));

    ((XQNamespaceConstructor*)$$)->setChildren(children);
  }
  ;

Element_XSLT:
    ElementAttrs_XSLT SequenceConstructor_XSLT _XSLT_END_ELEMENT_
  {
    XQElementConstructor *elem = (XQElementConstructor*)$1;
    $$ = elem;

    if(elem->getName() == 0) {
      yyerror(@1, "The xsl:element instruction does not have a {}name attribute");
    }

    if(elem->namespaceExpr != 0) {
      // Use fn:QName() to assign the correct URI
      VectorOfASTNodes args(XQillaAllocator<ASTNode*>(MEMMGR));
      args.push_back(elem->namespaceExpr);
      args.push_back(const_cast<ASTNode*>(elem->getName()));
      FunctionQName *name = WRAP(@1, new (MEMMGR) FunctionQName(args, MEMMGR));
      elem->setName(name);
      elem->namespaceExpr = 0;
    }

    VectorOfASTNodes *children = new (MEMMGR) VectorOfASTNodes(XQillaAllocator<ASTNode*>(MEMMGR));
    *children = $2->getChildren();
    elem->setChildren(children);
  }
  ;

ElementAttrs_XSLT:
    _XSLT_ELEMENT_
  {
    $$ = WRAP(@1, new (MEMMGR) XQElementConstructor(0, 0, 0, MEMMGR));
  }
  | ElementAttrs_XSLT _XSLT_NAME_ AttrValueTemplate_XSLT
  {
    $$ = $1;

    ASTNode *content = WRAP(@3, new (MEMMGR) XQSimpleContent($3, MEMMGR));
    ((XQElementConstructor*)$$)->setName(PRESERVE_NS(@2, content));
  }
  | ElementAttrs_XSLT _XSLT_NAMESPACE_A_ AttrValueTemplate_XSLT
  {
    $$ = $1;

    ASTNode *content = WRAP(@3, new (MEMMGR) XQSimpleContent($3, MEMMGR));
    ((XQElementConstructor*)$$)->namespaceExpr = PRESERVE_NS(@2, content);
  }
  ;

CopyOf_XSLT:
    CopyOfAttrs_XSLT _XSLT_END_ELEMENT_
  {
    XQCopyOf *as = (XQCopyOf*)$$;
    $$ = $1;

    if(as->getExpression() == 0) {
      yyerror(@1, "The xsl:copy-of instruction does not have a {}select attribute");
    }
  }
  ;

CopyOfAttrs_XSLT:
    _XSLT_COPY_OF_
  {
    $$ = WRAP(@$, new (MEMMGR) XQCopyOf(MEMMGR));
  }
  | CopyOfAttrs_XSLT _XSLT_SELECT_ Expr
  {
    $$ = $1;
    ((XQCopyOf*)$$)->setExpression(PRESERVE_NS(@2, $3));
  }
  | CopyOfAttrs_XSLT _XSLT_COPY_NAMESPACES_
  {
    $$ = $1;
    ((XQCopyOf*)$$)->setCopyNamespaces($2);
  }
  // TBD type and validation - jpcs
  ;

Copy_XSLT:
    CopyAttrs_XSLT SequenceConstructor_XSLT _XSLT_END_ELEMENT_
  {
    $$ = $1;
    XQCopy *as = (XQCopy*)$$;

    XQContextItem *ci = WRAP(@1, new (MEMMGR) XQContextItem(MEMMGR));
    as->setExpression(ci);

    as->setChildren($2->getChildren());
  }
  ;

CopyAttrs_XSLT:
    _XSLT_COPY_
  {
    $$ = WRAP(@$, new (MEMMGR) XQCopy(MEMMGR));
  }
  | CopyAttrs_XSLT _XSLT_COPY_NAMESPACES_
  {
    $$ = $1;
    ((XQCopy*)$$)->setCopyNamespaces($2);
  }
  | CopyAttrs_XSLT _XSLT_INHERIT_NAMESPACES_
  {
    $$ = $1;
    ((XQCopy*)$$)->setInheritNamespaces($2);
  }
  // TBD type and validation - jpcs
  ;

ForEach_XSLT:
    ForEachAttrs_XSLT SequenceConstructor_XSLT _XSLT_END_ELEMENT_
  {
    // TBD xsl:sort - jpcs
    $$ = $1;
    XQMap *map = (XQMap*)$$;
    map->setArg2($2);

    if(map->getArg1() == 0) {
      yyerror(@1, "The xsl:for-each instruction does not have a {}select attribute");
    }
  }
  ;

ForEachAttrs_XSLT:
    _XSLT_FOR_EACH_
  {
    $$ = WRAP(@1, new (MEMMGR) XQMap(0, 0, MEMMGR));
  }
  | ForEachAttrs_XSLT _XSLT_SELECT_ Expr
  {
    $$ = $1;
    ((XQMap*)$$)->setArg1(PRESERVE_NS(@2, $3));
  }
  ;




// [1]       Pattern       ::=       PathPattern
//       | Pattern '|' PathPattern
Pattern_XSLT:
    Pattern_XSLT _BAR_ PathPattern_XSLT
  {
    $$ = $1;
    $$->push_back(PRESERVE_NS(@3, $3));
  }
  | PathPattern_XSLT
  {
    $$ = new (MEMMGR) VectorOfASTNodes(XQillaAllocator<ASTNode*>(MEMMGR));
    $$->push_back(PRESERVE_NS(@1, $1));
  }
  ;

// [2]      PathPattern      ::=      RelativePathPattern
//       | '/' RelativePathPattern?
//       | '//' RelativePathPattern
//       | IdKeyPattern (('/' | '//') RelativePathPattern)?
PathPattern_XSLT:
    RelativePathPattern_XSLT
  {
    $$ = $1;
  }
  | _SLASH_
  {
    NodeTest *nt = new (MEMMGR) NodeTest();
    nt->setNodeType(Node::document_string);
    nt->setNameWildcard();
    nt->setNamespaceWildcard();
    $$ = WRAP(@1, new (MEMMGR) XQStep(XQStep::SELF, nt, MEMMGR));
  }
  | IdKeyPattern_XSLT
  {
    // . intersect id("a")

    VectorOfASTNodes oargs(XQillaAllocator<ASTNode*>(MEMMGR));
    oargs.push_back(WRAP(@1, new (MEMMGR) XQContextItem(MEMMGR)));
    oargs.push_back($1);

    $$ = WRAP(@1, new (MEMMGR) Intersect(oargs, MEMMGR));
  }
  ;

PathPatternStart_XSLT:
    _SLASH_
  {
    NodeTest *nt = new (MEMMGR) NodeTest();
    nt->setNodeType(Node::document_string);
    nt->setNameWildcard();
    nt->setNamespaceWildcard();
    $$ = WRAP(@1, new (MEMMGR) XQStep(XQStep::PARENT, nt, MEMMGR));
  }
  | _SLASH_SLASH_
  {
    NodeTest *nt = new (MEMMGR) NodeTest();
    nt->setNodeType(Node::document_string);
    nt->setNameWildcard();
    nt->setNamespaceWildcard();
    $$ = WRAP(@1, new (MEMMGR) XQStep(XQStep::ANCESTOR, nt, MEMMGR));
  }
  ;

// [6]      IdKeyPattern      ::=      'id' '(' IdValue ')'
//       | 'key' '(' StringLiteralXP ',' KeyValue ')'
IdKeyPattern_XSLT:
    _ID_ _LPAR_ IdValue_XSLT _RPAR_
  {
    VectorOfASTNodes fargs(XQillaAllocator<ASTNode*>(MEMMGR));
    fargs.push_back($3);

    $$ = WRAP(@1, new (MEMMGR) FunctionId(fargs, MEMMGR));
  }
  | _KEY_ _LPAR_ StringLiteral _COMMA_ KeyValue_XSLT _RPAR_
  {
//     VectorOfASTNodes fargs(XQillaAllocator<ASTNode*>(MEMMGR));
//     fargs.push_back($3);
//     fargs.push_back($5);

//     $$ = WRAP(@1, new (MEMMGR) FunctionKey(fargs, MEMMGR));

    // TBD key() - jpcs
    $$ = WRAP(@1, new (MEMMGR) XQContextItem(MEMMGR));
  }
  ;

// [7]      IdValue      ::=      StringLiteralXP | VarRef XP
IdValue_XSLT: StringLiteral | VarRef;

// [8]      KeyValue      ::=      Literal XP | VarRef XP
KeyValue_XSLT: Literal | VarRef;

//
// foo/bar/@baz -> self::attribute(baz)[parent::bar[parent::foo]]
//
// foo//bar ->self::bar[ancestor::foo]
//
// /foo/bar -> self::bar[parent::foo[parent::document-node()]]
//
// //foo -> self::foo[ancestor::document-node()]
//
// /foo[baz]/bar -> self::bar[parent::foo[baz][parent::document-node()]]
//
// id("a")/foo/bar -> self::bar[parent::foo[parent::node() intersect id("a")]]
//
// / -> self::document-node()
//
// id("a") -> . intersect id("a")
//

// [3]      RelativePathPattern      ::=      PatternStep (('/' | '//') RelativePathPattern)?
RelativePathPattern_XSLT:
    PatternStep_XSLT
  {
    $$ = $1;
  }
  | PathPatternStart_XSLT PatternStep_XSLT
  {
    $$ = WRAP(@1, new (MEMMGR) XQPredicate($2, $1, MEMMGR));
  }
  | IdKeyPattern_XSLT _SLASH_ PatternStep_XSLT
  {
    // id("a")/foo -> self::foo[parent::node() intersect id("a")]

    NodeTest *nt = new (MEMMGR) NodeTest();
    nt->setTypeWildcard();
    nt->setNameWildcard();
    nt->setNamespaceWildcard();
    XQStep *step = WRAP(@1, new (MEMMGR) XQStep(XQStep::PARENT, nt, MEMMGR));

    VectorOfASTNodes oargs(XQillaAllocator<ASTNode*>(MEMMGR));
    oargs.push_back(step);
    oargs.push_back($1);

    Intersect *intersect = WRAP(@1, new (MEMMGR) Intersect(oargs, MEMMGR));

    $$ = WRAP(@1, new (MEMMGR) XQPredicate($3, intersect, MEMMGR));
  }
  | IdKeyPattern_XSLT _SLASH_SLASH_ PatternStep_XSLT
  {
    // id("a")/foo -> self::foo[ancestor::node() intersect id("a")]

    NodeTest *nt = new (MEMMGR) NodeTest();
    nt->setTypeWildcard();
    nt->setNameWildcard();
    nt->setNamespaceWildcard();
    XQStep *step = WRAP(@1, new (MEMMGR) XQStep(XQStep::ANCESTOR, nt, MEMMGR));

    VectorOfASTNodes oargs(XQillaAllocator<ASTNode*>(MEMMGR));
    oargs.push_back(step);
    oargs.push_back($1);

    Intersect *intersect = WRAP(@1, new (MEMMGR) Intersect(oargs, MEMMGR));

    $$ = WRAP(@1, new (MEMMGR) XQPredicate($3, intersect, MEMMGR));
  }
  | RelativePathPattern_XSLT _SLASH_ PatternStep_XSLT
  {
    ASTNode *step = $1;
    while(step->getType() == ASTNode::PREDICATE)
      step = (ASTNode*)((XQPredicate*)step)->getExpression();

    ((XQStep*)step)->setAxis(XQStep::PARENT);

    $$ = WRAP(@2, new (MEMMGR) XQPredicate($3, $1, MEMMGR));
  }
  | RelativePathPattern_XSLT _SLASH_SLASH_ PatternStep_XSLT
  {
    ASTNode *step = $1;
    while(step->getType() == ASTNode::PREDICATE)
      step = (ASTNode*)((XQPredicate*)step)->getExpression();

    ((XQStep*)step)->setAxis(XQStep::ANCESTOR);

    $$ = WRAP(@2, new (MEMMGR) XQPredicate($3, $1, MEMMGR));
  }
  ;

// [4]      PatternStep      ::=      PatternAxis? NodeTest PredicateList
PatternStep_XSLT:
    PatternAxis_XSLT PatternStepPredicateList_XSLT
  {
    $$ = XQPredicate::addPredicates(WRAP(@1, new (MEMMGR) XQStep(XQStep::SELF, $1, MEMMGR)), $2);
  }
  ;

// [5]      PatternAxis      ::=      ('child' '::' | 'attribute' '::' | '@')
PatternAxis_XSLT:
    NodeTest
  {
    if(!$1->isNodeTypeSet())
      $1->setNodeType(Node::element_string);
    $$ = $1;
  }
  | _CHILD_ _COLON_COLON_ NodeTest
  {
    if(!$3->isNodeTypeSet())
      $3->setNodeType(Node::element_string);
    $$ = $3;
  }
  | _ATTRIBUTE_ _COLON_COLON_ NodeTest
  {
    if(!$3->isNodeTypeSet())
      $3->setNodeType(Node::attribute_string);
    $$ = $3;
  }
  | _AT_SIGN_ NodeTest
  {
    if(!$2->isNodeTypeSet())
      $2->setNodeType(Node::attribute_string);
    $$ = $2;
  }
  ;

// [85]    PredicateList    ::=    Predicate* 
// [86]    Predicate    ::=    "[" Expr "]" 
PatternStepPredicateList_XSLT:
  /* empty */
  {
    $$ = new (MEMMGR) VectorOfPredicates(MEMMGR);
  }
  | PatternStepPredicateList_XSLT _LSQUARE_ Expr _RSQUARE_
  {
    XQPredicate *pred = WRAP(@2, new (MEMMGR) XQPredicate($3, MEMMGR));
    $1->push_back(pred);
    $$ = $1; 
  }
  ;


////////////////////////////////////////////////////////////////////////////////////////////////////
// XQuery rules

// [1]    Module    ::=      VersionDecl? (MainModule | LibraryModule)
Module:
  VersionDecl MainModule
  | VersionDecl LibraryModule
  | MainModule
  | LibraryModule
  ;

// [2]    VersionDecl    ::=    <"xquery" "version" StringLiteral> ("encoding" StringLiteral)? Separator
// [2]   	VersionDecl	   ::=   	"xquery" (("encoding" StringLiteral) | ("version" StringLiteral ("encoding" StringLiteral)?)) Separator
VersionDecl:
  _XQUERY_ Version Separator
  | _XQUERY_ Version Encoding Separator
  | _XQUERY_ Encoding Separator
  ;

Version:
    _VERSION_ _STRING_LITERAL_
  {
    // TBD Set the language correctly on the context - jpcs
    if(XPath2Utils::equals($2,sz1_0)) {
      QP->_lexer->setVersion3(false);
      QP->_query->setVersion3(false);
    }
    else if(XPath2Utils::equals($2,sz3_0)) {
      if(!QP->_lexer->isVersion3()) {
        yyerror(@1, "This XQuery processor is not configured to support XQuery 3.0 [err:XQST0031]");
      }
      QP->_lexer->setVersion3(true);
      QP->_query->setVersion3(true);
    }
    else
      yyerror(@1, "This XQuery processor only supports version 1.0 and 3.0 [err:XQST0031]");
  }
  ;

Encoding:
    _ENCODING_ _STRING_LITERAL_
  {
    XMLCh *encName = $2;
    if((*encName < chLatin_A || *encName > chLatin_Z) && (*encName < chLatin_a || *encName > chLatin_z))
      yyerror(@1, "The specified encoding does not conform to the definition of EncName [err:XQST0087]");

    for(++encName; *encName; ++encName) {
      if((*encName < chLatin_A || *encName > chLatin_Z) &&
         (*encName < chLatin_a || *encName > chLatin_z) &&
         (*encName < chDigit_0 || *encName > chDigit_9) &&
         *encName != chPeriod && *encName != chDash)
        yyerror(@1, "The specified encoding does not conform to the definition of EncName [err:XQST0087]");
    }
  }
  ;

// [3]    MainModule    ::=    Prolog QueryBody 
MainModule: Prolog QueryBody;

// [4]    LibraryModule    ::=    ModuleDecl Prolog 
LibraryModule: ModuleDecl Prolog;

// [5]    ModuleDecl    ::=    <"module" "namespace"> NCName "=" URILiteral Separator
ModuleDecl:
  _MODULE_ _NAMESPACE_ NCName _EQUALS_ URILiteral Separator
  {
    if(*$5 == 0)
      yyerror(@5, "The literal that specifies the namespace of a module must not be of zero length [err:XQST0088]");
    QP->_query->setIsLibraryModule();
    QP->_query->setModuleTargetNamespace($5);
    SET_NAMESPACE(@3, $3, $5);
  }
  ;

// [6]    Prolog    ::=    ((Setter | Import | NamespaceDecl | DefaultNamespaceDecl | FTOptionDecl) Separator)* 
//               ((VarDecl | FunctionDecl | OptionDecl) Separator)*
Prolog:
  /* empty */
  | Prolog Setter Separator
  {
    CHECK_SECOND_STEP(@2, "a setter");
  }
  | Prolog Import Separator
  {
    CHECK_SECOND_STEP(@2, "an import");
  }
  | Prolog NamespaceDecl Separator
  {
    CHECK_SECOND_STEP(@2, "a namespace");
  }
  | Prolog DefaultNamespaceDecl Separator
  {
    CHECK_SECOND_STEP(@2, "a default namespace");
  }
  | Prolog VarDecl Separator
  {
    QP->_flags.set(BIT_DECLARE_SECOND_STEP);
  }
  | Prolog FunctionDecl Separator
  {
    QP->_flags.set(BIT_DECLARE_SECOND_STEP);
    XQUserFunction* decl=$2;
    QP->_query->addFunction(decl);
  }
  | Prolog TemplateDecl Separator
  {
    QP->_flags.set(BIT_DECLARE_SECOND_STEP);
    XQUserFunction* decl=$2;
    QP->_query->addFunction(decl);
  }
  | Prolog OptionDecl Separator
  {
    QP->_flags.set(BIT_DECLARE_SECOND_STEP);
  }
  | Prolog FTOptionDecl Separator
  {
    CHECK_SECOND_STEP(@2, "an ftoption declaration");
  }
;

// [7]    Setter    ::=   BoundarySpaceDecl  
//            | DefaultCollationDecl 
//            | BaseURIDecl 
//            | ConstructionDecl 
//            | OrderingModeDecl
//            | EmptyOrderDecl 
//            | CopyNamespacesDecl 
Setter:
  BoundarySpaceDecl  
  | DefaultCollationDecl 
  | BaseURIDecl 
  | ConstructionDecl 
  | OrderingModeDecl 
  | EmptyOrderDecl 
  | RevalidationDecl
  | CopyNamespacesDecl 
;  

// [8]    Import    ::=    SchemaImport | ModuleImport 
Import: SchemaImport | ModuleImport;

// [9]    Separator    ::=    ";" 
Separator: _SEMICOLON_;

// [10]    NamespaceDecl    ::=    <"declare" "namespace"> NCName "=" URILiteral 
NamespaceDecl:
  _DECLARE_ _NAMESPACE_ NCName _EQUALS_ URILiteral 
  {
    SET_NAMESPACE(@3, $3, $5);
  }
  ;

// [11]    BoundarySpaceDecl     ::=    <"declare" "boundary-space"> ("preserve" |  "strip") 
BoundarySpaceDecl :
  _DECLARE_ _BOUNDARY_SPACE_ _PRESERVE_
  {
    CHECK_SPECIFIED(@1, BIT_BOUNDARY_SPECIFIED, "boundary space", "XQST0068");
    CONTEXT->setPreserveBoundarySpace(true);
  }
  | _DECLARE_ _BOUNDARY_SPACE_ _STRIP_
  {
    CHECK_SPECIFIED(@1, BIT_BOUNDARY_SPECIFIED, "boundary space", "XQST0068");
    CONTEXT->setPreserveBoundarySpace(false);
  }
  ;

// [12]    DefaultNamespaceDecl    ::=    (<"declare" "default" "element"> |  <"declare" "default" "function">) "namespace" URILiteral 
DefaultNamespaceDecl:
  _DECLARE_ _DEFAULT_ _ELEMENT_ _NAMESPACE_ URILiteral
  {
    CHECK_SPECIFIED(@1, BIT_DEFAULTELEMENTNAMESPACE_SPECIFIED, "default element namespace", "XQST0066");
    CONTEXT->setDefaultElementAndTypeNS($5);
  }
  | _DECLARE_ _DEFAULT_ FunctionKeyword _NAMESPACE_ URILiteral
  { 
    CHECK_SPECIFIED(@1, BIT_DEFAULTFUNCTIONNAMESPACE_SPECIFIED, "default function namespace", "XQST0066");
    CONTEXT->setDefaultFuncNS($5);
  }
  ;

// [13]     OptionDecl     ::=     <"declare" "option"> QName StringLiteral
OptionDecl:
  _DECLARE_ _OPTION_ QNameValue _STRING_LITERAL_
  {
    // validate the QName
    QualifiedName qName($3);
    const XMLCh* prefix = qName.getPrefix();
    if(prefix == 0 || *prefix == 0)
      yyerror(@3, "The option name must have a prefix [err:XPST0081]");

    const XMLCh *uri = 0;
    try {
      LOCATION(@3, loc);
      uri = CONTEXT->getUriBoundToPrefix(prefix, &loc);
    }
    catch(NamespaceLookupException&) {
      yyerror(@3, "The option name is using an undefined namespace prefix [err:XPST0081]");
    }

    if(XPath2Utils::equals(uri, XQillaFunction::XMLChFunctionURI)) {
      if(XPath2Utils::equals(qName.getName(), option_projection)) {
        if(XPath2Utils::equals($4, SchemaSymbols::fgATTVAL_TRUE)) {
          CONTEXT->setProjection(true);
        }
        else if(XPath2Utils::equals($4, SchemaSymbols::fgATTVAL_FALSE)) {
          CONTEXT->setProjection(false);
        }
        else {
          yyerror(@4, "Unknown value for option xqilla:projection. Should be 'true' or 'false' [err:XQILLA]");
        }
      }
      else if(XPath2Utils::equals(qName.getName(), option_psvi)) {
        if(XPath2Utils::equals($4, SchemaSymbols::fgATTVAL_TRUE)) {
          CONTEXT->getDocumentCache()->setDoPSVI(true);
        }
        else if(XPath2Utils::equals($4, SchemaSymbols::fgATTVAL_FALSE)) {
          CONTEXT->getDocumentCache()->setDoPSVI(false);
        }
        else {
          yyerror(@4, "Unknown value for option xqilla:psvi. Should be 'true' or 'false' [err:XQILLA]");
        }
      }
      else if(XPath2Utils::equals(qName.getName(), option_lint)) {
        if(XPath2Utils::equals($4, SchemaSymbols::fgATTVAL_TRUE)) {
          CONTEXT->setDoLintWarnings(true);
        }
        else if(XPath2Utils::equals($4, SchemaSymbols::fgATTVAL_FALSE)) {
          CONTEXT->setDoLintWarnings(false);
        }
        else {
          yyerror(@4, "Unknown value for option xqilla:lint. Should be 'true' or 'false' [err:XQILLA]");
        }
      }
      else {
        yyerror(@3, "Unknown option name in the xqilla namespace [err:XQILLA]");
      }
    }
  }
  ;

// [14]    FTOptionDecl    ::=    "declare" "ft-option" FTMatchOptions
FTOptionDecl:
  _DECLARE_ _FT_OPTION_ FTMatchOptions
  {
    // TBD FTOptionDecl
  }
  ;

// [15]    OrderingModeDecl    ::=    <"declare" "ordering"> ("ordered" | "unordered") 
OrderingModeDecl:
  _DECLARE_ _ORDERING_ _ORDERED_
  {
    CHECK_SPECIFIED(@1, BIT_ORDERING_SPECIFIED, "ordering mode", "XQST0065");
    CONTEXT->setNodeSetOrdering(StaticContext::ORDERING_ORDERED);
  }
  | _DECLARE_ _ORDERING_ _UNORDERED_
  {
    CHECK_SPECIFIED(@1, BIT_ORDERING_SPECIFIED, "ordering mode", "XQST0065");
    CONTEXT->setNodeSetOrdering(StaticContext::ORDERING_UNORDERED);
  }
  ;

// [16]    EmptyOrderDecl    ::=    "declare" "default" "order" "empty" ("greatest" | "least")
EmptyOrderDecl:
  _DECLARE_ _DEFAULT_ _ORDER_ _EMPTY_ _GREATEST_
  { 
    CHECK_SPECIFIED(@1, BIT_EMPTYORDERING_SPECIFIED, "empty ordering mode", "XQST0069");
    CONTEXT->setDefaultFLWOROrderingMode(StaticContext::FLWOR_ORDER_EMPTY_GREATEST);
  }
  | _DECLARE_ _DEFAULT_ _ORDER_ _EMPTY_ _LEAST_
  { 
    CHECK_SPECIFIED(@1, BIT_EMPTYORDERING_SPECIFIED, "empty ordering mode", "XQST0069");
    CONTEXT->setDefaultFLWOROrderingMode(StaticContext::FLWOR_ORDER_EMPTY_LEAST);
  }
  ;

// [17]    CopyNamespacesDecl    ::=     <"declare" "copy-namespaces"> PreserveMode "," InheritMode
CopyNamespacesDecl:
  _DECLARE_ _COPY_NAMESPACES_ PreserveMode _COMMA_ InheritMode
  {
    CHECK_SPECIFIED(@1, BIT_COPYNAMESPACE_SPECIFIED, "copy namespace", "XQST0055");
    CONTEXT->setPreserveNamespaces($3);
    CONTEXT->setInheritNamespaces($5);
  }
;

// [18]     PreserveMode     ::=     "preserve" | "no-preserve"
PreserveMode:
  _PRESERVE_
  {
    $$ = true;
  }
  | _NO_PRESERVE_
  {
    $$ = false;
  }
  ;

// [19]     InheritMode     ::=     "inherit" | "no-inherit"
InheritMode:
  _INHERIT_
  {
    $$ = true;
  }
  | _NO_INHERIT_
  {
    $$ = false;
  }
  ;

// [20]    DefaultCollationDecl    ::=    <"declare" "default" "collation"> URILiteral 
DefaultCollationDecl:
  _DECLARE_ _DEFAULT_ _COLLATION_ URILiteral
  {
    CHECK_SPECIFIED(@1, BIT_COLLATION_SPECIFIED, "default collation", "XQST0038");
    try {
      LOCATION(@4, loc);
      CONTEXT->getCollation($4, &loc);
    }
    catch(ContextException&) {
      yyerror(@4, "The specified collation does not exist [err:XQST0038]");
    }
    CONTEXT->setDefaultCollation($4);
  }
;

// [21]    BaseURIDecl    ::=    <"declare" "base-uri"> URILiteral
BaseURIDecl:
  _DECLARE_ _BASE_URI_ URILiteral
  {
    CHECK_SPECIFIED(@1, BIT_BASEURI_SPECIFIED, "base URI", "XQST0032");
    CONTEXT->setBaseURI($3);
  }
;

// [22]    SchemaImport    ::=    <"import" "schema"> SchemaPrefix? URILiteral (<"at" URILiteral> ("," URILiteral)*)?
SchemaImport:
  _IMPORT_ _SCHEMA_ SchemaPrefix URILiteral ResourceLocations
  {
    if(XPath2Utils::equals($3, XMLUni::fgZeroLenString))
      CONTEXT->setDefaultElementAndTypeNS($4);
    else if(XPath2Utils::equals($4, XMLUni::fgZeroLenString))
      yyerror(@1, "A schema that has no target namespace cannot be bound to a non-empty prefix [err:XQST0057]");
    else {
      SET_NAMESPACE(@3, $3, $4);
    }
    LOCATION(@1, loc);
    CONTEXT->addSchemaLocation($4, $5, &loc);
  }
  | _IMPORT_ _SCHEMA_ URILiteral ResourceLocations
  {
    LOCATION(@1, loc);
    CONTEXT->addSchemaLocation($3, $4, &loc);
  }
;

ResourceLocations:
  /* empty */
  {
    $$ = NULL;
  }
  | _AT_ URILiteral
  {
    $$ = new (MEMMGR) VectorOfStrings(XQillaAllocator<const XMLCh*>(MEMMGR));
    $$->push_back($2);
  }
  | ResourceLocations _COMMA_ URILiteral
  {
    $1->push_back($3);
    $$ = $1;
  }
;

// [23]    SchemaPrefix    ::=    ("namespace" NCName "=") |  (<"default" "element"> "namespace") 
SchemaPrefix:
  _NAMESPACE_ NCName _EQUALS_
  {
    $$ = $2;
  }
  | _DEFAULT_ _ELEMENT_ _NAMESPACE_
  {
    $$ = (XMLCh*)XMLUni::fgZeroLenString;
  }
  ;

// [24]    ModuleImport    ::=    <"import" "module"> ("namespace" NCName "=")? URILiteral (<"at" URILiteral> ("," URILiteral)*)?
ModuleImport:
  _IMPORT_ _MODULE_ _NAMESPACE_ NCName _EQUALS_ URILiteral ResourceLocations
  {
    if(XMLString::stringLen($6)==0)
      yyerror(@6, "The literal that specifies the target namespace in a module import must not be of zero length [err:XQST0088]");

    SET_NAMESPACE(@4, $4, $6);

    LOCATION(@1, loc);
    QP->_query->importModule($6, $7, &loc);
  }
  | _IMPORT_ _MODULE_ URILiteral ResourceLocations
  {
    if(XMLString::stringLen($3)==0)
      yyerror(@3, "The literal that specifies the target namespace in a module import must not be of zero length [err:XQST0088]");

    LOCATION(@1, loc);
    QP->_query->importModule($3, $4, &loc);
  }
  ;

// [25]    VarDecl    ::=    <"declare" "variable" "$"> VarName TypeDeclaration? ((":=" ExprSingle) | "external")
VarDecl:
  _DECLARE_ _VARIABLE_ _DOLLAR_ VarName TypeDeclaration VarDeclValue
  {
    QP->_query->addVariable(WRAP(@1, new (MEMMGR) XQGlobalVariable($4, $5, $6, MEMMGR)));
  }
  ;

VarDeclValue:
  _COLON_EQUALS_ ExprSingle
  {
    $$ = $2;
  }
  | _EXTERNAL_
  {
    $$ = NULL;
  }
;

// [26]    ConstructionDecl    ::=    <"declare" "construction"> ("preserve" | "strip") 
ConstructionDecl:
  _DECLARE_ _CONSTRUCTION_ _PRESERVE_
  {
    CHECK_SPECIFIED(@1, BIT_CONSTRUCTION_SPECIFIED, "construction mode", "XQST0067");
    CONTEXT->setConstructionMode(StaticContext::CONSTRUCTION_MODE_PRESERVE);
  }
  | _DECLARE_ _CONSTRUCTION_ _STRIP_
  {
    CHECK_SPECIFIED(@1, BIT_CONSTRUCTION_SPECIFIED, "construction mode", "XQST0067");
    CONTEXT->setConstructionMode(StaticContext::CONSTRUCTION_MODE_STRIP);
  }
  ;

// [31]   	FunctionDecl	   ::=   	"declare" FunctionOptions "function" QName "(" ParamList? ")" ("as" SequenceType)? (FunctionBody | "external")
FunctionDecl:
    _DECLARE_ FunctionOptions FunctionKeyword FunctionName FunctionParamList FunctionDeclReturnType FunctionBody
  {
    $2->argSpecs = $5;
    $2->returnType = $6;
    $$ = WRAP(@1, new (MEMMGR) XQUserFunction($4, $2, $7, true, MEMMGR));
  }
  ;

FunctionKeyword: _FUNCTION_ | _FUNCTION_EXT_;

FunctionParamList:
  _LPAR_ _RPAR_
  {
    $$ = NULL;
  }
  | _LPAR_ ParamList _RPAR_
  {
    $$ = $2;
  }
  ;

// [37]        FunctionBody       ::=          EnclosedExpr
FunctionBody:
    EnclosedExpr
  | _EXTERNAL_
  {
    $$ = 0;
  }
  ;

// [32]   	FunctionOptions	   ::=   	(PrivateOption | DeterministicOption | "updating")*
FunctionOptions:
    /* empty */
  {
    $$ = new (MEMMGR) FunctionSignature(MEMMGR);
  }
  | FunctionOptions PrivateOption
  {
    switch($1->privateOption) {
    case FunctionSignature::OP_TRUE:
      yyerror(@2, "Function option 'private' already specified [err:XQST0106]");
    case FunctionSignature::OP_FALSE:
      yyerror(@2, "Function option 'public' already specified [err:XQST0106]");
    case FunctionSignature::OP_DEFAULT:
      $1->privateOption = $2 ? FunctionSignature::OP_TRUE : FunctionSignature::OP_FALSE;
      break;
    }
    $$ = $1;
  }
  | FunctionOptions DeterministicOption
  {
    switch($1->nondeterministic) {
    case FunctionSignature::OP_TRUE:
      yyerror(@2, "Function option 'nondeterministic' already specified [err:XQST0106]");
    case FunctionSignature::OP_FALSE:
      yyerror(@2, "Function option 'deterministic' already specified [err:XQST0106]");
    case FunctionSignature::OP_DEFAULT:
      $1->nondeterministic = $2 ? FunctionSignature::OP_TRUE : FunctionSignature::OP_FALSE;
      break;
    }
    $$ = $1;
  }
  | FunctionOptions _UPDATING_
  {
    if($1->updating != FunctionSignature::OP_DEFAULT)
      yyerror(@2, "Function option 'updating' already specified [err:XPST0003]");
    $1->updating = FunctionSignature::OP_TRUE;
    $$ = $1;
  }
  ;

// [33]   	PrivateOption	   ::=   	"private" | "public"
PrivateOption: _PRIVATE_ { $$ = true; } | _PUBLIC_ { $$ = false; };

// [34]   	DeterministicOption	   ::=   	"deterministic" | "nondeterministic"
DeterministicOption: _DETERMINISTIC_ { $$ = false; } | _NONDETERMINISTIC_ { $$ = true; };

FunctionDeclReturnType:
    /* empty */
  {
    $$ = 0;
  }
  | _AS_ SequenceType
  {
    $$ = $2;
  }
  ;

// [28]    ParamList    ::=    Param ("," Param)* 
ParamList:
  ParamList _COMMA_ Param
  {
    $1->push_back($3);
    $$ = $1;
  }
  | Param
  {
    ArgumentSpecs* paramList = new (MEMMGR) ArgumentSpecs(XQillaAllocator<ArgumentSpec*>(MEMMGR));
    paramList->push_back($1);
    $$ = paramList;
  }
;

// [29]    Param    ::=    "$" VarName TypeDeclaration? 
Param:
  _DOLLAR_ VarName TypeDeclaration
  {
    $$ = WRAP(@1, new (MEMMGR) ArgumentSpec($2, $3, MEMMGR));
  }
;

// [30]    EnclosedExpr    ::=    "{" Expr "}" 
EnclosedExpr:
    _LBRACE_ Expr _RBRACE_
    {
      $$ = $2;
    }
      ;

// [31]    QueryBody    ::=    Expr
QueryBody:
  Expr
  {
    if(QP->_lexer->isUpdate()) {
      QP->_query->setQueryBody(WRAP(@1, new (MEMMGR) UApplyUpdates($1, MEMMGR)));
    }
    else {
      QP->_query->setQueryBody($1);
    }
  }
  ;

// [32]    Expr    ::=    ExprSingle ("," ExprSingle)* 
Expr:
    Expr _COMMA_ ExprSingle
    {
      ASTNode* prevExpr=$1;
      if(prevExpr->getType()==ASTNode::SEQUENCE)
      {
        ((XQSequence *)prevExpr)->addItem($3);
        $$ = $1;
      }
      else {
        XQSequence *dis = WRAP(@2, new (MEMMGR) XQSequence(MEMMGR));
        dis->addItem($1);
        dis->addItem($3);
        $$ = dis;
      }
    }
    | ExprSingle
    ;

// [32]      ExprSingle    ::=    FLWORExpr | QuantifiedExpr | TypeswitchExpr | IfExpr | InsertExpr
//                               | DeleteExpr | RenameExpr | ReplaceExpr | TransformExpr | OrExpr
//                               | CallTemplateExpr | ApplyTemplatesExpr
ExprSingle:
    FLWORExpr
  | QuantifiedExpr
  | TypeswitchExpr
  | IfExpr
  | InsertExpr
  | DeleteExpr
  | RenameExpr
  | ReplaceExpr
  | TransformExpr
  | OrExpr
  | CallTemplateExpr
  | ApplyTemplatesExpr
  ;

// [42] FLWORExpr ::= InitialClause IntermediateClause* ReturnClause
FLWORExpr:
      FLWORTuples _RETURN_ ExprSingle
    {
      // Add a ContextTuple at the start
      TupleNode *tuples = setLastAncestor($1, WRAP(@1, new (MEMMGR) ContextTuple(MEMMGR)));

      // Check the correct clause order for XQuery 1.0
      // FLWORExpr ::= (ForClause |  LetClause)+ WhereClause? OrderByClause? "return" ExprSingle
      if(!QP->_lexer->isVersion3()) {
        const TupleNode *where = 0;
        const TupleNode *forlet = 0;
        const TupleNode *node = tuples;
        while(node) {
          switch(node->getType()) {
          case TupleNode::ORDER_BY:
            if(where)
              XQThrow3(StaticErrorException, X("XQParser"), X("Where clause after order by clause [err:XPST0003]"), where);
            else if(forlet)
              XQThrow3(StaticErrorException, X("XQParser"), X("For or let clause after order by clause [err:XPST0003]"), forlet);
            break;
          case TupleNode::WHERE:
            if(where)
              XQThrow3(StaticErrorException, X("XQParser"), X("Duplicate where clause [err:XPST0003]"), where);
            else if(forlet)
              XQThrow3(StaticErrorException, X("XQParser"), X("For or let clause after where clause [err:XPST0003]"), forlet);
            where = node;
            break;
          case TupleNode::FOR:
          case TupleNode::LET:
            forlet = node;
            break;
          default:
            break;
          }

          node = node->getParent();
        }
      }

      // Add the return expression
      $$ = WRAP(@2, new (MEMMGR) XQReturn(tuples, $3, MEMMGR));
    }
  ;

FLWORTuples:
    InitialClause
  | FLWORTuples OrderByClause
  {
    // Order by has a special check here, because a single OrderByClause can result
    // in multiple OrderByTuple objects.
    const TupleNode *node = $1;
    while(node) {
      if(node->getType() == TupleNode::ORDER_BY)
        XQThrow3(StaticErrorException, X("XQParser"), X("Duplicate order by clause [err:XPST0003]"), $2);
      node = node->getParent();
    }

    $$ = setLastAncestor($2, $1);
  }
  | FLWORTuples IntermediateClause
  {
    $$ = setLastAncestor($2, $1);
  }
  ;

// [43] InitialClause ::= ForClause | LetClause | WindowClause
InitialClause: ForClause | LetClause;

// [44] IntermediateClause ::= InitialClause | WhereClause | GroupByClause | OrderByClause | CountClause
IntermediateClause: InitialClause | WhereClause | CountClause;

// [35]    ForClause    ::=    "for" "$" VarName TypeDeclaration? PositionalVar? FTScoreVar? "in" ExprSingle
//                                        ("," "$" VarName TypeDeclaration? PositionalVar? FTScoreVar? "in" ExprSingle)*
ForClause:
    _FOR_ ForBindingList
    {
      $$ = $2;
    }
    ;

ForBindingList:
    ForBindingList _COMMA_ ForBinding
    {
      $$ = setLastAncestor($3, $1);
    }
    | ForBinding
    ;

ForBinding:
  _DOLLAR_ VarName TypeDeclaration PositionalVar FTScoreVar _IN_ ExprSingle 
  {
    // the SequenceType has been specified for each item of the sequence, but we can only apply to the
    // sequence itself, so allow it to match multiple matches
    $3->setOccurrence(SequenceType::STAR);
    $$ = WRAP(@1, new (MEMMGR) ForTuple(0, $2, $4, WRAP(@3, new (MEMMGR) XQTreatAs($7, $3, MEMMGR)), MEMMGR));
  }
  ;

// [36]    PositionalVar    ::=    "at" "$" VarName 
PositionalVar:
  /* empty */
  {
    $$ = NULL;
  }
  | _AT_ _DOLLAR_ VarName
  { 
    REJECT_NOT_XQUERY(PositionalVar, @1);

    $$ = $3; 
  }
;

// [37]    FTScoreVar    ::=    "score" "$" VarName
FTScoreVar:
  /* empty */
  {
    $$ = NULL;
  }
  | _SCORE_ _DOLLAR_ VarName
  {
    $$ = $3;
  }
;

// [38]    LetClause    ::= (("let" "$" VarName TypeDeclaration?) |
//                               ("let" "score" "$" VarName)) ":=" ExprSingle
//                               ("," (("$" VarName TypeDeclaration?) | FTScoreVar) ":=" ExprSingle)*
LetClause:
  _LET_ LetBindingList
  {
    $$ = $2;
  }
  ;

LetBindingList:
  LetBindingList _COMMA_ LetBinding
  {
    $$ = setLastAncestor($3, $1);
  }
  | LetBinding
;

LetBinding:
  _DOLLAR_ VarName TypeDeclaration _COLON_EQUALS_ ExprSingle
  {
    $$ = WRAP(@1, new (MEMMGR) LetTuple(0, $2, WRAP(@3, new (MEMMGR) XQTreatAs($5, $3, MEMMGR)), MEMMGR));
  }
  | _SCORE_ _DOLLAR_ VarName _COLON_EQUALS_ ExprSingle
  {
    ASTNode *literal = WRAP(@1, new (MEMMGR) XQLiteral(
                  SchemaSymbols::fgURI_SCHEMAFORSCHEMA,
                  SchemaSymbols::fgDT_INTEGER,
                  X("0"), AnyAtomicType::DECIMAL,
                  MEMMGR));
    $$ = WRAP(@1, new (MEMMGR) LetTuple(0, $3, literal, MEMMGR));
  }
  ;

// [39]    WhereClause    ::=    "where" ExprSingle 
WhereClause:
  _WHERE_ ExprSingle
  { 
    $$ = WRAP(@1, new (MEMMGR) WhereTuple(0, $2, MEMMGR));
  }
;

// [60] CountClause ::= "count" "$" VarName
CountClause:
    _COUNT_ _DOLLAR_ VarName
  {
    $$ = WRAP(@1, new (MEMMGR) CountTuple(0, $3, MEMMGR));
  }
  ;

// [40]    OrderByClause    ::=    (<"order" "by"> |  <"stable" "order" "by">) OrderSpecList 
OrderByClause:
  _ORDER_ _BY_ OrderSpecList
  {
    ((OrderByTuple*)getLastAncestor($3))->setUnstable();
    $$ = $3;
  }
  | _STABLE_ _ORDER_ _BY_ OrderSpecList
  {
    $$ = $4;
  }
;

// [41]    OrderSpecList    ::=    OrderSpec ("," OrderSpec)* 
OrderSpecList:
  OrderSpecList _COMMA_ OrderSpec
  {
    $$ = setLastAncestor($1, $3);
  }
  | OrderSpec
  ;

// [42]    OrderSpec    ::=    ExprSingle OrderModifier 
// [43]    OrderModifier    ::=    ("ascending" |  "descending")? (<"empty" "greatest"> |  <"empty" "least">)? ("collation" URILiteral)? 
OrderSpec:
  OrderExpr OrderDirection EmptyHandling
  {
    LOCATION(@1, loc);
    Collation *collation = CONTEXT->getDefaultCollation(&loc);
    
    $$ = WRAP(@1, new (MEMMGR) OrderByTuple(0, $1, OrderByTuple::STABLE | $2 | $3, collation, MEMMGR));
  }
  | OrderExpr OrderDirection EmptyHandling _COLLATION_ URILiteral
  {
    try {
      LOCATION(@4, loc);
      Collation *collation = CONTEXT->getCollation($5, &loc);

      $$ = WRAP(@1, new (MEMMGR) OrderByTuple(0, $1, OrderByTuple::STABLE | $2 | $3, collation, MEMMGR));
    }
    catch(ContextException&) {
      yyerror(@4, "The specified collation does not exist [err:XQST0076]");
    }
  }
;

OrderExpr:
  ExprSingle
  {
    SequenceType *zero_or_one = WRAP(@1, new (MEMMGR)
      SequenceType(new (MEMMGR) SequenceType::ItemType(SequenceType::ItemType::TEST_ANYTHING),
                   SequenceType::QUESTION_MARK));

    $$ = $1;
    $$ = WRAP(@1, new (MEMMGR) XQAtomize($$, MEMMGR));
    $$ = WRAP(@1, new (MEMMGR) XQPromoteUntyped($$, SchemaSymbols::fgURI_SCHEMAFORSCHEMA,
                                                SchemaSymbols::fgDT_STRING, MEMMGR));
    $$ = WRAP(@1, new (MEMMGR) XQTreatAs($$, zero_or_one, MEMMGR));
  }
;
  

OrderDirection:
  /* empty */
  {
    $$ = OrderByTuple::ASCENDING;
  }
  |  _ASCENDING_
  {
    $$ = OrderByTuple::ASCENDING;
  }
  | _DESCENDING_
  {
    $$ = OrderByTuple::DESCENDING;
  }
;

EmptyHandling:
  /* empty */
  {
    switch(CONTEXT->getDefaultFLWOROrderingMode()) {
    case StaticContext::FLWOR_ORDER_EMPTY_LEAST:
      $$ = OrderByTuple::EMPTY_LEAST; break;
    case StaticContext::FLWOR_ORDER_EMPTY_GREATEST:
      $$ = OrderByTuple::EMPTY_GREATEST; break;
    }
  }
  | _EMPTY_ _GREATEST_
  {
    $$ = OrderByTuple::EMPTY_GREATEST;
  }
  | _EMPTY_ _LEAST_
  {
    $$ = OrderByTuple::EMPTY_LEAST;
  }
;

// [44]    QuantifiedExpr    ::=    (<"some" "$"> |  <"every" "$">) VarName TypeDeclaration? "in" ExprSingle 
//                    ("," "$" VarName TypeDeclaration? "in" ExprSingle)* "satisfies" ExprSingle 
QuantifiedExpr:
  _SOME_ QuantifyBindingList _SATISFIES_ ExprSingle
  {
    // Add a ContextTuple at the start
    TupleNode *tmp = setLastAncestor($2, WRAP(@1, new (MEMMGR) ContextTuple(MEMMGR)));

    // Add the return expression
    $$ = WRAP(@3, new (MEMMGR) XQQuantified(XQQuantified::SOME, tmp, $4, MEMMGR));
  }
  | _EVERY_ QuantifyBindingList _SATISFIES_ ExprSingle
  {
    // Add a ContextTuple at the start
    TupleNode *tmp = setLastAncestor($2, WRAP(@1, new (MEMMGR) ContextTuple(MEMMGR)));

    // Add the return expression
    $$ = WRAP(@3, new (MEMMGR) XQQuantified(XQQuantified::EVERY, tmp, $4, MEMMGR));
  }
  ;

QuantifyBindingList:
  QuantifyBindingList _COMMA_ QuantifyBinding
  {
    $$ = setLastAncestor($3, $1);
  }
  | QuantifyBinding
  ;

QuantifyBinding:
  _DOLLAR_ VarName TypeDeclaration _IN_ ExprSingle 
  {
    // the SequenceType has been specified for each item of the sequence, but we can only apply to the
    // sequence itself, so allow it to match multiple matches
    $3->setOccurrence(SequenceType::STAR);
    $$ = WRAP(@1, new (MEMMGR) ForTuple(0, $2, 0, WRAP(@3, new (MEMMGR) XQTreatAs($5, $3, MEMMGR)), MEMMGR));
  }
;

// [74] TypeswitchExpr ::= "typeswitch" "(" Expr ")" CaseClause+ "default" ("$" VarName)? "return" ExprSingle
TypeswitchExpr:
  _TYPESWITCH_ _LPAR_ Expr _RPAR_ CaseClauseList DefaultCase
  {
    $$ = WRAP(@1, new (MEMMGR) XQTypeswitch( WRAP(@1, $3), $5, $6, MEMMGR));
  }
;

CaseClauseList:
  CaseClauseList CaseClause
  {
    XQTypeswitch::Cases::iterator it = $2->begin();
    for(; it != $2->end(); ++it) {
      $1->push_back(*it);
    }

    $$ = $1;
  }
  | CaseClause
  ;

DefaultCase:
  _DEFAULT_ _DOLLAR_ VarName _RETURN_ ExprSingle
  {
    $$ = WRAP(@1, new (MEMMGR) XQTypeswitch::Case($3, NULL, $5));
  }
  | _DEFAULT_ _RETURN_ ExprSingle
  {
    $$ = WRAP(@1, new (MEMMGR) XQTypeswitch::Case(NULL, NULL, $3));
  }
;

// [75] CaseClause ::= "case" ("$" VarName "as")? SequenceTypeUnion "return" ExprSingle
CaseClause:
    CaseSequenceTypeUnion _RETURN_ ExprSingle
  {
    // TBD Don't copy the expression when unions of SequenceTypes are implemented - jpcs
    XQTypeswitch::Cases::iterator it = $1->begin();
    if(it != $1->end()) {
      (*it)->setExpression($3);

      for(++it; it != $1->end(); ++it) {
        (*it)->setExpression($3->copy(CONTEXT));
      }
    }

    $$ = $1;
  }
  ;

// [75a] SequenceTypeUnion ::= SequenceType ("|" SequenceType)*
CaseSequenceTypeUnion:
    _CASE_ CaseClauseVariable SequenceType
  {
    $$ = new (MEMMGR) XQTypeswitch::Cases(XQillaAllocator<XQTypeswitch::Case*>(MEMMGR));
    $$->push_back(WRAP(@1, new (MEMMGR) XQTypeswitch::Case($2, $3, 0)));
  }
  | CaseSequenceTypeUnion _BAR_ SequenceType
  {
    REJECT_NOT_VERSION3(SequenceTypeUnion, @2);
    $1->push_back(WRAP(@3, new (MEMMGR) XQTypeswitch::Case($1->back()->getQName(), $3, 0)));
    $$ = $1;
  }
  ;

CaseClauseVariable:
    /* empty */
  {
    $$ = 0;
  }
  | _DOLLAR_ VarName _AS_
  {
    $$ = $2;
  }
  ;

// [47]    IfExpr    ::=    <"if" "("> Expr ")" "then" ExprSingle "else" ExprSingle 
IfExpr:
  _IF_ _LPAR_ Expr _RPAR_ _THEN_ ExprSingle _ELSE_ ExprSingle
  { 
    $$ = WRAP(@1, new (MEMMGR) XQIf($3, $6, $8, MEMMGR));
  }
;

// [48]    OrExpr    ::=    AndExpr ( "or"  AndExpr )* 
OrExpr:
  OrExpr _OR_ AndExpr
  {
    if($1->getType() == ASTNode::OPERATOR && ((XQOperator*)$1)->getOperatorName() == Or::name) {
      ((Or*)$1)->addArgument($3);
      $$ = $1;
    }
    else
      $$ = WRAP(@2, new (MEMMGR) Or(packageArgs($1, $3, MEMMGR), MEMMGR));
  }
  | AndExpr
  ;

// [49]    AndExpr    ::=    ComparisonExpr ( "and" ComparisonExpr )* 
AndExpr:
  AndExpr _AND_ ComparisonExpr
  {
    if($1->getType()==ASTNode::OPERATOR && ((XQOperator*)$1)->getOperatorName() == And::name) {
      ((And*)$1)->addArgument($3);
      $$ = $1;
    }
    else
      $$ = WRAP(@2, new (MEMMGR) And(packageArgs($1, $3, MEMMGR), MEMMGR));
  }
  | ComparisonExpr
  ;

// [50]    ComparisonExpr    ::=    FTContainsExpr ( (ValueComp 
//                  |  GeneralComp 
//                  |  NodeComp)  FTContainsExpr )? 
// [63]    GeneralComp    ::=    "=" |  "!=" |  "<" |  "<=" |  ">" |  ">=" 
// [64]    ValueComp    ::=    "eq" |  "ne" |  "lt" |  "le" |  "gt" |  "ge" 
// [65]    NodeComp    ::=    "is" |  "<<" |  ">>" 
ComparisonExpr:
  FTContainsExpr _EQUALS_ FTContainsExpr
  {
    $$ = WRAP(@2, new (MEMMGR) GeneralComp(GeneralComp::EQUAL,packageArgs($1,$3, MEMMGR),MEMMGR));
  }  
  | FTContainsExpr _BANG_EQUALS_ FTContainsExpr
  {
    $$ = WRAP(@2, new (MEMMGR) GeneralComp(GeneralComp::NOT_EQUAL,packageArgs($1,$3, MEMMGR),MEMMGR));
  }  
  | FTContainsExpr _LESS_THAN_OP_OR_TAG_ { /* Careful! */ QP->_lexer->undoLessThan(); } FTContainsExpr
  {
    $$ = WRAP(@2, new (MEMMGR) GeneralComp(GeneralComp::LESS_THAN,packageArgs($1,$4, MEMMGR),MEMMGR));
  }  
  | FTContainsExpr _LESS_THAN_EQUALS_ FTContainsExpr
  {
    $$ = WRAP(@2, new (MEMMGR) GeneralComp(GeneralComp::LESS_THAN_EQUAL,packageArgs($1,$3, MEMMGR),MEMMGR));
  }  
  | FTContainsExpr _GREATER_THAN_ FTContainsExpr
  {
    $$ = WRAP(@2, new (MEMMGR) GeneralComp(GeneralComp::GREATER_THAN,packageArgs($1,$3, MEMMGR),MEMMGR));
  }  
  | FTContainsExpr _GREATER_THAN_EQUALS_ FTContainsExpr
  {
    $$ = WRAP(@2, new (MEMMGR) GeneralComp(GeneralComp::GREATER_THAN_EQUAL,packageArgs($1,$3, MEMMGR),MEMMGR));
  }  
  | FTContainsExpr _EQ_ FTContainsExpr
  {
    $$ = WRAP(@2, new (MEMMGR) Equals(packageArgs($1, $3, MEMMGR),MEMMGR));
  }  
  | FTContainsExpr _NE_ FTContainsExpr
  {
    $$ = WRAP(@2, new (MEMMGR) NotEquals(packageArgs($1, $3, MEMMGR),MEMMGR));
  }  
  | FTContainsExpr _LT_ FTContainsExpr
  {
    $$ = WRAP(@2, new (MEMMGR) LessThan(packageArgs($1, $3, MEMMGR),MEMMGR));
  }  
  | FTContainsExpr _LE_ FTContainsExpr
  {
    $$ = WRAP(@2, new (MEMMGR) LessThanEqual(packageArgs($1, $3, MEMMGR),MEMMGR));
  }  
  | FTContainsExpr _GT_ FTContainsExpr
  {
    $$ = WRAP(@2, new (MEMMGR) GreaterThan(packageArgs($1, $3, MEMMGR),MEMMGR));
  }  
  | FTContainsExpr _GE_ FTContainsExpr
  {
    $$ = WRAP(@2, new (MEMMGR) GreaterThanEqual(packageArgs($1, $3, MEMMGR),MEMMGR));
  }  
  | FTContainsExpr _IS_ FTContainsExpr
  {
    $$ = WRAP(@2, new (MEMMGR) NodeComparison(packageArgs($1, $3, MEMMGR), MEMMGR));
  }
  | FTContainsExpr _LESS_THAN_LESS_THAN_ FTContainsExpr
  {
    $$ = WRAP(@2, new (MEMMGR) OrderComparison(packageArgs($1, $3, MEMMGR), true, MEMMGR));
  }
  | FTContainsExpr _GREATER_THAN_GREATER_THAN_ FTContainsExpr
  {
    $$ = WRAP(@2, new (MEMMGR) OrderComparison(packageArgs($1, $3, MEMMGR), false,MEMMGR));
  }
  | FTContainsExpr
  ;

// [51]    FTContainsExpr    ::=    RangeExpr ( "contains" "text" FTSelection FTIgnoreOption? )?
FTContainsExpr:
  RangeExpr _CONTAINS_ _TEXT_ FTSelection
  {
    $$ = WRAP(@2, new (MEMMGR) FTContains($1, $4, NULL, MEMMGR));
  }
  | RangeExpr _CONTAINS_ _TEXT_ FTSelection FTIgnoreOption
  {
    $$ = WRAP(@2, new (MEMMGR) FTContains($1, $4, $5, MEMMGR));
  }
  | RangeExpr
  ;

// [52]    RangeExpr    ::=    AdditiveExpr ( "to"  AdditiveExpr )?
RangeExpr:
  AdditiveExpr _TO_ AdditiveExpr
  {
    VectorOfASTNodes *args = new (MEMMGR) VectorOfASTNodes(XQillaAllocator<ASTNode*>(MEMMGR));
    args->push_back($1);
    args->push_back($3);
    $$ = WRAP(@2, new (MEMMGR) XQFunctionCall(0, BuiltInModules::core.uri, MEMMGR->getPooledString("to"), args, MEMMGR));
  }
  | AdditiveExpr
  ;

// [53]    AdditiveExpr    ::=    MultiplicativeExpr ( ("+" |  "-")  MultiplicativeExpr )* 
AdditiveExpr:
  AdditiveExpr _PLUS_ MultiplicativeExpr
  {
    $$ = WRAP(@2, new (MEMMGR) Plus(packageArgs($1, $3, MEMMGR),MEMMGR));
  }
  | AdditiveExpr _MINUS_ MultiplicativeExpr
  {
    $$ = WRAP(@2, new (MEMMGR) Minus(packageArgs($1, $3, MEMMGR),MEMMGR));
  }
  | MultiplicativeExpr
  ;

// [54]    MultiplicativeExpr    ::=    UnionExpr ( ("*" |  "div" |  "idiv" |  "mod")  UnionExpr )* 
MultiplicativeExpr:
  MultiplicativeExpr _ASTERISK_ UnionExpr 
  {
    $$ = WRAP(@2, new (MEMMGR) Multiply(packageArgs($1, $3, MEMMGR),MEMMGR));
  }
  | MultiplicativeExpr _DIV_ UnionExpr 
  {
    $$ = WRAP(@2, new (MEMMGR) Divide(packageArgs($1, $3, MEMMGR),MEMMGR));
  }
  | MultiplicativeExpr _IDIV_ UnionExpr 
  {
    $$ = WRAP(@2, new (MEMMGR) IntegerDivide(packageArgs($1, $3, MEMMGR),MEMMGR));
  }
  | MultiplicativeExpr _MOD_ UnionExpr 
  {
    $$ = WRAP(@2, new (MEMMGR) Mod(packageArgs($1, $3, MEMMGR),MEMMGR));
  }
  | UnionExpr 
  ;

// [55]    UnionExpr    ::=    IntersectExceptExpr ( ("union" |  "|")  IntersectExceptExpr )* 
UnionExpr:
      UnionExpr _BAR_ IntersectExceptExpr
    {
      $$ = WRAP(@2, new (MEMMGR) Union(packageArgs($1, $3, MEMMGR),MEMMGR));
    }
    | UnionExpr _UNION_ IntersectExceptExpr
    {
      $$ = WRAP(@2, new (MEMMGR) Union(packageArgs($1, $3, MEMMGR),MEMMGR));
    }
  | IntersectExceptExpr
    ;

// [56]    IntersectExceptExpr    ::=    InstanceofExpr ( ("intersect" |  "except")  InstanceofExpr )* 
IntersectExceptExpr:
      IntersectExceptExpr _INTERSECT_ InstanceofExpr
    {
      $$ = WRAP(@2, new (MEMMGR) Intersect(packageArgs($1, $3, MEMMGR),MEMMGR));
    }
    | IntersectExceptExpr _EXCEPT_ InstanceofExpr
    {
      $$ = WRAP(@2, new (MEMMGR) Except(packageArgs($1, $3, MEMMGR),MEMMGR));
    }
  | InstanceofExpr
    ;

// [57]    InstanceofExpr    ::=    TreatExpr ( <"instance" "of"> SequenceType )? 
InstanceofExpr:
  TreatExpr _INSTANCE_ _OF_ SequenceType
  {
    ASTNode *falseExpr =
      WRAP(@2, new (MEMMGR) XQFunctionCall(0, XQFunction::XMLChFunctionURI,
                                           MEMMGR->getPooledString("false"), 0, MEMMGR));
    XQTypeswitch::Case *defcase =
      WRAP(@1, new (MEMMGR) XQTypeswitch::Case(NULL, NULL, falseExpr));

    ASTNode *trueExpr =
      WRAP(@2, new (MEMMGR) XQFunctionCall(0, XQFunction::XMLChFunctionURI,
                                           MEMMGR->getPooledString("true"), 0, MEMMGR));
    XQTypeswitch::Cases *cases = new (MEMMGR)
      XQTypeswitch::Cases(XQillaAllocator<XQTypeswitch::Case*>(MEMMGR));
    cases->push_back(WRAP(@2, new (MEMMGR) XQTypeswitch::Case(NULL, $4, trueExpr)));

    $$ = WRAP(@2, new (MEMMGR) XQTypeswitch($1, cases, defcase, MEMMGR));
  }
  | TreatExpr
  ;

// [58]    TreatExpr    ::=    CastableExpr ( <"treat" "as"> SequenceType )? 
TreatExpr:
  CastableExpr _TREAT_ _AS_ SequenceType
  {
    XQTreatAs* treatAs = new (MEMMGR) XQTreatAs($1,$4,MEMMGR, XQTreatAs::err_XPDY0050);
    $$ = WRAP(@2, treatAs);
  }
  | CastableExpr
  ;

// [59]    CastableExpr    ::=    CastExpr ( <"castable" "as"> SingleType )? 
CastableExpr:
  CastExpr _CASTABLE_ _AS_ SingleType
  {
    $$ = WRAP(@2, new (MEMMGR) XQCastableAs($1,$4,MEMMGR));
  }
  | CastExpr
  ;

// [60]    CastExpr    ::=    UnaryExpr (<"cast" "as"> SingleType)?
CastExpr:
  UnaryExpr _CAST_ _AS_ SingleType
  {
    $$ = WRAP(@1, new (MEMMGR) XQCastAs($1,$4,MEMMGR));
  }
  | UnaryExpr
  ;

// [61]    UnaryExpr    ::=    ("-" |  "+")* ValueExpr 
UnaryExpr:
      _MINUS_ UnaryExpr
    {
      VectorOfASTNodes args(XQillaAllocator<ASTNode*>(MEMMGR));
      args.push_back($2);
      $$ = WRAP(@1, new (MEMMGR) UnaryMinus(/*positive*/false, args, MEMMGR));
    }
    | _PLUS_ UnaryExpr
    {
      VectorOfASTNodes args(XQillaAllocator<ASTNode*>(MEMMGR));
      args.push_back($2);
      $$ = WRAP(@1, new (MEMMGR) UnaryMinus(/*positive*/true, args, MEMMGR));
    }
  | ValueExpr 
    ;

// [62]    ValueExpr    ::=    ValidateExpr | PathExpr | ExtensionExpr
ValueExpr:
    ValidateExpr
  | PathExpr
  | ExtensionExpr
  ;

// [66]    ValidateExpr    ::=    (<"validate" "{"> |  
//                   (<"validate" ValidationMode> "{")
//                  ) Expr "}" 
// [67]    ValidationMode    ::=    "lax" | "strict"
ValidateExpr:
    _VALIDATE_ _LBRACE_ Expr _RBRACE_
    {
      $$ = WRAP(@1, new (MEMMGR) XQValidate($3,DocumentCache::VALIDATION_STRICT,MEMMGR));
    }
  | _VALIDATE_ _LAX_ _LBRACE_ Expr _RBRACE_
    {
      $$ = WRAP(@1, new (MEMMGR) XQValidate($4,DocumentCache::VALIDATION_LAX,MEMMGR));
    }
  | _VALIDATE_ _STRICT_ _LBRACE_ Expr _RBRACE_
    {
      $$ = WRAP(@1, new (MEMMGR) XQValidate($4,DocumentCache::VALIDATION_STRICT,MEMMGR));
    }
  ;

// [68]     ExtensionExpr     ::=     Pragma+ "{" Expr? "}"
ExtensionExpr:
    PragmaList _LBRACE_ _RBRACE_
  {
    REJECT_NOT_XQUERY(ExtensionExpr, @1);

    // we don't support any pragma
    yyerror(@1, "This pragma is not recognized, and no alternative expression is specified [err:XQST0079]");
  }
  | PragmaList _LBRACE_ Expr _RBRACE_
  {
    REJECT_NOT_XQUERY(ExtensionExpr, @1);

    // we don't support any pragma
    $$ = $3;
  }
  ;

PragmaList: Pragma | PragmaList Pragma;

// [69]     Pragma     ::=     "(#" S? QName (S PragmaContents)? "#)"
Pragma:
    _PRAGMA_OPEN_ OptionalWhitespace QNameValue PragmaContents
      {
        // validate the QName
        QualifiedName qName($3);
        const XMLCh* prefix=qName.getPrefix();
        if(prefix == NULL || *prefix == 0)
          yyerror(@3, "The pragma name must have a prefix [err:XPST0081]");

        try {
          LOCATION(@3, loc);
          CONTEXT->getUriBoundToPrefix(prefix, &loc);
        }
        catch(NamespaceLookupException&) {
          yyerror(@3, "The pragma name is using an undefined namespace prefix [err:XPST0081]");
        }
      }
  ;

// [70]     PragmaContents     ::=     (Char* - (Char* '#)' Char*))
PragmaContents:
  _PRAGMA_CONTENT_
  | _WHITESPACE_ _PRAGMA_CONTENT_
  {
    $$ = $2;
  }
;

// [71]    PathExpr    ::=    ("/" RelativePathExpr?) |  ("//" RelativePathExpr) |  RelativePathExpr 
PathExpr:
  LeadingSlash
  | LeadingSlash RelativePathExpr
  {
    XQNav *nav = GET_NAVIGATION(@1, $1);
    nav->addStep($2);
    $$ = nav;
  }
  | _SLASH_SLASH_ RelativePathExpr
  {
    XQNav *nav = WRAP(@1, new (MEMMGR) XQNav(MEMMGR));

    VectorOfASTNodes args(XQillaAllocator<ASTNode*>(MEMMGR));
    FunctionRoot *root = WRAP(@1, new (MEMMGR) FunctionRoot(args, MEMMGR));

    SequenceType *documentNode = WRAP(@1, new (MEMMGR)
      SequenceType(new (MEMMGR) SequenceType::ItemType(SequenceType::ItemType::TEST_DOCUMENT)));

    nav->addStep(WRAP(@1, new (MEMMGR) XQTreatAs(root, documentNode, MEMMGR)));

    NodeTest *step = new (MEMMGR) NodeTest();
    step->setTypeWildcard();
    step->setNameWildcard();
    step->setNamespaceWildcard();
    nav->addStep(WRAP(@1, new (MEMMGR) XQStep(XQStep::DESCENDANT_OR_SELF, step, MEMMGR)));

    nav->addStep($2);
    $$ = nav;
  }
  | RelativePathExpr
  ;

LeadingSlash:
  _SLASH_
  {
    VectorOfASTNodes args(XQillaAllocator<ASTNode*>(MEMMGR));
    FunctionRoot *root = WRAP(@1, new (MEMMGR) FunctionRoot(args, MEMMGR));

    SequenceType *documentNode = WRAP(@1, new (MEMMGR)
      SequenceType(new (MEMMGR) SequenceType::ItemType(SequenceType::ItemType::TEST_DOCUMENT)));

    $$ = WRAP(@1, new (MEMMGR) XQTreatAs(root, documentNode, MEMMGR));
  }
;

// [72]    RelativePathExpr    ::=    StepExpr (("/" |  "//") StepExpr)* 
RelativePathExpr:
  RelativePathExpr _SLASH_ StepExpr
  {
    XQNav *nav = GET_NAVIGATION(@1, $1);
    nav->addStep($3);
    $$ = nav;
  }
  | RelativePathExpr _SLASH_SLASH_ StepExpr
  {
    XQNav *nav = GET_NAVIGATION(@1, $1);

    NodeTest *step = new (MEMMGR) NodeTest();
    step->setTypeWildcard();
    step->setNameWildcard();
    step->setNamespaceWildcard();
    nav->addStep(WRAP(@2, new (MEMMGR) XQStep(XQStep::DESCENDANT_OR_SELF, step, MEMMGR)));
    nav->addStep($3);

    $$ = nav;
  }
  | StepExpr
  ;

// [73]    StepExpr    ::=    AxisStep |  PostfixExpr
StepExpr: AxisStep | PostfixExpr;

// [74]    AxisStep    ::=    (ForwardStep |  ReverseStep) PredicateList 
AxisStep:
    ForwardStepPredicateList
  | ReverseStepPredicateList
  {
    $$ = WRAP(@1, new (MEMMGR) XQDocumentOrder($1, MEMMGR));
  }
  ;

// [85]    PredicateList    ::=    Predicate* 
// [86]    Predicate    ::=    "[" Expr "]" 
ForwardStepPredicateList:
    ForwardStep
  | ForwardStepPredicateList _LSQUARE_ Expr _RSQUARE_
  {
    $$ = WRAP(@2, new (MEMMGR) XQPredicate($1, $3, MEMMGR));
  }
  ;
ReverseStepPredicateList:
    ReverseStep
  | ReverseStepPredicateList _LSQUARE_ Expr _RSQUARE_
  {
    $$ = WRAP(@2, new (MEMMGR) XQPredicate($1, $3, /*reverse*/true, MEMMGR));
  }
  ;


// [75]    ForwardStep    ::=    (ForwardAxis NodeTest) |  AbbrevForwardStep 
ForwardStep:
  ForwardAxis NodeTest
  {
    if(!$2->isNodeTypeSet()) {
      switch($1) {
      case XQStep::NAMESPACE: $2->setNodeType(Node::namespace_string); break;
      case XQStep::ATTRIBUTE: $2->setNodeType(Node::attribute_string); break;
      default: $2->setNodeType(Node::element_string); break;
      }
    }

    $$ = WRAP(@1, new (MEMMGR) XQStep($1,$2,MEMMGR));
  }
  | AbbrevForwardStep
  ;

// [76]    ForwardAxis    ::=    <"child" "::">
//                |  <"descendant" "::">
//                |  <"attribute" "::">
//                |  <"self" "::">
//                |  <"descendant-or-self" "::"> 
//                |  <"following-sibling" "::">
//                |  <"following" "::">
ForwardAxis:
  _CHILD_ _COLON_COLON_
  {
    $$ = XQStep::CHILD;
  }
  | _DESCENDANT_ _COLON_COLON_
  {
    $$ = XQStep::DESCENDANT;
  }
  | _ATTRIBUTE_ _COLON_COLON_
  {
    $$ = XQStep::ATTRIBUTE;
  }
  | _SELF_ _COLON_COLON_
  {
    $$ = XQStep::SELF;
  }
  | _DESCENDANT_OR_SELF_ _COLON_COLON_
  {
    $$ = XQStep::DESCENDANT_OR_SELF;
  }
  | _FOLLOWING_SIBLING_ _COLON_COLON_
  {
    $$ = XQStep::FOLLOWING_SIBLING;
  }
  | _FOLLOWING_ _COLON_COLON_
  {
    $$ = XQStep::FOLLOWING;
  }
  | _NAMESPACE_ _COLON_COLON_
  {
    $$ = XQStep::NAMESPACE;
  }
  ;

// [77]    AbbrevForwardStep    ::=    "@"? NodeTest
AbbrevForwardStep:
  _AT_SIGN_ NodeTest
  {
    if(!$2->isNodeTypeSet()) {
      $2->setNodeType(Node::attribute_string);
    }

    $$ = WRAP(@1, new (MEMMGR) XQStep(XQStep::ATTRIBUTE, $2, MEMMGR));
  }
  | NodeTest
  {
    XQStep::Axis axis = XQStep::CHILD;
    SequenceType::ItemType *itemtype = $1->getItemType();
    if(itemtype != 0 && itemtype->getItemTestType() == SequenceType::ItemType::TEST_ATTRIBUTE) {
      axis = XQStep::ATTRIBUTE;
    }
    else if(!$1->isNodeTypeSet()) {
      $1->setNodeType(Node::element_string);
    }

    $$ = WRAP(@1, new (MEMMGR) XQStep(axis, $1, MEMMGR));
  }
  ;

// [78]    ReverseStep    ::=    (ReverseAxis NodeTest) |  AbbrevReverseStep 
ReverseStep:
  ReverseAxis NodeTest
  {
    if(!$2->isNodeTypeSet()) {
      $2->setNodeType(Node::element_string);
    }

    $$ = WRAP(@1, new (MEMMGR) XQStep($1, $2, MEMMGR));
  }
  | AbbrevReverseStep 
  ;

// [79]    ReverseAxis    ::=    <"parent" "::"> 
//                | <"ancestor" "::">
//                | <"preceding-sibling" "::">
//                | <"preceding" "::">
//                | <"ancestor-or-self" "::">
ReverseAxis:
  _PARENT_ _COLON_COLON_
  {
    $$ = XQStep::PARENT;
  }
  | _ANCESTOR_ _COLON_COLON_
  {
    $$ = XQStep::ANCESTOR;
  }
  | _PRECEDING_SIBLING_ _COLON_COLON_
  {
    $$ = XQStep::PRECEDING_SIBLING;
  }
  | _PRECEDING_ _COLON_COLON_
  {
    $$ = XQStep::PRECEDING;
  }
  | _ANCESTOR_OR_SELF_ _COLON_COLON_
  {
    $$ = XQStep::ANCESTOR_OR_SELF;
  }
  ;

// [80]    AbbrevReverseStep    ::=    ".." 
AbbrevReverseStep:
  _DOT_DOT_
  {
    NodeTest *step = new (MEMMGR) NodeTest();
    step->setNameWildcard();
    step->setNamespaceWildcard();
    step->setTypeWildcard();
    $$ = WRAP(@1, new (MEMMGR) XQStep(XQStep::PARENT, step, MEMMGR));
  }  
  ;

// [81]    NodeTest    ::=    KindTest |  NameTest 
NodeTest:
  KindTest
  {
    $$ = new (MEMMGR) NodeTest();
    $$->setItemType($1);
  }
  | NameTest
  ;

// [82]    NameTest    ::=    QName |  Wildcard 
NameTest:
  QName
  {
    NodeTest *step = new (MEMMGR) NodeTest();
    step->setNodePrefix($1->getPrefix());
    step->setNodeName($1->getName());
    $$ = step;
  }
  | Wildcard
  ;

// [83]    Wildcard    ::=    "*" |  <NCName ":" "*"> |  <"*" ":" NCName> 
Wildcard:
  _ASTERISK_
  {
    NodeTest *step = new (MEMMGR) NodeTest();
    step->setNameWildcard();
    step->setNamespaceWildcard();
    $$ = step;
  }
  | _NCNAME_COLON_STAR_
  {
    NodeTest *step = new (MEMMGR) NodeTest();
    step->setNodePrefix($1);
    step->setNameWildcard();
    $$ = step;
  }
  | _STAR_COLON_NCNAME_
  {
    NodeTest *step = new (MEMMGR) NodeTest();
    step->setNodeName($1);
    step->setNamespaceWildcard();
    $$ = step;
  }
  ;

// [121]   	PostfixExpr	   ::=   	PrimaryExpr (Predicate | ArgumentList)*
// [124]   	Predicate	   ::=   	"[" Expr "]"
PostfixExpr:
    PrimaryExpr
  | PostfixExpr _LSQUARE_ Expr _RSQUARE_
  {
    $$ = WRAP(@2, new (MEMMGR) XQPredicate($1, $3, MEMMGR));
  }
  | DynamicFunctionInvocation
  ;

// [87]    PrimaryExpr    ::=    Literal |  VarRef | ParenthesizedExpr | ContextItemExpr | FunctionCall | Constructor | OrderedExpr | UnorderedExpr
//                               | FunctionItemExpr
PrimaryExpr:
    Literal
  | VarRef
  | ParenthesizedExpr
  | ContextItemExpr
  | FunctionCall
  | Constructor
  | OrderedExpr 
  | UnorderedExpr
  | FunctionItemExpr
  ;

// [88]    Literal    ::=    NumericLiteral |  StringLiteral 
Literal: NumericLiteral | StringLiteral;

// [89]    NumericLiteral    ::=    IntegerLiteral |  DecimalLiteral |  DoubleLiteral 
NumericLiteral: IntegerLiteral | DecimalLiteral | DoubleLiteral;

// [90]    VarRef    ::=    "$" VarName 
VarRef:
  _DOLLAR_ VarName
  {
    $$ = WRAP(@1, new (MEMMGR) XQVariable($2, MEMMGR));
  }
  ;

// [91]    VarName    ::=    QName
VarName: QNameValue;
    
// [92]    ParenthesizedExpr    ::=    "(" Expr? ")" 
ParenthesizedExpr:
  _LPAR_ Expr _RPAR_
  { 
    $$ = $2;
  }
  | _LPAR_ _RPAR_
  { 
    $$ = WRAP(@1, new (MEMMGR) XQSequence(MEMMGR));
  }
;

// [93]    ContextItemExpr    ::=    "." 
ContextItemExpr:
  _DOT_
  {
    $$ = WRAP(@1, new (MEMMGR) XQContextItem(MEMMGR));
  }
  ;

// [94]    OrderedExpr    ::=    <"ordered" "{"> Expr "}" 
OrderedExpr:
  _ORDERED_ _LBRACE_ Expr _RBRACE_
  {
    REJECT_NOT_XQUERY(OrderedExpr, @1);

    $$ = WRAP(@1, new (MEMMGR) XQOrderingChange(StaticContext::ORDERING_ORDERED, $3, MEMMGR));
  }
  ;

// [95]    UnorderedExpr    ::=    <"unordered" "{"> Expr "}" 
UnorderedExpr:
  _UNORDERED_ _LBRACE_ Expr _RBRACE_
  {
    REJECT_NOT_XQUERY(UnorderedExpr, @1);

    $$ = WRAP(@1, new (MEMMGR) XQOrderingChange(StaticContext::ORDERING_UNORDERED, $3, MEMMGR));
  }
  ;

// [134]   	FunctionCall	   ::=   	QName "(" (Argument ("," Argument)*)? ")"
FunctionCall:
  FunctionName _LPAR_ _RPAR_
  {
    $$ = WRAP(@1, new (MEMMGR) XQFunctionCall($1, NULL, MEMMGR));
  }
  | FunctionName _LPAR_ FunctionCallArgumentList _RPAR_
  {
    bool partial = false;
    VectorOfASTNodes::iterator i;
    for(i = $3->begin(); i != $3->end(); ++i) {
      if(*i == 0) {
        partial = true;
        break;
      }
    }

    if(partial) {
      // This is a partial function application
      XQFunctionRef *ref = WRAP(@1, new (MEMMGR) XQFunctionRef($1, $3->size(), MEMMGR));
      $$ = WRAP(@2, new (MEMMGR) XQPartialApply(ref, $3, MEMMGR));
    }
    else {
      $$ = WRAP(@1, new (MEMMGR) XQFunctionCall($1, $3, MEMMGR));
    }
  }
  ;

FunctionCallArgumentList:
  FunctionCallArgumentList _COMMA_ Argument
  {
    $1->push_back($3);
    $$ = $1;
  }
  | Argument
  {
    $$ = new (MEMMGR) VectorOfASTNodes(XQillaAllocator<ASTNode*>(MEMMGR));
    $$->push_back($1);
  }  
  ;

// [135]   	Argument	   ::=   	ExprSingle | ArgumentPlaceholder
// [136]   	ArgumentPlaceholder	   ::=   	"?"
Argument:
    ExprSingle
  | _QUESTION_MARK_
  {
    REJECT_NOT_VERSION3(Argument, @1);
    $$ = 0;
  }
  ;

// [97]    Constructor    ::=    DirectConstructor | ComputedConstructor 
Constructor:
  DirectConstructor
  {
    REJECT_NOT_XQUERY(Constructor, @1);
  }
  | ComputedConstructor
  {
    REJECT_NOT_XQUERY(Constructor, @1);
  }
  ;

// [98]    DirectConstructor    ::=    DirElemConstructor
//                   | DirCommentConstructor
//                   | DirPIConstructor 
DirectConstructor: DirElemConstructor | DirCommentConstructor | DirPIConstructor;

// [99]    DirElemConstructor    ::=    "<" QName DirAttributeList ("/>" |  (">" DirElementContent* "</" QName S? ">")) 
DirElemConstructor:
  _LESS_THAN_OP_OR_TAG_ DirElemConstructorQName DirAttributeList OptionalWhitespace _EMPTY_TAG_CLOSE_
  { 
    VectorOfASTNodes* content = new (MEMMGR) VectorOfASTNodes(XQillaAllocator<ASTNode*>(MEMMGR));
    ASTNode *name = WRAP(@2, new (MEMMGR) XQDirectName($2, /*useDefaultNamespace*/true, MEMMGR));
    $$ = WRAP(@1, new (MEMMGR) XQElementConstructor(name, $3, content, MEMMGR));
  }
  | _LESS_THAN_OP_OR_TAG_ DirElemConstructorQName DirAttributeList OptionalWhitespace _START_TAG_CLOSE_ DirElementContent _END_TAG_OPEN_ DirElemConstructorQName OptionalWhitespace _END_TAG_CLOSE_
  { 
    if(!XPath2Utils::equals($2, $8))
      yyerror(@7, "Close tag does not match open tag");
    ASTNode *name = WRAP(@2, new (MEMMGR) XQDirectName($2, /*useDefaultNamespace*/true, MEMMGR));
    $$ = WRAP(@1, new (MEMMGR) XQElementConstructor(name, $3, $6, MEMMGR));
  }
  ;

DirElemConstructorQName: _QNAME_ | _XMLNS_QNAME_;

// [100]    DirAttributeList    ::=    (S (QName S? "=" S? DirAttributeValue)?)* 
DirAttributeList: 
  /* empty */
  {
    $$ = new (MEMMGR) VectorOfASTNodes(XQillaAllocator<ASTNode*>(MEMMGR));
  }
  | DirAttributeList _WHITESPACE_ _QNAME_ OptionalWhitespace _VALUE_INDICATOR_ OptionalWhitespace DirAttributeValue
  {
    $$ = $1;

    ASTNode *name = WRAP(@3, new (MEMMGR) XQDirectName($3, /*useDefaultNamespace*/false, MEMMGR));
    ASTNode *attrItem = WRAP(@3, new (MEMMGR) XQAttributeConstructor(name, $7,MEMMGR));

    $$->push_back(attrItem);
  }
  | DirAttributeList _WHITESPACE_ _XMLNS_QNAME_ OptionalWhitespace _VALUE_INDICATOR_ OptionalWhitespace LiteralDirAttributeValue
  {
    $$ = $1;

    ASTNode *name = WRAP(@3, new (MEMMGR) XQDirectName($3, /*useDefaultNamespace*/false, MEMMGR));
    ASTNode *attrItem = WRAP(@3, new (MEMMGR) XQAttributeConstructor(name, $7,MEMMGR));

    $$->insert($$->begin(), attrItem);
  }
  ;

OptionalWhitespace: /* empty */ | _WHITESPACE_;

// [101]    DirAttributeValue    ::=    ('"' (EscapeQuot |  QuotAttrValueContent)* '"')
//                 |  ("'" (EscapeApos |  AposAttrValueContent)* "'") 
DirAttributeValue:
  _OPEN_QUOT_ QuotAttrValueContent _CLOSE_QUOT_
  { 
    $$ = $2;
  }
  | _OPEN_APOS_ AposAttrValueContent _CLOSE_APOS_
  { 
    $$ = $2;
  }
  ;

LiteralDirAttributeValue:
  _OPEN_QUOT_ LiteralQuotAttrValueContent _CLOSE_QUOT_
  { 
    $$ = $2;
  }
  | _OPEN_APOS_ LiteralAposAttrValueContent _CLOSE_APOS_
  { 
    $$ = $2;
  }
  ;

// [102]    QuotAttrValueContent    ::=    QuotAttContentChar
//                    |  CommonContent
// [105]    CommonContent    ::=    PredefinedEntityRef | CharRef | "{{" | "}}" | EnclosedExpr 
QuotAttrValueContent:
  /* empty */
  { 
    $$ = new (MEMMGR) VectorOfASTNodes(XQillaAllocator<ASTNode*>(MEMMGR));
  }
  | QuotAttrValueContent EnclosedExpr
  {
    $$ = $1;
    $$->push_back($2);
  }
  | QuotAttrValueContent _QUOT_ATTR_CONTENT_
  {
    $$ = $1;
    $$->push_back(WRAP(@2, new (MEMMGR) XQLiteral(SchemaSymbols::fgURI_SCHEMAFORSCHEMA,
                               SchemaSymbols::fgDT_STRING,
                               $2, AnyAtomicType::STRING, MEMMGR)));
  }
  ;

LiteralQuotAttrValueContent:
  /* empty */
  { 
    $$ = new (MEMMGR) VectorOfASTNodes(XQillaAllocator<ASTNode*>(MEMMGR));
  }
  | LiteralQuotAttrValueContent EnclosedExpr
  {
    yyerror(@2, "Namespace URI of a namespace declaration must be a literal [err:XQST0022]");
  }
  | LiteralQuotAttrValueContent _QUOT_ATTR_CONTENT_
  {
    $$ = $1;
    $$->push_back(WRAP(@2, new (MEMMGR) XQLiteral(SchemaSymbols::fgURI_SCHEMAFORSCHEMA,
                               SchemaSymbols::fgDT_STRING,
                               $2, AnyAtomicType::STRING, MEMMGR)));
  }
  ;

// [103]    AposAttrValueContent    ::=    AposAttContentChar
//                    |  CommonContent
// [105]    CommonContent    ::=    PredefinedEntityRef | CharRef | "{{" | "}}" | EnclosedExpr 
AposAttrValueContent:
  /* empty */
  { 
    $$ = new (MEMMGR) VectorOfASTNodes(XQillaAllocator<ASTNode*>(MEMMGR));
  }
  | AposAttrValueContent EnclosedExpr
  {
    $$ = $1;
    $$->push_back($2);
  }
  | AposAttrValueContent _APOS_ATTR_CONTENT_
  {
    $$ = $1;
    $$->push_back(WRAP(@2, new (MEMMGR) XQLiteral(SchemaSymbols::fgURI_SCHEMAFORSCHEMA,
                               SchemaSymbols::fgDT_STRING,
                               $2, AnyAtomicType::STRING, MEMMGR)));
  }
  ;

LiteralAposAttrValueContent:
  /* empty */
  { 
    $$ = new (MEMMGR) VectorOfASTNodes(XQillaAllocator<ASTNode*>(MEMMGR));
  }
  | LiteralAposAttrValueContent EnclosedExpr
  {
    yyerror(@2, "Namespace URI of a namespace declaration must be a literal [err:XQST0022]");
  }
  | LiteralAposAttrValueContent _APOS_ATTR_CONTENT_
  {
    $$ = $1;
    $$->push_back(WRAP(@2, new (MEMMGR) XQLiteral(SchemaSymbols::fgURI_SCHEMAFORSCHEMA,
                               SchemaSymbols::fgDT_STRING,
                               $2, AnyAtomicType::STRING, MEMMGR)));
  }
  ;

// [104]    DirElementContent    ::=    DirectConstructor 
//                  |  ElementContentChar
//                  |  CdataSection 
//                  |  CommonContent
// [105]    CommonContent    ::=    PredefinedEntityRef | CharRef | "{{" | "}}" | EnclosedExpr 
// [110]    CDataSection    ::=    "<![CDATA[" CDataSectionContents "]]>"
// [111]    CDataSectionContents    ::=    (Char* - (Char* ']]>' Char*)) 
DirElementContent:
  /* empty */
  {
    $$ = new (MEMMGR) VectorOfASTNodes(XQillaAllocator<ASTNode*>(MEMMGR)); 
  }
  |  DirElementContent DirectConstructor
  {
    $$ = $1;
    $$->push_back($2);
  }
  | DirElementContent EnclosedExpr
  {
    $$ = $1;
    $$->push_back($2);
  }
  | DirElementContent _ELEMENT_CONTENT_
  {
    $$ = $1;
    $$->push_back(WRAP(@2, new (MEMMGR) XQLiteral(SchemaSymbols::fgURI_SCHEMAFORSCHEMA,
                               SchemaSymbols::fgDT_STRING,
                               $2, AnyAtomicType::STRING, MEMMGR)));
  }
  | DirElementContent _WHITESPACE_ELEMENT_CONTENT_
  {
    $$ = $1;
    if(CONTEXT->getPreserveBoundarySpace()) {
      $$->push_back(WRAP(@2, new (MEMMGR) XQLiteral(SchemaSymbols::fgURI_SCHEMAFORSCHEMA,
                                 SchemaSymbols::fgDT_STRING,
                                 $2, AnyAtomicType::STRING, MEMMGR)));
    }
  }
  ;

// [106]    DirCommentConstructor    ::=    "<!--" DirCommentContents "-->"
DirCommentConstructor:
  _XML_COMMENT_START_ DirCommentContents _XML_COMMENT_END_
  {
    ASTNode *value = WRAP(@1, new (MEMMGR) XQLiteral(
                  SchemaSymbols::fgURI_SCHEMAFORSCHEMA,
                  SchemaSymbols::fgDT_STRING,
                  $2, AnyAtomicType::STRING,
                  MEMMGR));
    $$ = WRAP(@1, new (MEMMGR) XQCommentConstructor(value, MEMMGR));
  }
  ;

// [107]    DirCommentContents    ::=    ((Char - '-') | <'-' (Char - '-')>)* 
DirCommentContents:
  /* empty */
  {
    $$ = (XMLCh*)XMLUni::fgZeroLenString;
  }
  | _XML_COMMENT_CONTENT_
  ;

// [108]    DirPIConstructor    ::=    "<?" PITarget (S DirPIContents)? "?>"
DirPIConstructor:
  _PI_START_ _PI_TARGET_ DirPIContents
  {
    ASTNode *value = WRAP(@3, new (MEMMGR) XQLiteral(
                  SchemaSymbols::fgURI_SCHEMAFORSCHEMA,
                  SchemaSymbols::fgDT_STRING,
                  $3, AnyAtomicType::STRING,
                  MEMMGR));
    $$ = WRAP(@1, new (MEMMGR) XQPIConstructor(
                    WRAP(@2, new (MEMMGR) XQLiteral(
                  SchemaSymbols::fgURI_SCHEMAFORSCHEMA,
                  SchemaSymbols::fgDT_STRING,
                  $2, AnyAtomicType::STRING,
                  MEMMGR)), 
                  value, MEMMGR));
  }
  ;

// [109]    DirPIContents    ::=    (Char* - (Char* '?>' Char*)) 
DirPIContents:
  _PI_CONTENT_
  | _WHITESPACE_ _PI_CONTENT_
  {
    $$ = $2;
  }
  ;

// [152] ComputedConstructor ::= CompDocConstructor
//                             | CompElemConstructor
//                             | CompAttrConstructor
//                             | CompNamespaceConstructor
//                             | CompTextConstructor
//                             | CompCommentConstructor
//                             | CompPIConstructor
ComputedConstructor:
  CompDocConstructor
  | CompElemConstructor
  | CompAttrConstructor
  | CompNamespaceConstructor
  | CompTextConstructor
  | CompCommentConstructor
  | CompPIConstructor 
  ;

// [113]    CompDocConstructor    ::=    <"document" "{"> Expr "}" 
CompDocConstructor:
  _DOCUMENT_ _LBRACE_ Expr _RBRACE_
  {
    $$ = WRAP(@1, new (MEMMGR) XQDocumentConstructor($3, MEMMGR));
  }
  ;

// [114]    CompElemConstructor    ::=    (<"element" QName "{"> |  (<"element" "{"> Expr "}" "{")) ContentExpr? "}" 
CompElemConstructor:
  _ELEMENT_ CompElementName ContentExpr
  {
    VectorOfASTNodes* empty = new (MEMMGR) VectorOfASTNodes(XQillaAllocator<ASTNode*>(MEMMGR));
    $$ = WRAP(@1, new (MEMMGR) XQElementConstructor($2, empty, $3, MEMMGR));
  }
  ;

CompElementName:
  _CONSTR_QNAME_
  {
    $$ = WRAP(@1, new (MEMMGR) XQDirectName($1, /*useDefaultNamespace*/true, MEMMGR));
  }
  | _LBRACE_ Expr _RBRACE_
  {
    $$ = $2;
  }
  ;

// [115]    ContentExpr    ::=    Expr
ContentExpr:
  _LBRACE_ _RBRACE_
  {
    $$ = new (MEMMGR) VectorOfASTNodes(XQillaAllocator<ASTNode*>(MEMMGR));
  }
  | _LBRACE_ Expr _RBRACE_
  {
    $$ = new (MEMMGR) VectorOfASTNodes(XQillaAllocator<ASTNode*>(MEMMGR));
    $$->push_back($2);
  }
  ;

// [116]    CompAttrConstructor    ::=    (<"attribute" QName "{"> |  (<"attribute" "{"> Expr "}" "{")) Expr? "}" 
CompAttrConstructor:
  _ATTRIBUTE_ CompAttrName ContentExpr
  {
    $$ = WRAP(@1, new (MEMMGR) XQAttributeConstructor($2, $3, MEMMGR));
  }
  ;

CompAttrName:
  _CONSTR_QNAME_
  {
    $$ = WRAP(@1, new (MEMMGR) XQDirectName($1, /*useDefaultNamespace*/false, MEMMGR));
  }
  | _LBRACE_ Expr _RBRACE_
  {
    $$ = $2;
  }
  ;

// [157]   CompNamespaceConstructor   ::=   "namespace" (Prefix | ("{" PrefixExpr "}")) "{" URIExpr? "}"
// [159]   PrefixExpr   ::=   Expr
// [160]   URIExpr   ::=   Expr
CompNamespaceConstructor:
  _NAMESPACE_ CompPINCName ContentExpr
  {
    $$ = WRAP(@1, new (MEMMGR) XQNamespaceConstructor($2, $3, MEMMGR));
  }
  ;

// [117]    CompTextConstructor    ::=    <"text" "{"> Expr "}" 
CompTextConstructor:
  _TEXT_ _LBRACE_ Expr _RBRACE_
  {
    $$ = WRAP(@1, new (MEMMGR) XQTextConstructor($3, MEMMGR));
  }
  ;

// [118]    CompCommentConstructor    ::=    <"comment" "{"> Expr "}" 
CompCommentConstructor:
  _COMMENT_ _LBRACE_ Expr _RBRACE_
  {
    $$ = WRAP(@1, new (MEMMGR) XQCommentConstructor($3, MEMMGR));
  }
  ;

// [119]    CompPIConstructor    ::=    (<"processing-instruction" NCName "{"> | (<"processing-instruction" "{"> Expr "}" "{")) Expr? "}" 
CompPIConstructor:
  _PROCESSING_INSTRUCTION_ CompPINCName CompPIConstructorContent
  {
    $$ = WRAP(@1, new (MEMMGR) XQPIConstructor($2, $3, MEMMGR));
  }
  ;

CompPINCName:
  _CONSTR_QNAME_
  {
    // Check for a colon
    for(XMLCh *tmp = $1; *tmp; ++tmp)
      if(*tmp == ':') yyerror(@1, "Expecting an NCName, found a QName");

    $$ = WRAP(@1, new (MEMMGR) XQLiteral(SchemaSymbols::fgURI_SCHEMAFORSCHEMA,
                               SchemaSymbols::fgDT_STRING,
                               $1, AnyAtomicType::STRING, MEMMGR));
  }
  | _LBRACE_ Expr _RBRACE_
  {
    $$ = $2;
  }
  ;

CompPIConstructorContent:
  _LBRACE_ _RBRACE_
  {
    $$ = WRAP(@1, new (MEMMGR) XQSequence(MEMMGR));
  }
  | _LBRACE_ Expr _RBRACE_
  {
    $$ = $2;
  }
  ;

// [120]    SingleType    ::=    AtomicType "?"? 
SingleType:
  AtomicType SingleTypeOccurrence
  {
    SequenceType* seq = WRAP(@1, new (MEMMGR) SequenceType());
    seq->setItemType(new (MEMMGR) SequenceType::ItemType(SequenceType::ItemType::TEST_ATOMIC_TYPE, NULL, $1));
    seq->setOccurrence($2);
    $$ = seq;
  }
  ;

SingleTypeOccurrence:
  /* empty */
  {
    $$ = SequenceType::EXACTLY_ONE;
  }
  | _QUESTION_MARK_
  {
    $$ = SequenceType::QUESTION_MARK;
  }
;

// [121]    TypeDeclaration    ::=    "as" SequenceType 
TypeDeclaration:
  /* empty */
  {
    $$ = WRAP(@$, new (MEMMGR) SequenceType(new (MEMMGR) SequenceType::ItemType(SequenceType::ItemType::TEST_ANYTHING), SequenceType::STAR));
  }
  | _AS_ SequenceType
  {
    REJECT_NOT_XQUERY(TypeDeclaration, @1);

    $$ = $2;
  }
  ;

// [122]    SequenceType    ::=    (ItemType OccurrenceIndicator?) |  <" empty-sequence" "(" ")">
SequenceType:
  ItemType OccurrenceIndicator
  {
    SequenceType* seq = WRAP(@1, new (MEMMGR) SequenceType());
    seq->setItemType($1);
    seq->setOccurrence($2);
    $$ = seq;
  }
  | _EMPTY_SEQUENCE_ _LPAR_ _RPAR_
  { 
    $$ = WRAP(@1, new (MEMMGR) SequenceType()); 
  }
;


// [123]    OccurrenceIndicator    ::=    "*" |  "+" |  "?"
OccurrenceIndicator:
  /* empty */
  { $$ = SequenceType::EXACTLY_ONE; }
  | _ASTERISK_
  { $$ = SequenceType::STAR; }
  | _PLUS_
  { $$ = SequenceType::PLUS; }
  | _QUESTION_MARK_
  { $$ = SequenceType::QUESTION_MARK; }
  ;

// ItemType ::= KindTest | "item" "(" ")" | AtomicType | FunctionTest | ParenthesizedItemType
ItemType:
    AtomicType 
  {
    $$ = new (MEMMGR) SequenceType::ItemType(SequenceType::ItemType::TEST_ATOMIC_TYPE, NULL, $1);
  }
  | _ITEM_ _LPAR_ _RPAR_
  {
    $$ = new (MEMMGR) SequenceType::ItemType(SequenceType::ItemType::TEST_ANYTHING);
  }
  | KindTest
  | FunctionTest
  | ParenthesizedItemType
  ;

// [125]    AtomicType    ::=    QName 
AtomicType: QName;

// [126]    KindTest    ::=    DocumentTest
//              |  ElementTest
//              |  AttributeTest
//              |  SchemaElementTest
//              |  SchemaAttributeTest
//              |  PITest 
//              |  CommentTest 
//              |  TextTest 
//              |  AnyKindTest 
KindTest:
     DocumentTest
  |  ElementTest
  |  AttributeTest
  |  SchemaElementTest
  |  SchemaAttributeTest
  |  PITest
  |  CommentTest
  |  TextTest
  |  NamespaceNodeTest
  |  AnyKindTest
  ;

// [127]    AnyKindTest    ::=    <"node" "("> ")" 
AnyKindTest:
  _NODE_ _LPAR_ _RPAR_
  {
    $$ = new (MEMMGR) SequenceType::ItemType(SequenceType::ItemType::TEST_NODE);
  }
  ;

// [128]    DocumentTest    ::=    <"document-node" "("> (ElementTest | SchemaElementTest)? ")"
DocumentTest:
  _DOCUMENT_NODE_ _LPAR_ _RPAR_ 
  {
    $$ = new (MEMMGR) SequenceType::ItemType(SequenceType::ItemType::TEST_DOCUMENT);
  }
  | _DOCUMENT_NODE_ _LPAR_ ElementTest _RPAR_ 
  {
    $$ = $3;
    $$->setItemTestType(SequenceType::ItemType::TEST_DOCUMENT);
  }
  | _DOCUMENT_NODE_ _LPAR_ SchemaElementTest _RPAR_ 
  {
    $$ = $3;
    $$->setItemTestType(SequenceType::ItemType::TEST_SCHEMA_DOCUMENT);
  }
  ;
  
// [129]    TextTest    ::=    <"text" "("> ")" 
TextTest:
  _TEXT_ _LPAR_ _RPAR_
  {
    $$ = new (MEMMGR) SequenceType::ItemType(SequenceType::ItemType::TEST_TEXT);
  }
  ;

// [130]    CommentTest    ::=    <"comment" "("> ")" 
CommentTest: 
  _COMMENT_ _LPAR_ _RPAR_
  {
    $$ = new (MEMMGR) SequenceType::ItemType(SequenceType::ItemType::TEST_COMMENT);
  }
  ;

// [178] NamespaceNodeTest ::= "namespace-node" "(" ")"
NamespaceNodeTest:
  _NAMESPACE_NODE_ _LPAR_ _RPAR_
  {
    REJECT_NOT_VERSION3(NamespaceNodeTest, @1);
    $$ = new (MEMMGR) SequenceType::ItemType(SequenceType::ItemType::TEST_NAMESPACE);
  }
  ;

// [131]    PITest    ::=    <"processing-instruction" "("> (NCName | StringLiteral)? ")" 
PITest:
  _PROCESSING_INSTRUCTION_ _LPAR_ _RPAR_
  {
    $$ = new (MEMMGR) SequenceType::ItemType(SequenceType::ItemType::TEST_PI);
  }
  | _PROCESSING_INSTRUCTION_ _LPAR_ NCName _RPAR_
  {
    $$ = new (MEMMGR) SequenceType::ItemType(SequenceType::ItemType::TEST_PI, new (MEMMGR) QualifiedName($3, MEMMGR));
  }
  | _PROCESSING_INSTRUCTION_ _LPAR_ _STRING_LITERAL_ _RPAR_
  {
    $$ = new (MEMMGR) SequenceType::ItemType(SequenceType::ItemType::TEST_PI, new (MEMMGR) QualifiedName($3, MEMMGR));
  }
  ;

// [132]    AttributeTest    ::=    <"attribute" "("> (AttribNameOrWildcard ("," TypeName)?)? ")" 
AttributeTest:
  _ATTRIBUTE_ _LPAR_ _RPAR_ 
  {
    $$ = new (MEMMGR) SequenceType::ItemType(SequenceType::ItemType::TEST_ATTRIBUTE);
  }
  | _ATTRIBUTE_ _LPAR_ AttribNameOrWildcard _RPAR_ 
  {
    $$ = new (MEMMGR) SequenceType::ItemType(SequenceType::ItemType::TEST_ATTRIBUTE,$3);
  }
  | _ATTRIBUTE_ _LPAR_ AttribNameOrWildcard _COMMA_ TypeName _RPAR_ 
  {
    $$ = new (MEMMGR) SequenceType::ItemType(SequenceType::ItemType::TEST_ATTRIBUTE,$3,$5);
  }
  ;

// [133]    AttribNameOrWildcard    ::=    AttributeName | "*" 
AttribNameOrWildcard:
  AttributeName
  | _ASTERISK_
  {
    $$ = NULL;
  }
  ;

// [134]    SchemaAttributeTest    ::=    <"schema-attribute" "("> AttributeDeclaration ")" 
SchemaAttributeTest:
  _SCHEMA_ATTRIBUTE_ _LPAR_ AttributeDeclaration _RPAR_
  {
    $$ = new (MEMMGR) SequenceType::ItemType(SequenceType::ItemType::TEST_SCHEMA_ATTRIBUTE,$3);
  }
  ;

// [135]    AttributeDeclaration    ::=    AttributeName 
AttributeDeclaration:
  AttributeName 
  ;

// [136]    ElementTest    ::=    <"element" "("> (ElementNameOrWildcard ("," TypeName "?"?)?)? ")" 
ElementTest:
  _ELEMENT_ _LPAR_ _RPAR_ 
  {
    $$ = new (MEMMGR) SequenceType::ItemType(SequenceType::ItemType::TEST_ELEMENT);
  }
  | _ELEMENT_ _LPAR_ ElementNameOrWildcard _RPAR_ 
  {
    $$ = new (MEMMGR) SequenceType::ItemType(SequenceType::ItemType::TEST_ELEMENT,$3);
  }
  | _ELEMENT_ _LPAR_ ElementNameOrWildcard _COMMA_ TypeName _RPAR_ 
  {
    $$ = new (MEMMGR) SequenceType::ItemType(SequenceType::ItemType::TEST_ELEMENT,$3,$5);
  }
  | _ELEMENT_ _LPAR_ ElementNameOrWildcard _COMMA_ TypeName _QUESTION_MARK_ _RPAR_ 
  {
    $$ = new (MEMMGR) SequenceType::ItemType(SequenceType::ItemType::TEST_ELEMENT,$3,$5);
    $$->setAllowNilled(true);
  }
  ;

// [137]    ElementNameOrWildcard    ::=    ElementName | "*" 
ElementNameOrWildcard:
  ElementName
  | _ASTERISK_
  {
    $$ = NULL;
  }
  ;

// [138]    SchemaElementTest    ::=    <"schema-element" "("> ElementDeclaration ")" 
SchemaElementTest:
  _SCHEMA_ELEMENT_ _LPAR_ ElementDeclaration _RPAR_
  {
    $$ = new (MEMMGR) SequenceType::ItemType(SequenceType::ItemType::TEST_SCHEMA_ELEMENT,$3);
  }
  ;

// [139]    ElementDeclaration    ::=    ElementName 
ElementDeclaration: ElementName;

// [140]    AttributeName    ::=    QName
AttributeName: QName;

// [141]    ElementName    ::=    QName
ElementName: QName;

// [142]    TypeName    ::=    QName
TypeName: QName;

// [143]     URILiteral     ::=     StringLiteral
URILiteral:
  _STRING_LITERAL_
  {
    // the string must be whitespace-normalized
    XMLString::collapseWS($1, MEMMGR);
    if($1 && *$1 && !XPath2Utils::isValidURI($1, MEMMGR))
      yyerror(@1, "The URI literal is not valid [err:XQST0046]");
    $$ = $1;
  }
  ;

// [144]    	FTSelection 	   ::=    	FTOr FTPosFilter* ("weight" RangeExpr)?
FTSelection:
  FTOr FTPosFilters FTSelectionWeight
  {
    // TBD weight
    $$ = $1;

    for(VectorOfFTOptions::iterator i = $2->begin();
        i != $2->end(); ++i) {
      (*i)->setArgument($$);
      $$ = *i;
    }
/*     delete $2; */
  }
  ;

FTSelectionWeight:
  /* empty */
{
}
| _WEIGHT_ RangeExpr
{
}
;

FTPosFilters:
  /* empty */
{
  $$ = new (MEMMGR) VectorOfFTOptions(XQillaAllocator<FTOption*>(MEMMGR));
}
| FTPosFilters FTPosFilter
{
  if($2 != NULL) $1->push_back($2);
  $$ = $1;
}
;

// [145]    FTOr    ::=    FTAnd ( "ftor" FTAnd )*
FTOr:
  FTOr _FTOR_ FTAnd
/*   FTOr _OR_ FTAnd */
  {
    if($1->getType() == FTSelection::OR) {
      FTOr *op = (FTOr*)$1;
      op->addArg($3);
      $$ = op;
    }
    else {
      $$ = WRAP(@2, new (MEMMGR) FTOr($1, $3, MEMMGR));
    }
  }
  | FTAnd
  ;

// [146]    FTAnd    ::=    FTMildnot ( "ftand" FTMildnot )*
FTAnd:
  FTAnd _FTAND_ FTMildnot
/*   FTAnd _AND_ FTMildnot */
  {
    if($1->getType() == FTSelection::AND) {
      FTAnd *op = (FTAnd*)$1;
      op->addArg($3);
      $$ = op;
    }
    else {
      $$ = WRAP(@2, new (MEMMGR) FTAnd($1, $3, MEMMGR));
    }
  }
  | FTMildnot
  ;

// [147]    FTMildnot    ::=    FTUnaryNot ( "not" "in" FTUnaryNot )*
FTMildnot:
  FTMildnot _NOT_ _IN_ FTUnaryNot
  {
    $$ = WRAP(@2, new (MEMMGR) FTMildnot($1, $4, MEMMGR));
  }
  | FTUnaryNot
  ;

// [148]    	FTUnaryNot 	   ::=    	("ftnot")? FTPrimaryWithOptions
FTUnaryNot:
  _FTNOT_ FTPrimaryWithOptions
/*   _NOT_ FTPrimaryWithOptions */
  {
    $$ = WRAP(@1, new (MEMMGR) FTUnaryNot($2, MEMMGR));
  }
  | FTPrimaryWithOptions
  ;

// [149]    	FTPrimaryWithOptions 	   ::=    	FTPrimary FTMatchOptions?
FTPrimaryWithOptions:
  FTPrimary
| FTPrimary FTMatchOptions
{
  // TBD match options
  $$ = $1;
}
;

// [150]    	FTPrimary 	   ::=    	(FTWords FTTimes?) | ("(" FTSelection ")") | FTExtensionSelection
FTPrimary:
  FTWords FTTimes
{
  // TBD FTTimes
  $$ = $1;
}
| _LPAR_ FTSelection _RPAR_
{
  $$ = $2;
}
| FTExtensionSelection
;

// [151]    	FTWords 	   ::=    	FTWordsValue FTAnyallOption?
// [152]    	FTWordsValue 	   ::=    	Literal | ("{" Expr "}")
FTWords:
  Literal FTAnyallOption
  {
    $$ = WRAP(@1, new (MEMMGR) FTWords($1, $2, MEMMGR));
  }
  | _LBRACE_ Expr _RBRACE_ FTAnyallOption
  {
    $$ = WRAP(@2, new (MEMMGR) FTWords($2, $4, MEMMGR));
  }
  ;

// [153]    	FTExtensionSelection 	   ::=    	Pragma+ "{" FTSelection? "}"
FTExtensionSelection:
  PragmaList _LBRACE_ _RBRACE_
{
  // we don't support any pragma
  yyerror(@1, "This pragma is not recognized, and no alternative expression is specified [err:XQST0079]");
}
| PragmaList _LBRACE_ FTSelection _RBRACE_
{
  // we don't support any pragma
  $$ = $3;
}
;

// [154]    	FTAnyallOption 	   ::=    	("any" "word"?) | ("all" "words"?) | "phrase"
FTAnyallOption:
  /* empty */
  {
    $$ = FTWords::ANY;
  }
  | _ANY_
  {
    $$ = FTWords::ANY;
  }
  | _ANY_ _WORD_
  {
    $$ = FTWords::ANY_WORD;
  }
  | _ALL_
  {
    $$ = FTWords::ALL;
  }
  | _ALL_ _WORDS_
  {
    $$ = FTWords::ALL_WORDS;
  }
  | _PHRASE_
  {
    $$ = FTWords::PHRASE;
  }
  ;


// [155]    	FTTimes 	   ::=    	"occurs" FTRange "times"
FTTimes:
  /* empty */
| _OCCURS_ FTRange _TIMES_
{
  yyerror(@1, "The FTTimes operator is not supported. [err:FTST0005]");
}
;

// [156] FTRange ::= ("exactly" AdditiveExpr)
//                 | ("at" "least" AdditiveExpr)
//                 | ("at" "most" AdditiveExpr)
//                 | ("from" AdditiveExpr "to" AdditiveExpr)
FTRange:
  _EXACTLY_ AdditiveExpr
  {
    $$.type = FTRange::EXACTLY;
    $$.arg1 = $2;
    $$.arg2 = 0;
  }
  | _AT_LM_ _LEAST_ AdditiveExpr
  {
    $$.type = FTRange::AT_LEAST;
    $$.arg1 = $3;
    $$.arg2 = 0;
  }
  | _AT_LM_ _MOST_ AdditiveExpr
  {
    $$.type = FTRange::AT_MOST;
    $$.arg1 = $3;
    $$.arg2 = 0;
  }
  | _FROM_ AdditiveExpr _TO_ AdditiveExpr
  {
    $$.type = FTRange::FROM_TO;
    $$.arg1 = $2;
    $$.arg2 = $4;
  }
  ;

// [157]    FTPosFilter ::=    FTOrder | FTWindow | FTDistance | FTScope | FTContent
// [158]    FTOrder     ::=    "ordered"
// [159]    FTWindow    ::=    "window" AdditiveExpr FTUnit
// [160]    FTDistance  ::=    "distance" FTRange FTUnit
// [162]    FTScope     ::=    ("same" | "different") FTBigUnit
// [164]    FTContent   ::=    ("at" "start") | ("at" "end") | ("entire" "content")
FTPosFilter:
  _ORDERED_
  {
    $$ = WRAP(@1, new (MEMMGR) FTOrder(MEMMGR));
  }
  | _WINDOW_ AdditiveExpr FTUnit
  {
    $$ = WRAP(@1, new (MEMMGR) FTWindow($2, $3, MEMMGR));
  }
  | _DISTANCE_ FTRange FTUnit
  {
    $$ = WRAP(@1, new (MEMMGR) FTDistance($2, $3, MEMMGR));
  }
  | _SAME_ FTBigUnit
  {
    $$ = WRAP(@1, new (MEMMGR) FTScope(FTScope::SAME, $2, MEMMGR));
  }
  | _DIFFERENT_ FTBigUnit
  {
    $$ = WRAP(@1, new (MEMMGR) FTScope(FTScope::DIFFERENT, $2, MEMMGR));
  }
  | _AT_ _START_
  {
    $$ = WRAP(@1, new (MEMMGR) FTContent(FTContent::AT_START, MEMMGR));
  }
  | _AT_ _END_
  {
    $$ = WRAP(@1, new (MEMMGR) FTContent(FTContent::AT_END, MEMMGR));
  }
  | _ENTIRE_ _CONTENT_
  {
    $$ = WRAP(@1, new (MEMMGR) FTContent(FTContent::ENTIRE_CONTENT, MEMMGR));
  }
  ;

// [161]    	FTUnit 	   ::=    	"words" | "sentences" | "paragraphs"
FTUnit:
  _WORDS_
  {
    $$ = FTOption::WORDS;
  }
  | _SENTENCES_
  {
    $$ = FTOption::SENTENCES;
  }
  | _PARAGRAPHS_
  {
    $$ = FTOption::PARAGRAPHS;
  }
  ;

// [163]    	FTBigUnit 	   ::=    	"sentence" | "paragraph"
FTBigUnit:
  _SENTENCE_
  {
    $$ = FTOption::SENTENCES;
  }
  | _PARAGRAPH_
  {
    $$ = FTOption::PARAGRAPHS;
  }
  ;

// [165]    FTMatchOptions    ::=    ("using" FTMatchOption)+    /* xgc: multiple-match-options */
FTMatchOptions:
  _USING_ FTMatchOption
| FTMatchOptions _USING_ FTMatchOption
;

// [166]    FTMatchOption ::= FTLanguageOption
//                          | FTWildCardOption
//                          | FTThesaurusOption
//                          | FTStemOption
//                          | FTCaseOption
//                          | FTDiacriticsOption
//                          | FTStopWordOption
//                          | FTExtensionOption
FTMatchOption:
  FTLanguageOption
| FTWildCardOption
| FTThesaurusOption
| FTStemOption
| FTCaseOption
| FTDiacriticsOption
| FTStopWordOption
| FTExtensionOption
;

// [167]    FTCaseOption ::= ("case" "insensitive")
//                         | ("case" "sensitive")
//                         | "lowercase"
//                         | "uppercase"
FTCaseOption:
    _CASE_S_ _INSENSITIVE_
  {
    CONTEXT->setFTCase(StaticContext::CASE_INSENSITIVE);
  }
  | _CASE_S_ _SENSITIVE_
  {
    CONTEXT->setFTCase(StaticContext::CASE_SENSITIVE);
  }
  | _LOWERCASE_
  {
    CONTEXT->setFTCase(StaticContext::LOWERCASE);
  }
  | _UPPERCASE_
  {
    CONTEXT->setFTCase(StaticContext::UPPERCASE);
  }
  ;

// [168]    FTDiacriticsOption ::= ("diacritics" "insensitive")
//                               | ("diacritics" "sensitive")
FTDiacriticsOption:
    _DIACRITICS_ _INSENSITIVE_
  {
    std::cerr << "diacritics insensitive" << std::endl;
  }
  | _DIACRITICS_ _SENSITIVE_
  {
    std::cerr << "diacritics sensitive" << std::endl;
  }
  ;

// [169]    FTStemOption    ::=    "stemming" | ("no" "stemming")
FTStemOption:
    _STEMMING_
  {
    std::cerr << "using stemming" << std::endl;
  }
  | _NO_ _STEMMING_
  {
    std::cerr << "no stemming" << std::endl;
  }
  ;

// [170]    FTThesaurusOption ::= ("thesaurus" (FTThesaurusID | "default"))
//                              | ("thesaurus" "(" (FTThesaurusID | "default") ("," FTThesaurusID)* ")")
//                              | ("no" "thesaurus")
FTThesaurusOption:
    _THESAURUS_ FTThesaurusID
  {
    std::cerr << "using thesaurus" << std::endl;
  }
  | _THESAURUS_ _DEFAULT_
  {
    std::cerr << "using thesaurus default" << std::endl;
  }
  | _THESAURUS_ _LPAR_ FTThesaurusID FTThesaurusIDList _RPAR_
  {
    std::cerr << "using thesaurus ()" << std::endl;
  }
  | _THESAURUS_ _LPAR_ _DEFAULT_ FTThesaurusIDList _RPAR_
  {
    std::cerr << "using thesaurus (default)" << std::endl;
  }
  | _NO_ _THESAURUS_
  {
    std::cerr << "no thesaurus" << std::endl;
  }
  ;

FTThesaurusIDList:
  /* empty */
  {
  }
  | FTThesaurusIDList _COMMA_ FTThesaurusID
  {
  }
  ;

// [171]    FTThesaurusID    ::=    "at" URILiteral ("relationship" StringLiteral)? (FTRange "levels")?
FTThesaurusID:
  _AT_ URILiteral
  {
    std::cerr << "at StringLiteral" << std::endl;
  }
  | _AT_ URILiteral _RELATIONSHIP_ StringLiteral
  {
    std::cerr << "at StringLiteral relationship StringLiteral" << std::endl;
  }
  | _AT_ URILiteral FTRange _LEVELS_
  {
    std::cerr << "at StringLiteral levels" << std::endl;
  }
  | _AT_ URILiteral _RELATIONSHIP_ StringLiteral FTRange _LEVELS_
  {
    std::cerr << "at StringLiteral relationship StringLiteral levels" << std::endl;
  }
  ;

// [172]    FTStopWordOption ::= ("stop" "words" FTStopWords FTStopWordsInclExcl*)
//                             | ("no" "stop" "words")
//                             | ("stop" "words" "default" FTStopWordsInclExcl*)
FTStopWordOption:
    _STOP_ _WORDS_ FTStopWords FTStopWordsInclExclList
  {
    yyerror(@1, "FTStopWordOption is not supported. [err:FTST0006]");
  }
  | _NO_ _STOP_ _WORDS_
  {
    yyerror(@1, "FTStopWordOption is not supported. [err:FTST0006]");
  }
  | _STOP_ _WORDS_ _DEFAULT_ FTStopWordsInclExclList
  {
    yyerror(@1, "FTStopWordOption is not supported. [err:FTST0006]");
  }
  ;

FTStopWordsInclExclList:
  /* empty */
  {
  }
  | FTStopWordsInclExclList FTStopWordsInclExcl
  {
  }
  ;

// [173]    FTStopWords ::= ("at" URILiteral)
//                        | ("(" StringLiteral ("," StringLiteral)* ")")
FTStopWords:
  _AT_ URILiteral
  {
    std::cerr << "at URILiteral" << std::endl;
  }
  | _LPAR_ FTStopWordsStringList _RPAR_
  {
    std::cerr << "()" << std::endl;
  }
  ;

FTStopWordsStringList:
  StringLiteral
  {
    std::cerr << "StringLiteral" << std::endl;
  }
  | FTStopWordsStringList _COMMA_ StringLiteral
  {
    std::cerr << ", StringLiteral" << std::endl;
  }
  ;

// [174]    FTStopWordsInclExcl    ::=    ("union" | "except") FTStopWords
FTStopWordsInclExcl:
  _UNION_ FTStopWords
  {
    std::cerr << "union" << std::endl;
  }
  | _EXCEPT_ FTStopWords
  {
    std::cerr << "except" << std::endl;
  }
  ;

// [175]    FTLanguageOption    ::=    "language" StringLiteral
FTLanguageOption:
  _LANGUAGE_ StringLiteral
  {
    std::cerr << "language StringLiteral" << std::endl;
  }
  ;

// [176]    FTWildCardOption    ::=    "wildcards" | ("no" "wildcards")
FTWildCardOption:
   _WILDCARDS_
  {
    std::cerr << "using wildcards" << std::endl;
  }
  | _NO_ _WILDCARDS_
  {
    std::cerr << "no wildcards" << std::endl;
  }
  ;

// [177]    FTExtensionOption    ::=    "option" QName StringLiteral
FTExtensionOption:
  _OPTION_ QNameValue _STRING_LITERAL_
{
}
;

// [178]    FTIgnoreOption    ::=    "without" "content" UnionExpr
FTIgnoreOption:
  _WITHOUT_C_ _CONTENT_ UnionExpr
  {
    $$ = $3;
  }
  ;

// [141]      RevalidationDecl      ::=      "declare" "revalidation" ("strict" | "lax" | "skip")
RevalidationDecl:
  _DECLARE_ _REVALIDATION_ _STRICT_
  {
    CHECK_SPECIFIED(@1, BIT_REVALIDATION_SPECIFIED, "revalidation", "XUST0003");
    CONTEXT->setRevalidationMode(DocumentCache::VALIDATION_STRICT);
  }
  | _DECLARE_ _REVALIDATION_ _LAX_
  {
    CHECK_SPECIFIED(@1, BIT_REVALIDATION_SPECIFIED, "revalidation", "XUST0003");
    CONTEXT->setRevalidationMode(DocumentCache::VALIDATION_LAX);
  }
  | _DECLARE_ _REVALIDATION_ _SKIP_
  {
    CHECK_SPECIFIED(@1, BIT_REVALIDATION_SPECIFIED, "revalidation", "XUST0003");
    CONTEXT->setRevalidationMode(DocumentCache::VALIDATION_SKIP);
  }
  ;

// [142]      InsertExprTargetChoice ::=     (("as" ("first" | "last"))? "into") | "after" | "before"
// [143]      InsertExpr             ::=     "insert" ("node" | "nodes") SourceExpr InsertExprTargetChoice TargetExpr
// [147]      SourceExpr      ::=      ExprSingle
// [148]      TargetExpr      ::=      ExprSingle
InsertExpr:
  InsertExprBegin ExprSingle _AS_ _FIRST_ _INTO_ ExprSingle
  {
    $$ = WRAP(@1, new (MEMMGR) UInsertAsFirst($2, $6, MEMMGR));
  }
  | InsertExprBegin ExprSingle _AS_ _LAST_ _INTO_ ExprSingle
  {
    $$ = WRAP(@1, new (MEMMGR) UInsertAsLast($2, $6, MEMMGR));
  }
  | InsertExprBegin ExprSingle _INTO_ ExprSingle
  {
    $$ = WRAP(@1, new (MEMMGR) UInsertInto($2, $4, MEMMGR));
  }
  | InsertExprBegin ExprSingle _AFTER_ ExprSingle
  {
    $$ = WRAP(@1, new (MEMMGR) UInsertAfter($2, $4, MEMMGR));
  }
  | InsertExprBegin ExprSingle _BEFORE_ ExprSingle
  {
    $$ = WRAP(@1, new (MEMMGR) UInsertBefore($2, $4, MEMMGR));
  }
  ;

InsertExprBegin: _INSERT_ _NODE_ | _INSERT_ _NODES_ ;

// [144]      DeleteExpr      ::=      "delete" ("node" | "nodes") TargetExpr
// [148]      TargetExpr      ::=      ExprSingle
DeleteExpr:
  DeleteExprBegin ExprSingle
  {
    $$ = WRAP(@1, new (MEMMGR) UDelete($2, MEMMGR));
  }
  ;

DeleteExprBegin: _DELETE_ _NODE_ | _DELETE_ _NODES_ ;

// [145]     ReplaceExpr     ::=     "replace" ("value" "of")? "node" TargetExpr "with" ExprSingle
// [148]      TargetExpr      ::=      ExprSingle
ReplaceExpr:
  _REPLACE_ _VALUE_ _OF_ _NODE_ ExprSingle _WITH_ ExprSingle
  {
    $$ = WRAP(@1, new (MEMMGR) UReplaceValueOf($5, $7, MEMMGR));
  }
  | _REPLACE_ _NODE_ ExprSingle _WITH_ ExprSingle
  {
    $$ = WRAP(@1, new (MEMMGR) UReplace($3, $5, MEMMGR));
  }
  ;

// [146]     RenameExpr     ::=     "rename" "node" TargetExpr "as" NewNameExpr
// [148]      TargetExpr      ::=      ExprSingle
// [149]      NewNameExpr      ::=      ExprSingle
RenameExpr:
  _RENAME_ _NODE_ ExprSingle _AS_ ExprSingle
  {
    $$ = WRAP(@1, new (MEMMGR) URename($3, $5, MEMMGR));
  }
  ;

// [150] TransformExpr ::= "copy" "$" VarName ":=" ExprSingle ("," "$" VarName ":=" ExprSingle)* "modify" ExprSingle "return" ExprSingle
TransformExpr:
  _COPY_ TransformBindingList _MODIFY_ ExprSingle _RETURN_ ExprSingle
  {
    $$ = WRAP(@1, new (MEMMGR) UTransform($2, $4, $6, MEMMGR));
  }
  ;

TransformBindingList:
  TransformBindingList _COMMA_ TransformBinding
  {
    $1->push_back($3);
    $$ = $1;
  }
  | TransformBinding
  {
    $$ = new (MEMMGR) VectorOfCopyBinding(XQillaAllocator<CopyBinding*>(MEMMGR));
    $$->push_back($1);
  }
  ;

TransformBinding:
  _DOLLAR_ VarName _COLON_EQUALS_ ExprSingle
  {
    $$ = WRAP(@1, new (MEMMGR) CopyBinding(MEMMGR, $2, $4));
  }
  ;


// [174]     IntegerLiteral     ::=     Digits
IntegerLiteral:
  _INTEGER_LITERAL_
  {
    $$ = WRAP(@1, new (MEMMGR) XQLiteral(
                  SchemaSymbols::fgURI_SCHEMAFORSCHEMA,
                  SchemaSymbols::fgDT_INTEGER,
                  $1, AnyAtomicType::DECIMAL,
                  MEMMGR));
  }
  ;

// [175]     DecimalLiteral     ::=     ("." Digits) | (Digits "." [0-9]*)
DecimalLiteral:
  _DECIMAL_LITERAL_
  {
    $$ = WRAP(@1, new (MEMMGR) XQLiteral(
                  SchemaSymbols::fgURI_SCHEMAFORSCHEMA,
                  SchemaSymbols::fgDT_DECIMAL,
                  $1, AnyAtomicType::DECIMAL,
                  MEMMGR));
  }
  ;

// [176]     DoubleLiteral     ::=     (("." Digits) | (Digits ("." [0-9]*)?)) [eE] [+-]? Digits
DoubleLiteral:
  _DOUBLE_LITERAL_
  {
    $$ = WRAP(@1, new (MEMMGR) XQLiteral(
                  SchemaSymbols::fgURI_SCHEMAFORSCHEMA,
                  SchemaSymbols::fgDT_DOUBLE,
                  $1, AnyAtomicType::DOUBLE,
                  MEMMGR));
  }
  ;

// [177]     StringLiteral     ::=     ('"' (PredefinedEntityRef | CharRef | ('"' '"') | [^"&])* '"') | 
//                    ("'" (PredefinedEntityRef | CharRef | ("'" "'") | [^'&])* "'")
StringLiteral:
  _STRING_LITERAL_
  {
    $$ = WRAP(@1, new (MEMMGR) XQLiteral(
                  SchemaSymbols::fgURI_SCHEMAFORSCHEMA,
                  SchemaSymbols::fgDT_STRING,
                  $1, AnyAtomicType::STRING,
                  MEMMGR));
  }
  ;

NCName:
  QNameValue
  {
    // Check for a colon
    for(XMLCh *tmp = $1; *tmp; ++tmp)
      if(*tmp == ':') yyerror(@1, "Expecting an NCName, found a QName");
  }
  ;

QName:
  QNameValue
  {
    $$ = new (MEMMGR) QualifiedName($1,MEMMGR);
  }
  ;

////////////////////////////////////////////////////////////////////////////////////////////////////
// Template Extension Rules

// TemplateDecl ::= "declare" "template" ((("name" QName)? ("mode" TemplateDeclModes)? "matches" Pattern) | ("name" QName))
//                  ("(" ParamList? ")" ("as" SequenceType)?)? EnclosedExpr
TemplateDecl:
    _DECLARE_ _TEMPLATE_ _NAME_ QNameValue EnclosedExpr
  {
    FunctionSignature *signature = new (MEMMGR) FunctionSignature(MEMMGR);
    $$ = WRAP(@1, new (MEMMGR) XQUserFunction($4, 0, signature, $5, MEMMGR));
  }
  | _DECLARE_ _TEMPLATE_ _NAME_ QNameValue TemplateParamList TemplateSequenceType EnclosedExpr
  {
    FunctionSignature *signature = new (MEMMGR) FunctionSignature($5, $6, MEMMGR);
    $$ = WRAP(@1, new (MEMMGR) XQUserFunction($4, 0, signature, $7, MEMMGR));
  }
  | _DECLARE_ _TEMPLATE_ _NAME_ QNameValue TemplateDeclModesSection _MATCHES_ Pattern_XSLT EnclosedExpr
  {
    FunctionSignature *signature = new (MEMMGR) FunctionSignature(MEMMGR);
    $$ = WRAP(@1, new (MEMMGR) XQUserFunction($4, $7, signature, $8, MEMMGR));
    $$->setModeList($5);
  }
  | _DECLARE_ _TEMPLATE_ _NAME_ QNameValue TemplateDeclModesSection _MATCHES_ Pattern_XSLT TemplateParamList TemplateSequenceType EnclosedExpr
  {
    FunctionSignature *signature = new (MEMMGR) FunctionSignature($8, $9, MEMMGR);
    $$ = WRAP(@1, new (MEMMGR) XQUserFunction($4, $7, signature, $10, MEMMGR));
    $$->setModeList($5);
  }
  | _DECLARE_ _TEMPLATE_ TemplateDeclModesSection _MATCHES_ Pattern_XSLT EnclosedExpr
  {
    FunctionSignature *signature = new (MEMMGR) FunctionSignature(MEMMGR);
    $$ = WRAP(@1, new (MEMMGR) XQUserFunction(0, $5, signature, $6, MEMMGR));
    $$->setModeList($3);
  }
  | _DECLARE_ _TEMPLATE_ TemplateDeclModesSection _MATCHES_ Pattern_XSLT TemplateParamList TemplateSequenceType EnclosedExpr
  {
    FunctionSignature *signature = new (MEMMGR) FunctionSignature($6, $7, MEMMGR);
    $$ = WRAP(@1, new (MEMMGR) XQUserFunction(0, $5, signature, $8, MEMMGR));
    $$->setModeList($3);
  }
  ;

TemplateSequenceType:
    /* empty */
  {
    $$ = 0;
  }
  | _AS_ SequenceType
  {
    $$ = $2;
  }
  ;

// TBD required and tunnel - jpcs
TemplateParamList:
    _LPAR_ _RPAR_
  {
    $$ = 0;
  }
  | _LPAR_ ParamList _RPAR_
  {
    $$ = $2;
  }
  ;

TemplateDeclModesSection:
    /* empty */
  {
    $$ = new (MEMMGR) XQUserFunction::ModeList(XQillaAllocator<XQUserFunction::Mode*>(MEMMGR));
    $$->push_back(WRAP(@$, new (MEMMGR) XQUserFunction::Mode(XQUserFunction::Mode::DEFAULT)));
  }
  | _MODE_ TemplateDeclModes
  {
    $$ = $2;
  }
  ;

// TemplateDeclModes ::= TemplateDeclMode ("," TemplateDeclMode)*
TemplateDeclModes:
    TemplateDeclMode
  {
    $$ = new (MEMMGR) XQUserFunction::ModeList(XQillaAllocator<XQUserFunction::Mode*>(MEMMGR));
    $$->push_back($1);
  }
  | TemplateDeclModes _COMMA_ TemplateDeclMode
  {
    $1->push_back($3);
    $$ = $1;
  }
  ;

// TemplateDeclMode ::= QName | "#default" | "#all"
TemplateDeclMode:
    _QNAME_
  {
    $$ = WRAP(@1, new (MEMMGR) XQUserFunction::Mode($1));
  }
  | _HASH_DEFAULT_
  {
    $$ = WRAP(@1, new (MEMMGR) XQUserFunction::Mode(XQUserFunction::Mode::DEFAULT));
  }
  | _HASH_ALL_
  {
    $$ = WRAP(@1, new (MEMMGR) XQUserFunction::Mode(XQUserFunction::Mode::ALL));
  }
  ;

// CallTemplateExpr ::= "call" "template" QName ("with" "(" TemplateArgumentList ")")?
CallTemplateExpr:
    _CALL_ _TEMPLATE_ QNameValue
  {
    $$ = WRAP(@1, new (MEMMGR) XQCallTemplate($3, 0, MEMMGR));
  }
  | _CALL_ _TEMPLATE_ QNameValue _WITH_ _LPAR_ TemplateArgumentList _RPAR_
  {
    $$ = WRAP(@1, new (MEMMGR) XQCallTemplate($3, $6, MEMMGR));
  }
  ;

// ApplyTemplatesExpr ::= "apply" "templates" ("mode" ApplyTemplatesMode)? ExprSingle ("with" "(" TemplateArgumentList ")")?
ApplyTemplatesExpr:
    _APPLY_ _TEMPLATES_ ExprSingle
  {
    $$ = WRAP(@1, new (MEMMGR) XQApplyTemplates($3, 0, 0, MEMMGR));
  }
  |_APPLY_ _TEMPLATES_ ExprSingle _WITH_ _LPAR_ TemplateArgumentList _RPAR_
  {
    $$ = WRAP(@1, new (MEMMGR) XQApplyTemplates($3, $6, 0, MEMMGR));
  }
  | _APPLY_ _TEMPLATES_ ExprSingle _MODE_ ApplyTemplatesMode
  {
    $$ = WRAP(@1, new (MEMMGR) XQApplyTemplates($3, 0, $5, MEMMGR));
  }
  |_APPLY_ _TEMPLATES_ ExprSingle _MODE_ ApplyTemplatesMode _WITH_ _LPAR_ TemplateArgumentList _RPAR_
  {
    $$ = WRAP(@1, new (MEMMGR) XQApplyTemplates($3, $8, $5, MEMMGR));
  }
  ;

// ApplyTemplatesMode ::= QName | "#default" | "#current"
ApplyTemplatesMode:
    QNameValue
  {
    $$ = WRAP(@1, new (MEMMGR) XQUserFunction::Mode($1));
  }
  | _HASH_DEFAULT_
  {
    $$ = WRAP(@1, new (MEMMGR) XQUserFunction::Mode(XQUserFunction::Mode::DEFAULT));
  }
  | _HASH_CURRENT_
  {
    $$ = WRAP(@1, new (MEMMGR) XQUserFunction::Mode(XQUserFunction::Mode::CURRENT));
  }
  ;

// TemplateArgumentList ::= TemplateArgument ("," TemplateArgument)*
TemplateArgumentList:
    TemplateArgument
  {
    $$ = new (MEMMGR) TemplateArguments(XQillaAllocator<XQTemplateArgument*>(MEMMGR));
    $$->push_back($1);
  }
  | TemplateArgumentList _COMMA_ TemplateArgument
  {
    $$ = $1;
    $$->push_back($3);
  }
  ;

// TemplateArgument ::= "tunnel"? "$" VarName TypeDeclaration? ":=" ExprSingle
// TBD tunnel - jpcs
TemplateArgument:
    _DOLLAR_ VarName TypeDeclaration _COLON_EQUALS_ ExprSingle
  {
    if($3 != 0)
      $5 = $3->convertFunctionArg($5, CONTEXT, /*numericfunction*/false, $3);
    $$ = WRAP(@1, new (MEMMGR) XQTemplateArgument($2, $5, MEMMGR));
  }
  ;


////////////////////////////////////////////////////////////////////////////////////////////////////
// XQuery 1.1 Functionality

// [161] FunctionItemExpr ::= LiteralFunctionItem | InlineFunction
FunctionItemExpr: LiteralFunctionItem | InlineFunction;

// [162] LiteralFunctionItem ::= QName "#" IntegerLiteral
LiteralFunctionItem:
    FunctionName _HASH_ _INTEGER_LITERAL_
  {
    $$ = WRAP(@1, new (MEMMGR) XQFunctionRef($1, atoi(UTF8($3)), MEMMGR));
  }
  ;

// [163] InlineFunction ::= "function" "(" ParamList? ")" ("as" SequenceType)? EnclosedExpr
InlineFunction:
    _FUNCTION_EXT_ FunctionParamList TypeDeclaration EnclosedExpr
  {
    FunctionSignature *signature = new (MEMMGR) FunctionSignature($2, $3, MEMMGR);
    XQUserFunction *func = WRAP(@1, new (MEMMGR) XQUserFunction(0, signature, $4, false, MEMMGR));
    $$ = WRAP(@1, new (MEMMGR) XQInlineFunction(func, MEMMGR));
  }
  ;

// [121]   	PostfixExpr	   ::=   	PrimaryExpr (Predicate | ArgumentList)*
// [122]   	ArgumentList	   ::=   	"(" (Argument ("," Argument)*)? ")"
DynamicFunctionInvocation:
    PostfixExpr _LPAR_ _RPAR_
  {
    REJECT_NOT_VERSION3(DynamicFunctionInvocation, @1);
    $$ = WRAP(@2, new (MEMMGR) XQFunctionDeref($1, new (MEMMGR) VectorOfASTNodes(XQillaAllocator<ASTNode*>(MEMMGR)), MEMMGR));
  }
  | PostfixExpr _LPAR_ FunctionCallArgumentList _RPAR_
  {
    REJECT_NOT_VERSION3(DynamicFunctionInvocation, @1);

    bool partial = false;
    VectorOfASTNodes::iterator i;
    for(i = $3->begin(); i != $3->end(); ++i) {
      if(*i == 0) {
        partial = true;
        break;
      }
    }

    if(partial) {
      // This is a partial function application
      $$ = WRAP(@2, new (MEMMGR) XQPartialApply($1, $3, MEMMGR));
    }
    else {
      $$ = WRAP(@2, new (MEMMGR) XQFunctionDeref($1, $3, MEMMGR));
    }
  }
  ;

// [189] FunctionTest ::= AnyFunctionTest | TypedFunctionTest
FunctionTest: AnyFunctionTest | TypedFunctionTest;

// [190] AnyFunctionTest ::= "function" "(" "*" ")"
AnyFunctionTest:
    _FUNCTION_EXT_ _LPAR_ _ASTERISK_ _RPAR_
  {
    $$ = new (MEMMGR) SequenceType::ItemType(SequenceType::ItemType::TEST_FUNCTION);
  }
  ;

// [191] TypedFunctionTest ::= "function" "(" (SequenceType ("," SequenceType)*)? ")" "as" SequenceType
TypedFunctionTest:
    _FUNCTION_EXT_ _LPAR_ _RPAR_ _AS_ SequenceType
  {
    $$ = new (MEMMGR) SequenceType::ItemType(new (MEMMGR) VectorOfSequenceTypes(XQillaAllocator<SequenceType*>(MEMMGR)), $5);
  }
  | _FUNCTION_EXT_ _LPAR_ FunctionTypeArguments _RPAR_ _AS_ SequenceType
  {
    $$ = new (MEMMGR) SequenceType::ItemType($3, $6);
  }
  ;

FunctionTypeArguments:
    SequenceType
  {
    $$ = new (MEMMGR) VectorOfSequenceTypes(XQillaAllocator<SequenceType*>(MEMMGR));
    $$->push_back($1);
  }
  | FunctionTypeArguments _COMMA_ SequenceType
  {
    $$ = $1;
    $$->push_back($3);
  }
  ;

// [192] ParenthesizedItemType ::= "(" ItemType ")"
ParenthesizedItemType:
    _LPAR_ ItemType _RPAR_
  {
    $$ = $2;
  }
  ;

////////////////////////////////////////////////////////////////////////////////////////////////////

QNameValue:
FunctionName | _ATTRIBUTE_ | _COMMENT_ | _DOCUMENT_NODE_ | _ELEMENT_ | _ITEM_ | _IF_ | _NODE_ |
_PROCESSING_INSTRUCTION_ | _SCHEMA_ATTRIBUTE_ | _SCHEMA_ELEMENT_ | _TEXT_ | _TYPESWITCH_ | _EMPTY_SEQUENCE_ |
_FUNCTION_EXT_ | _NAMESPACE_NODE_
  ;

FunctionName:
_QNAME_ | _BOUNDARY_SPACE_ | _FT_OPTION_ | _BASE_URI_ | _LAX_ | _STRICT_ | _IDIV_ | _CHILD_ | _DESCENDANT_ | _SELF_ |
_DESCENDANT_OR_SELF_ | _FOLLOWING_SIBLING_ | _FOLLOWING_ | _PARENT_ | _ANCESTOR_ | _PRECEDING_SIBLING_ | _PRECEDING_ |
_ANCESTOR_OR_SELF_ | _DOCUMENT_ | _NOT_ | _SENSITIVE_ | _INSENSITIVE_ | _DIACRITICS_ | _WITHOUT_ | _STEMMING_ |
_THESAURUS_ | _STOP_ | _WILDCARDS_ | _ENTIRE_ | _CONTENT_ | _WORD_ | _TYPE_ | _START_ | _END_ | _MOST_ | _SKIP_ |
_COPY_ | _VALUE_ | _EQ_ | _NE_ | _LT_ | _LE_ | _GT_ | _GE_ | _AT_ | _VARIABLE_ | _RETURN_ | _FOR_ |
_IN_ | _LET_ | _WHERE_ | _BY_ | _ORDER_ | _STABLE_ | _ASCENDING_ | _DESCENDING_ | _EMPTY_ | _GREATEST_ |
_LEAST_ | _COLLATION_ | _SOME_ | _EVERY_ | _SATISFIES_ | _CASE_ | _AS_ | _THEN_ | _ELSE_ | _OR_ | _AND_ | _INSTANCE_ |
_OF_ | _CASTABLE_ | _TO_ | _DIV_ | _MOD_ | _UNION_ | _INTERSECT_ | _EXCEPT_ | _VALIDATE_ | _CAST_ | _TREAT_ | _IS_ |
_PRESERVE_ | _STRIP_ | _NAMESPACE_ | _EXTERNAL_ | _ENCODING_ | _NO_PRESERVE_ | _INHERIT_ | _NO_INHERIT_ | _DECLARE_ |
_CONSTRUCTION_ | _ORDERING_ | _DEFAULT_ | _COPY_NAMESPACES_ | _OPTION_ | _VERSION_ | _IMPORT_ | _SCHEMA_ |
_FUNCTION_ | _SCORE_ | _USING_ | _NO_ | _CONTAINS_ | _WEIGHT_ | _WINDOW_ | _DISTANCE_ | _OCCURS_ | _TIMES_ | _SAME_ |
_DIFFERENT_ | _LOWERCASE_ | _UPPERCASE_ | _RELATIONSHIP_ | _LEVELS_ | _LANGUAGE_ | _ANY_ | _ALL_ | _PHRASE_ |
_EXACTLY_ | _FROM_ | _WORDS_ | _SENTENCES_ | _PARAGRAPHS_ | _SENTENCE_ | _PARAGRAPH_ | _REPLACE_ | _MODIFY_ | _FIRST_ |
_INSERT_ | _BEFORE_ | _AFTER_ | _REVALIDATION_ | _WITH_ | _NODES_ | _RENAME_ | _LAST_ | _DELETE_ | _INTO_ | _UPDATING_ |
_ORDERED_ | _UNORDERED_ | _ID_ | _KEY_ | _TEMPLATE_ | _MATCHES_ | _NAME_ | _CALL_ | _APPLY_ | _TEMPLATES_ | _MODE_ |
_FTOR_ | _FTAND_ | _FTNOT_ | _PRIVATE_ | _PUBLIC_ | _DETERMINISTIC_ | _NONDETERMINISTIC_
  ;

/* _XQUERY_ | */
/* _MODULE_ | */

%%

}  // namespace XQParser


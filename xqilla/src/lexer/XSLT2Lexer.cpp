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

#include "XSLT2Lexer.hpp"

#include <sstream>

#include <xqilla/ast/XQLiteral.hpp>
#include <xqilla/ast/XQAttributeConstructor.hpp>
#include <xqilla/utils/UTF8Str.hpp>
#include <xqilla/utils/XPath2Utils.hpp>
#include <xqilla/dom-api/impl/XQillaNSResolverImpl.hpp>

#include <xercesc/util/BinInputStream.hpp>
#include <xercesc/validators/schema/SchemaSymbols.hpp>
#include <xercesc/util/XMLChar.hpp>
#include <xercesc/internal/DGXMLScanner.hpp>
#include <xercesc/sax/Locator.hpp>
#include <xercesc/sax/SAXException.hpp>
#include <xercesc/util/XMLEntityResolver.hpp>

#ifdef XQ_DEBUG_LEXER

#include <iostream>
#define RECOGNIZE(token) { std::cerr << "Recognize: "#token << std::endl; return token; }

#else

#define RECOGNIZE(token) return token

#endif

XERCES_CPP_NAMESPACE_USE;
using namespace std;

const XMLCh XSLT_URI[] = { 'h', 't', 't', 'p', ':', '/', '/', 'w', 'w', 'w', '.', 'w', '3', '.', 'o', 'r', 'g', '/',
                           '1', '9', '9', '9', '/', 'X', 'S', 'L', '/', 'T', 'r', 'a', 'n', 's', 'f', 'o', 'r', 'm', 0 };
const XMLCh STYLESHEET_NAME[] = { 's', 't', 'y', 'l', 'e', 's', 'h', 'e', 'e', 't', 0 };
const XMLCh TRANSFORM_NAME[] = { 't', 'r', 'a', 'n', 's', 'f', 'o', 'r', 'm', 0 };
const XMLCh TEMPLATE_NAME[] = { 't', 'e', 'm', 'p', 'l', 'a', 't', 'e', 0 };
const XMLCh FUNCTION_NAME[] = { 'f', 'u', 'n', 'c', 't', 'i', 'o', 'n', 0 };
const XMLCh VALUE_OF_NAME[] = { 'v', 'a', 'l', 'u', 'e', '-', 'o', 'f', 0 };
const XMLCh TEXT_NAME[] = { 't', 'e', 'x', 't', 0 };
const XMLCh APPLY_TEMPLATES_NAME[] = { 'a', 'p', 'p', 'l', 'y', '-', 't', 'e', 'm', 'p', 'l', 'a', 't', 'e', 's', 0 };
const XMLCh CALL_TEMPLATE_NAME[] = { 'c', 'a', 'l', 'l', '-', 't', 'e', 'm', 'p', 'l', 'a', 't', 'e', 0 };
const XMLCh WITH_PARAM_NAME[] = { 'w', 'i', 't', 'h', '-', 'p', 'a', 'r', 'a', 'm', 0 };
const XMLCh SEQUENCE_NAME[] = { 's', 'e', 'q', 'u', 'e', 'n', 'c', 'e', 0 };
const XMLCh PARAM_NAME[] = { 'p', 'a', 'r', 'a', 'm', 0 };
const XMLCh CHOOSE_NAME[] = { 'c', 'h', 'o', 'o', 's', 'e', 0 };
const XMLCh WHEN_NAME[] = { 'w', 'h', 'e', 'n', 0 };
const XMLCh OTHERWISE_NAME[] = { 'o', 't', 'h', 'e', 'r', 'w', 'i', 's', 'e', 0 };
const XMLCh IF_NAME[] = { 'i', 'f', 0 };
const XMLCh VARIABLE_NAME[] = { 'v', 'a', 'r', 'i', 'a', 'b', 'l', 'e', 0 };
const XMLCh COMMENT_NAME[] = { 'c', 'o', 'm', 'm', 'e', 'n', 't', 0 };
const XMLCh PI_NAME[] = { 'p', 'r', 'o', 'c', 'e', 's', 's', 'i', 'n', 'g', '-', 'i', 'n', 's', 't', 'r', 'u', 'c', 't', 'i', 'o', 'n', 0 };
const XMLCh DOCUMENT_NAME[] = { 'd', 'o', 'c', 'u', 'm', 'e', 'n', 't', 0 };
const XMLCh ATTRIBUTE_NAME[] = { 'a', 't', 't', 'r', 'i', 'b', 'u', 't', 'e', 0 };
const XMLCh NAMESPACE_NAME[] = { 'n', 'a', 'm', 'e', 's', 'p', 'a', 'c', 'e', 0 };
const XMLCh ANALYZE_STRING_NAME[] = { 'a', 'n', 'a', 'l', 'y', 'z', 'e', '-', 's', 't', 'r', 'i', 'n', 'g', 0 };
const XMLCh MATCHING_SUBSTRING_NAME[] = { 'm', 'a', 't', 'c', 'h', 'i', 'n', 'g', '-', 's', 'u', 'b', 's', 't', 'r', 'i', 'n', 'g', 0 };
const XMLCh NON_MATCHING_SUBSTRING_NAME[] = { 'n', 'o', 'n', '-', 'm', 'a', 't', 'c', 'h', 'i', 'n', 'g', '-', 's', 'u', 'b', 's', 't', 'r', 'i', 'n', 'g', 0 };
const XMLCh COPY_OF_NAME[] = { 'c', 'o', 'p', 'y', '-', 'o', 'f', 0 };
const XMLCh COPY_NAME[] = { 'c', 'o', 'p', 'y', 0 };
const XMLCh FOR_EACH_NAME[] = { 'f', 'o', 'r', '-', 'e', 'a', 'c', 'h', 0 };
const XMLCh OUTPUT_NAME[] = { 'o', 'u', 't', 'p', 'u', 't', 0 };
const XMLCh IMPORT_SCHEMA_NAME[] = { 'i', 'm', 'p', 'o', 'r', 't', '-', 's', 'c', 'h', 'e', 'm', 'a', 0 };
const XMLCh ELEMENT_NAME[] = { 'e', 'l', 'e', 'm', 'e', 'n', 't', 0 };

const XMLCh VERSION_NAME[] = { 'v', 'e', 'r', 's', 'i', 'o', 'n', 0 };
const XMLCh MATCH_NAME[] = { 'm', 'a', 't', 'c', 'h', 0 };
const XMLCh AS_NAME[] = { 'a', 's', 0 };
const XMLCh SELECT_NAME[] = { 's', 'e', 'l', 'e', 'c', 't', 0 };
const XMLCh MODE_NAME[] = { 'm', 'o', 'd', 'e', 0 };
const XMLCh NAME_NAME[] = { 'n', 'a', 'm', 'e', 0 };
const XMLCh PRIORITY_NAME[] = { 'p', 'r', 'i', 'o', 'r', 'i', 't', 'y', 0 };
const XMLCh TUNNEL_NAME[] = { 't', 'u', 'n', 'n', 'e', 'l', 0 };
const XMLCh REQUIRED_NAME[] = { 'r', 'e', 'q', 'u', 'i', 'r', 'e', 'd', 0 };
const XMLCh OVERRIDE_NAME[] = { 'o', 'v', 'e', 'r', 'r', 'i', 'd', 'e', 0 };
const XMLCh TEST_NAME[] = { 't', 'e', 's', 't', 0 };
const XMLCh SEPARATOR_NAME[] = { 's', 'e', 'p', 'a', 'r', 'a', 't', 'o', 'r', 0 };
const XMLCh REGEX_NAME[] = { 'r', 'e', 'g', 'e', 'x', 0 };
const XMLCh FLAGS_NAME[] = { 'f', 'l', 'a', 'g', 's', 0 };
const XMLCh COPY_NAMESPACES_NAME[] = { 'c', 'o', 'p', 'y', '-', 'n', 'a', 'm', 'e', 's', 'p', 'a', 'c', 'e', 's', 0 };
const XMLCh INHERIT_NAMESPACES_NAME[] = { 'i', 'n', 'h', 'e', 'r', 'i', 't', '-', 'n', 'a', 'm', 'e', 's', 'p', 'a', 'c', 'e', 's', 0 };
const XMLCh METHOD_NAME[] = { 'm', 'e', 't', 'h', 'o', 'd', 0 };
const XMLCh BYTE_ORDER_MARK_NAME[] = { 'b', 'y', 't', 'e', '-', 'o', 'r', 'd', 'e', 'r', '-', 'm', 'a', 'r', 'k', 0 };
const XMLCh CDATA_SECTION_ELEMENTS_NAME[] = { 'c', 'd', 'a', 't', 'a', '-', 's', 'e', 'c', 't', 'i', 'o', 'n', '-', 'e', 'l', 'e', 'm', 'e', 'n', 't', 's', 0 };
const XMLCh DOCTYPE_PUBLIC_NAME[] = { 'd', 'o', 'c', 't', 'y', 'p', 'e', '-', 'p', 'u', 'b', 'l', 'i', 'c', 0 };
const XMLCh DOCTYPE_SYSTEM_NAME[] = { 'd', 'o', 'c', 't', 'y', 'p', 'e', '-', 's', 'y', 's', 't', 'e', 'm', 0 };
const XMLCh ENCODING_NAME[] = { 'e', 'n', 'c', 'o', 'd', 'i', 'n', 'g', 0 };
const XMLCh ESCAPE_URI_ATTRIBUTES_NAME[] = { 'e', 's', 'c', 'a', 'p', 'e', '-', 'u', 'r', 'i', '-', 'a', 't', 't', 'r', 'i', 'b', 'u', 't', 'e', 's', 0 };
const XMLCh INCLUDE_CONTENT_TYPE_NAME[] = { 'i', 'n', 'c', 'l', 'u', 'd', 'e', '-', 'c', 'o', 'n', 't', 'e', 'n', 't', '-', 't', 'y', 'p', 'e', 0 };
const XMLCh INDENT_NAME[] = { 'i', 'n', 'd', 'e', 'n', 't', 0 };
const XMLCh MEDIA_TYPE_NAME[] = { 'm', 'e', 'd', 'i', 'a', '-', 't', 'y', 'p', 'e', 0 };
const XMLCh NORMALIZATION_FORM_NAME[] = { 'n', 'o', 'r', 'm', 'a', 'l', 'i', 'z', 'a', 't', 'i', 'o', 'n', '-', 'f', 'o', 'r', 'm', 0 };
const XMLCh OMIT_XML_DECLARATION_NAME[] = { 'o', 'm', 'i', 't', '-', 'x', 'm', 'l', '-', 'd', 'e', 'c', 'l', 'a', 'r', 'a', 't', 'i', 'o', 'n', 0 };
const XMLCh STANDALONE_NAME[] = { 's', 't', 'a', 'n', 'd', 'a', 'l', 'o', 'n', 'e', 0 };
const XMLCh UNDECLARE_PREFIXES_NAME[] = { 'u', 'n', 'd', 'e', 'c', 'l', 'a', 'r', 'e', '-', 'p', 'r', 'e', 'f', 'i', 'x', 'e', 's', 0 };
const XMLCh USE_CHARACTER_MAPS_NAME[] = { 'u', 's', 'e', '-', 'c', 'h', 'a', 'r', 'a', 'c', 't', 'e', 'r', '-', 'm', 'a', 'p', 's', 0 };
const XMLCh EXCLUDE_RESULT_PREFIXES_NAME[] = { 'e', 'x', 'c', 'l', 'u', 'd', 'e', '-', 'r', 'e', 's', 'u', 'l', 't', '-', 'p', 'r', 'e', 'f', 'i', 'x', 'e', 's', 0 };
const XMLCh XPATH_DEFAULT_NAMESPACE_NAME[] = { 'x', 'p', 'a', 't', 'h', '-', 'd', 'e', 'f', 'a', 'u', 'l', 't', '-', 'n', 'a', 'm', 'e', 's', 'p', 'a', 'c', 'e', 0 };
const XMLCh SCHEMA_LOCATION_NAME[] = { 's', 'c', 'h', 'e', 'm', 'a', '-', 'l', 'o', 'c', 'a', 't', 'i', 'o', 'n', 0 };

const XMLCh SPACE_NAME[] = { 's', 'p', 'a', 'c', 'e', 0 };
const XMLCh PRESERVE_NAME[] = { 'p', 'r', 'e', 's', 'e', 'r', 'v', 'e', 0 };
const XMLCh DEFAULT_NAME[] = { 'd', 'e', 'f', 'a', 'u', 'l', 't', 0 };

const XMLCh YES_VALUE[] = { 'y', 'e', 's', 0 };
const XMLCh NO_VALUE[] = { 'n', 'o', 0 };

struct AttrData
{
  const XMLCh *name;
  int token;

  enum {
    PATTERN,
    SEQUENCE_TYPE,
    EXPRESSION,
    ATTR_VALUE_TEMPLATE,
    STRING,
    QNAME,
    YESNO,
    TEMPLATE_MODES,
    OUTPUT_METHOD,
    QNAMES
  } type;
};

#define AS_ATTR_DATA        { AS_NAME,        _XSLT_AS_,        AttrData::SEQUENCE_TYPE       }
#define SELECT_ATTR_DATA    { SELECT_NAME,    _XSLT_SELECT_,    AttrData::EXPRESSION          }
#define NAME_ATTR_DATA      { NAME_NAME,      _XSLT_NAME_,      AttrData::QNAME               }
#define SEPARATOR_ATTR_DATA { SEPARATOR_NAME, _XSLT_SEPARATOR_, AttrData::ATTR_VALUE_TEMPLATE }
#define END_ATTR_DATA       { 0,              0,                AttrData::STRING              }

const AttrData STYLESHEET_ATTR_DATA[] = {
  { VERSION_NAME, _XSLT_VERSION_, AttrData::STRING },
  { EXCLUDE_RESULT_PREFIXES_NAME, _XSLT_EXCLUDE_RESULT_PREFIXES_, AttrData::STRING },
  // TBD the rest of the attrs - jpcs
  END_ATTR_DATA
};

const AttrData TEMPLATE_ATTR_DATA[] = {
  { MATCH_NAME, _XSLT_MATCH_, AttrData::PATTERN },
  NAME_ATTR_DATA,
  { PRIORITY_NAME, _XSLT_PRIORITY_, AttrData::EXPRESSION },
  { MODE_NAME, _XSLT_MODE_, AttrData::TEMPLATE_MODES },
  AS_ATTR_DATA,
  END_ATTR_DATA
};

const AttrData FUNCTION_ATTR_DATA[] = {
  NAME_ATTR_DATA,
  AS_ATTR_DATA,
  { OVERRIDE_NAME, _XSLT_OVERRIDE_, AttrData::YESNO },
  END_ATTR_DATA
};

const AttrData VALUE_OF_ATTR_DATA[] = {
  SELECT_ATTR_DATA,
  SEPARATOR_ATTR_DATA,
  END_ATTR_DATA
};

const AttrData APPLY_TEMPLATES_ATTR_DATA[] = {
  { MODE_NAME, _XSLT_MODE_, AttrData::TEMPLATE_MODES },
  SELECT_ATTR_DATA,
  END_ATTR_DATA
};

const AttrData CALL_TEMPLATE_ATTR_DATA[] = {
  NAME_ATTR_DATA,
  END_ATTR_DATA
};

const AttrData WITH_PARAM_ATTR_DATA[] = {
  NAME_ATTR_DATA,
  SELECT_ATTR_DATA,
  AS_ATTR_DATA,
  { TUNNEL_NAME, _XSLT_TUNNEL_, AttrData::YESNO },
  END_ATTR_DATA
};

const AttrData SEQUENCE_ATTR_DATA[] = {
  SELECT_ATTR_DATA,
  END_ATTR_DATA
};

const AttrData PARAM_ATTR_DATA[] = {
  NAME_ATTR_DATA,
  SELECT_ATTR_DATA,
  AS_ATTR_DATA,
  { REQUIRED_NAME, _XSLT_REQUIRED_, AttrData::YESNO },
  { TUNNEL_NAME, _XSLT_TUNNEL_, AttrData::YESNO },
  END_ATTR_DATA
};

const AttrData G_TEST_ATTR_DATA[] = {
  { TEST_NAME, _XSLT_TEST_, AttrData::EXPRESSION },
  END_ATTR_DATA
};

const AttrData VARIABLE_ATTR_DATA[] = {
  NAME_ATTR_DATA,
  SELECT_ATTR_DATA,
  AS_ATTR_DATA,
  END_ATTR_DATA
};

const AttrData COMMENT_ATTR_DATA[] = {
  SELECT_ATTR_DATA,
  END_ATTR_DATA
};

const AttrData PI_ATTR_DATA[] = {
  { NAME_NAME, _XSLT_NAME_, AttrData::ATTR_VALUE_TEMPLATE },
  SELECT_ATTR_DATA,
  END_ATTR_DATA
};

const AttrData ATTRIBUTE_ATTR_DATA[] = {
  { NAME_NAME,      _XSLT_NAME_,        AttrData::ATTR_VALUE_TEMPLATE },
  { NAMESPACE_NAME, _XSLT_NAMESPACE_A_, AttrData::ATTR_VALUE_TEMPLATE },
  SELECT_ATTR_DATA,
  SEPARATOR_ATTR_DATA,
  // TBD validation and type - jpcs
  END_ATTR_DATA
};

const AttrData NAMESPACE_ATTR_DATA[] = {
  { NAME_NAME,      _XSLT_NAME_,        AttrData::ATTR_VALUE_TEMPLATE },
  SELECT_ATTR_DATA,
  END_ATTR_DATA
};

const AttrData ELEMENT_ATTR_DATA[] = {
  { NAME_NAME,      _XSLT_NAME_,        AttrData::ATTR_VALUE_TEMPLATE },
  { NAMESPACE_NAME, _XSLT_NAMESPACE_A_, AttrData::ATTR_VALUE_TEMPLATE },
  // TBD inherit-namespaces - jpcs
  // TBD use-attribute-sets - jpcs
  // TBD validation and type - jpcs
  END_ATTR_DATA
};

const AttrData ANALYZE_STRING_ATTR_DATA[] = {
  SELECT_ATTR_DATA,
  { REGEX_NAME, _XSLT_REGEX_, AttrData::ATTR_VALUE_TEMPLATE },
  { FLAGS_NAME, _XSLT_FLAGS_, AttrData::ATTR_VALUE_TEMPLATE },
  END_ATTR_DATA
};

const AttrData COPY_OF_ATTR_DATA[] = {
  SELECT_ATTR_DATA,
  { COPY_NAMESPACES_NAME, _XSLT_COPY_NAMESPACES_, AttrData::YESNO },
  // TBD validation and type - jpcs
  END_ATTR_DATA
};

const AttrData COPY_ATTR_DATA[] = {
  { COPY_NAMESPACES_NAME, _XSLT_COPY_NAMESPACES_, AttrData::YESNO },
  { INHERIT_NAMESPACES_NAME, _XSLT_INHERIT_NAMESPACES_, AttrData::YESNO },
  // TBD attribute sets - jpcs
  // TBD validation and type - jpcs
  END_ATTR_DATA
};

const AttrData FOR_EACH_ATTR_DATA[] = {
  SELECT_ATTR_DATA
};

const AttrData OUTPUT_ATTR_DATA[] = {
  // <xsl:output
  //   name? = qname
  NAME_ATTR_DATA,
  //   method? = "xml" | "html" | "xhtml" | "text" | qname-but-not-ncname
  { METHOD_NAME, _XSLT_METHOD_, AttrData::OUTPUT_METHOD },
  //   byte-order-mark? = "yes" | "no"
  { BYTE_ORDER_MARK_NAME, _XSLT_BYTE_ORDER_MARK_, AttrData::YESNO },
  //   cdata-section-elements? = qnames
  { CDATA_SECTION_ELEMENTS_NAME, _XSLT_CDATA_SECTION_ELEMENTS_, AttrData::QNAMES },
  //   doctype-public? = string
  { DOCTYPE_PUBLIC_NAME, _XSLT_DOCTYPE_PUBLIC_, AttrData::STRING },
  //   doctype-system? = string
  { DOCTYPE_SYSTEM_NAME, _XSLT_DOCTYPE_SYSTEM_, AttrData::STRING },
  //   encoding? = string
  { ENCODING_NAME, _XSLT_ENCODING_, AttrData::STRING },
  //   escape-uri-attributes? = "yes" | "no"
  { ESCAPE_URI_ATTRIBUTES_NAME, _XSLT_ESCAPE_URI_ATTRIBUTES_, AttrData::YESNO },
  //   include-content-type? = "yes" | "no"
  { INCLUDE_CONTENT_TYPE_NAME, _XSLT_INCLUDE_CONTENT_TYPE_, AttrData::YESNO },
  //   indent? = "yes" | "no"
  { INDENT_NAME, _XSLT_INDENT_, AttrData::YESNO },
  //   media-type? = string
  { MEDIA_TYPE_NAME, _XSLT_MEDIA_TYPE_, AttrData::STRING },
  //   normalization-form? = "NFC" | "NFD" | "NFKC" | "NFKD" | "fully-normalized" | "none" | nmtoken
  { NORMALIZATION_FORM_NAME, _XSLT_NORMALIZATION_FORM_, AttrData::STRING },
  //   omit-xml-declaration? = "yes" | "no"
  { OMIT_XML_DECLARATION_NAME, _XSLT_OMIT_XML_DECLARATION_, AttrData::YESNO },
  //   standalone? = "yes" | "no" | "omit"
  { STANDALONE_NAME, _XSLT_STANDALONE_, AttrData::STRING },
  //   undeclare-prefixes? = "yes" | "no"
  { UNDECLARE_PREFIXES_NAME, _XSLT_UNDECLARE_PREFIXES_, AttrData::YESNO },
  //   use-character-maps? = qnames
  { USE_CHARACTER_MAPS_NAME, _XSLT_USE_CHARACTER_MAPS_, AttrData::QNAMES },
  //   version? = nmtoken />
  { VERSION_NAME, _XSLT_VERSION_, AttrData::STRING }
};

const AttrData IMPORT_SCHEMA_ATTR_DATA[] = {
  { NAMESPACE_NAME, _XSLT_NAMESPACE_STR_, AttrData::STRING },
  { SCHEMA_LOCATION_NAME, _XSLT_SCHEMA_LOCATION_, AttrData::STRING },
  END_ATTR_DATA
};

struct InstructionInfo
{
  const XMLCh *name;
  int token;
  const AttrData *attrs;

  enum {
    STRIP,
    PRESERVE,
    DEFAULT
  } whitespace;
};

const InstructionInfo INSTRUCTION_INFO[] = {
  { STYLESHEET_NAME,             _XSLT_STYLESHEET_,             STYLESHEET_ATTR_DATA,      InstructionInfo::STRIP    },
  { TRANSFORM_NAME,              _XSLT_STYLESHEET_,             STYLESHEET_ATTR_DATA,      InstructionInfo::STRIP    },
  { TEMPLATE_NAME,               _XSLT_TEMPLATE_,               TEMPLATE_ATTR_DATA,        InstructionInfo::DEFAULT  },
  { FUNCTION_NAME,               _XSLT_FUNCTION_,               FUNCTION_ATTR_DATA,        InstructionInfo::DEFAULT  },
  { VALUE_OF_NAME,               _XSLT_VALUE_OF_,               VALUE_OF_ATTR_DATA,        InstructionInfo::DEFAULT  },
  { TEXT_NAME,                   _XSLT_TEXT_,                   0,                         InstructionInfo::PRESERVE },
  { APPLY_TEMPLATES_NAME,        _XSLT_APPLY_TEMPLATES_,        APPLY_TEMPLATES_ATTR_DATA, InstructionInfo::STRIP    },
  { CALL_TEMPLATE_NAME,          _XSLT_CALL_TEMPLATE_,          CALL_TEMPLATE_ATTR_DATA,   InstructionInfo::STRIP    },
  { WITH_PARAM_NAME,             _XSLT_WITH_PARAM_,             WITH_PARAM_ATTR_DATA,      InstructionInfo::DEFAULT  },
  { SEQUENCE_NAME,               _XSLT_SEQUENCE_,               SEQUENCE_ATTR_DATA,        InstructionInfo::DEFAULT  },
  { PARAM_NAME,                  _XSLT_PARAM_,                  PARAM_ATTR_DATA,           InstructionInfo::DEFAULT  },
  { CHOOSE_NAME,                 _XSLT_CHOOSE_,                 0,                         InstructionInfo::STRIP    },
  { WHEN_NAME,                   _XSLT_WHEN_,                   G_TEST_ATTR_DATA,          InstructionInfo::DEFAULT  },
  { OTHERWISE_NAME,              _XSLT_OTHERWISE_,              0,                         InstructionInfo::DEFAULT  },
  { IF_NAME,                     _XSLT_IF_,                     G_TEST_ATTR_DATA,          InstructionInfo::DEFAULT  },
  { VARIABLE_NAME,               _XSLT_VARIABLE_,               VARIABLE_ATTR_DATA,        InstructionInfo::DEFAULT  },
  { COMMENT_NAME,                _XSLT_COMMENT_,                COMMENT_ATTR_DATA,         InstructionInfo::DEFAULT  },
  { PI_NAME,                     _XSLT_PI_,                     PI_ATTR_DATA,              InstructionInfo::DEFAULT  },
  { DOCUMENT_NAME,               _XSLT_DOCUMENT_,               0,                         InstructionInfo::DEFAULT  },
  { ATTRIBUTE_NAME,              _XSLT_ATTRIBUTE_,              ATTRIBUTE_ATTR_DATA,       InstructionInfo::DEFAULT  },
  { ANALYZE_STRING_NAME,         _XSLT_ANALYZE_STRING_,         ANALYZE_STRING_ATTR_DATA,  InstructionInfo::STRIP    },
  { MATCHING_SUBSTRING_NAME,     _XSLT_MATCHING_SUBSTRING_,     0,                         InstructionInfo::DEFAULT  },
  { NON_MATCHING_SUBSTRING_NAME, _XSLT_NON_MATCHING_SUBSTRING_, 0,                         InstructionInfo::DEFAULT  },
  { COPY_OF_NAME,                _XSLT_COPY_OF_,                COPY_OF_ATTR_DATA,         InstructionInfo::DEFAULT  },
  { COPY_NAME,                   _XSLT_COPY_,                   COPY_ATTR_DATA,            InstructionInfo::DEFAULT  },
  { FOR_EACH_NAME,               _XSLT_FOR_EACH_,               FOR_EACH_ATTR_DATA,        InstructionInfo::DEFAULT  },
  { OUTPUT_NAME,                 _XSLT_OUTPUT_,                 OUTPUT_ATTR_DATA,          InstructionInfo::DEFAULT  },
  { IMPORT_SCHEMA_NAME,          _XSLT_IMPORT_SCHEMA_,          IMPORT_SCHEMA_ATTR_DATA,   InstructionInfo::DEFAULT  },
  { ELEMENT_NAME,                _XSLT_ELEMENT_,                ELEMENT_ATTR_DATA,         InstructionInfo::DEFAULT  },
  { NAMESPACE_NAME,              _XSLT_NAMESPACE_,              NAMESPACE_ATTR_DATA,       InstructionInfo::DEFAULT  },
  { 0, 0 }
};

// Strip elements:
//
//         xsl:analyze-string
//     xsl:apply-imports
//         xsl:apply-templates
//     xsl:attribute-set
//         xsl:call-template
//     xsl:character-map
//         xsl:choose
//     xsl:next-match
//         xsl:stylesheet
//         xsl:transform

XSLT2Lexer::XSLT2Lexer(DynamicContext *context, const InputSource &srcToUse, XQilla::Language lang)
  : Lexer(context->getMemoryManager(), lang, context->getMemoryManager()->getPooledString(srcToUse.getSystemId()), 0, 0),
    context_(context),
    childLexer_(0),
    elementStack_(0),
    state_(LANG_TOKEN),
    textBuffer_(1023, context->getMemoryManager()),
    textToCreate_(false)
{
}

XSLT2Lexer::~XSLT2Lexer()
{
  while(elementStack_ != 0) {
    popElementStack();
  }
}

void XSLT2Lexer::undoLessThan()
{
  if(childLexer_.get() != 0) {
    childLexer_->undoLessThan();
  }
}

int XSLT2Lexer::lang_token_state(YYSTYPE* pYYLVAL, YYLTYPE* pYYLOC)
{
  state_ = NEXT_EVENT;
  RECOGNIZE(_LANG_XSLT2_);
}

int XSLT2Lexer::attrs_state(YYSTYPE* pYYLVAL, YYLTYPE* pYYLOC)
{
  const XMLCh *prefix, *uri, *name, *value;
  unsigned int length;
  getEventName(prefix, uri, name);
  getEventValue(value, length, offsets_);
  getEventLocation(pYYLOC);

  if(XPath2Utils::equals(uri, XMLUni::fgXMLURIName) && XPath2Utils::equals(name, SPACE_NAME)) {
    value = mm_->getPooledString(value, length);
    // Keep track of the xml:space policy
    if(XPath2Utils::equals(value, PRESERVE_NAME)) {
      elementStack_->xmlSpacePreserve = true;
    }
    else if(XPath2Utils::equals(value, DEFAULT_NAME)) {
      elementStack_->xmlSpacePreserve = false;
    }
  }
  else if(elementStack_->info != 0 && uri == 0) {
    if(elementStack_->info->attrs != 0) {
      for(const AttrData *entry = elementStack_->info->attrs; entry->name != 0; ++entry) {
        if(XPath2Utils::equals(name, entry->name)) {
          switch(entry->type) {
          case AttrData::PATTERN:
          case AttrData::SEQUENCE_TYPE:
          case AttrData::EXPRESSION:
            getValueLocation(pYYLOC);
            childLexer_.set(new XQLexer(mm_, m_szQueryFile, m_lineno, m_columnno, value, length, offsets_,
                                        (XQilla::Language)((m_language & ~XQilla::XSLT2) | XQilla::XPATH2)));
            break;
          case AttrData::ATTR_VALUE_TEMPLATE:
            getValueLocation(pYYLOC);
            childLexer_.set(new XQLexer(mm_, m_szQueryFile, m_lineno, m_columnno, value, length, offsets_,
                                        (XQilla::Language)((m_language & ~XQilla::XSLT2) | XQilla::XPATH2), XQLexer::MODE_ATTR_VALUE_TEMPLATE));
            break;
          case AttrData::TEMPLATE_MODES:
            getValueLocation(pYYLOC);
            childLexer_.set(new XQLexer(mm_, m_szQueryFile, m_lineno, m_columnno, value, length, offsets_,
                                        (XQilla::Language)((m_language & ~XQilla::XSLT2) | XQilla::XPATH2), XQLexer::MODE_TEMPLATE_MODES));
            break;
          case AttrData::OUTPUT_METHOD:
            getValueLocation(pYYLOC);
            childLexer_.set(new XQLexer(mm_, m_szQueryFile, m_lineno, m_columnno, value, length, offsets_,
                                        (XQilla::Language)((m_language & ~XQilla::XSLT2) | XQilla::XPATH2), XQLexer::MODE_OUTPUT_METHOD));
            break;
          case AttrData::QNAMES:
            getValueLocation(pYYLOC);
            childLexer_.set(new XQLexer(mm_, m_szQueryFile, m_lineno, m_columnno, value, length, offsets_,
                                        (XQilla::Language)((m_language & ~XQilla::XSLT2) | XQilla::XPATH2), XQLexer::MODE_QNAMES));
            break;
          case AttrData::STRING:
            pYYLVAL->str = (XMLCh*)mm_->getPooledString(value, length);
            break;
          case AttrData::QNAME:
            pYYLVAL->str = (XMLCh*)mm_->getPooledString(value, length);
            if(!XMLChar1_0::isValidQName(value, length)) {
              getValueLocation(pYYLOC);
              std::ostringstream oss;
              oss << "The attribute value \"" << UTF8(pYYLVAL->str) << "\" is not a valid xs:QName [err:XTSE0020]";
              error(oss.str().c_str());
            }
            break;
          case AttrData::YESNO:
            XMLBuffer valueBuf;
            valueBuf.set(value, length);
            XMLString::collapseWS(valueBuf.getRawBuffer());

            if(XPath2Utils::equals(valueBuf.getRawBuffer(), YES_VALUE)) {
              pYYLVAL->boolean = true;
            }
            else if(XPath2Utils::equals(valueBuf.getRawBuffer(), NO_VALUE)) {
              pYYLVAL->boolean = false;
            }
            else {
              getValueLocation(pYYLOC);
              error("The attribute does not have a value of \"yes\" or \"no\" [err:XTSE0020]");
            }

            break;
          }

          state_ = NEXT_EVENT;
          RECOGNIZE(entry->token);
        }
      }
    }

    // Is it one of the standard attributes?
    if(XPath2Utils::equals(name, XPATH_DEFAULT_NAMESPACE_NAME)) {
      // TBD xpath-default-name - jpcs
    }
  }

  if(elementStack_->info == 0) {
    pYYLVAL->astNode = makeQNameLiteral(uri, prefix, name);

    // Set up childLexer_ to lex the attribute value as an attribute value template
    getValueLocation(pYYLOC);
    childLexer_.set(new XQLexer(mm_, m_szQueryFile, m_lineno, m_columnno, value, length, offsets_,
                                (XQilla::Language)((m_language & ~XQilla::XSLT2) | XQilla::XPATH2), XQLexer::MODE_ATTR_VALUE_TEMPLATE));

    state_ = NEXT_EVENT;
    RECOGNIZE(_XSLT_ATTR_NAME_);
  }
  else if(uri != 0) {
    // Extension attribute - ignore for the moment
    state_ = NEXT_EVENT;
    return yylex(pYYLVAL, pYYLOC);
  }
  else {
    ostringstream oss;
    oss << "Unexpected attribute {" << UTF8(uri) << "}" << UTF8(name) << " [err:XTSE0090]";
    error(oss.str().c_str());
  }

  // Never happens
  RECOGNIZE(MYEOF);
}

#define checkTextBuffer(uri, name) \
{ \
  if(textToCreate_) { \
    bool isWS = XMLChar1_0::isAllSpaces(textBuffer_.getRawBuffer(), textBuffer_.getLen()); \
    if(((name) != 0 && XPath2Utils::equals((uri), XSLT_URI) && \
        XPath2Utils::equals((name), PARAM_NAME) && isWS) || \
       (!elementStack_->reportWhitespace() && isWS)) { \
      textBuffer_.reset(); \
      textToCreate_ = false; \
    } \
    else { \
      pYYLVAL->astNode = makeStringLiteral(mm_->getPooledString(textBuffer_.getRawBuffer())); \
\
      textBuffer_.reset(); \
      textToCreate_ = false; \
\
      state_ = CURRENT_EVENT; \
      if(isWS) { \
        RECOGNIZE(_XSLT_WS_TEXT_NODE_); \
      } \
      else { \
        RECOGNIZE(_XSLT_TEXT_NODE_); \
      } \
    } \
  } \
}

int XSLT2Lexer::next_event_state(YYSTYPE* pYYLVAL, YYLTYPE* pYYLOC)
{
  nextEvent(pYYLOC);
  state_ = CURRENT_EVENT;
  return current_event_state(pYYLVAL, pYYLOC);
}

void XSLT2Lexer::setNamespace(const XMLCh *prefix, const XMLCh *uri)
{
  if(elementStack_->info != 0 && elementStack_->info->token == _XSLT_STYLESHEET_) {
    // If this is the xsl:stylesheet element, add the namespace bindings to the context
    if(prefix == 0 || *prefix == 0)
      context_->setDefaultElementAndTypeNS(uri);
    else context_->setNamespaceBinding(prefix, uri);
  }
  else {
    if(elementStack_->nsResolver == elementStack_->prev->nsResolver) {
      elementStack_->nsResolver = new (mm_) XQillaNSResolverImpl(mm_, elementStack_->nsResolver != 0 ?
                                                                 elementStack_->nsResolver : context_->getNSResolver());
    }

    elementStack_->nsResolver->addNamespaceBinding(prefix, uri);
  }
}

int XSLT2Lexer::current_event_state(YYSTYPE* pYYLVAL, YYLTYPE* pYYLOC)
{
  while(true) {
    switch(getEventType()) {
    case START_DOCUMENT:
      pushElementStack();
      break;
    case END_DOCUMENT:
      popElementStack();

      // End of file
      RECOGNIZE(MYEOF);
      break;
    case START_ELEMENT: {
      const XMLCh *prefix, *uri, *name;
      getEventName(prefix, uri, name);

      checkTextBuffer(uri, name);

      getEventLocation(pYYLOC);
      pushElementStack();

      // See if we recognize the element
      if(XPath2Utils::equals(uri, XSLT_URI)) {
        for(const InstructionInfo *entry = INSTRUCTION_INFO; entry->name != 0; ++entry) {
          if(XPath2Utils::equals(name, entry->name)) {
            elementStack_->info = entry;
          }
        }
        if(elementStack_->info == 0) {
          std::ostringstream oss;
          oss << "Unrecognized XSLT element {" << UTF8(uri) << "}" << UTF8(name);
          error(oss.str().c_str());
        }
      }

      state_ = NEXT_EVENT;
      if(elementStack_->info != 0) {
        RECOGNIZE(elementStack_->info->token);
      }

      pYYLVAL->astNode = makeQNameLiteral(uri, prefix, name);

      RECOGNIZE(_XSLT_ELEMENT_NAME_);
      break;
    }
    case END_ELEMENT:
      checkTextBuffer(0, 0);

      getEventLocation(pYYLOC);

      // Put off popping the stack until before the next token
      state_ = POP_STACK;
      RECOGNIZE(_XSLT_END_ELEMENT_);
      break;
    case TEXT:
      if(!textToCreate_) getEventLocation(pYYLOC);

      const XMLCh *value;
      unsigned int length;
      getEventValue(value, length, offsets_);

      textBuffer_.append(value, length);
      textToCreate_ = true;
      break;
    case NAMESPACE: {
      const XMLCh *prefix, *uri, *name, *value;
      unsigned int length;
      getEventName(prefix, uri, name);
      getEventValue(value, length, offsets_);

      const XMLCh *nsuri = mm_->getPooledString(value, length);

      if(!XPath2Utils::equals(uri, XMLUni::fgXMLNSURIName) &&
         XPath2Utils::equals(name, XPATH_DEFAULT_NAMESPACE_NAME)) {
        setNamespace(0, nsuri);
      }
      else {
        if(prefix != 0 && *prefix != 0) {
          setNamespace(name, nsuri);
        }

        if(elementStack_->info == 0) {
          VectorOfASTNodes *valueVector = new (mm_) VectorOfASTNodes(XQillaAllocator<ASTNode*>(mm_));
          valueVector->push_back(makeStringLiteral(nsuri));

          pYYLVAL->astNode = wrap(new (mm_) XQAttributeConstructor(makeDirectName(prefix, name), valueVector, mm_));

          state_ = NEXT_EVENT;
          RECOGNIZE(_XSLT_XMLNS_ATTR_);
        }
      }
      break;
    }
    case ATTRIBUTE:
      return attrs_state(pYYLVAL, pYYLOC);
    }

    nextEvent(pYYLOC);
  }

  // Never happens
  RECOGNIZE(MYEOF);
}

int XSLT2Lexer::yylex(YYSTYPE* pYYLVAL, YYLTYPE* pYYLOC)
{
  if(childLexer_.get() != 0) {
    int token = childLexer_->yylex(pYYLVAL, pYYLOC);
    if(token != MYEOF) {
      m_lineno = pYYLOC->first_line;
      m_columnno = pYYLOC->first_column;
      return token;
    }

    childLexer_.set(0);
  }

  switch(state_) {
  case LANG_TOKEN:
    return lang_token_state(pYYLVAL, pYYLOC);
  case POP_STACK:
    popElementStack();
    state_ = NEXT_EVENT;
    // Fall through
  case NEXT_EVENT:
    return next_event_state(pYYLVAL, pYYLOC);
  case CURRENT_EVENT:
    return current_event_state(pYYLVAL, pYYLOC);
  }

  // Never happens
  RECOGNIZE(MYEOF);
}

ASTNode *XSLT2Lexer::wrap(ASTNode *result) const
{
  result->setLocationInfo(m_szQueryFile, m_lineno, m_columnno);
  return result;
}

ASTNode *XSLT2Lexer::makeQNameLiteral(const XMLCh *uri, const XMLCh *prefix, const XMLCh *name) const
{
  return wrap(new (mm_) XQQNameLiteral(SchemaSymbols::fgURI_SCHEMAFORSCHEMA,
                                       SchemaSymbols::fgDT_QNAME,
                                       uri, prefix, name, mm_));
}

ASTNode *XSLT2Lexer::makeDirectName(const XMLCh *prefix, const XMLCh *name) const
{
  static const XMLCh colon[] = { ':', 0 };

  return wrap(new (mm_) XQDirectName(prefix == 0 ? name : XPath2Utils::concatStrings(prefix, colon, name, mm_), /*isAttr*/true, mm_));
}

ASTNode *XSLT2Lexer::makeStringLiteral(const XMLCh *value) const
{
  return wrap(new (mm_) XQLiteral(SchemaSymbols::fgURI_SCHEMAFORSCHEMA,
                                  SchemaSymbols::fgDT_STRING,
                                  value, AnyAtomicType::STRING, mm_));
}

void XSLT2Lexer::pushElementStack()
{
  elementStack_ = new ElementStackEntry(elementStack_);
}

void XSLT2Lexer::popElementStack()
{
  ElementStackEntry *tmp = elementStack_->prev;
  delete elementStack_;
  elementStack_ = tmp;
}

DOMXPathNSResolver *XSLT2Lexer::getNSResolver() const
{
  return elementStack_->nsResolver;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

XSLT2Lexer::ElementStackEntry::ElementStackEntry(ElementStackEntry *p)
  : info(0),
    xmlSpacePreserve(p ? p->xmlSpacePreserve : false),
    nsResolver(p ? p->nsResolver : 0),
    prev(p)
{
}

bool XSLT2Lexer::ElementStackEntry::reportWhitespace() const
{
  if(xmlSpacePreserve) return info == 0 || info->whitespace != InstructionInfo::STRIP;
  return info != 0 && info->whitespace == InstructionInfo::PRESERVE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef HAVE_FAXPP

static inline const XMLCh *nullTerm(const FAXPP_Text &text, XPath2MemoryManager *mm)
{
  return mm->getPooledString((XMLCh*)text.ptr, text.len / sizeof(XMLCh));
}

FAXPPXSLT2Lexer::FAXPPXSLT2Lexer(DynamicContext *context, const InputSource &srcToUse, XQilla::Language lang)
  : XSLT2Lexer(context, srcToUse, lang),
    wrapper_(context->getDocumentCache()->getXMLEntityResolver()),
    eventType_(END_ELEMENT),
    attrIndex_(0)
{
  // We'll normalize attributes ourselves
  FAXPP_set_normalize_attrs(wrapper_.parser, false);

  FAXPP_Error err = wrapper_.parseInputSource(srcToUse, context->getMemoryManager());

  if(err == OUT_OF_MEMORY) error("Out of memory");
  if(err == UNSUPPORTED_ENCODING) error("Unsupported encoding");
}

FAXPPXSLT2Lexer::~FAXPPXSLT2Lexer()
{
}

bool FAXPPXSLT2Lexer::nextNamespace()
{
  const FAXPP_Event *event = FAXPP_get_current_event(wrapper_.parser);

  eventType_ = NAMESPACE;
  while(attrIndex_ < event->attr_count) {
    FAXPP_Attribute *attr = &event->attrs[attrIndex_];

    if(attr->xmlns_attr)
      return true;

    // Recoginze [xsl:]xpath-default-namespace as a type NAMESPACE too
    if(XPath2Utils::equals(nullTerm(event->uri, mm_), XSLT_URI)) {
      if(attr->uri.len == 0 && XPath2Utils::equals(nullTerm(attr->name, mm_), XPATH_DEFAULT_NAMESPACE_NAME)) {
        attr->xmlns_attr = 1;
        return true;
      }
    }
    else {
      if(XPath2Utils::equals(nullTerm(attr->name, mm_), XPATH_DEFAULT_NAMESPACE_NAME) &&
         XPath2Utils::equals(nullTerm(attr->uri, mm_), XSLT_URI)) {
        attr->xmlns_attr = 1;
        return true;
      }
    }

    ++attrIndex_;
  }

  attrIndex_ = 0;
  return nextAttribute();
}

bool FAXPPXSLT2Lexer::nextAttribute()
{
  const FAXPP_Event *event = FAXPP_get_current_event(wrapper_.parser);

  eventType_ = ATTRIBUTE;
  while(attrIndex_ < event->attr_count) {
    if(!event->attrs[attrIndex_].xmlns_attr)
      return true;
    ++attrIndex_;
  }

  eventType_ = (XSLT2Lexer::EventType)-1;
  return false;
}

void FAXPPXSLT2Lexer::nextEvent(YYLTYPE* pYYLOC)
{
  const FAXPP_Event *event = FAXPP_get_current_event(wrapper_.parser);

  if(eventType_ == START_ELEMENT) {
    attrIndex_ = 0;
    if(nextNamespace()) return;
  }

  if(eventType_ == NAMESPACE) {
    ++attrIndex_;
    if(nextNamespace()) return;
  }

  if(eventType_ == ATTRIBUTE) {
    ++attrIndex_;
    if(nextAttribute()) return;
  }

  if(eventType_ != END_ELEMENT && event->type == SELF_CLOSING_ELEMENT_EVENT) {
    eventType_ = END_ELEMENT;
    return;
  }

  eventType_ = (XSLT2Lexer::EventType)-1;
  while(eventType_ == (XSLT2Lexer::EventType)-1) {
    FAXPP_Error err = FAXPP_next_event(wrapper_.parser);
    if(err != NO_ERROR) {
      setErrorLocation(pYYLOC);

      ostringstream oss;
      oss << "Error parsing document: " << FAXPP_err_to_string(err);
      error(oss.str().c_str());
    }

    event = FAXPP_get_current_event(wrapper_.parser);
    switch(event->type) {
    case SELF_CLOSING_ELEMENT_EVENT:
    case START_ELEMENT_EVENT:
      eventType_ = START_ELEMENT;
      break;
    case END_ELEMENT_EVENT:
      eventType_ = END_ELEMENT;
      break;
    case ENTITY_REFERENCE_EVENT:
      // TBD handle external parsed entities - jpcs
      if(event->value.ptr == 0) break;
      // Fall through
    case DEC_CHAR_REFERENCE_EVENT:
    case HEX_CHAR_REFERENCE_EVENT:
    case CHARACTERS_EVENT:
    case CDATA_EVENT:
      eventType_ = TEXT;
      break;
    case START_DOCUMENT_EVENT:
      eventType_ = START_DOCUMENT;
      break;
    case END_DOCUMENT_EVENT:
      eventType_ = END_DOCUMENT;
      break;
    case COMMENT_EVENT:
    case PI_EVENT:
    case IGNORABLE_WHITESPACE_EVENT:
    case DOCTYPE_EVENT:
    case ENTITY_REFERENCE_START_EVENT:
    case ENTITY_REFERENCE_END_EVENT:
    case START_EXTERNAL_ENTITY_EVENT:
    case END_EXTERNAL_ENTITY_EVENT:
    case NO_EVENT:
      // Ignore
      break;
    }
  }
}

XSLT2Lexer::EventType FAXPPXSLT2Lexer::getEventType()
{
  return eventType_;
}

void FAXPPXSLT2Lexer::getEventName(const XMLCh *&prefix, const XMLCh *&uri, const XMLCh *&localname)
{
  const FAXPP_Event *event = FAXPP_get_current_event(wrapper_.parser);

  if(eventType_ == ATTRIBUTE || eventType_ == NAMESPACE) {
    FAXPP_Attribute *attr = &event->attrs[attrIndex_];
//     setLocation(pYYLOC, attr);

    prefix = nullTerm(attr->prefix, mm_);
    uri = nullTerm(attr->uri, mm_);
    localname = nullTerm(attr->name, mm_);
  }
  else {
    prefix = nullTerm(event->prefix, mm_);
    uri = nullTerm(event->uri, mm_);
    localname = nullTerm(event->name, mm_);
  }
}

void FAXPPXSLT2Lexer::getEventValue(const XMLCh *&value, unsigned int &length, std::vector<XQLexer::ValueOffset> &offsets)
{
  const FAXPP_Event *event = FAXPP_get_current_event(wrapper_.parser);
  offsets.clear();

  if(eventType_ == ATTRIBUTE || eventType_ == NAMESPACE) {
    FAXPP_Attribute *attr = &event->attrs[attrIndex_];
    value_.reset();

    // Normalize the attribute value and calculate offsets into
    // it to compensate for things like expanded entity references
    // and line ending conversion.
    const FAXPP_AttrValue *aval = &attr->value;
    int index = 0, line = aval->line, column = aval->column + 1;
    for(; aval; aval = aval->next) {
      if((unsigned)line != aval->line || (unsigned)column != (aval->column + 1)) {
        offsets.push_back(XQLexer::ValueOffset(index, aval->line - line, (aval->column + 1) - column));
        line = aval->line;
        column = aval->column + 1;
      }

      const XMLCh *pos = (const XMLCh*)aval->value.ptr;
      const XMLCh *end = (const XMLCh*)((char*)aval->value.ptr + aval->value.len);
      for(; pos < end; ++pos) {
        switch(*pos) {
        case '\r':
          if((pos + 1) < end && *(pos + 1) == '\n') ++pos;
          // Fall through
        case '\n':
          offsets.push_back(XQLexer::ValueOffset(index, 1, -column));
          line += 1;
          column = 1;
          value_.append(' ');
          break;
        case '\t':
          value_.append(' ');
          ++column;
          break;
        default:
          value_.append(*pos);
          ++column;
          break;
        }
        ++index;
      }
    }

    value = value_.getRawBuffer();
    length = value_.getLen();
  }
  else {
    value = (XMLCh*)event->value.ptr;
    length = event->value.len / sizeof(XMLCh);
  }
}

void FAXPPXSLT2Lexer::getEventLocation(YYLTYPE* pYYLOC)
{
  if(eventType_ == ATTRIBUTE || eventType_ == NAMESPACE) {
    FAXPP_Attribute *attr = &FAXPP_get_current_event(wrapper_.parser)->attrs[attrIndex_];
    setLocation(pYYLOC, attr);
  }
  else {
    setLocation(pYYLOC, FAXPP_get_current_event(wrapper_.parser));
  }
}

void FAXPPXSLT2Lexer::getValueLocation(YYLTYPE* pYYLOC)
{
  const FAXPP_Event *event = FAXPP_get_current_event(wrapper_.parser);

  if(eventType_ == ATTRIBUTE || eventType_ == NAMESPACE) {
    FAXPP_Attribute *attr = &event->attrs[attrIndex_];
    setLocation(pYYLOC, &attr->value);
  }
  else {
    setLocation(pYYLOC, event);
  }
}

void FAXPPXSLT2Lexer::setLocation(YYLTYPE* pYYLOC, const FAXPP_Event *event)
{
  pYYLOC->first_line = m_lineno = event->line;
  pYYLOC->first_column = m_columnno = event->column + 1;
}

void FAXPPXSLT2Lexer::setLocation(YYLTYPE* pYYLOC, const FAXPP_Attribute *attr)
{
  pYYLOC->first_line = m_lineno = attr->line;
  pYYLOC->first_column = m_columnno = attr->column + 1;
}

void FAXPPXSLT2Lexer::setLocation(YYLTYPE* pYYLOC, const FAXPP_AttrValue *attrval)
{
  pYYLOC->first_line = m_lineno = attrval->line;
  pYYLOC->first_column = m_columnno = attrval->column + 1;
}

void FAXPPXSLT2Lexer::setErrorLocation(YYLTYPE* pYYLOC)
{
  pYYLOC->first_line = m_lineno = FAXPP_get_error_line(wrapper_.parser);
  pYYLOC->first_column = m_columnno = FAXPP_get_error_column(wrapper_.parser) + 1;
}

#endif // HAVE_FAXPP

////////////////////////////////////////////////////////////////////////////////////////////////////

XercesXSLT2Lexer::XercesXSLT2Lexer(DynamicContext *context, const InputSource &srcToUse, XQilla::Language lang)
  : XSLT2Lexer(context, srcToUse, lang),
    grammarResolver_(0),
    scanner_(0),
    currentEvent_(0),
    firstEvent_(0),
    lastEvent_(0)
{
  try {
    grammarResolver_ = new GrammarResolver(0);
    scanner_ = new DGXMLScanner(0, grammarResolver_);
    scanner_->setURIStringPool(grammarResolver_->getStringPool());
    scanner_->setDocHandler(this);
    scanner_->setEntityHandler(this);
    scanner_->setDoNamespaces(true);

    scanner_->scanFirst(srcToUse, pptoken_);
  }
  catch(const SAXException& toCatch) {
    ostringstream oss;
    oss << "Error parsing document: " << UTF8(toCatch.getMessage());
    error(oss.str().c_str());
  }
  catch(const XMLException& toCatch) {
    ostringstream oss;
    oss << "Error parsing document: " << UTF8(toCatch.getMessage());
    error(oss.str().c_str());
  }
}

XercesXSLT2Lexer::~XercesXSLT2Lexer()
{
  delete currentEvent_;

  CachedEvent *tmp;
  while(firstEvent_) {
    tmp = firstEvent_;
    firstEvent_ = firstEvent_->next;
    delete tmp;
  }

  delete scanner_;
  delete grammarResolver_;
}

void XercesXSLT2Lexer::nextEvent(YYLTYPE* pYYLOC)
{
  if(currentEvent_) {
    if(currentEvent_->type == END_DOCUMENT)
      return;

    delete currentEvent_;
    currentEvent_ = 0;
  }

  while(currentEvent_ == 0) {
    if(firstEvent_ != 0) {
      currentEvent_ = firstEvent_;
      firstEvent_ = firstEvent_->next;
      if(firstEvent_ == 0) lastEvent_ = 0;
      currentEvent_->next = 0;
    }
    else {
      try {
        scanner_->scanNext(pptoken_);
      }
      catch(const SAXException& toCatch) {
        ostringstream oss;
        oss << "Error parsing document: " << UTF8(toCatch.getMessage());
        error(oss.str().c_str());
      }
      catch(const XMLException& toCatch) {
        ostringstream oss;
        oss << "Error parsing document: " << UTF8(toCatch.getMessage());
        error(oss.str().c_str());
      }
    }
  }
}

XSLT2Lexer::EventType XercesXSLT2Lexer::getEventType()
{
  return currentEvent_->type;
}

void XercesXSLT2Lexer::getEventName(const XMLCh *&prefix, const XMLCh *&uri, const XMLCh *&localname)
{
  prefix = currentEvent_->prefix;
  uri = currentEvent_->uri;
  localname = currentEvent_->localname;
}

void XercesXSLT2Lexer::getEventValue(const XMLCh *&value, unsigned int &length, std::vector<XQLexer::ValueOffset> &offsets)
{
  offsets.clear();
  value = currentEvent_->value.getRawBuffer();
  length = (unsigned int) currentEvent_->value.getLen();
}

void XercesXSLT2Lexer::getEventLocation(YYLTYPE* pYYLOC)
{
  pYYLOC->first_line = m_lineno = (int) currentEvent_->line;
  pYYLOC->first_column = m_columnno = (int) currentEvent_->column;
}

void XercesXSLT2Lexer::getValueLocation(YYLTYPE* pYYLOC)
{
  pYYLOC->first_line = m_lineno = (int) currentEvent_->line;
  pYYLOC->first_column = m_columnno = (int) currentEvent_->column;
}

InputSource* XercesXSLT2Lexer::resolveEntity(XMLResourceIdentifier* resourceIdentifier)
{
  if(context_->getXMLEntityResolver()) 
    return context_->getXMLEntityResolver()->resolveEntity(resourceIdentifier);
  return 0;
}

void XercesXSLT2Lexer::startDocument()
{
  lastEvent_ = new CachedEvent(START_DOCUMENT, scanner_->getLocator(), lastEvent_);
  if(firstEvent_ == 0) firstEvent_ = lastEvent_;
}

void XercesXSLT2Lexer::endDocument()
{
  lastEvent_ = new CachedEvent(END_DOCUMENT, scanner_->getLocator(), lastEvent_);
  if(firstEvent_ == 0) firstEvent_ = lastEvent_;
}

#if _XERCES_VERSION >= 30000
void XercesXSLT2Lexer::startElement(const XMLElementDecl& elemDecl, const unsigned int urlId,
                                    const XMLCh* const elemPrefix,
                                    const RefVectorOf<XMLAttr>& attrList,
                                    const XMLSize_t attrCount, const bool isEmpty, const bool isRoot)
#else
void XercesXSLT2Lexer::startElement(const XMLElementDecl& elemDecl, const unsigned int urlId,
                                    const XMLCh* const elemPrefix,
                                    const RefVectorOf<XMLAttr>& attrList,
                                    const unsigned int attrCount, const bool isEmpty, const bool isRoot)
#endif
{
  lastEvent_ = new CachedEvent(mm_->getPooledString(elemPrefix),
                               mm_->getPooledString(scanner_->getURIText(urlId)),
                               mm_->getPooledString(elemDecl.getBaseName()),
                               scanner_->getLocator(), lastEvent_);
  if(firstEvent_ == 0) firstEvent_ = lastEvent_;

  unsigned int i;
  for(i = 0; i < attrCount; ++i) {
    const XMLAttr *attr = attrList.elementAt(i);

    if(attr->getURIId() == scanner_->getXMLNSNamespaceId()) {
      lastEvent_ = new CachedEvent(NAMESPACE,
                                   mm_->getPooledString(attr->getPrefix()),
                                   mm_->getPooledString(scanner_->getURIText(attr->getURIId())),
                                   mm_->getPooledString(attr->getName()),
                                   attr->getValue(),
                                   scanner_->getLocator(), lastEvent_);
      if(firstEvent_ == 0) firstEvent_ = lastEvent_;
    }
    else if(XPath2Utils::equals(attr->getName(), XMLUni::fgXMLNSString)) {
      lastEvent_ = new CachedEvent(NAMESPACE,
                                   mm_->getPooledString(attr->getPrefix()),
                                   XMLUni::fgXMLNSURIName,
                                   mm_->getPooledString(attr->getName()),
                                   attr->getValue(),
                                   scanner_->getLocator(), lastEvent_);
      if(firstEvent_ == 0) firstEvent_ = lastEvent_;
    }

    // Recoginze [xsl:]xpath-default-namespace as a type NAMESPACE too
    if(XPath2Utils::equals(scanner_->getURIText(urlId), XSLT_URI)) {
      if(attr->getURIId() == 0 && XPath2Utils::equals(attr->getName(), XPATH_DEFAULT_NAMESPACE_NAME)) {
        lastEvent_ = new CachedEvent(NAMESPACE,
                                     mm_->getPooledString(attr->getPrefix()),
                                     mm_->getPooledString(scanner_->getURIText(attr->getURIId())),
                                     mm_->getPooledString(attr->getName()),
                                     attr->getValue(),
                                     scanner_->getLocator(), lastEvent_);
        if(firstEvent_ == 0) firstEvent_ = lastEvent_;
      }
    }
    else {
      if(XPath2Utils::equals(attr->getName(), XPATH_DEFAULT_NAMESPACE_NAME) &&
         XPath2Utils::equals(scanner_->getURIText(attr->getURIId()), XSLT_URI)) {
        lastEvent_ = new CachedEvent(NAMESPACE,
                                     mm_->getPooledString(attr->getPrefix()),
                                     mm_->getPooledString(scanner_->getURIText(attr->getURIId())),
                                     mm_->getPooledString(attr->getName()),
                                     attr->getValue(),
                                     scanner_->getLocator(), lastEvent_);
        if(firstEvent_ == 0) firstEvent_ = lastEvent_;
      }
    }
  }

  for(i = 0; i < attrCount; ++i) {
    const XMLAttr *attr = attrList.elementAt(i);

    if(attr->getURIId() == scanner_->getXMLNSNamespaceId() ||
       XPath2Utils::equals(attr->getName(), XMLUni::fgXMLNSString))
      continue;

    if(XPath2Utils::equals(scanner_->getURIText(urlId), XSLT_URI)) {
      if(attr->getURIId() == 0 && XPath2Utils::equals(attr->getName(), XPATH_DEFAULT_NAMESPACE_NAME))
        continue;
    }
    else {
      if(XPath2Utils::equals(attr->getName(), XPATH_DEFAULT_NAMESPACE_NAME) &&
         XPath2Utils::equals(scanner_->getURIText(attr->getURIId()), XSLT_URI))
        continue;
    }

    lastEvent_ = new CachedEvent(ATTRIBUTE,
                                 mm_->getPooledString(attr->getPrefix()),
                                 mm_->getPooledString(scanner_->getURIText(attr->getURIId())),
                                 mm_->getPooledString(attr->getName()),
                                 attr->getValue(),
                                 scanner_->getLocator(), lastEvent_);
    if(firstEvent_ == 0) firstEvent_ = lastEvent_;
  }

  if(isEmpty) {
    lastEvent_ = new CachedEvent(END_ELEMENT, scanner_->getLocator(), lastEvent_);
    if(firstEvent_ == 0) firstEvent_ = lastEvent_;
  }
}

#if _XERCES_VERSION >= 30000
void XercesXSLT2Lexer::docCharacters(const XMLCh* const chars, const XMLSize_t length, const bool cdataSection)
#else
void XercesXSLT2Lexer::docCharacters(const XMLCh* const chars, const unsigned int length, const bool cdataSection)
#endif
{
  lastEvent_ = new CachedEvent(chars, length, scanner_->getLocator(), lastEvent_);
  if(firstEvent_ == 0) firstEvent_ = lastEvent_;
}

void XercesXSLT2Lexer::endElement(const XMLElementDecl& elemDecl, const unsigned int urlId,
                                  const bool isRoot, const XMLCh* const elemPrefix)
{
  lastEvent_ = new CachedEvent(END_ELEMENT, scanner_->getLocator(), lastEvent_);
  if(firstEvent_ == 0) firstEvent_ = lastEvent_;
}

#define emptyToNull(p) ((p && *p) ? (p) : 0)

XercesXSLT2Lexer::CachedEvent::CachedEvent(EventType t, const Locator *locator, CachedEvent *p)
  : type(t),
    prefix(0),
    uri(0),
    localname(0),
    line(locator->getLineNumber()),
    column(locator->getColumnNumber()),
    prev(p),
    next(0)
{
  if(p) p->next = this;
}

XercesXSLT2Lexer::CachedEvent::CachedEvent(const XMLCh *p, const XMLCh *u, const XMLCh *l, const Locator *locator, CachedEvent *pv)
  : type(START_ELEMENT),
    prefix(emptyToNull(p)),
    uri(emptyToNull(u)),
    localname(l),
    line(locator->getLineNumber()),
    column(locator->getColumnNumber()),
    prev(pv),
    next(0)
{
  if(pv) pv->next = this;
}

XercesXSLT2Lexer::CachedEvent::CachedEvent(EventType t, const XMLCh *p, const XMLCh *u, const XMLCh *l, const XMLCh *v, const Locator *locator, CachedEvent *pv)
  : type(t),
    prefix(emptyToNull(p)),
    uri(emptyToNull(u)),
    localname(l),
    line(locator->getLineNumber()),
    column(locator->getColumnNumber()),
    prev(pv),
    next(0)
{
  value.set(v);
  if(pv) pv->next = this;
}

XercesXSLT2Lexer::CachedEvent::CachedEvent(const XMLCh *v, size_t length, const Locator *locator, CachedEvent *p)
  : type(TEXT),
    prefix(0),
    uri(0),
    localname(0),
    line(locator->getLineNumber()),
    column(locator->getColumnNumber()),
    prev(p),
    next(0)
{
  value.set(v, length);
  if(p) p->next = this;
}

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

#ifndef _XPATH2UTILS_HPP
#define _XPATH2UTILS_HPP

#include <xqilla/framework/XQillaExport.hpp>

#include <vector>

class XPath2MemoryManager;

#include <xercesc/util/XMLString.hpp>
#include <xercesc/framework/XMLBuffer.hpp>
#include <xercesc/dom/DOMNode.hpp>
#include <xercesc/dom/DOMDocument.hpp>

XERCES_CPP_NAMESPACE_BEGIN
class InputSource;
class BinInputStream;
XERCES_CPP_NAMESPACE_END

/**class with methods that are used by v2 that are DOM related*/ 
class XQILLA_API XPath2Utils
{
public:

  /**
   * Escape a URI. Boolean says escape reserved chars.
   *
   * NOTE: The returned buffer is dynamically allocated and is the
   * responsibility of the caller to delete it when not longer needed. 
   */
  static const XMLCh* escapeURI(const XMLCh* const str, bool escapeRes, XPath2MemoryManager* memMgr);
  static bool isValidURI(const XMLCh* const str, XERCES_CPP_NAMESPACE_QUALIFIER MemoryManager* memMgr);
  
  static const XMLCh* concatStrings(const XMLCh* src1, const XMLCh srcChar, XPath2MemoryManager* memMgr);
  static const XMLCh* concatStrings(const XMLCh* src1, const XMLCh* src2, XPath2MemoryManager* memMgr);
  static const XMLCh* concatStrings(const XMLCh* src1 ,const XMLCh* src2, const XMLCh* src3, XPath2MemoryManager* memMgr);
   
  static const XMLCh* asStr(const XMLCh src, XPath2MemoryManager* memMgr);

  static void numToBuf(unsigned int n, XERCES_CPP_NAMESPACE_QUALIFIER XMLBuffer &buf);
  static void numToBuf(int n, XERCES_CPP_NAMESPACE_QUALIFIER XMLBuffer &buf);

  static bool equals(const XMLCh *const str1, const XMLCh *const str2);
  static int compare(const XMLCh *str1, const XMLCh *str2);

  static int intStrlen(const XMLCh *src);
  static unsigned int uintStrlen(const XMLCh *src);
  /**
   * Return the specified substring.
   */
  static const XMLCh* subString( const XMLCh* srcStr, unsigned int offset, unsigned int count, XPath2MemoryManager* memMgr);

  static const XMLCh* deleteData( const XMLCh* const target, unsigned int offset, unsigned int count, XPath2MemoryManager* memMgr );

  static const XMLCh* toLower(const XMLCh* const target, XPath2MemoryManager* memMgr);

  static const XMLCh* toUpper(const XMLCh* const target, XPath2MemoryManager* memMgr);

  static const XMLCh* toCollapsedWS(const XMLCh* const target, XPath2MemoryManager* memMgr);

  static const XMLCh* normalizeEOL(const XMLCh* const src, XPath2MemoryManager* memMgr);

  /*** 
   * Extract the individual values from a list of whitespace separated 
   * values (values) and puts them into valList. 
   */
  static std::vector<const XMLCh*> getVal(const XMLCh* values, XPath2MemoryManager* memMgr);

  /* Determine whether or not an XMLCh* is contained within a vector of XMLCh's */
  static bool containsString(std::vector<const XMLCh*> values, const XMLCh* val); 

  /** Gets the owner document for the given node */
  static const XERCES_CPP_NAMESPACE_QUALIFIER DOMDocument *getOwnerDoc(const XERCES_CPP_NAMESPACE_QUALIFIER DOMNode *node);

  static bool readSource(const XERCES_CPP_NAMESPACE_QUALIFIER InputSource &src, XERCES_CPP_NAMESPACE_QUALIFIER MemoryManager *mm,
                         XERCES_CPP_NAMESPACE_QUALIFIER XMLBuffer &result, bool sniffEncoding = false);
  static void readSource(XERCES_CPP_NAMESPACE_QUALIFIER BinInputStream *stream, XERCES_CPP_NAMESPACE_QUALIFIER MemoryManager *mm,
                         XERCES_CPP_NAMESPACE_QUALIFIER XMLBuffer &result, const XMLCh *encoding = 0, bool sniffEncoding = false);

};

inline bool XPath2Utils::equals(const XMLCh *const str1, const XMLCh *const str2) {
  if(str1 == str2) return true;

  if(str1 == 0) return *str2 == 0; // str2 == 0 is handled by the first line
  if(str2 == 0) return *str1 == 0; // str1 == 0 is handled by the first line

  register const XMLCh* psz1 = str1;
  register const XMLCh* psz2 = str2;

  while(*psz1 == *psz2) {
    // If either has ended, then they both ended, so equal
    if(*psz1 == 0)
      return true;
                                                                                                                                                              
    // Move upwards for the next round
    ++psz1;
    ++psz2;
  }
  return false;
}

inline int XPath2Utils::compare(const XMLCh *str1, const XMLCh *str2) {
  if(str1 == str2) return 0;

  if(str1 == 0) return -*str2; // str2 == 0 is handled by the first line
  if(str2 == 0) return *str1; // str1 == 0 is handled by the first line

  register int cmp;
  while((cmp = *str1 - *str2) == 0) {
    // If either has ended, then they both ended, so equal
    if(*str1 == 0) break;

    // Move upwards for the next round
    ++str1;
    ++str2;
  }
  return cmp;
}

inline const XERCES_CPP_NAMESPACE_QUALIFIER DOMDocument *XPath2Utils::getOwnerDoc(const XERCES_CPP_NAMESPACE_QUALIFIER DOMNode *node) {
  if(node->getNodeType() == XERCES_CPP_NAMESPACE_QUALIFIER DOMNode::DOCUMENT_NODE) {
    return (XERCES_CPP_NAMESPACE_QUALIFIER DOMDocument*)node;
  }
  else {
    return node->getOwnerDocument();
  }
}

inline void XPath2Utils::numToBuf(unsigned int n, XERCES_CPP_NAMESPACE_QUALIFIER XMLBuffer &buf)
{
  if(n >= 10) numToBuf(n / 10, buf);
  buf.append('0' + (n % 10));
}

inline void XPath2Utils::numToBuf(int n, XERCES_CPP_NAMESPACE_QUALIFIER XMLBuffer &buf)
{
  if(n < 0) {
    buf.append('-');
    numToBuf((unsigned int)-n, buf);
  } else {
    numToBuf((unsigned int)n, buf);
  }
}

inline int XPath2Utils::intStrlen(const XMLCh *src)
{
  return (int) XERCES_CPP_NAMESPACE_QUALIFIER XMLString::stringLen(src);        
}

inline unsigned int XPath2Utils::uintStrlen(const XMLCh *src)
{
  return (unsigned int) XERCES_CPP_NAMESPACE_QUALIFIER XMLString::stringLen(src);        
}
#endif


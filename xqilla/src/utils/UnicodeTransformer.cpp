/*
 * Copyright (c) 2001-2008
 *     DecisionSoft Limited. All rights reserved.
 * Copyright (c) 2004-2008
 *     Oracle. All rights reserved.
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
 *
 * $Id$
 */

#include <xqilla/utils/UnicodeTransformer.hpp>
#include <xercesc/framework/XMLBuffer.hpp>
#include <xqilla/framework/XPath2MemoryManager.hpp>
#include <vector>
extern "C" {
#include <xqilla/utils/utf8proc.h>
}

#define NO_COMPOSITION 0xFFFFFFFF

using namespace std;
#if defined(XERCES_HAS_CPP_NAMESPACE)
XERCES_CPP_NAMESPACE_USE
#endif

static XMLCh* transformUTF16(const XMLCh *source, int options, XPath2MemoryManager *mm)
{
  // Count word number
  ssize_t srcSize;
  const XMLCh* pch = source;
  for (srcSize = 0; *pch++; srcSize++);

  // Create a piece of buffer for storaging result. The size is larger than the
  // source for decompressing/SpecialCasing/Casefolding and so on. The code
  // consider that a "double size" is enough for most of applied environment.
  // If the size is not enough, the buffer will be enlarge.
  ssize_t dstSize = srcSize * 2 + 1;
  if (dstSize < 1024) 
    dstSize = 1024;

  AutoDeallocate<XMLCh> buf(mm, dstSize * sizeof(int32_t));

  // Begin transform
  ssize_t result = utf16proc_decompose((const uint16_t *)source, srcSize, (int32_t*)buf.get(), dstSize, options);
  // do nothig if hits internal errors
  if (result < 0)
      return 0;
  // Create a new buffer if the buffer is not large enough
  if (result > dstSize) {
    // We have to create a bigger one.
    dstSize = result + 1;
    XMLCh* moreMm = (XMLCh*)mm->allocate(dstSize*sizeof(int32_t));
    buf.set(moreMm);

    result = utf16proc_decompose((const uint16_t *)source, srcSize, (int32_t*)buf.get(), dstSize, options);
    if (result < 0 || result > dstSize)
      return 0;
  }

  result = utf16proc_reencode((int32_t*)buf.get(), result, options);
  if (result < 0)
    return 0;  
      
  // Write the end and push it into stream
  XMLCh* output = (XMLCh *)buf.adopt();
  output[result] = XMLCh(0);
  
  return output; 
}

XMLCh* UnicodeTransformer::normalizeC(const XMLCh* source, XPath2MemoryManager* mm)
{
  return transformUTF16(source, UTF8PROC_COMPOSE, mm);
}

XMLCh* UnicodeTransformer::normalizeD(const XMLCh* source, XPath2MemoryManager* mm)
{
  return transformUTF16(source, UTF8PROC_DECOMPOSE, mm);
}

XMLCh* UnicodeTransformer::normalizeKC(const XMLCh* source, XPath2MemoryManager* mm)
{
  return transformUTF16(source, UTF8PROC_COMPAT | UTF8PROC_COMPOSE, mm);
}

XMLCh* UnicodeTransformer::normalizeKD(const XMLCh* source, XPath2MemoryManager* mm)
{
  return transformUTF16(source, UTF8PROC_COMPAT | UTF8PROC_DECOMPOSE, mm);
}

XMLCh* UnicodeTransformer::caseFold(const XMLCh* source, XPath2MemoryManager* mm)
{
  return transformUTF16(source, UTF8PROC_CASEFOLD, mm);
}

XMLCh* UnicodeTransformer::removeDiacritics(const XMLCh* source, XPath2MemoryManager* mm)
{
  return transformUTF16(source, UTF8PROC_DECOMPOSE | UTF8PROC_REMOVE_DIACRITIC, mm);
}

XMLCh* UnicodeTransformer::caseFoldAndRemoveDiacritics(const XMLCh* source, XPath2MemoryManager* mm)
{
  return transformUTF16(source, UTF8PROC_DECOMPOSE | UTF8PROC_CASEFOLD | UTF8PROC_REMOVE_DIACRITIC, mm);
}

XMLCh* UnicodeTransformer::lowerCase(const XMLCh* source, XPath2MemoryManager* mm)
{
  return transformUTF16(source, UTF8PROC_LOWERCASE, mm);
}

XMLCh* UnicodeTransformer::upperCase(const XMLCh* source, XPath2MemoryManager* mm)
{
  return transformUTF16(source, UTF8PROC_UPPERCASE, mm);
}

XMLCh* UnicodeTransformer::words(const XMLCh* source, XPath2MemoryManager* mm)
{
  return transformUTF16(source, UTF8PROC_WORDBOUND, mm);
}

XMLCh* UnicodeTransformer::sentences(const XMLCh* source, XPath2MemoryManager* mm)
{
  return transformUTF16(source, UTF8PROC_SENTENCEBOUND, mm);
}

XMLCh* UnicodeTransformer::wordsAndSentences(const XMLCh* source, XPath2MemoryManager *mm)
{
  AutoDeallocate<XMLCh> tmp(transformUTF16(source, UTF8PROC_STRIPCC, mm), mm);
  return transformUTF16(tmp.get(), UTF8PROC_WORDBOUND | UTF8PROC_SENTENCEBOUND, mm);
}

XMLCh* UnicodeTransformer::transform(const XMLCh* source, int options, XPath2MemoryManager *mm)
{
  return transformUTF16(source, options, mm);
}

bool UnicodeTransformer::isPunctuation(const XMLCh* source, int &codeSize)
{
  codeSize = 1;
  if((*source) >=0xD800 && (*source) <= 0xDBFF){
    codeSize = 2;
  }

  const utf8proc_property_t *property; 
  if(codeSize == 2){
    int32_t* uc = (int32_t *)source;
    property = utf8proc_get_property(*uc);
  } else {
    property = utf8proc_get_property(*source);
  }

  if(property->category == UTF8PROC_CATEGORY_PC
    || property->category == UTF8PROC_CATEGORY_PD
    || property->category == UTF8PROC_CATEGORY_PS
    || property->category == UTF8PROC_CATEGORY_PE
    || property->category == UTF8PROC_CATEGORY_PI
    || property->category == UTF8PROC_CATEGORY_PF
    || property->category == UTF8PROC_CATEGORY_PO)
    return true;

  return false;
}

bool UnicodeTransformer::isSpaceSeparator(const XMLCh* source)
{

  const utf8proc_property_t *property; 
  property = utf8proc_get_property(*source);

  if(property->category == UTF8PROC_CATEGORY_ZS)
    return true;

  return false;
}


bool UnicodeTransformer::isSpacesOrPunctuations(const XMLCh* start, const XMLCh* end)
{
  const XMLCh* tmp = start;
  int codeLen = 1;
  
  while(tmp != end){
    if( !isSpaceSeparator(tmp) || !isPunctuation(tmp, codeLen))
      return false;

    tmp = tmp + codeLen;;
  }

  return true;
}

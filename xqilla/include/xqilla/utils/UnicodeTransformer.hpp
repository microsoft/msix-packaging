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

#ifndef UNICODETRANSFORMER_HPP
#define UNICODETRANSFORMER_HPP

#include <xqilla/framework/XQillaExport.hpp>
#include <xercesc/framework/XMLBuffer.hpp>

class XPath2MemoryManager;
///
class XQILLA_API UnicodeTransformer
{
public:
  static XMLCh* normalizeC(const XMLCh* source, XPath2MemoryManager* mm);
  static XMLCh* normalizeD(const XMLCh* source, XPath2MemoryManager* mm);
  static XMLCh* normalizeKC(const XMLCh* source, XPath2MemoryManager* mm);
  static XMLCh* normalizeKD(const XMLCh* source, XPath2MemoryManager* mm);

  static XMLCh* removeDiacritics(const XMLCh* source, XPath2MemoryManager* mm);
  static XMLCh* caseFold(const XMLCh* source, XPath2MemoryManager* mm);
  static XMLCh* caseFoldAndRemoveDiacritics(const XMLCh* source, XPath2MemoryManager* mm);

  static XMLCh* lowerCase(const XMLCh* source, XPath2MemoryManager* mm);
  static XMLCh* upperCase(const XMLCh* source, XPath2MemoryManager* mm);

  static XMLCh* words(const XMLCh* source, XPath2MemoryManager* mm);
  static XMLCh* sentences(const XMLCh* source, XPath2MemoryManager* mm);
  static XMLCh* wordsAndSentences(const XMLCh* source, XPath2MemoryManager* mm);

  static XMLCh* transform(const XMLCh* source, int options, XPath2MemoryManager* mm);
 
  static bool isPunctuation(const XMLCh* source, int &codeSize);
  static bool isSpaceSeparator(const XMLCh* source);
  static bool isSpacesOrPunctuations(const XMLCh* start, const XMLCh* end);
};

#endif

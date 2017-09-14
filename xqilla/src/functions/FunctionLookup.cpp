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
#include <xqilla/functions/FunctionLookup.hpp>
#include <xqilla/context/DynamicContext.hpp>
#include <xqilla/functions/FuncFactory.hpp>
#include <xqilla/functions/ExternalFunction.hpp>
#include <xqilla/exceptions/StaticErrorException.hpp>
#include <xqilla/framework/XPath2MemoryManagerImpl.hpp>
#include <xqilla/utils/XPath2Utils.hpp>

XERCES_CPP_NAMESPACE_USE

#define SECONDARY_KEY(func)(((func)->getMinArgs() << 16) | (func)->getMaxArgs())

FunctionLookup *FunctionLookup::g_globalFunctionTable = 0;
XPath2MemoryManager *FunctionLookup::g_memMgr = 0;

FunctionLookup::FunctionLookup(XPath2MemoryManager* memMgr)
  : _funcTable(197, false, memMgr),
    _exFuncTable(7, false, memMgr)
{
}

FunctionLookup::~FunctionLookup()
{
}

void FunctionLookup::insertFunction(FuncFactory *func)
{
  // Use similar algorithm to lookup in order to detect overlaps
  // in argument numbers
  RefHash2KeysTableOfEnumerator<FuncFactory> iterator(const_cast<RefHash2KeysTableOf< FuncFactory >* >(&_funcTable));
  //
  // Walk the matches for the primary key (name) looking for overlaps:
  //   ensure func->max < min OR func->min > max
  //
  iterator.setPrimaryKey(func->getURINameHash());
  while(iterator.hasMoreElements())
    {
      FuncFactory *entry= &(iterator.nextElement());
      if ((func->getMaxArgs() < entry->getMinArgs()) ||
          (func->getMinArgs() > entry->getMaxArgs()))
        continue;
      // overlap -- throw exception
      XMLBuffer buf;
      buf.set(X("Multiple functions have the same expanded QName and number of arguments {"));
      buf.append(func->getURI());
      buf.append(X("}"));
      buf.append(func->getName());
      buf.append(X("#"));
      if(func->getMinArgs() >= entry->getMinArgs() &&
         func->getMinArgs() <= entry->getMaxArgs())
        XPath2Utils::numToBuf((unsigned int)func->getMinArgs(), buf);
      else
        XPath2Utils::numToBuf((unsigned int)entry->getMinArgs(), buf);
      buf.append(X(" [err:XQST0034]."));
      XQThrow2(StaticErrorException,X("FunctionLookup::insertFunction"), buf.getRawBuffer());
    }
  // Ok to add function
  size_t secondaryKey = SECONDARY_KEY(func);
  _funcTable.put((void*)func->getURINameHash(), (int)secondaryKey, func);
}

void FunctionLookup::removeFunction(FuncFactory *func)
{
  size_t secondaryKey = SECONDARY_KEY(func);
  _funcTable.removeKey((void*)func->getURINameHash(), (int)secondaryKey);
}

ASTNode* FunctionLookup::lookUpFunction(const XMLCh* URI, const XMLCh* fname,
                                        const VectorOfASTNodes &args, XPath2MemoryManager* memMgr) const
{
  if (this != g_globalFunctionTable) {
    ASTNode *ret = g_globalFunctionTable->lookUpFunction(
                                                         URI, fname, args, memMgr);
    if (ret)
      return ret;
  }

  RefHash2KeysTableOfEnumerator<FuncFactory> iterator(const_cast<RefHash2KeysTableOf< FuncFactory >* >(&_funcTable));
  //
  // Walk the matches for the primary key (name) looking for matches
  // based on allowable parameters
  //
  XMLBuffer key;
  key.set(fname);
  key.append(':');
  key.append(URI);
  iterator.setPrimaryKey(key.getRawBuffer());
  size_t nargs = args.size();
  while(iterator.hasMoreElements()) {
    FuncFactory *entry= &(iterator.nextElement());
    if (entry->getMinArgs() <= nargs &&
        entry->getMaxArgs() >= nargs)
      return entry->createInstance(args, memMgr);
  }
  return NULL;
}

//
// external functions are hashed on name+uri (primary) and numargs (secondary)
//
void FunctionLookup::insertExternalFunction(const ExternalFunction *func)
{
  size_t secondaryKey = func->getNumberOfArguments();
  _exFuncTable.put((void*)func->getURINameHash(), (int)secondaryKey, func);
}

const ExternalFunction *FunctionLookup::lookUpExternalFunction(
  const XMLCh* URI, const XMLCh* fname, size_t numArgs) const
{
  size_t secondaryKey = numArgs;
  XMLBuffer key;
  key.set(fname);
  key.append(':');
  key.append(URI);
  return _exFuncTable.get(key.getRawBuffer(), (int)secondaryKey);
}

void FunctionLookup::copyExternalFunctionsTo(DynamicContext *context) const
{
  RefHash2KeysTableOfEnumerator<const ExternalFunction> en(const_cast<RefHash2KeysTableOf<const ExternalFunction>*>(&_exFuncTable));
  while(en.hasMoreElements()) {
    context->addExternalFunction(&en.nextElement());
  }
}

/*
 * Global initialization and access
 */
static void initGlobalTable(FunctionLookup *t, MemoryManager *memMgr);

// static
XPath2MemoryManager* FunctionLookup::getMemoryManager()
{
  return g_memMgr;
}

// static
void FunctionLookup::initialize()
{
  /* global table is allocated via the memory manager, so
     no need to delete it at this time
     if (g_globalFunctionTable)
     delete g_globalFunctionTable;
  */
  if (g_memMgr)
    delete g_memMgr;
  g_memMgr = new XPath2MemoryManagerImpl();
  g_globalFunctionTable = new (g_memMgr) FunctionLookup(g_memMgr);
  initGlobalTable(g_globalFunctionTable, g_memMgr);
}

// static
void FunctionLookup::terminate()
{
  if (g_memMgr) {
    delete g_memMgr;
    g_memMgr = 0;
    g_globalFunctionTable = 0;
  }
  /* no need to delete this -- memMgr cleans up
     if (g_globalFunctionTable) {
     delete g_globalFunctionTable;
     g_globalFunctionTable = 0;
     }
  */
}

// static
void FunctionLookup::insertGlobalFunction(FuncFactory *func)
{
  g_globalFunctionTable->insertFunction(func);
}

// static
void FunctionLookup::insertGlobalExternalFunction(const ExternalFunction *func)
{
  g_globalFunctionTable->insertExternalFunction(func);
}

// static
ASTNode* FunctionLookup::lookUpGlobalFunction(
                                              const XMLCh* URI, const XMLCh* fname,
                                              const VectorOfASTNodes &args,
                                              XPath2MemoryManager* memMgr,
                                              const FunctionLookup *contextTable)
{
  if(contextTable)
    return contextTable->lookUpFunction(URI, fname, args, memMgr);
  return g_globalFunctionTable->lookUpFunction(URI, fname, args, memMgr);
}

// static
const ExternalFunction *FunctionLookup::lookUpGlobalExternalFunction(
  const XMLCh* URI, const XMLCh* fname, size_t numArgs,
  const FunctionLookup *contextTable)
{
  const ExternalFunction *ef =
    g_globalFunctionTable->lookUpExternalFunction(
      URI, fname, numArgs);
  if (!ef && contextTable)
    ef = contextTable->lookUpExternalFunction(
      URI, fname, numArgs);
  return ef;
}

#include <xqilla/functions/FuncFactoryTemplate.hpp>
#include <xqilla/functions/FunctionAbs.hpp>
#include <xqilla/functions/FunctionBaseURI.hpp>
#include <xqilla/functions/FunctionCeiling.hpp>
#include <xqilla/functions/FunctionCollection.hpp>
#include <xqilla/functions/FunctionCompare.hpp>
#include <xqilla/functions/FunctionConcat.hpp>
#include <xqilla/functions/FunctionContains.hpp>
#include <xqilla/functions/FunctionCount.hpp>
#include <xqilla/functions/FunctionCurrentDate.hpp>
#include <xqilla/functions/FunctionCurrentDateTime.hpp>
#include <xqilla/functions/FunctionCurrentTime.hpp>
#include <xqilla/functions/FunctionDateTime.hpp>
#include <xqilla/functions/FunctionDefaultCollation.hpp>
#include <xqilla/functions/FunctionDistinctValues.hpp>
#include <xqilla/functions/FunctionDoc.hpp>
#include <xqilla/functions/FunctionDocument.hpp>
#include <xqilla/functions/FunctionDocAvailable.hpp>
#include <xqilla/functions/FunctionDocumentURI.hpp>
#include <xqilla/functions/FunctionEmpty.hpp>
#include <xqilla/functions/FunctionEndsWith.hpp>
#include <xqilla/functions/FunctionError.hpp>
#include <xqilla/functions/FunctionQName.hpp>
#include <xqilla/functions/FunctionFloor.hpp>
#include <xqilla/functions/FunctionPrefixFromQName.hpp>
#include <xqilla/functions/FunctionLocalNameFromQName.hpp>
#include <xqilla/functions/FunctionNamespaceURIFromQName.hpp>
#include <xqilla/functions/FunctionId.hpp>
#include <xqilla/functions/FunctionIdref.hpp>
#include <xqilla/functions/FunctionImplicitTimezone.hpp>
#include <xqilla/functions/FunctionLang.hpp>
#include <xqilla/functions/FunctionLast.hpp>
#include <xqilla/functions/FunctionLocalname.hpp>
#include <xqilla/functions/FunctionLowerCase.hpp>
#include <xqilla/functions/FunctionMatches.hpp>
#include <xqilla/functions/FunctionName.hpp>
#include <xqilla/functions/FunctionNamespaceUri.hpp>
#include <xqilla/functions/FunctionNilled.hpp>
#include <xqilla/functions/FunctionNodeName.hpp>
#include <xqilla/functions/FunctionNormalizeSpace.hpp>
#include <xqilla/functions/FunctionNormalizeUnicode.hpp>
#include <xqilla/functions/FunctionNot.hpp>
#include <xqilla/functions/FunctionNumber.hpp>
#include <xqilla/functions/FunctionPosition.hpp>
#include <xqilla/functions/FunctionReplace.hpp>
#include <xqilla/functions/FunctionResolveURI.hpp>
#include <xqilla/functions/FunctionRoot.hpp>
#include <xqilla/functions/FunctionRound.hpp>
#include <xqilla/functions/FunctionRoundHalfToEven.hpp>
#include <xqilla/functions/FunctionStartsWith.hpp>
#include <xqilla/functions/FunctionStaticBaseURI.hpp>
#include <xqilla/functions/FunctionString.hpp>
#include <xqilla/functions/FunctionStringLength.hpp>
#include <xqilla/functions/FunctionStringToCodepoints.hpp>
#include <xqilla/functions/FunctionCodepointsToString.hpp>
#include <xqilla/functions/FunctionSubstring.hpp>
#include <xqilla/functions/FunctionTokenize.hpp>
#include <xqilla/functions/FunctionTrace.hpp>
#include <xqilla/functions/FunctionUnordered.hpp>
#include <xqilla/functions/FunctionUpperCase.hpp>
#include <xqilla/functions/FunctionYearsFromDuration.hpp>
#include <xqilla/functions/FunctionMonthsFromDuration.hpp>
#include <xqilla/functions/FunctionDaysFromDuration.hpp>
#include <xqilla/functions/FunctionHoursFromDuration.hpp>
#include <xqilla/functions/FunctionMinutesFromDuration.hpp>
#include <xqilla/functions/FunctionSecondsFromDuration.hpp>
#include <xqilla/functions/FunctionYearFromDateTime.hpp>
#include <xqilla/functions/FunctionMonthFromDateTime.hpp>
#include <xqilla/functions/FunctionDayFromDateTime.hpp>
#include <xqilla/functions/FunctionHoursFromDateTime.hpp>
#include <xqilla/functions/FunctionMinutesFromDateTime.hpp>
#include <xqilla/functions/FunctionSecondsFromDateTime.hpp>
#include <xqilla/functions/FunctionTimezoneFromDateTime.hpp>
#include <xqilla/functions/FunctionYearFromDate.hpp>
#include <xqilla/functions/FunctionMonthFromDate.hpp>
#include <xqilla/functions/FunctionDayFromDate.hpp>
#include <xqilla/functions/FunctionTimezoneFromDate.hpp>
#include <xqilla/functions/FunctionHoursFromTime.hpp>
#include <xqilla/functions/FunctionMinutesFromTime.hpp>
#include <xqilla/functions/FunctionSecondsFromTime.hpp>
#include <xqilla/functions/FunctionTimezoneFromTime.hpp>
#include <xqilla/functions/FunctionAdjustDateTimeToTimezone.hpp>
#include <xqilla/functions/FunctionAdjustDateToTimezone.hpp>
#include <xqilla/functions/FunctionAdjustTimeToTimezone.hpp>
#include <xqilla/functions/FunctionHead.hpp>
#include <xqilla/functions/FunctionTail.hpp>
// Updates
#include <xqilla/update/FunctionPut.hpp>
// XSLT 2.0 functions
#include <xqilla/functions/FunctionUnparsedText.hpp>
#include <xqilla/functions/FunctionRegexGroup.hpp>
// XQilla extension functions
#include <xqilla/functions/FunctionParseXML.hpp>
#include <xqilla/functions/FunctionParseJSON.hpp>
#include <xqilla/functions/FunctionSerializeJSON.hpp>
#include <xqilla/functions/FunctionParseHTML.hpp>
#include <xqilla/functions/FunctionTime.hpp>
#include <xqilla/functions/FunctionAnalyzeString.hpp>
#include <xqilla/functions/FunctionCaseFold.hpp>
#include <xqilla/functions/FunctionWords.hpp>
#include <xqilla/functions/FunctionSentences.hpp>
#include <xqilla/functions/FunctionExplain.hpp>
// Higher Order Functions extension functions
#include <xqilla/functions/FunctionFunctionArity.hpp>
#include <xqilla/functions/FunctionFunctionName.hpp>
//exslt math functions
#include <xqilla/functions/FunctionSqrt.hpp>
#include <xqilla/functions/FunctionSin.hpp>
#include <xqilla/functions/FunctionCos.hpp>
#include <xqilla/functions/FunctionTan.hpp>
#include <xqilla/functions/FunctionAcos.hpp>
#include <xqilla/functions/FunctionAsin.hpp>
#include <xqilla/functions/FunctionAtan.hpp>
#include <xqilla/functions/FunctionLog.hpp>
#include <xqilla/functions/FunctionExp.hpp>
#include <xqilla/functions/FunctionPower.hpp>

void FunctionLookup::insertUpdateFunctions(XPath2MemoryManager *memMgr)
{
  // Update functions
  //   fn:put
  insertFunction(new (memMgr) FuncFactoryTemplate<FunctionPut>(memMgr));
}

static void initGlobalTable(FunctionLookup *t, MemoryManager *memMgr)
{
  // From the XPath2 Function & Operators list

  // Accessors:
  //   fn:node-name
  t->insertFunction(new (memMgr) FuncFactoryTemplate<FunctionNodeName>(memMgr));
  //   fn:nilled
  t->insertFunction(new (memMgr) FuncFactoryTemplate<FunctionNilled>(memMgr));
  //   fn:string
  t->insertFunction(new (memMgr) FuncFactoryTemplate<FunctionString>(memMgr));
  //   fn:base-uri
  t->insertFunction(new (memMgr) FuncFactoryTemplate<FunctionBaseURI>(memMgr));
  //   fn:document-uri
  t->insertFunction(new (memMgr) FuncFactoryTemplate<FunctionDocumentURI>(memMgr));


  // Debug Functions:
  //   fn:error
  t->insertFunction(new (memMgr) FuncFactoryTemplate<FunctionError>(memMgr));
  //   fn:trace
  t->insertFunction(new (memMgr) FuncFactoryTemplate<FunctionTrace>(memMgr));

  // Special Constructor Functions:
  //   fn:dateTime
  t->insertFunction(new (memMgr) FuncFactoryTemplate<FunctionDateTime>(memMgr));

  // Functions on numeric values:
  //   fn:abs
  t->insertFunction(new (memMgr) FuncFactoryTemplate<FunctionAbs>(memMgr));
  //   fn:ceiling
  t->insertFunction(new (memMgr) FuncFactoryTemplate<FunctionCeiling>(memMgr));
  //   fn:floor
  t->insertFunction(new (memMgr) FuncFactoryTemplate<FunctionFloor>(memMgr));
  //   fn:round
  t->insertFunction(new (memMgr) FuncFactoryTemplate<FunctionRound>(memMgr));
  //   fn:round-half-to-even
  t->insertFunction(new (memMgr) FuncFactoryTemplate<FunctionRoundHalfToEven>(memMgr));

  // Functions on strings
  //   fn:codepoints-to-string
  t->insertFunction(new (memMgr) FuncFactoryTemplate<FunctionCodepointsToString>(memMgr));
  //   fn:string-to-codepoints
  t->insertFunction(new (memMgr) FuncFactoryTemplate<FunctionStringToCodepoints>(memMgr));
  //   fn:compare
  t->insertFunction(new (memMgr) FuncFactoryTemplate<FunctionCompare>(memMgr));
  //   fn:concat
  t->insertFunction(new (memMgr) FuncFactoryTemplate<FunctionConcat>(memMgr));
  //   fn:substring
  t->insertFunction(new (memMgr) FuncFactoryTemplate<FunctionSubstring>(memMgr));
  //   fn:string-length
  t->insertFunction(new (memMgr) FuncFactoryTemplate<FunctionStringLength>(memMgr));
  //   fn:normalize-space
  t->insertFunction(new (memMgr) FuncFactoryTemplate<FunctionNormalizeSpace>(memMgr));
  //   fn:normalize-unicode
  t->insertFunction(new (memMgr) FuncFactoryTemplate<FunctionNormalizeUnicode>(memMgr));
  //   fn:upper-case
  t->insertFunction(new (memMgr) FuncFactoryTemplate<FunctionUpperCase>(memMgr));
  //   fn:lower-case
  t->insertFunction(new (memMgr) FuncFactoryTemplate<FunctionLowerCase>(memMgr));
  //   fn:contains
  t->insertFunction(new (memMgr) FuncFactoryTemplate<FunctionContains>(memMgr));
  //   fn:starts-with
  t->insertFunction(new (memMgr) FuncFactoryTemplate<FunctionStartsWith>(memMgr));
  //   fn:ends-with
  t->insertFunction(new (memMgr) FuncFactoryTemplate<FunctionEndsWith>(memMgr));

  //   fn:matches
  t->insertFunction(new (memMgr) FuncFactoryTemplate<FunctionMatches>(memMgr));
  //   fn:replace
  t->insertFunction(new (memMgr) FuncFactoryTemplate<FunctionReplace>(memMgr));
  //   fn:tokenize
  t->insertFunction(new (memMgr) FuncFactoryTemplate<FunctionTokenize>(memMgr));

  // Functions on boolean values
  //   fn:not
  t->insertFunction(new (memMgr) FuncFactoryTemplate<FunctionNot>(memMgr));

  // Functions on date values
  //   fn:years-from-duration
  t->insertFunction(new (memMgr) FuncFactoryTemplate<FunctionYearsFromDuration>(memMgr));
  //   fn:months-from-duration
  t->insertFunction(new (memMgr) FuncFactoryTemplate<FunctionMonthsFromDuration>(memMgr));
  //   fn:days-from-duration
  t->insertFunction(new (memMgr) FuncFactoryTemplate<FunctionDaysFromDuration>(memMgr));
  //   fn:hours-from-duration
  t->insertFunction(new (memMgr) FuncFactoryTemplate<FunctionHoursFromDuration>(memMgr));
  //   fn:minutes-from-duration
  t->insertFunction(new (memMgr) FuncFactoryTemplate<FunctionMinutesFromDuration>(memMgr));
  //   fn:seconds-from-duration
  t->insertFunction(new (memMgr) FuncFactoryTemplate<FunctionSecondsFromDuration>(memMgr));
  //   fn:year-from-dateTime
  t->insertFunction(new (memMgr) FuncFactoryTemplate<FunctionYearFromDateTime>(memMgr));
  //   fn:month-from-dateTime
  t->insertFunction(new (memMgr) FuncFactoryTemplate<FunctionMonthFromDateTime>(memMgr));
  //   fn:day-from-dateTime
  t->insertFunction(new (memMgr) FuncFactoryTemplate<FunctionDayFromDateTime>(memMgr));
  //   fn:hours-from-dateTime
  t->insertFunction(new (memMgr) FuncFactoryTemplate<FunctionHoursFromDateTime>(memMgr));
  //   fn:minutes-from-dateTime
  t->insertFunction(new (memMgr) FuncFactoryTemplate<FunctionMinutesFromDateTime>(memMgr));
  //   fn:seconds-from-dateTime
  t->insertFunction(new (memMgr) FuncFactoryTemplate<FunctionSecondsFromDateTime>(memMgr));
  //   fn:timezone-from-dateTime
  t->insertFunction(new (memMgr) FuncFactoryTemplate<FunctionTimezoneFromDateTime>(memMgr));
  //   fn:year-from-date
  t->insertFunction(new (memMgr) FuncFactoryTemplate<FunctionYearFromDate>(memMgr));
  //   fn:month-from-date
  t->insertFunction(new (memMgr) FuncFactoryTemplate<FunctionMonthFromDate>(memMgr));
  //   fn:day-from-date
  t->insertFunction(new (memMgr) FuncFactoryTemplate<FunctionDayFromDate>(memMgr));
  //   fn:timezone-from-date
  t->insertFunction(new (memMgr) FuncFactoryTemplate<FunctionTimezoneFromDate>(memMgr));
  //   fn:hours-from-time
  t->insertFunction(new (memMgr) FuncFactoryTemplate<FunctionHoursFromTime>(memMgr));
  //   fn:minutes-from-time
  t->insertFunction(new (memMgr) FuncFactoryTemplate<FunctionMinutesFromTime>(memMgr));
  //   fn:seconds-from-time
  t->insertFunction(new (memMgr) FuncFactoryTemplate<FunctionSecondsFromTime>(memMgr));
  //   fn:timezone-from-time
  t->insertFunction(new (memMgr) FuncFactoryTemplate<FunctionTimezoneFromTime>(memMgr));
  //   fn:adjust-dateTime-to-timezone
  t->insertFunction(new (memMgr) FuncFactoryTemplate<FunctionAdjustDateTimeToTimezone>(memMgr));
  //   fn:adjust-date-to-timezone
  t->insertFunction(new (memMgr) FuncFactoryTemplate<FunctionAdjustDateToTimezone>(memMgr));
  //   fn:adjust-time-to-timezone
  t->insertFunction(new (memMgr) FuncFactoryTemplate<FunctionAdjustTimeToTimezone>(memMgr));

  // Functions on QName values
  //   fn:QName
  t->insertFunction(new (memMgr) FuncFactoryTemplate<FunctionQName>(memMgr));
  //   fn:prefix-from-QName
  t->insertFunction(new (memMgr) FuncFactoryTemplate<FunctionPrefixFromQName>(memMgr));
  //   fn:local-name-from-QName
  t->insertFunction(new (memMgr) FuncFactoryTemplate<FunctionLocalNameFromQName>(memMgr));
  //   fn:namespace-uri-from-QName
  t->insertFunction(new (memMgr) FuncFactoryTemplate<FunctionNamespaceURIFromQName>(memMgr));

  // Functions on anyURI values
  //   fn:resolve-URI
  t->insertFunction(new (memMgr) FuncFactoryTemplate<FunctionResolveURI>(memMgr));

  // Functions on nodes
  //   fn:name
  t->insertFunction(new (memMgr) FuncFactoryTemplate<FunctionName>(memMgr));
  //   fn:local-name
  t->insertFunction(new (memMgr) FuncFactoryTemplate<FunctionLocalname>(memMgr));
  //   fn:namespace-uri
  t->insertFunction(new (memMgr) FuncFactoryTemplate<FunctionNamespaceUri>(memMgr));
  //   fn:number
  t->insertFunction(new (memMgr) FuncFactoryTemplate<FunctionNumber>(memMgr));
  //   fn:lang
  t->insertFunction(new (memMgr) FuncFactoryTemplate<FunctionLang>(memMgr));
  //   fn:root
  t->insertFunction(new (memMgr) FuncFactoryTemplate<FunctionRoot>(memMgr));

  // Functions on sequences
  //   fn:empty
  t->insertFunction(new (memMgr) FuncFactoryTemplate<FunctionEmpty>(memMgr));
  //   fn:distinct-values
  t->insertFunction(new (memMgr) FuncFactoryTemplate<FunctionDistinctValues>(memMgr));
  //   fn:unordered
  t->insertFunction(new (memMgr) FuncFactoryTemplate<FunctionUnordered>(memMgr));
  //   fn:head
  t->insertFunction(new (memMgr) FuncFactoryTemplate<FunctionHead>(memMgr));
  //   fn:tail
  t->insertFunction(new (memMgr) FuncFactoryTemplate<FunctionTail>(memMgr));
  
  //   fn:count
  t->insertFunction(new (memMgr) FuncFactoryTemplate<FunctionCount>(memMgr));

  //   fn:id
  t->insertFunction(new (memMgr) FuncFactoryTemplate<FunctionId>(memMgr));
  //   fn:idref
  t->insertFunction(new (memMgr) FuncFactoryTemplate<FunctionIdref>(memMgr));
  //   fn:doc
  t->insertFunction(new (memMgr) FuncFactoryTemplate<FunctionDoc>(memMgr));
  //   fn:doc-available
  t->insertFunction(new (memMgr) FuncFactoryTemplate<FunctionDocAvailable>(memMgr));
  //   fn:collection
  t->insertFunction(new (memMgr) FuncFactoryTemplate<FunctionCollection>(memMgr));
  
  // Context functions
  //   fn:position
  t->insertFunction(new (memMgr) FuncFactoryTemplate<FunctionPosition>(memMgr));
  //   fn:last
  t->insertFunction(new (memMgr) FuncFactoryTemplate<FunctionLast>(memMgr));
  //   fn:current-dateTime
  t->insertFunction(new (memMgr) FuncFactoryTemplate<FunctionCurrentDateTime>(memMgr));
  //   fn:current-date
  t->insertFunction(new (memMgr) FuncFactoryTemplate<FunctionCurrentDate>(memMgr));
  //   fn:current-time
  t->insertFunction(new (memMgr) FuncFactoryTemplate<FunctionCurrentTime>(memMgr));
  //   fn:implicit-timezone
  t->insertFunction(new (memMgr) FuncFactoryTemplate<FunctionImplicitTimezone>(memMgr));
  //   fn:default-collation
  t->insertFunction(new (memMgr) FuncFactoryTemplate<FunctionDefaultCollation>(memMgr));
  //   fn:static-base-uri
  t->insertFunction(new (memMgr) FuncFactoryTemplate<FunctionStaticBaseURI>(memMgr));

  // XSLT 2.0 functions
  t->insertFunction(new (memMgr) FuncFactoryTemplate<FunctionUnparsedText>(memMgr));
  t->insertFunction(new (memMgr) FuncFactoryTemplate<FunctionRegexGroup>(memMgr));
  t->insertFunction(new (memMgr) FuncFactoryTemplate<FunctionDocument>(memMgr));

  // XQilla extension functions
  t->insertFunction(new (memMgr) FuncFactoryTemplate<FunctionParseXML>(memMgr));
  t->insertFunction(new (memMgr) FuncFactoryTemplate<FunctionParseJSON>(memMgr));
  t->insertFunction(new (memMgr) FuncFactoryTemplate<FunctionSerializeJSON>(memMgr));
  t->insertFunction(new (memMgr) FuncFactoryTemplate<FunctionTime>(memMgr));
  t->insertFunction(new (memMgr) FuncFactoryTemplate<FunctionAnalyzeString>(memMgr));
  t->insertFunction(new (memMgr) FuncFactoryTemplate<FunctionCaseFold>(memMgr));
  t->insertFunction(new (memMgr) FuncFactoryTemplate<FunctionWords>(memMgr));
  t->insertFunction(new (memMgr) FuncFactoryTemplate<FunctionSentences>(memMgr));
  t->insertFunction(new (memMgr) FuncFactoryTemplate<FunctionExplain>(memMgr));
  t->insertFunction(new (memMgr) FuncFactoryTemplate<FunctionDrop>(memMgr));

#ifdef HAVE_LIBTIDY
  t->insertFunction(new (memMgr) FuncFactoryTemplate<FunctionParseHTML>(memMgr));
#endif

  // Higher Order Functions extension functions
  t->insertFunction(new (memMgr) FuncFactoryTemplate<FunctionFunctionArity>(memMgr));
  t->insertFunction(new (memMgr) FuncFactoryTemplate<FunctionFunctionName>(memMgr));

  //exslt math functions
  t->insertFunction(new (memMgr) FuncFactoryTemplate<FunctionAbs>(EXSLTMathFunction::XMLChFunctionURI, FunctionAbs::name, 1, 1, memMgr));
  t->insertFunction(new (memMgr) FuncFactoryTemplate<FunctionSqrt>(memMgr));
  t->insertFunction(new (memMgr) FuncFactoryTemplate<FunctionSin>(memMgr));
  t->insertFunction(new (memMgr) FuncFactoryTemplate<FunctionCos>(memMgr));
  t->insertFunction(new (memMgr) FuncFactoryTemplate<FunctionTan>(memMgr));
  t->insertFunction(new (memMgr) FuncFactoryTemplate<FunctionAcos>(memMgr));
  t->insertFunction(new (memMgr) FuncFactoryTemplate<FunctionAsin>(memMgr));
  t->insertFunction(new (memMgr) FuncFactoryTemplate<FunctionAtan>(memMgr));
  t->insertFunction(new (memMgr) FuncFactoryTemplate<FunctionLog>(memMgr));
  t->insertFunction(new (memMgr) FuncFactoryTemplate<FunctionExp>(memMgr));
  t->insertFunction(new (memMgr) FuncFactoryTemplate<FunctionPower>(memMgr));
}

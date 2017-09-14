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
#include <xqilla/functions/FunctionReplace.hpp>
#include <xqilla/items/ATStringOrDerived.hpp>
#include <xqilla/context/DynamicContext.hpp>
#include <xqilla/exceptions/FunctionException.hpp>
#include <xqilla/items/DatatypeFactory.hpp>
#include <xercesc/util/regx/RegularExpression.hpp>
#include <xercesc/util/XMLExceptMsgs.hpp>
#include <xercesc/util/ParseException.hpp>
#include <xercesc/validators/schema/SchemaSymbols.hpp>
#include <xercesc/util/XMLUni.hpp>
#include <xercesc/util/ParseException.hpp>


#if defined(XERCES_HAS_CPP_NAMESPACE)
XERCES_CPP_NAMESPACE_USE
#endif

const XMLCh FunctionReplace::name[] = {
  chLatin_r, chLatin_e, chLatin_p, 
  chLatin_l, chLatin_a, chLatin_c, 
  chLatin_e, chNull 
};
const unsigned int FunctionReplace::minArgs = 3;
const unsigned int FunctionReplace::maxArgs = 4;

/** 
 * fn:replace($input as xs:string?, $pattern as xs:string, $replacement as xs:string) as xs:string
 * fn:replace($input as xs:string?, $pattern as xs:string, $replacement as xs:string, $flags as xs:string) as xs:string
 */
  
FunctionReplace::FunctionReplace(const VectorOfASTNodes &args, XPath2MemoryManager* memMgr)
  : RegExpFunction(name, "($input as xs:string?, $pattern as xs:string, $replacement as xs:string, $flags as xs:string) as xs:string", args, memMgr)
{
}

ASTNode *FunctionReplace::staticTypingImpl(StaticContext *context)
{
  _src.clearExceptType();
  calculateSRCForArguments(context);

  //either there are 3 args, and regexp should be a constant,
  //or there is a flags argument as well, and it should also be a constant
  if(context && !regExp_ && _args[1]->isConstant() &&
    (getNumArgs() == 3 || (getNumArgs() == 4 && _args[3]->isConstant())))
  {
    XPath2MemoryManager* memMgr = context->getMemoryManager();

    AutoDelete<DynamicContext> dContext(context->createDynamicContext());
    dContext->setMemoryManager(memMgr);

    Item::Ptr patternItem = getParamNumber(2, dContext)->next(dContext);
    pattern_ = (XMLCh*) patternItem->asString(dContext);

    options_ = (XMLCh*) XMLUni::fgZeroLenString;
    if(getNumArgs() == 4)
    {
      Item::Ptr optsItem = getParamNumber(4, dContext)->next(dContext);
      options_ = (XMLCh*) optsItem->asString(dContext);
    	checkRegexpOpts(options_, "FunctionReplace::staticTypingImpl");
  	}

    // Always turn off head character optimisation, since it is broken
    XMLBuffer optionsBuf;
    optionsBuf.set(options_);
    optionsBuf.append(chLatin_H);

  	try
    {
      regExp_ = new (memMgr) RegularExpression(pattern_, optionsBuf.getRawBuffer(), memMgr);
    } catch (ParseException &e){
      processParseException(e, "FunctionReplace::staticTypingImpl", memMgr);
    }
  }

  return this;
}


Sequence FunctionReplace::createSequence(DynamicContext* context, int flags) const
{
  XPath2MemoryManager* memMgr = context->getMemoryManager();

  const XMLCh* input = XMLUni::fgZeroLenString;
  Item::Ptr inputString = getParamNumber(1,context)->next(context);
  if(inputString.notNull())
    input = inputString->asString(context);
  const XMLCh *replacement = getParamNumber(3,context)->next(context)->asString(context);

  const XMLCh *ptr;
  for(ptr = replacement; *ptr != chNull; ++ptr) {
    if(*ptr == chDollarSign) {
      ++ptr;
      
      //check that after the '$' is a digit
      if(!XMLString::isDigit(*ptr))
        XQThrow(FunctionException, X("FunctionReplace::createSequence"), X("Invalid replacement pattern - '$' without following digit [err:FORX0004]"));
    }
    else if(*ptr == chBackSlash) {
      ++ptr;
      
      //check that after the '\' is a '$' or '\'
      if(*ptr != chDollarSign && *ptr != chBackSlash) {
        XQThrow(FunctionException, X("FunctionReplace::createSequence"), X("Invalid replacement pattern - '\\' without following '$' or '\\' [err:FORX0004]"));
      }
    }
  }

  if(regExp_)
  {
    try
    {
      AutoDeallocate<const XMLCh> result(replace(input, regExp_, replacement, memMgr), memMgr);
      return Sequence(context->getItemFactory()->createString(result.get(), context), memMgr);
    } catch (RuntimeException &e){
      processRuntimeException(e, "FunctionReplace::createSequence");
    }
  }

  //get here if regExp has not been compiled before
  const XMLCh *pattern = getParamNumber(2,context)->next(context)->asString(context);

  const XMLCh *options = XMLUni::fgZeroLenString;
  if(getNumArgs()>3)
    options=getParamNumber(4,context)->next(context)->asString(context);
  
  //Check that the options are valid - throw an exception if not (can have s,m,i and x)
  //Note: Are allowed to duplicate the letters.
  checkRegexpOpts(options, "FunctionReplace::createSequence");

  // Now attempt to replace
  try {
    AutoDeallocate<const XMLCh> result(replace(input, pattern, replacement, options, memMgr), memMgr);
    return Sequence(context->getItemFactory()->createString(result.get(), context), memMgr);
  } catch (ParseException &e){ 
    processParseException(e, "FunctionReplace::createSequence", memMgr);
  } catch (RuntimeException &e){
    processRuntimeException(e, "FunctionReplace::createSequence");
  }

  // Never happens
  return Sequence(memMgr);
}

const XMLCh *FunctionReplace::replace(const XMLCh *input, const XMLCh *pattern, const XMLCh *replacement, const XMLCh *options, MemoryManager *mm)
{
  // Always turn off head character optimisation, since it is broken
  XMLBuffer optionsBuf;
  optionsBuf.set(options);
  optionsBuf.append(chLatin_H);

  //Now attempt to replace
  RegularExpression regEx(pattern, optionsBuf.getRawBuffer(), mm);
#ifdef HAVE_ALLMATCHES
  return regEx.replace(input, replacement, mm);
#else
  return regEx.replace(input, replacement);
#endif
}

const XMLCh *FunctionReplace::replace(const XMLCh *input, const RegularExpression* regExp,
                                      const XMLCh *replacement, MemoryManager *mm)
{
#ifdef HAVE_ALLMATCHES
  return regExp->replace(input, replacement, mm);
#else
  return regExp->replace(input, replacement);
#endif
}


void FunctionReplace::processRuntimeException(RuntimeException &e, const char* sourceMsg) const
{
    if(e.getCode()==XMLExcepts::Regex_RepPatMatchesZeroString)
      XQThrow(FunctionException, X(sourceMsg), X("The pattern matches the zero-length string [err:FORX0003]"));
    else if(e.getCode()==XMLExcepts::Regex_InvalidRepPattern)
      XQThrow(FunctionException, X(sourceMsg), X("Invalid replacement pattern [err:FORX0004]"));
    else 
      XQThrow(FunctionException, X(sourceMsg), e.getMessage());
}


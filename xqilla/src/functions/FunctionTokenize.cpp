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
#include <xqilla/functions/FunctionTokenize.hpp>
#include <xqilla/items/ATStringOrDerived.hpp>
#include <xqilla/context/DynamicContext.hpp>
#include <xqilla/exceptions/FunctionException.hpp>
#include <xqilla/items/DatatypeFactory.hpp>
#include <xqilla/utils/XPath2Utils.hpp>
#include <xercesc/util/RefArrayVectorOf.hpp>
#include <xercesc/util/regx/RegularExpression.hpp>
#include <xercesc/util/ParseException.hpp>
#include <xercesc/util/XMLUni.hpp>

#if defined(XERCES_HAS_CPP_NAMESPACE)
XERCES_CPP_NAMESPACE_USE
#endif

const XMLCh FunctionTokenize::name[] = {
  chLatin_t, chLatin_o, chLatin_k, 
  chLatin_e, chLatin_n, chLatin_i, 
  chLatin_z, chLatin_e, chNull 
};
const unsigned int FunctionTokenize::minArgs = 2;
const unsigned int FunctionTokenize::maxArgs = 3;

/** 
 * fn:tokenize($input as xs:string?, $pattern as xs:string) as xs:string*
 * fn:tokenize($input as xs:string?, $pattern as xs:string, $flags as xs:string) as xs:string*
 */
  
FunctionTokenize::FunctionTokenize(const VectorOfASTNodes &args, XPath2MemoryManager* memMgr)
  : RegExpFunction(name, "($input as xs:string?, $pattern as xs:string, $flags as xs:string) as xs:string*", args, memMgr)
    
{
}

ASTNode *FunctionTokenize::staticTypingImpl(StaticContext *context)
{
  _src.clearExceptType();
  calculateSRCForArguments(context);

  //either there are 2 args, and regexp should be a constant,
  //or there is a flags argument as well, and it should also be a constant
  if(context && !regExp_ && _args[1]->isConstant() &&
    (getNumArgs() == 2 || (getNumArgs() == 3 && _args[2]->isConstant())))
  {
    XPath2MemoryManager* memMgr = context->getMemoryManager();

    AutoDelete<DynamicContext> dContext(context->createDynamicContext());
    dContext->setMemoryManager(memMgr);

    Item::Ptr patternItem = getParamNumber(2, dContext)->next(dContext);
    pattern_ = (XMLCh*) patternItem->asString(dContext);

    options_ = (XMLCh*) XMLUni::fgZeroLenString;
    if(getNumArgs() == 3)
    {
      Item::Ptr optsItem = getParamNumber(3, dContext)->next(dContext);
      options_ = (XMLCh*) optsItem->asString(dContext);
    	checkRegexpOpts(options_, "FunctionTokenize::staticTypingImpl");
  	}

    // Always turn off head character optimisation, since it is broken
    XMLBuffer optionsBuf;
    optionsBuf.set(options_);
    optionsBuf.append(chLatin_H);

  	try
    {
      regExp_ = new (memMgr) RegularExpression(pattern_, optionsBuf.getRawBuffer(), memMgr);
      if(regExp_->matches(XMLUni::fgZeroLenString))
        XQThrow(FunctionException, X("FunctionTokenize::staticTypingImpl"), X("The pattern matches the zero-length string [err:FORX0003]"));
    } catch (ParseException &e){
      processParseException(e, "FunctionTokenize::staticTypingImpl", memMgr);
    }
  }

  return this;
}


Sequence FunctionTokenize::createSequence(DynamicContext* context, int flags) const
{
  XPath2MemoryManager* memMgr = context->getMemoryManager();

  // If the value of $operand1 is the empty sequence, the empty sequence is returned.
  Item::Ptr inputString = getParamNumber(1,context)->next(context);
  if(inputString.isNull())
    return Sequence(memMgr);

  const XMLCh *input=inputString->asString(context);
  // If the value of $operand1 is the zero-length string, the empty sequence is returned.
    if(XPath2Utils::equals(input, XMLUni::fgZeroLenString))
    return Sequence(memMgr);

  //Now attempt to tokenize
  AutoDelete<RefArrayVectorOf<XMLCh> > toks(0);

  if(regExp_)
  {
    try
    {
      toks.set(regExp_->tokenize(input));
    } catch (RuntimeException &e){
      processRuntimeException(e, "FunctionReplace::createSequence");
    }
  } else {

    const XMLCh *pattern=getParamNumber(2,context)->next(context)->asString(context);

    const XMLCh *options = XMLUni::fgZeroLenString;
    if(getNumArgs()>2)
      options=getParamNumber(3,context)->next(context)->asString(context);

    //Check that the options are valid - throw an exception if not (can have s,m,i and x)
    //Note: Are allowed to duplicate the letters.
    checkRegexpOpts(options, "FunctionTokenize::createSequence");

    try {
      // Always turn off head character optimisation, since it is broken
      XMLBuffer optionsBuf(1023, context->getMemoryManager());
      optionsBuf.set(options);
      optionsBuf.append(chLatin_H);

      RegularExpression regEx(pattern, optionsBuf.getRawBuffer(), memMgr);
      if(regEx.matches(XMLUni::fgZeroLenString))
        XQThrow(FunctionException, X("FunctionTokenize::createSequence"), X("The pattern matches the zero-length string [err:FORX0003]"));
      toks.set(regEx.tokenize(input));
    } catch (ParseException &e){
      processParseException(e, "FunctionTokenize::createSequence", memMgr);
    } catch (RuntimeException &e){
      processRuntimeException(e, "FunctionTokenize::createSequence");
    }
  }

  Sequence resultSeq(toks->size(),memMgr);

  for(unsigned int i = 0; i < toks->size(); ++i){
    resultSeq.addItem(context->getItemFactory()->createString(toks->elementAt(i), context));
  }

  return resultSeq;
}

void FunctionTokenize::processRuntimeException(RuntimeException &e, const char* sourceMsg) const
{
    if(e.getCode()==XMLExcepts::Regex_InvalidRepPattern)
      XQThrow(FunctionException, X(sourceMsg), X("Invalid replacement pattern [err:FORX0004]"));
    else
      XQThrow(FunctionException, X(sourceMsg), e.getMessage());
}

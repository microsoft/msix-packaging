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
#include <xqilla/ast/XQFunction.hpp>
#include <xqilla/context/DynamicContext.hpp>
#include <xqilla/schema/SequenceType.hpp>
#include <xqilla/exceptions/FunctionException.hpp>
#include <xqilla/items/ATDurationOrDerived.hpp>
#include <xqilla/items/ATStringOrDerived.hpp>
#include <xqilla/items/ATUntypedAtomic.hpp>
#include <xqilla/utils/XPath2Utils.hpp>
#include <xqilla/exceptions/StaticErrorException.hpp>

#include "../lexer/XQLexer.hpp"

#include <xercesc/util/XMLString.hpp>
#include <xercesc/util/XMLUniDefs.hpp>
#include <xercesc/util/XMLUni.hpp>
#include <xercesc/validators/schema/SchemaSymbols.hpp>
#include <xercesc/util/XMLStringTokenizer.hpp>

#include <string>
#include <map>

#include <assert.h>

#if defined(XERCES_HAS_CPP_NAMESPACE)
XERCES_CPP_NAMESPACE_USE
#endif

/* http://www.w3.org/2005/xpath-functions */
const XMLCh XQFunction::XMLChFunctionURI[] =
{
    chLatin_h,       chLatin_t,       chLatin_t, 
    chLatin_p,       chColon,         chForwardSlash, 
    chForwardSlash,  chLatin_w,       chLatin_w, 
    chLatin_w,       chPeriod,        chLatin_w,
    chDigit_3,       chPeriod,        chLatin_o, 
    chLatin_r,       chLatin_g,       chForwardSlash, 
    chDigit_2,       chDigit_0,       chDigit_0, 
    chDigit_5,       chForwardSlash,  chLatin_x, 
    chLatin_p,       chLatin_a,       chLatin_t, 
    chLatin_h,       chDash,          chLatin_f, 
    chLatin_u,       chLatin_n,       chLatin_c,
    chLatin_t,       chLatin_i,       chLatin_o, 
    chLatin_n,       chLatin_s,       chNull
};

XQFunction::XQFunction(const XMLCh *name, const char *signature, const VectorOfASTNodes &args,
                       XPath2MemoryManager* memMgr)
  : ASTNodeImpl(FUNCTION, memMgr),
    name_(name), uri_(XMLChFunctionURI),
    sigString_(signature),
    signature_(0),
    _args(args)
{
}


// bool XQFunction::isSubsetOf(const ASTNode *other) const
// {
//   if(other->getType() != getType()) return false;

//   const XQFunction *o = (const XQFunction*)other;
//   if(!XPath2Utils::equals(_fURI, o._fURI) ||
//      !XPath2Utils::equals(_fName, o._fName) ||
//      _args.size() != o._args.size())
//     return false;

//   VectorOfASTNodes::iterator i = _args.begin();
//   VectorOfASTNodes::iterator j = o._args.begin();
//   for(; i != _args.end(); ++i, ++j) {
//     // TBD non-deterministic functions - jpcs
//     if(!(*i)->isEqualTo(*j))
//       return false;
//   }

//   return true;
// }

ASTNode *XQFunction::staticResolution(StaticContext *context)
{
  resolveArguments(context);
  return this;
}

ASTNode *XQFunction::staticTypingImpl(StaticContext *context)
{
  _src.clearExceptType();
  calculateSRCForArguments(context);
  return this;
}

void XQFunction::parseSignature(StaticContext *context)
{
  if(sigString_ && !signature_) {
    XPath2MemoryManager *mm = context->getMemoryManager();
    XQLexer lexer(mm, _LANG_FUNCTION_SIGNATURE_, 0, 1, 1, mm->getPooledString(sigString_));
    XQParserArgs args(&lexer);
    XQParser::yyparse(&args);
    signature_ = args._signature;
    signature_->staticResolution(context);

    if(signature_->argSpecs) {
      // If the signature has too many arguments, remove some
      while(signature_->argSpecs->size() > _args.size()) {
        signature_->argSpecs->back()->release(mm);
        signature_->argSpecs->pop_back();
      }

      // If the signature has too few arguments, duplicate the last one.
      // (This is really only for fn:concat.)
      if(signature_->argSpecs->size() < _args.size()) {
        XMLBuffer buf(20);
        ArgumentSpec *argSpec = signature_->argSpecs->back();
        unsigned int last = _args.size() + 1 - signature_->argSpecs->size();
        for(unsigned int i = 2; i <= last; ++i) {
          ArgumentSpec *newArgSpec = new (mm) ArgumentSpec(argSpec, mm);
          buf.set(argSpec->getName());
          XPath2Utils::numToBuf(i, buf);
          newArgSpec->setName(mm->getPooledString(buf.getRawBuffer()));
          newArgSpec->setQName(0);
          signature_->argSpecs->push_back(newArgSpec);
        }

        buf.set(argSpec->getName());
        XPath2Utils::numToBuf(1, buf);
        argSpec->setName(mm->getPooledString(buf.getRawBuffer()));
        argSpec->setQName(0);
      }
    }
  }
}

void XQFunction::resolveArguments(StaticContext *context, bool numericFunction)
{
  parseSignature(context);

  size_t paramNumber = 0;
  for(VectorOfASTNodes::iterator i = _args.begin(); i != _args.end(); ++i, ++paramNumber) {
    if(paramNumber < signature_->argSpecs->size()) {
      *i = (*signature_->argSpecs)[paramNumber]->getType()->
        convertFunctionArg(*i, context, numericFunction, this);
    }
    *i = (*i)->staticResolution(context);
  }
}

void XQFunction::calculateSRCForArguments(StaticContext *context, bool checkTimezone)
{
  for(VectorOfASTNodes::iterator i = _args.begin(); i != _args.end(); ++i) {
    _src.add((*i)->getStaticAnalysis());

    if((*i)->getStaticAnalysis().isUpdating()) {
      XQThrow(StaticErrorException,X("XQFunction::staticTyping"),
              X("It is a static error for an argument to a function "
                "to be an updating expression [err:XUST0001]"));
    }

    if(checkTimezone && (*i)->isDateOrTimeAndHasNoTimezone(context))
      _src.implicitTimezoneUsed(true);
  }

  if(context) {
    if(signature_ && signature_->returnType) {
      bool isPrimitive;
      signature_->returnType->getStaticType(_src.getStaticType(), context, isPrimitive, this);
    }
    else {
      // Default type is item()*
      _src.getStaticType() = StaticType(StaticType::ITEM_TYPE, 0, StaticType::UNLIMITED);
    }
  }
}

Result XQFunction::getParamNumber(size_t number, DynamicContext* context, int flags) const
{
  assert(number > 0);
  assert(number <= getNumArgs());
  return _args[number - 1]->createResult(context, flags);
}

size_t XQFunction::getNumArgs() const
{
  return _args.size();
}

Result XQFunction::createResult(DynamicContext* context, int flags) const
{
  return createSequence(context);
}

Sequence XQFunction::createSequence(DynamicContext* context, int flags) const
{
  return Sequence(context->getMemoryManager());
}

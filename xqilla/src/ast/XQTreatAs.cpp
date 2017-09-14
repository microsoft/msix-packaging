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
#include <assert.h>
#include <sstream>

#include <xqilla/ast/XQTreatAs.hpp>
#include <xqilla/schema/SequenceType.hpp>
#include <xqilla/context/DynamicContext.hpp>
#include <xqilla/items/Item.hpp>
#include <xqilla/utils/XPath2Utils.hpp>
#include <xqilla/items/ATUntypedAtomic.hpp>
#include <xqilla/exceptions/XPath2TypeMatchException.hpp>
#include <xqilla/context/ContextHelpers.hpp>
#include <xqilla/context/MessageListener.hpp>

#include <xercesc/validators/schema/SchemaSymbols.hpp>

XERCES_CPP_NAMESPACE_USE
using namespace std;

const XMLCh XQTreatAs::err_XPDY0050[] = { 'e', 'r', 'r', ':', 'X', 'P', 'D', 'Y', '0', '0', '5', '0', 0 };
const XMLCh XQTreatAs::err_XPTY0004[] = { 'e', 'r', 'r', ':', 'X', 'P', 'T', 'Y', '0', '0', '0', '4', 0 };

XQTreatAs::XQTreatAs(ASTNode* expr, SequenceType* exprType, XPath2MemoryManager* memMgr, const XMLCh *errorCode)
  : ASTNodeImpl(TREAT_AS, memMgr),
    _expr(expr),
    _exprType(exprType),
    _errorCode(errorCode),
    _doTypeCheck(true),
    _doCardinalityCheck(true),
    _isExact(false)
{
  if(_errorCode == 0) _errorCode = err_XPTY0004;
}

XQTreatAs::XQTreatAs(ASTNode* expr, SequenceType *exprType, const XMLCh *errorCode, bool doTypeCheck, bool doCardinalityCheck,
                     const StaticType &treatType, bool isExact, XPath2MemoryManager* memMgr)
  : ASTNodeImpl(TREAT_AS, memMgr),
    _expr(expr),
    _exprType(exprType),
    _errorCode(errorCode),
    _doTypeCheck(doTypeCheck),
    _doCardinalityCheck(doCardinalityCheck),
    _treatType(treatType),
    _isExact(isExact)
{
}

ASTNode* XQTreatAs::staticResolution(StaticContext *context)
{
  _exprType->staticResolution(context);

  if(_exprType->getOccurrenceIndicator() == SequenceType::QUESTION_MARK ||
     _exprType->getOccurrenceIndicator() == SequenceType::EXACTLY_ONE) {
    AutoNodeSetOrderingReset orderReset(context);
    _expr = _expr->staticResolution(context);
  }
  else {
    _expr = _expr->staticResolution(context);
  }

  _exprType->getStaticType(_treatType, context, _isExact, this);

  return this;
}

ASTNode *XQTreatAs::staticTypingImpl(StaticContext *context)
{
  _src.clear();

  // Do as much static time type checking as we can, given the
  // limited static typing that we implement
  const StaticType &actualType = _expr->getStaticAnalysis().getStaticType();

  // Get a better static type by looking at our expression's type too
  _src.getStaticType() = _treatType;
  _src.getStaticType() &= actualType;

  _src.setProperties(_expr->getStaticAnalysis().getProperties());
  _src.add(_expr->getStaticAnalysis());

  StaticType::TypeMatch match = _treatType.matches(actualType);

  if(!_expr->getStaticAnalysis().isUpdating()) {
    if((match.type == StaticType::NEVER || match.cardinality == StaticType::NEVER) &&
       _errorCode != err_XPDY0050) {
      // It never matches
      XMLBuffer buf;
      buf.set(X("Expression does not match type "));
      _exprType->toBuffer(buf);
      buf.append(X(" - the expression has a static type of "));
      actualType.typeToBuf(buf);
      buf.append(X(" ["));
      buf.append(_errorCode);
      buf.append(X("]"));
      XQThrow(XPath2TypeMatchException, X("XQTreatAs::staticTyping"), buf.getRawBuffer());
    }

    MessageListener *mlistener = context ? context->getMessageListener() : 0;
    if(mlistener && context->getDoLintWarnings() && _errorCode != err_XPDY0050) {
      if(match.type == StaticType::PROBABLY_NOT || match.cardinality == StaticType::PROBABLY_NOT) {
        // It might not match
        XMLBuffer buf;
        buf.set(X("The expression might not match type "));
        _exprType->toBuffer(buf);
        buf.append(X(" - the expression has a static type of "));
        actualType.typeToBuf(buf);
        mlistener->warning(buf.getRawBuffer(), this);
      }
    }

    if(_isExact && match.type == StaticType::ALWAYS) {
      _doTypeCheck = false;
    }
    if(match.cardinality == StaticType::ALWAYS) {
      _doCardinalityCheck = false;
    }

    if(!_doTypeCheck && !_doCardinalityCheck) {
      // It always matches
      return substitute(_expr);
    }
  }

  return this;
}

ASTNode *XQTreatAs::getExpression() const {
  return _expr;
}

SequenceType *XQTreatAs::getSequenceType() const {
  return _exprType;
}

void XQTreatAs::setExpression(ASTNode *item) {
  _expr = item;
}

Result XQTreatAs::createResult(DynamicContext* context, int flags) const
{
  Result result = _expr->createResult(context, flags);
  if(_doCardinalityCheck &&
     (_exprType->getOccurrenceIndicator() != SequenceType::STAR ||
      _exprType->getItemType() == NULL)) {
    result = _exprType->occurrenceMatches(result, this, _errorCode);
  }
  if(_doTypeCheck) {
    result = _exprType->typeMatches(result, this, _errorCode);
  }
  return result;
}


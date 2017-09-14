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

#include <xqilla/framework/XQillaExport.hpp>
#include <xqilla/ast/XQGlobalVariable.hpp>
#include <xqilla/runtime/Sequence.hpp>
#include <xqilla/runtime/ClosureResult.hpp>
#include <xqilla/schema/SequenceType.hpp>
#include <xqilla/context/VariableStore.hpp>
#include <xqilla/context/VariableTypeStore.hpp>
#include <xqilla/utils/XPath2NSUtils.hpp>
#include <xqilla/context/DynamicContext.hpp>
#include <xqilla/context/ContextHelpers.hpp>
#include <xqilla/exceptions/IllegalArgumentException.hpp>
#include <xqilla/exceptions/XPath2TypeMatchException.hpp>
#include <xqilla/exceptions/StaticErrorException.hpp>
#include <xercesc/framework/XMLBuffer.hpp>
#include <xqilla/ast/XQTreatAs.hpp>
#include <xqilla/functions/XQUserFunction.hpp>
#include <xqilla/simple-api/XQQuery.hpp>
#include <xqilla/optimizer/StaticTyper.hpp>

#if defined(XERCES_HAS_CPP_NAMESPACE)
XERCES_CPP_NAMESPACE_USE
#endif

XQGlobalVariable::XQGlobalVariable(const XMLCh* varQName, SequenceType* seqType, ASTNode* value, XPath2MemoryManager *mm, bool isParam)
  : isParam_(isParam),
    required_(!isParam),
    xpath1Compat_(false),
    m_szQName(mm->getPooledString(varQName)),
    m_szURI(0),
    m_szLocalName(0),
    m_Type(seqType),
    m_Value(value),
    _src(mm),
    staticTyped_(BEFORE)
{
}

static const XMLCh err_XPTY0004[] = { 'e', 'r', 'r', ':', 'X', 'P', 'T', 'Y', '0', '0', '0', '4', 0 };

void XQGlobalVariable::execute(DynamicContext* context) const
{
  try {
    if(m_Value == NULL || isParam_) {
      // It's an external declaration, so check the user has set the value in the variable store
      Result value = context->getGlobalVariableStore()->getVar(m_szURI, m_szLocalName);
      if(!value.isNull()) {
        if(m_Type != NULL) {
          if(isParam_) {
            // Convert the external value using the function conversion rules
            Result matchesRes = m_Type->convertFunctionArg(value, context, xpath1Compat_, m_Type, err_XPTY0004);
            context->setExternalVariable(m_szURI, m_szLocalName, matchesRes->toSequence(context));
          }
          else {
            // Check the external value's type
            Result matchesRes = m_Type->matches(value, m_Type, err_XPTY0004);
            while(matchesRes->next(context).notNull()) {}
          }
        }

        return;
      }

      if(m_Value == NULL) {
        XMLBuffer errMsg;
        errMsg.set(X("A value for the external variable "));
        if(m_szQName != 0) {
          errMsg.append(m_szQName);
        }
        else {
          errMsg.append('{');
          errMsg.append(m_szURI);
          errMsg.append('}');
          errMsg.append(m_szLocalName);
        }
        errMsg.append(X(" has not been provided [err:XPTY0002]"));
        XQThrow(::IllegalArgumentException,X("XQGlobalVariable::createSequence"),errMsg.getRawBuffer());
      }
    }

    // TBD Could use our own VariableStore implementation - jpcs
    context->setExternalVariable(m_szURI, m_szLocalName, m_Value->createResult(context));
  }
  catch(const XPath2TypeMatchException &ex) {
    XMLBuffer errMsg;
    errMsg.set(X("The value for the global variable "));
    if(m_szQName != 0) {
      errMsg.append(m_szQName);
    }
    else {
      errMsg.append('{');
      errMsg.append(m_szURI);
      errMsg.append('}');
      errMsg.append(m_szLocalName);
    }
    errMsg.append(X(" does not match the declared type: "));
    errMsg.append(ex.getError());
    XQThrow(XPath2TypeMatchException,X("XQGlobalVariable::createSequence"),errMsg.getRawBuffer());
  }
}

void XQGlobalVariable::resolveName(StaticContext *context)
{
  XPath2MemoryManager *mm = context->getMemoryManager();

  // variables with no prefix are in no namespace
  if(m_szLocalName == 0) {
    const XMLCh* prefix=XPath2NSUtils::getPrefix(m_szQName, mm);
    if(prefix && *prefix)
      m_szURI = context->getUriBoundToPrefix(prefix, this);
    m_szLocalName = XPath2NSUtils::getLocalName(m_szQName);
  }
}

void XQGlobalVariable::staticResolution(StaticContext* context)
{
  XPath2MemoryManager *mm = context->getMemoryManager();

  xpath1Compat_ = context->getXPath1CompatibilityMode();

  resolveName(context);

  if(m_Type) m_Type->staticResolution(context);

  if(m_Value != NULL) {
    if(m_Type != NULL) {
      m_Value = new (mm) XQTreatAs(m_Value, m_Type, mm);
      m_Value->setLocationInfo(this);
    }
    m_Value = m_Value->staticResolution(context);
  }

  // Set up a default StaticType
  if(m_Type != 0) {
    bool isPrimitive;
    m_Type->getStaticType(_src.getStaticType(), context, isPrimitive, m_Type);
  }
  else {
    _src.getStaticType() = StaticType(StaticType::ITEM_TYPE, 0, StaticType::UNLIMITED);
  }
  _src.setProperties(0);
}

void XQGlobalVariable::staticTypingOnce(StaticContext* context, StaticTyper *styper)
{
  switch(staticTyped_) {
  case AFTER: return;
  case DURING: {
      XMLBuffer buf;
      buf.append(X("The initializing expression for variable {"));
      buf.append(m_szURI);
      buf.append(X("}"));
      buf.append(m_szLocalName);
      buf.append(X(" depends on itself [err:XQST0054]"));
      XQThrow(StaticErrorException, X("XQGlobalVariable::staticTypingOnce"), buf.getRawBuffer());
  }
  case BEFORE: break;
  }

  staticTyped_ = DURING;

  StaticTyper::PrologItem breadcrumb(this, styper->getTrail());
  AutoReset<StaticTyper::PrologItem*> autorReset2(styper->getTrail());
  styper->getTrail() = &breadcrumb;;

  GlobalVariables globalsUsed(XQillaAllocator<XQGlobalVariable*>(context->getMemoryManager()));
  {
    AutoReset<GlobalVariables*> autoReset(styper->getGlobalsUsed());
    styper->getGlobalsUsed() = &globalsUsed;
    staticTyping(context, styper);
  }

  if(!globalsUsed.empty()) {
    // Static type the global variables from this module which we depend on
    GlobalVariables::iterator it = globalsUsed.begin();
    for(; it != globalsUsed.end(); ++it) {
      // XQuery 1.0 doesn't allow forward references
      XQQuery *module = context->getModule();
      if(!module->getVersion3()) {
        GlobalVariables::const_iterator it2 = module->getVariables().begin();
        for(; it2 != module->getVariables().end(); ++it2) {
          if(*it2 == *it) break;
          if(*it2 == this) {
            XMLBuffer errMsg;
            errMsg.set(X("Cannot refer to global variable with name {"));
            errMsg.append((*it)->getVariableURI());
            errMsg.append(X("}"));
            errMsg.append((*it)->getVariableLocalName());
            errMsg.append(X(" because it is declared later [err:XPST0008]"));
            XQThrow(StaticErrorException,X("XQGlobalVariable::staticTypingOnce"), errMsg.getRawBuffer());
          }
        }
      }

      (*it)->staticTypingOnce(context, styper);
    }

    // Re-static type this global variable
    staticTyping(context, styper);
  }

  staticTyped_ = AFTER;
  styper->getGlobalsOrder()->push_back(this);
}

void XQGlobalVariable::resetStaticTypingOnce()
{
  staticTyped_ = BEFORE;
}

void XQGlobalVariable::staticTyping(StaticContext* context, StaticTyper *styper)
{
  VariableTypeStore* varStore = context->getVariableTypeStore();

  if(m_Value != NULL) {
    XQUserFunction::staticTypeFunctionCalls(m_Value, context, styper);

    // Add UNDEFINEDVAR to properties, since variables aren't in scope for themselves
    _src.setProperties(_src.getProperties() | StaticAnalysis::UNDEFINEDVAR);

    m_Value = m_Value->staticTyping(context, styper);
    _src.copy(m_Value->getStaticAnalysis());

    if(m_Value->getStaticAnalysis().isUpdating()) {
      XQThrow(StaticErrorException,X("XQGlobalVariable::staticTyping"),
              X("It is a static error for the initializing expression of a global variable "
                "to be an updating expression [err:XUST0001]"));
    }
  }

  if(m_Value == 0 || !required_) {
    if(m_Type != 0) {
      bool isPrimitive;
      m_Type->getStaticType(_src.getStaticType(), context, isPrimitive, m_Type);
    }
    else {
      _src.getStaticType() = StaticType(StaticType::ITEM_TYPE, 0, StaticType::UNLIMITED);
    }
    _src.setProperties(0);
  }

  varStore->declareGlobalVar(m_szURI, m_szLocalName, _src, this);
}

const XMLCh* XQGlobalVariable::getVariableName() const
{
  return m_szQName;
}

bool XQGlobalVariable::isExternal() const
{
  return (m_Value==NULL);
}

void XQGlobalVariable::setVariableExpr(ASTNode* value)
{
  m_Value=value;
}

const SequenceType *XQGlobalVariable::getSequenceType() const
{
  return m_Type;
}

const ASTNode *XQGlobalVariable::getVariableExpr() const
{
  return m_Value;
}

const XMLCh *XQGlobalVariable::getVariableURI() const
{
  return m_szURI;
}

const XMLCh *XQGlobalVariable::getVariableLocalName() const
{
  return m_szLocalName;
}

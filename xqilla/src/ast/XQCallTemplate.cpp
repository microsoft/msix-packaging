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

#include <xqilla/ast/XQCallTemplate.hpp>
#include <xqilla/ast/XQApplyTemplates.hpp>
#include <xqilla/ast/XQDOMConstructor.hpp>
#include <xqilla/functions/XQUserFunction.hpp>
#include <xqilla/exceptions/StaticErrorException.hpp>
#include <xqilla/context/DynamicContext.hpp>
#include <xqilla/utils/XPath2NSUtils.hpp>
#include <xqilla/utils/XPath2Utils.hpp>
#include <xqilla/functions/FunctionSignature.hpp>

#include <xercesc/framework/XMLBuffer.hpp>

using namespace std;
XERCES_CPP_NAMESPACE_USE;

XQTemplateArgument::XQTemplateArgument(const XMLCh *qn, ASTNode *v, XPath2MemoryManager *mm)
  : qname(mm->getPooledString(qn)),
    uri(0),
    name(0),
    value(v),
    seqType(0),
    varSrc(mm)
{
}

XQCallTemplate::XQCallTemplate(const XMLCh *qname, TemplateArguments *args, XPath2MemoryManager *mm)
  : ASTNodeImpl(CALL_TEMPLATE, mm),
    qname_(qname),
    uri_(0),
    name_(0),
    astName_(0),
    args_(args),
    templates_(XQillaAllocator<XQUserFunction*>(mm))
{
}

XQCallTemplate::XQCallTemplate(ASTNode *qname, TemplateArguments *args, XPath2MemoryManager *mm)
  : ASTNodeImpl(CALL_TEMPLATE, mm),
    qname_(0),
    uri_(0),
    name_(0),
    astName_(qname),
    args_(args),
    templates_(XQillaAllocator<XQUserFunction*>(mm))
{
}

XQCallTemplate::XQCallTemplate(const XMLCh *qname, const XMLCh *uri, const XMLCh *name, ASTNode *astName, TemplateArguments *args,
                               const UserFunctions &templates, XPath2MemoryManager *mm)
  : ASTNodeImpl(CALL_TEMPLATE, mm),
    qname_(qname),
    uri_(uri),
    name_(name),
    astName_(astName),
    args_(args),
    templates_(XQillaAllocator<XQUserFunction*>(mm))
{
  templates_ = templates;
}

ASTNode* XQCallTemplate::staticResolution(StaticContext *context) 
{
  XPath2MemoryManager *mm = context->getMemoryManager();

  // Resolve the template name
  if(astName_) {
    astName_ = new (mm) XQNameExpression(astName_, mm);
    astName_->setLocationInfo(this);
    astName_ = astName_->staticResolution(context);
  }
  else if(name_ == 0) {
    const XMLCh *prefix = XPath2NSUtils::getPrefix(qname_, mm);
    name_ = XPath2NSUtils::getLocalName(qname_);

    if(prefix == 0 || *prefix == 0) {
      uri_ = context->getDefaultFuncNS();
    }
    else {
      uri_ = context->getUriBoundToPrefix(prefix, this);
    }
  }

  // Resolve the call template argument names and check for duplicates
  TemplateArguments::iterator it;
  if(args_ != 0) {
    for(it = args_->begin(); it != args_->end(); ++it) {
      if((*it)->name == 0) {
        (*it)->uri = context->getUriBoundToPrefix(XPath2NSUtils::getPrefix((*it)->qname, mm), this);
        (*it)->name = XPath2NSUtils::getLocalName((*it)->qname);
      }
      // The template instance will call staticResolution on the argument values

      for(TemplateArguments::iterator it2 = args_->begin(); it2 != it; ++it2) {
        if(XPath2Utils::equals((*it)->name, (*it2)->name) &&
           XPath2Utils::equals((*it)->uri, (*it2)->uri)) {
          XMLBuffer buf;
          buf.set(X("Two template parameters have the name {"));
          buf.append((*it)->uri);
          buf.append(X("}"));
          buf.append((*it)->name);
          buf.append(X(" [err:XTSE0670]"));

          XQThrow3(StaticErrorException, X("XQCallTemplate::staticResolution"), buf.getRawBuffer(), *it);
        }
      }
    }
  }

  if(astName_ != 0) return this;

  // Lookup the template
  const XQUserFunction *tplt = context->lookUpNamedTemplate(uri_, name_);
  if(tplt == 0) {
    XMLBuffer buf;
    buf.set(X("A template called {"));
    buf.append(uri_);
    buf.append(X("}"));
    buf.append(name_);
    buf.append(X(" is not defined [err:XTSE0650]"));

    XQThrow(StaticErrorException, X("XQCallTemplate::staticResolution"), buf.getRawBuffer());
  }

  // Check the template's expected parameters against the call template arguments,
  // building the input argument list as we go
  vector<bool> argUsed(args_ ? args_->size() : 0, false);
  VectorOfASTNodes newArgs = VectorOfASTNodes(XQillaAllocator<ASTNode*>(mm));

  if(tplt->getSignature()->argSpecs != 0) {
    ArgumentSpecs::const_iterator argIt;
    for(argIt = tplt->getSignature()->argSpecs->begin(); argIt != tplt->getSignature()->argSpecs->end(); ++argIt) {

      bool found = false;
      if(args_ != 0) {
        vector<bool>::iterator argUsedIt = argUsed.begin();
        for(it = args_->begin(); it != args_->end(); ++it, ++argUsedIt) {
          if(XPath2Utils::equals((*it)->name, (*argIt)->getName()) &&
             XPath2Utils::equals((*it)->uri, (*argIt)->getURI())) {
            found = true;
            newArgs.push_back((*it)->value);
            *argUsedIt = true;
            break;
          }
        }
      }

      if(!found) {
        // [ERR XTSE0690] It is a static error  if a template that is invoked using xsl:call-template
        // declares a template parameter  specifying required="yes" and not specifying tunnel="yes", if
        // no value for this parameter is supplied by the calling instruction.
        XMLBuffer buf;
        buf.set(X("No value is specified for the template parameter {"));
        buf.append((*argIt)->getURI());
        buf.append(X("}"));
        buf.append((*argIt)->getName());
        buf.append(X(" [err:XTSE0690]"));

        XQThrow(StaticErrorException, X("XQCallTemplate::staticResolution"), buf.getRawBuffer());
      }
    }
  }

  // Check for call template arguments that shouldn't be specified
  if(args_ != 0) {
    vector<bool>::iterator argUsedIt = argUsed.begin();
    for(it = args_->begin(); it != args_->end(); ++it, ++argUsedIt) {
      if(!(*argUsedIt)) {
        // [ERR XTSE0680] In the case of xsl:call-template, it is a static error to pass a non-tunnel
        // parameter named x to a template that does not have a template parameter named x, unless
        // backwards compatible behavior is enabled for the xsl:call-template  instruction. This is not
        // an error in the case of xsl:apply-templates, xsl:apply-imports, and xsl:next-match; in these
        // cases the parameter is simply ignored.        
        XMLBuffer buf;
        buf.set(X("The template does not have a parameter named {"));
        buf.append((*it)->uri);
        buf.append(X("}"));
        buf.append((*it)->name);
        buf.append(X(" [err:XTSE0680]"));

        XQThrow3(StaticErrorException, X("XQCallTemplate::staticResolution"), buf.getRawBuffer(), *it);
      }
    }
  }

  ASTNode *result = tplt->createInstance(newArgs, mm);
  result->setLocationInfo(this);
  return result->staticResolution(context);
}

ASTNode *XQCallTemplate::staticTypingImpl(StaticContext *context)
{
  _src.clear();

  // Calculate our static type from the template instances with names
  if(context)
    templates_ = context->getTemplateRules();

  bool first = true;

  UserFunctions::iterator inIt;
  for(inIt = templates_.begin(); inIt != templates_.end(); ++inIt) {
    if((*inIt) == 0 || (*inIt)->getName() == 0) {
      *inIt = 0;
      continue;
    }

    if(first) {
      first = false;
      _src.getStaticType() = (*inIt)->getBodyStaticAnalysis().getStaticType();
      _src.setProperties((*inIt)->getBodyStaticAnalysis().getProperties());
    } else {
      _src.getStaticType() |= (*inIt)->getBodyStaticAnalysis().getStaticType();
      _src.setProperties(_src.getProperties() & (*inIt)->getBodyStaticAnalysis().getProperties());
    }
    _src.add((*inIt)->getBodyStaticAnalysis());
  }

  TemplateArguments::iterator it;

  if(args_ != 0) {
    for(it = args_->begin(); it != args_->end(); ++it) {
      if(!_src.removeVariable((*it)->uri, (*it)->name)) {
        // TBD This parameter will never be used - jpcs
      }
    }
  }

  // At this point we know astName_ is not null
  _src.add(astName_->getStaticAnalysis());

  if(args_ != 0) {
    for(it = args_->begin(); it != args_->end(); ++it) {
      _src.add((*it)->value->getStaticAnalysis());
    }
  }

  return this;
}

Result XQCallTemplate::createResult(DynamicContext *context, int flags) const
{
  AnyAtomicType::Ptr itemName = astName_->createResult(context)->next(context);
  const ATQNameOrDerived *pQName = (const ATQNameOrDerived*)itemName.get();
  const XMLCh *uri = pQName->getURI();
  const XMLCh *localname = pQName->getName();

  // Lookup the template
  const XQUserFunction *tplt = context->lookUpNamedTemplate(uri, localname);
  if(tplt == 0) {
    XMLBuffer buf;
    buf.set(X("A template called {"));
    buf.append(uri);
    buf.append(X("}"));
    buf.append(localname);
    buf.append(X(" is not defined [err:XTSE0650]"));

    XQThrow(StaticErrorException, X("XQCallTemplate::staticResolution"), buf.getRawBuffer());
  }

  return XQApplyTemplates::executeTemplate(tplt, args_, 0, context, this);
}

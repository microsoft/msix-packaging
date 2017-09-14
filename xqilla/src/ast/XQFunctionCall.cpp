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

#include <xqilla/ast/XQFunctionCall.hpp>
#include <xqilla/exceptions/FunctionException.hpp>
#include <xqilla/exceptions/StaticErrorException.hpp>
#include <xqilla/context/DynamicContext.hpp>
#include <xqilla/utils/XPath2NSUtils.hpp>
#include <xqilla/utils/XPath2Utils.hpp>

#include <xercesc/framework/XMLBuffer.hpp>

XERCES_CPP_NAMESPACE_USE;

XQFunctionCall::XQFunctionCall(const XMLCh *qname, VectorOfASTNodes *args, XPath2MemoryManager *mm)
  : ASTNodeImpl(FUNCTION_CALL, mm),
    prefix_(XPath2NSUtils::getPrefix(qname, mm)),
    uri_(0),
    name_(XPath2NSUtils::getLocalName(qname)),
    args_(args ? args : new (mm) VectorOfASTNodes(XQillaAllocator<ASTNode*>(mm)))
{
}

XQFunctionCall::XQFunctionCall(const XMLCh *prefix, const XMLCh *uri, const XMLCh *name, VectorOfASTNodes *args, XPath2MemoryManager *mm)
  : ASTNodeImpl(FUNCTION_CALL, mm),
    prefix_(prefix),
    uri_(uri),
    name_(name),
    args_(args ? args : new (mm) VectorOfASTNodes(XQillaAllocator<ASTNode*>(mm)))
{
}

ASTNode* XQFunctionCall::staticResolution(StaticContext *context) 
{
  if(uri_ == 0) {
    if(prefix_ == 0 || *prefix_ == 0) {
      uri_ = context->getDefaultFuncNS();
    }
    else {
      uri_ = context->getUriBoundToPrefix(prefix_, this);
    }
  }

  ASTNode *result = context->lookUpFunction(uri_, name_, *args_, this);
  if(result == 0) {
    XMLBuffer buf;
    buf.set(X("A function called {"));
    buf.append(uri_);
    buf.append(X("}"));
    buf.append(name_);
    buf.append(X(" with "));
    XPath2Utils::numToBuf(args_ ? (unsigned int)args_->size() : 0, buf);
    buf.append(X(" arguments is not defined [err:XPST0017]"));

    XQThrow(StaticErrorException, X("XQFunctionCall::staticResolution"), buf.getRawBuffer());
  }

  // Our arguments don't belong to us anymore
  for(VectorOfASTNodes::iterator i = args_->begin(); i != args_->end(); ++i) {
    *i = 0;
  }
  // Release this object
  this->release();

  return result->staticResolution(context);
}

ASTNode *XQFunctionCall::staticTypingImpl(StaticContext *context)
{
  // Should never happen
  return this;
}

Result XQFunctionCall::createResult(DynamicContext* context, int flags) const
{
  // Should never happen
  return 0;
}

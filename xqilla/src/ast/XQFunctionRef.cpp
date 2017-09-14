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

#include <xqilla/ast/XQFunctionRef.hpp>
#include <xqilla/ast/XQInlineFunction.hpp>
#include <xqilla/context/DynamicContext.hpp>
#include <xqilla/context/VariableTypeStore.hpp>
#include <xqilla/context/ContextHelpers.hpp>
#include <xqilla/utils/XPath2NSUtils.hpp>
#include <xqilla/utils/XPath2Utils.hpp>
#include <xqilla/utils/XStr.hpp>
#include <xqilla/schema/SequenceType.hpp>
#include <xqilla/exceptions/StaticErrorException.hpp>
#include <xqilla/functions/FunctionSignature.hpp>

#include "../items/impl/FunctionRefImpl.hpp"

XERCES_CPP_NAMESPACE_USE;
using namespace std;

XQFunctionRef::XQFunctionRef(const XMLCh *qname, unsigned int numArgs, XPath2MemoryManager *mm)
  : ASTNodeImpl(FUNCTION_REF, mm),
    qname_(qname),
    numArgs_(numArgs)
{
}

ASTNode *XQFunctionRef::staticResolution(StaticContext *context)
{
  XPath2MemoryManager *mm = context->getMemoryManager();

  const XMLCh *prefix = XPath2NSUtils::getPrefix(qname_, mm);
  const XMLCh *name = XPath2NSUtils::getLocalName(qname_);

  const XMLCh *uri;
  if(prefix == 0 || *prefix == 0) {
    uri = context->getDefaultFuncNS();
  }
  else {
    uri = context->getUriBoundToPrefix(prefix, this);
  }

  FunctionSignature *signature = 0;
  ASTNode *instance = FunctionRefImpl::createInstance(uri, name, numArgs_, context, this, signature);
  if(instance == 0) {
    XMLBuffer buf;
    buf.set(X("A function called {"));
    buf.append(uri);
    buf.append(X("}"));
    buf.append(name);
    buf.append(X(" with "));
    XPath2Utils::numToBuf(numArgs_, buf);
    buf.append(X(" arguments is not defined [err:XPST0017]"));

    XQThrow(StaticErrorException, X("XQFunctionRef::staticResolution"), buf.getRawBuffer());
  }

  instance = instance->staticResolution(context);

  ASTNode *result = new (mm) XQInlineFunction(0, prefix, uri, name, numArgs_, signature, instance, mm);
  result->setLocationInfo(result);
  this->release();
  return result; // Don't call staticResolution() on result
}

ASTNode *XQFunctionRef::staticTypingImpl(StaticContext *context)
{
  // Should never happen
  assert(false);
  return this;
}

Result XQFunctionRef::createResult(DynamicContext *context, int flags) const
{
  // Should never happen
  assert(false);
  return 0;
}

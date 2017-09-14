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

#include "../../config/xqilla_config.h"
#include "FunctionRefImpl.hpp"
#include <xqilla/ast/XQVariable.hpp>
#include <xqilla/ast/XQFunction.hpp>
#include <xqilla/ast/XQCastAs.hpp>
#include <xqilla/exceptions/XPath2TypeMatchException.hpp>
#include <xqilla/functions/FuncFactory.hpp>
#include <xqilla/functions/FunctionSignature.hpp>
#include <xqilla/context/impl/VarStoreImpl.hpp>
#include <xqilla/context/DynamicContext.hpp>
#include <xqilla/context/ContextHelpers.hpp>
#include <xqilla/context/ItemFactory.hpp>
#include <xqilla/utils/XPath2Utils.hpp>
#include <xqilla/schema/SequenceType.hpp>
#include <xqilla/events/EventHandler.hpp>
#include <xqilla/runtime/ClosureResult.hpp>

#include <xercesc/validators/schema/SchemaSymbols.hpp>

XERCES_CPP_NAMESPACE_USE;
using namespace std;

static const XMLCh constructorArgName[] = { 'a', 'r', 'g', 0 };

FunctionRefImpl::FunctionRefImpl(const XMLCh *prefix, const XMLCh *uri, const XMLCh *localname,
                                 const FunctionSignature *signature, const ASTNode *instance,
                                 const StaticAnalysis &sa, DynamicContext *context)
  : prefix_(prefix),
    uri_(uri),
    name_(localname),
    signature_(signature),
    signatureOwned_(false),
    instance_(instance),
    varStore_(context->getMemoryManager())
{
  // Copy the variables we need into our local storage
  varStore_.cacheVariableStore(sa, context->getVariableStore());
}

FunctionRefImpl::FunctionRefImpl(const FunctionRefImpl *other, const Result &argument, unsigned int argNum, DynamicContext *context)
  : prefix_(other->prefix_),
    uri_(other->uri_),
    name_(other->name_),
    signature_(new (context->getMemoryManager()) FunctionSignature(other->signature_, argNum, context->getMemoryManager())),
    signatureOwned_(true),
    instance_(other->instance_),
    varStore_(other->varStore_, context->getMemoryManager())
{
  ArgumentSpecs::const_iterator argsIt = other->signature_->argSpecs->begin();
  for(unsigned int i = 0; i < argNum; ++i) {
    ++argsIt;
  }

  varStore_.setVar((*argsIt)->getURI(), (*argsIt)->getName(), argument);
}

FunctionRefImpl::~FunctionRefImpl()
{
  if(signatureOwned_)
    const_cast<FunctionSignature*>(signature_)->release();
}

class FunctionRefScope : public VariableStore
{
public:
  FunctionRefScope(const FunctionRefImpl::Ptr func, const VectorOfResults &args, DynamicContext *context)
    : func_(func),
      scope_(context->getMemoryManager(), context->getVariableStore())
  {
    if(func_->signature_->argSpecs) {
      VectorOfResults::const_iterator i = args.begin();
      ArgumentSpecs::const_iterator argsIt = func_->signature_->argSpecs->begin();
      for(; i != args.end(); ++i, ++argsIt) {
        scope_.setVar((*argsIt)->getURI(), (*argsIt)->getName(), *i);
      }
    }
  }

  Result getVar(const XMLCh *namespaceURI, const XMLCh *name) const
  {
    Result result = func_->varStore_.getVar(namespaceURI, name);
    if(!result.isNull()) return result;
    return scope_.getVar(namespaceURI, name);
  }

  void getInScopeVariables(std::vector<std::pair<const XMLCh*, const XMLCh*> > &variables) const
  {
    func_->varStore_.getInScopeVariables(variables);
    scope_.getInScopeVariables(variables);
  }

private:
  FunctionRefImpl::Ptr func_;
  VarStoreImpl scope_;
};

Result FunctionRefImpl::execute(const VectorOfResults &args, DynamicContext *context, const LocationInfo *location) const
{
  if(args.size() != getNumArgs()) {
    XMLBuffer buf;
    buf.set(X("The function item invoked does not accept "));
    XPath2Utils::numToBuf((unsigned int)args.size(), buf);
    buf.append(X(" arguments - found item of type "));
    typeToBuffer(context, buf);
    buf.append(X(" [err:XPTY0004]"));
    XQThrow3(XPath2TypeMatchException, X("FunctionRefImpl::execute"), buf.getRawBuffer(), location);
  }

  FunctionRefScope scope(this, args, context);
  AutoVariableStoreReset vsReset(context, &scope);
  return instance_->createResult(context);
}

ATQNameOrDerived::Ptr FunctionRefImpl::getName(const DynamicContext *context) const
{
  if(name_ == 0) return 0;
  return context->getItemFactory()->createQName(uri_, prefix_, name_, context);
}

size_t FunctionRefImpl::getNumArgs() const
{
  if(signature_->argSpecs == 0) return 0;
  return signature_->argSpecs->size();
}

FunctionRef::Ptr FunctionRefImpl::partialApply(const Result &arg, unsigned int argNum, DynamicContext *context, const LocationInfo *location) const
{
  if(getNumArgs() < argNum) {
    XMLBuffer buf;
    buf.set(X("The function item argument to fn:partial-apply() must have an arity of at least "));
    XPath2Utils::numToBuf(argNum, buf);
    buf.append(X(" - found item of type "));
    typeToBuffer(context, buf);
    buf.append(X(" [err:TBD]"));
    XQThrow3(XPath2TypeMatchException, X("FunctionRefImpl::partialApply"), buf.getRawBuffer(), location);
  }

  return new FunctionRefImpl(this, arg, argNum - 1, context);
}

void FunctionRefImpl::generateEvents(EventHandler *events, const DynamicContext *context,
                                     bool preserveNS, bool preserveType) const
{
  // TBD What is the correct way to generate events for a FunctionRef? - jpcs
  events->atomicItemEvent(AnyAtomicType::STRING, asString(context), 0, 0);
}

const XMLCh *FunctionRefImpl::asString(const DynamicContext *context) const
{
  XMLBuffer buf;
  signature_->toBuffer(buf, /*typeSyntax*/false);
  return context->getMemoryManager()->getPooledString(buf.getRawBuffer());
}

void FunctionRefImpl::typeToBuffer(DynamicContext *context, XMLBuffer &buffer) const
{
  signature_->toBuffer(buffer, /*typeSyntax*/true);
}

void *FunctionRefImpl::getInterface(const XMLCh *name) const
{
  return 0;
}

ASTNode *FunctionRefImpl::createInstance(const FuncFactory *factory, const FunctionSignature *signature,
                                         XPath2MemoryManager *mm, const LocationInfo *location)
{
  VectorOfASTNodes newArgs = VectorOfASTNodes(XQillaAllocator<ASTNode*>(mm));
  if(signature->argSpecs) {
    ArgumentSpecs::const_iterator argsIt = signature->argSpecs->begin();
    for(; argsIt != signature->argSpecs->end(); ++argsIt) {
      assert((*argsIt)->getName() != 0);
      XQVariable *var = new (mm) XQVariable((*argsIt)->getURI(), (*argsIt)->getName(), mm);
      var->setLocationInfo(location);
      newArgs.push_back(var);
    }
  }

  ASTNode *instance = factory->createInstance(newArgs, mm);
  instance->setLocationInfo(location);
  return instance;
}

ASTNode *FunctionRefImpl::createInstance(const XMLCh *uri, const XMLCh *name, unsigned int numArgs,
                                         StaticContext *context, const LocationInfo *location,
                                         FunctionSignature *&signature)
{
  XPath2MemoryManager *mm = context->getMemoryManager();

  VectorOfASTNodes newArgs = VectorOfASTNodes(XQillaAllocator<ASTNode*>(mm));
  for(unsigned int i = 0; i < numArgs; ++i) {
    newArgs.push_back(0); // Dummy argument
  }

  ASTNode *result = context->lookUpFunction(uri, name, newArgs, location);
  if(!result) return 0;

  switch(result->getType()) {
  case ASTNode::FUNCTION:
  case ASTNode::USER_FUNCTION: {
    XQFunction *function = (XQFunction*)result;
    function->parseSignature(context);
    signature = new (mm) FunctionSignature(function->getSignature(), mm);

    if(signature->argSpecs) {
      // Fill in the arguments with XQVariable objects
      ArgumentSpecs::const_iterator argsIt = signature->argSpecs->begin();
      VectorOfASTNodes &args = const_cast<VectorOfASTNodes&>(function->getArguments());
      for(VectorOfASTNodes::iterator i = args.begin(); i != args.end(); ++i, ++argsIt) {
        (*i) = new (mm) XQVariable((*argsIt)->getURI(), (*argsIt)->getName(), mm);
        (*i)->setLocationInfo(location);
      }
    }

    break;
  }
  case ASTNode::CAST_AS: {
    XQCastAs *cast = (XQCastAs*)result;

    // Fill in the argument with an XQVariable object
    XQVariable *var = new (mm) XQVariable(0, constructorArgName, mm);
    var->setLocationInfo(location);
    cast->setExpression(var);

    // Create a signature for the constructor function
    SequenceType *argType = new (mm) SequenceType(SchemaSymbols::fgURI_SCHEMAFORSCHEMA,
                                                  AnyAtomicType::fgDT_ANYATOMICTYPE,
                                                  SequenceType::QUESTION_MARK, mm);
    argType->setLocationInfo(location);

    ArgumentSpec *arg = new (mm) ArgumentSpec(constructorArgName, argType, mm);
    arg->setLocationInfo(location);

    ArgumentSpecs *args = new (mm) ArgumentSpecs(XQillaAllocator<ArgumentSpec*>(mm));
    args->push_back(arg);

    signature = new (mm) FunctionSignature(args, cast->getSequenceType(), mm);
    signature->staticResolution(context);
    break;
  }
  default:
    assert(false);
    break;
  }

  return result;
}

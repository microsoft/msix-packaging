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
#include <xercesc/validators/schema/SchemaSymbols.hpp>
#include <xqilla/ast/XQOperator.hpp>
#include <xqilla/context/DynamicContext.hpp>
#include <xqilla/exceptions/XPath2TypeCastException.hpp>
#include <xqilla/items/AnyAtomicType.hpp>
#include <xqilla/items/ATBooleanOrDerived.hpp>
#include <xqilla/items/Node.hpp>
#include <xqilla/items/DatatypeFactory.hpp>

XQOperator::XQOperator(const XMLCh* opName, const VectorOfASTNodes &args, XPath2MemoryManager* memMgr)
  : ASTNodeImpl(OPERATOR, memMgr),
  _args(args)
{
  _opName=opName;
}

void XQOperator::addArgument(ASTNode* arg)
{
	_args.push_back(arg);
}

ASTNode* XQOperator::getArgument(unsigned int index) const
{
	assert(index<_args.size());
	return _args[index];
}

void XQOperator::setArgument(unsigned int index, ASTNode *arg)
{
	assert(index<_args.size());
	_args[index] = arg;
}

void XQOperator::removeArgument(unsigned int index)
{
	assert(index<_args.size());
	_args.erase(_args.begin() + index);
}

unsigned int XQOperator::getNumArgs() const
{
  return (unsigned int)_args.size();
}

const XMLCh* XQOperator::getOperatorName() const
{
  return _opName;
}

const VectorOfASTNodes &XQOperator::getArguments() const {
  return _args;
}

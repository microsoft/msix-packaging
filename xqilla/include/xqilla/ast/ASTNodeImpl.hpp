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

#ifndef _ASTNODEIMPL_HPP
#define _ASTNODEIMPL_HPP

class DynamicContext;
class Numeric;

#include <xqilla/ast/ASTNode.hpp>
#include <xqilla/runtime/Result.hpp>
#include <xqilla/ast/StaticAnalysis.hpp>

/** class which is used as base for all the different data types */
class XQILLA_API ASTNodeImpl : public ASTNode
{
public:
  ASTNodeImpl(whichType type, XPath2MemoryManager* memMgr);
  virtual ~ASTNodeImpl();

  virtual whichType getType() const;
	
  virtual void release();
  virtual ASTNode *copy(DynamicContext *context) const;

  virtual bool isSubsetOf(const ASTNode *other) const;
  virtual bool isEqualTo(const ASTNode *other) const;

  virtual ASTNode *staticTyping(StaticContext *context, StaticTyper *styper);

  /** Returns true if this ASTNode is an instance of XQSequence or XQLiteral */
  virtual bool isConstant() const;
  /** Returns true if this ASTNode has no predicates, and is an instance of
      XQSequence or XQLiteral. If the literal value of this ASTNode
      is a single DateOrTimeType, then !hasTimezone() on it must return true,
      otherwise this method will return false. */
  virtual bool isDateOrTimeAndHasNoTimezone(StaticContext* context) const;

  /// Default - returns an empty update list
  virtual PendingUpdateList createUpdateList(DynamicContext *context) const;

  /// Default implementation calls ATBooleanOrDerived::isTrue()
  virtual BoolResult boolResult(DynamicContext* context) const;

  /**
   * Can be overridden by derived classes. Default implementation returns a
   * NavStepResult constructed using contextItems and this expression.
   */
  virtual Result iterateResult(const Result &contextItems, DynamicContext* context) const;

  /** Returns the result of this expression via the EventHandler provided.
      Default implementation uses result returned from createResult(). */
  virtual EventGenerator::Ptr generateEvents(EventHandler *events, DynamicContext *context,
                                               bool preserveNS, bool preserveType) const;

  /** Returns the given ASTNode* after setting the referenced var to 0 and releasing this */
  ASTNode *substitute(ASTNode *&result);

  virtual XPath2MemoryManager *getMemoryManager() const;

  virtual const StaticAnalysis &getStaticAnalysis() const;

protected:
  StaticAnalysis _src;
  whichType _type;
  XPath2MemoryManager* _memMgr;
};

#endif

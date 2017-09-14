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

#ifndef _ASTNODE_HPP
#define _ASTNODE_HPP

#include <vector>

#include <xqilla/framework/XQillaExport.hpp>
#include <xqilla/framework/XPath2MemoryManager.hpp>
#include <xqilla/events/EventGenerator.hpp>
#include <xqilla/ast/LocationInfo.hpp>

class DynamicContext;
class StaticContext;
class Result;
class StaticAnalysis;
class PendingUpdateList;
class EventHandler;
class StaticTyper;
struct BoolResult;

class XQILLA_API ASTNode : public LocationInfo
{
public:
  ///enum for data types
  typedef enum {
    LITERAL,
    NUMERIC_LITERAL,
    QNAME_LITERAL,
    SEQUENCE,
    FUNCTION,
    NAVIGATION,
    VARIABLE,
    STEP,
    IF,
    CASTABLE_AS,
    CAST_AS,
    TREAT_AS,
    OPERATOR,
    CONTEXT_ITEM,
    DOM_CONSTRUCTOR,
    QUANTIFIED,
    TYPESWITCH,
    VALIDATE,
    FUNCTION_CALL,
    USER_FUNCTION,
    ORDERING_CHANGE,
    FUNCTION_COERCION,
    PROMOTE_UNTYPED,
    PROMOTE_NUMERIC,
    PROMOTE_ANY_URI,
    DOCUMENT_ORDER,
    PREDICATE,
    ATOMIZE,
    EBV,
    FTCONTAINS,
    UDELETE,
    URENAME,
    UREPLACE,
    UREPLACE_VALUE_OF,
    UTRANSFORM,
    UINSERT_AS_FIRST,
    UINSERT_AS_LAST,
    UINSERT_INTO,
    UINSERT_AFTER,
    UINSERT_BEFORE,
    UAPPLY_UPDATES,
    NAME_EXPRESSION,
    CONTENT_SEQUENCE,
    DIRECT_NAME,
    RETURN,
    NAMESPACE_BINDING,
    FUNCTION_CONVERSION,
    SIMPLE_CONTENT,
    ANALYZE_STRING,
    CALL_TEMPLATE,
    APPLY_TEMPLATES,
    INLINE_FUNCTION,
    FUNCTION_REF,
    FUNCTION_DEREF,
    PARTIAL_APPLY,
    COPY_OF,
    COPY,
    MAP,
    DEBUG_HOOK
  } whichType;

  ASTNode() : userData_(0) {}
  virtual ~ASTNode() {}

  virtual void release() = 0;
  virtual ASTNode *copy(DynamicContext *context) const = 0;

  virtual bool isSubsetOf(const ASTNode *other) const = 0;
  virtual bool isEqualTo(const ASTNode *other) const = 0;

  virtual whichType getType() const = 0;
  virtual XPath2MemoryManager *getMemoryManager() const = 0;

  virtual ASTNode *staticResolution(StaticContext *context) = 0;
  virtual ASTNode *staticTyping(StaticContext *context, StaticTyper *styper) = 0;
  virtual ASTNode *staticTypingImpl(StaticContext *context) = 0;

  /// Returns the StaticAnalysis for this ASTNode
  virtual const StaticAnalysis &getStaticAnalysis() const = 0;

  /** Returns true if this ASTNode has no predicates, and is an instance of
      XQSequence or XQLiteral */
  virtual bool isConstant() const = 0;
  /** Returns true if this ASTNode has no predicates, and is an instance of
      XQSequence or XQLiteral. If the literal value of this ASTNode
      is a single DateOrTimeType, then !hasTimezone() on it must return true,
      otherwise this method will return false. */
  virtual bool isDateOrTimeAndHasNoTimezone(StaticContext* context) const = 0;

  /** Returns a Result iterator for the results of this expression.
      The flags parameter is currently unused */
  virtual Result createResult(DynamicContext* context, int flags=0) const = 0;

  /** Returns a result iterator with the result of applying this expression to
      each of the given context items in turn. */
  virtual Result iterateResult(const Result &contextItems, DynamicContext* context) const = 0;

  /** Returns the result of this expression as a boolean */
  virtual BoolResult boolResult(DynamicContext* context) const = 0;

  /** Returns the result of this expression via the EventHandler provided.
      An EventGenerator may be returned to be called as a tail call optimization */
  virtual EventGenerator::Ptr generateEvents(EventHandler *events, DynamicContext *context,
                                             bool preserveNS, bool preserveType) const = 0;

  void generateAndTailCall(EventHandler *events, DynamicContext *context,
                           bool preserveNS, bool preserveType) const;

  /// Executes an update expression
  virtual PendingUpdateList createUpdateList(DynamicContext *context) const = 0;

  void *getUserData() const { return userData_; }
  void setUserData(void *data) { userData_ = data; }

private:
  void *userData_;
};

inline void ASTNode::generateAndTailCall(EventHandler *events, DynamicContext *context,
                                         bool preserveNS, bool preserveType) const
{
  EventGenerator::generateAndTailCall(generateEvents(events, context, preserveNS, preserveType),
                                      events, context);
}

typedef std::vector<ASTNode*,XQillaAllocator<ASTNode*> > VectorOfASTNodes;

#endif


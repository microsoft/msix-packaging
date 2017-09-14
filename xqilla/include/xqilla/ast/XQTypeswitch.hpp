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

#ifndef XQTYPESWITCH_HPP
#define XQTYPESWITCH_HPP

#include <xqilla/ast/ASTNodeImpl.hpp>
#include <xqilla/context/impl/VarStoreImpl.hpp>

class SequenceType;
class ResultBuffer;
template<class TYPE> class Scope;

class XQILLA_API XQTypeswitch : public ASTNodeImpl
{
public:
  class Case : public LocationInfo
  {
  public:
    Case(const XMLCh *qname, SequenceType *seqType, ASTNode *expr);
    Case(const XMLCh *qname, const XMLCh *uri, const XMLCh *name, SequenceType *seqType,
         const StaticType &treatType, bool isExact, ASTNode *expr);

    void staticResolution(StaticContext* context);
    void staticTyping(const StaticAnalysis &var_src, StaticContext* context,
                      StaticAnalysis &src, bool &possiblyUpdating, bool first);

    const XMLCh *getQName() const { return qname_; }
    const XMLCh *getURI() const { return uri_; }
    const XMLCh *getName() const { return name_; }

    bool isVariableUsed() const { return qname_ != 0; }
    void setVariableNotUsed() { qname_ = 0; }

    SequenceType *getSequenceType() const { return seqType_; }
    void setSequenceType(SequenceType *s) { seqType_ = s; }

    const StaticType &getTreatType() const { return treatType_; }
    bool getIsExact() const { return isExact_; }

    ASTNode *getExpression() const { return expr_; }
    void setExpression(ASTNode *expr) { expr_ = expr; }

  private:
    const XMLCh *qname_;
    const XMLCh *uri_;
    const XMLCh *name_;
    SequenceType *seqType_;
    StaticType treatType_;
    bool isExact_;
    ASTNode *expr_;
  };

  typedef std::vector<Case*, XQillaAllocator<Case*> > Cases;

  XQTypeswitch(ASTNode *expr, Cases *cases, Case *defaultCase, XPath2MemoryManager *mm);

  virtual ASTNode *staticResolution(StaticContext *context);
  virtual ASTNode *staticTypingImpl(StaticContext *context);

  virtual Result createResult(DynamicContext* context, int flags=0) const;
  virtual EventGenerator::Ptr generateEvents(EventHandler *events, DynamicContext *context,
                              bool preserveNS, bool preserveType) const;
  virtual PendingUpdateList createUpdateList(DynamicContext *context) const;

  const Case *chooseCase(DynamicContext *context, Sequence &resultSeq) const;

  ASTNode *getExpression() const { return expr_; }
  void setExpression(ASTNode *expr) { expr_ = expr; }

  const Case *getDefaultCase() const { return default_; }
  const Cases *getCases() const { return cases_; }

private:
  ASTNode *expr_;
  Cases *cases_;
  Case *default_;
};

#endif

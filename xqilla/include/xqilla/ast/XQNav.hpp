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

#ifndef _XQNav_HPP
#define _XQNav_HPP

#include <vector>
#include <xqilla/framework/XQillaExport.hpp>

#include <xqilla/ast/ASTNodeImpl.hpp>
#include <xqilla/ast/XQStep.hpp>

///this class calculates a node list based on a context
class XQILLA_API XQNav : public ASTNodeImpl
{
public:
  class XQILLA_API StepInfo {
  public:
    StepInfo() : step(0) {}
    StepInfo(ASTNode *s) : step(s) {}
    ASTNode *step;
  };
  typedef std::vector<StepInfo,XQillaAllocator<StepInfo> > Steps;

  XQNav(XPath2MemoryManager* memMgr);
  XQNav(ASTNode *step, XPath2MemoryManager* memMgr);
  XQNav(const Steps &steps, bool sortAdded, XPath2MemoryManager* memMgr);
  virtual ~XQNav();
	

  /** Add a new NavigationStep to the sequence of navigation steps to
      be performed. This operation is used by parse tree classes to
      turn themselves into an optimised, reusable navigation. **/
  void addStep(const StepInfo &step);

  /** Add a new NavigationStep to the start of the sequence of
      navigation steps to be performed.**/
  void addStepFront(ASTNode* step);

  virtual Result createResult(DynamicContext* context, int flags=0) const;

  virtual ASTNode* staticResolution(StaticContext *context);
  virtual ASTNode *staticTypingImpl(StaticContext *context);

  const Steps &getSteps() const;

  bool getSortAdded() const { return _sortAdded; }
  void setSortAdded(bool value) { _sortAdded = value; }

  static unsigned int combineProperties(unsigned int prev_props, unsigned int step_props);

protected:
  //list of steps to be performed.
  Steps _steps;
  bool _sortAdded;
};

class XQILLA_API NavStepResult : public ResultImpl
{
public:
  NavStepResult(const Result &parent, const ASTNode *step, size_t contextSize);

  Item::Ptr next(DynamicContext *context);

private:
  Result parent_;
  const ASTNode *step_;
  Result stepResult_;
  size_t contextPos_;
  size_t contextSize_;
  Item::Ptr contextItem_;
};

class XQILLA_API IntermediateStepCheckResult : public ResultImpl
{
public:
  IntermediateStepCheckResult(const LocationInfo *o, const Result &parent);

  Item::Ptr next(DynamicContext *context);

private:
  Result parent_;
};

class XQILLA_API LastStepCheckResult : public ResultImpl
{
public:
  LastStepCheckResult(const LocationInfo *o, const Result &parent);

  Item::Ptr next(DynamicContext *context);

private:
  Result parent_;
  int _nTypeOfItemsInLastStep;
};

#endif



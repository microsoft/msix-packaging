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

#ifndef CONTEXTHELPERS_HPP
#define CONTEXTHELPERS_HPP

#include <xqilla/framework/XQillaExport.hpp>
#include <xqilla/context/DynamicContext.hpp>

class XQILLA_API AutoNodeSetOrderingReset
{
public:
  AutoNodeSetOrderingReset(StaticContext* context, StaticContext::NodeSetOrdering ordering = StaticContext::ORDERING_UNORDERED)
  {
    context_ = context;
    if(context_) {
      ordering_ = context->getNodeSetOrdering();
      context->setNodeSetOrdering(ordering);
    }
  }

  ~AutoNodeSetOrderingReset()
  {
    if(context_) {
      context_->setNodeSetOrdering(ordering_);
    }
  }

protected:
  StaticContext* context_;
  StaticContext::NodeSetOrdering ordering_;  
};

class XQILLA_API AutoContextItemTypeReset
{
public:
  AutoContextItemTypeReset(StaticContext* context)
  {
    context_ = context;
    if(context_) {
      sType_ = context->getContextItemType();
    }
  }

  AutoContextItemTypeReset(StaticContext* context, const StaticType &sType)
  {
    context_ = context;
    if(context_) {
      sType_ = context->getContextItemType();
      context->setContextItemType(sType);
    }
  }

  ~AutoContextItemTypeReset()
  {
    if(context_) {
      context_->setContextItemType(sType_);
    }
  }

protected:
  StaticContext* context_;
  StaticType sType_;
};

class XQILLA_API AutoNsScopeReset
{
public:
  AutoNsScopeReset(StaticContext* context, XERCES_CPP_NAMESPACE_QUALIFIER DOMXPathNSResolver* newResolver)
  {
    context_=context;
    if(context_) {
      _oldNSResolver=context_->getNSResolver();
      _defaultElementAndTypeNS=context->getDefaultElementAndTypeNS();
      context_->setNSResolver(newResolver);
    }
  }

  ~AutoNsScopeReset()
  {
    if(context_) {
      context_->setNSResolver(_oldNSResolver);
      context_->setDefaultElementAndTypeNS(_defaultElementAndTypeNS);
    }
  }

protected:
  StaticContext* context_;
  const XERCES_CPP_NAMESPACE_QUALIFIER DOMXPathNSResolver* _oldNSResolver;
  const XMLCh *_defaultElementAndTypeNS;
  
};

class XQILLA_API AutoContextInfoReset
{
public:
  AutoContextInfoReset(DynamicContext *context)
    : oldContextItem(context->getContextItem()),
      oldContextPosition(context->getContextPosition()),
      oldContextSize(context->getContextSize()),
      context_(context)
  {
  }

  AutoContextInfoReset(DynamicContext *context, const Item::Ptr &contextItem, size_t contextPosition = 0, size_t contextSize = 0)
    : oldContextItem(context->getContextItem()),
      oldContextPosition(context->getContextPosition()),
      oldContextSize(context->getContextSize()),
      context_(context)
  {
    context->setContextItem(contextItem);
    context->setContextPosition(contextPosition);
    context->setContextSize(contextSize);
  }

  ~AutoContextInfoReset()
  {
    resetContextInfo();
  }

  void resetContextInfo()
  {
    context_->setContextItem(oldContextItem);
    context_->setContextPosition(oldContextPosition);
    context_->setContextSize(oldContextSize);
  }

  Item::Ptr oldContextItem;
  size_t oldContextPosition;
  size_t oldContextSize;

private:
  DynamicContext* context_;
};

class XQILLA_API AutoDocumentCacheReset
{
public:
  AutoDocumentCacheReset(DynamicContext* context)
    : oldDC(const_cast<DocumentCache*>(context->getDocumentCache())),
      context_ (context)
  {
  }

  ~AutoDocumentCacheReset()
  {
    context_->setDocumentCache(oldDC);
  }

  DocumentCache *oldDC;

protected:
  DynamicContext* context_;
};

class XQILLA_API AutoVariableStoreReset
{
public:
  AutoVariableStoreReset(DynamicContext *context, const VariableStore *store = 0)
  {
    context_ = context;
    _oldVarStore = context_->getVariableStore();
    if(store)
      context_->setVariableStore(store);
  }

  ~AutoVariableStoreReset()
  {
    context_->setVariableStore(_oldVarStore);
  }

  void reset()
  {
    context_->setVariableStore(_oldVarStore);
  }

protected:
  DynamicContext *context_;
  const VariableStore *_oldVarStore;
};

class XQILLA_API AutoRegexGroupStoreReset
{
public:
  AutoRegexGroupStoreReset(DynamicContext *context, const RegexGroupStore *store = 0)
  {
    context_ = context;
    _oldRegexStore = context_->getRegexGroupStore();
    if(store)
      context_->setRegexGroupStore(store);
  }

  ~AutoRegexGroupStoreReset()
  {
    context_->setRegexGroupStore(_oldRegexStore);
  }

  void reset()
  {
    context_->setRegexGroupStore(_oldRegexStore);
  }

protected:
  DynamicContext *context_;
  const RegexGroupStore *_oldRegexStore;
};

class XQILLA_API AutoMessageListenerReset
{
public:
  AutoMessageListenerReset(StaticContext* context, MessageListener *listener = 0)
  {
    context_ = context;
    if(context_) {
      listener_ = context->getMessageListener();
      context->setMessageListener(listener);
    }
  }

  ~AutoMessageListenerReset()
  {
    if(context_) {
      context_->setMessageListener(listener_);
    }
  }

protected:
  StaticContext* context_;
  MessageListener *listener_;  
};

class XQILLA_API AutoStackFrameReset
{
public:
  AutoStackFrameReset(DynamicContext *context, const StackFrame *frame)
  {
    context_ = context;
    _oldFrame = context_->getStackFrame();
    context_->setStackFrame(frame);
  }

  ~AutoStackFrameReset()
  {
    context_->setStackFrame(_oldFrame);
  }

  void reset()
  {
    context_->setStackFrame(_oldFrame);
  }

protected:
  DynamicContext *context_;
  const StackFrame *_oldFrame;
};

template<typename T> class XQILLA_API AutoReset
{
public:
  AutoReset(T &orig)
    : orig_(orig)
  {
    old_ = orig;
  }

  ~AutoReset()
  {
    reset();
  }

  void reset()
  {
    orig_ = old_;
  }

protected:
  T &orig_;
  T old_;
};

#endif

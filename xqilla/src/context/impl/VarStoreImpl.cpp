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
#include <xqilla/context/impl/VarStoreImpl.hpp>
#include <xqilla/runtime/Result.hpp>
#include <xqilla/utils/XPath2Utils.hpp>
#include <xqilla/context/DynamicContext.hpp>

XERCES_CPP_NAMESPACE_USE;
using namespace std;

VarStoreImpl::VarStoreImpl(XPath2MemoryManager *mm, const VariableStore *parent)
  : store_(0),
    parent_(parent),
    mm_(mm)
{
}

VarStoreImpl::VarStoreImpl(const VarStoreImpl &other, XPath2MemoryManager *mm)
  : store_(0),
    parent_(other.parent_),
    mm_(mm)
{
  VarEntry *entry = other.store_;
  while(entry) {
    store_ = new VarEntry(entry->uri, entry->name, entry->buffer, store_);
    entry = entry->prev;
  }
}

VarStoreImpl::~VarStoreImpl()
{
  clear();
}

Result VarStoreImpl::getVar(const XMLCh *namespaceURI, const XMLCh *name) const
{
  VarEntry *entry = store_;
  while(entry) {
    if(XPath2Utils::equals(name, entry->name) &&
       XPath2Utils::equals(namespaceURI, entry->uri)) {
      return entry->buffer.createResult();
    }

    entry = entry->prev;
  }

  if(parent_ != 0) return parent_->getVar(namespaceURI, name);

  return 0;
}

void VarStoreImpl::getInScopeVariables(vector<pair<const XMLCh*, const XMLCh*> > &variables) const
{
  VarEntry *entry = store_;
  while(entry) {
    variables.push_back(pair<const XMLCh*, const XMLCh*>(entry->uri, entry->name));
    entry = entry->prev;
  }

  if(parent_ != 0) parent_->getInScopeVariables(variables);
}

void VarStoreImpl::setVar(const XMLCh *namespaceURI, const XMLCh *name, const Result &value, unsigned int readCount)
{
  VarEntry *entry = store_;
  while(entry) {
    if(XPath2Utils::equals(name, entry->name) &&
       XPath2Utils::equals(namespaceURI, entry->uri)) {
      entry->buffer = ResultBuffer(value, readCount);
      return;
    }

    entry = entry->prev;
  }

  store_ = new VarEntry(mm_->getPooledString(namespaceURI), mm_->getPooledString(name),
                        value, readCount, store_);
}

void VarStoreImpl::clear()
{
  VarEntry *tmp;
  while(store_) {
    tmp = store_;
    store_ = store_->prev;
    delete tmp;
  }
}

void VarStoreImpl::cacheVariableStore(const StaticAnalysis &src, const VariableStore *toCache)
{
  for(int i = 0; i < StaticAnalysis::HASH_SIZE; ++i) {
    StaticAnalysis::VarEntry *entry = src.variablesUsed()[i];
    while(entry) {
      store_ = new VarEntry(entry->uri, entry->name, toCache->getVar(entry->uri, entry->name),
                            ResultBufferImpl::UNLIMITED_COUNT, store_);
      entry = entry->prev;
    }
  }
  if (parent_ == 0) parent_ = toCache;
}

VarStoreImpl::VarEntry::VarEntry(const XMLCh *u, const XMLCh *n, const Result &r, unsigned int readCount, VarEntry *p)
  : uri(u),
    name(n),
    buffer(r, readCount),
    prev(p)
{
}

VarStoreImpl::VarEntry::VarEntry(const XMLCh *u, const XMLCh *n, const ResultBuffer &buffer, VarEntry *p)
  : uri(u),
    name(n),
    buffer(buffer),
    prev(p)
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////

SingleVarStore::SingleVarStore() : uri_(0), name_(0), parent_(0) {}

Result SingleVarStore::getVar(const XMLCh *namespaceURI, const XMLCh *name) const
{
  if(XPath2Utils::equals(name, name_) && XPath2Utils::equals(namespaceURI, uri_))
    return value;

  return parent_->getVar(namespaceURI, name);
}

void SingleVarStore::getInScopeVariables(vector<pair<const XMLCh*, const XMLCh*> > &variables) const
{
  variables.push_back(pair<const XMLCh*, const XMLCh*>(uri_, name_));

  if(parent_ != 0) parent_->getInScopeVariables(variables);
}

void SingleVarStore::setAsVariableStore(const XMLCh *namespaceURI, const XMLCh *name, DynamicContext *context)
{
  uri_ = namespaceURI;
  name_ = name;

  parent_ = context->getVariableStore();
  context->setVariableStore(this);
}

void SingleVarStore::unsetAsVariableStore(DynamicContext *context)
{
  context->setVariableStore(parent_);
}

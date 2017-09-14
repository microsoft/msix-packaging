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

#ifndef _VARSTOREIMPL_HPP
#define _VARSTOREIMPL_HPP

#include <xqilla/context/VariableStore.hpp>
#include <xqilla/runtime/Sequence.hpp>
#include <xqilla/runtime/ResultBuffer.hpp>

#include <xercesc/util/StringPool.hpp>
#include <xercesc/util/RefHash2KeysTableOf.hpp>

class DynamicContext;
class StaticAnalysis;

/** Default implementation of VariableStore that stores variable values in a hash map, and
    proxies calls to a parent VariableStore if the value is not found in it's map. */
class XQILLA_API VarStoreImpl : public VariableStore
{
public:
  VarStoreImpl(XPath2MemoryManager *mm, const VariableStore *parent = 0);
  VarStoreImpl(const VarStoreImpl &other, XPath2MemoryManager *mm);
  ~VarStoreImpl();

  virtual Result getVar(const XMLCh *namespaceURI, const XMLCh *name) const;
  virtual void getInScopeVariables(std::vector<std::pair<const XMLCh*, const XMLCh*> > &variables) const;

  void setVar(const XMLCh *namespaceURI, const XMLCh *name, const Result &value, unsigned int readCount = ResultBufferImpl::UNLIMITED_COUNT);
  void clear();

  void cacheVariableStore(const StaticAnalysis &src, const VariableStore *toCache);

private:
  class VarEntry {
  public:
    VarEntry(const XMLCh *uri, const XMLCh *name, const Result &result, unsigned int readCount, VarEntry *prev);
    VarEntry(const XMLCh *uri, const XMLCh *name, const ResultBuffer &buffer, VarEntry *prev);

    const XMLCh *uri, *name;
    ResultBuffer buffer;
    VarEntry *prev;
  };

  VarEntry *store_;

  const VariableStore *parent_;
  XPath2MemoryManager *mm_;
};

class XQILLA_API SingleVarStore : public VariableStore
{
public:
  SingleVarStore();

  virtual Result getVar(const XMLCh *namespaceURI, const XMLCh *name) const;
  virtual void getInScopeVariables(std::vector<std::pair<const XMLCh*, const XMLCh*> > &variables) const;

  void setAsVariableStore(const XMLCh *namespaceURI, const XMLCh *name, DynamicContext *context);
  void unsetAsVariableStore(DynamicContext *context);

  Sequence value;

private:
  const XMLCh *uri_, *name_;
  const VariableStore *parent_;
};

#endif




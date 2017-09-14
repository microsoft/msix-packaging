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

#ifndef _EXTERNALFUNCTION_HPP
#define _EXTERNALFUNCTION_HPP

#include <xqilla/framework/XQillaExport.hpp>

#include <xercesc/util/XMLUniDefs.hpp>

class Result;
class PendingUpdateList;
class DynamicContext;
class XPath2MemoryManager;

class XQILLA_API ExternalFunction
{
public:
  class XQILLA_API Arguments
  {
  public:
    virtual ~Arguments() {}

    /// Return the result for the argument index specified. Arguments indexes are zero based.
    virtual Result getArgument(size_t index, DynamicContext *context) const = 0;
  };

  virtual ~ExternalFunction() {}

  const XMLCh *getURI() const { return uri_; }
  const XMLCh *getName() const { return name_; }
  const XMLCh *getURINameHash() const { return uriName_; }
  unsigned int getNumberOfArguments() const { return numArgs_; }

  /// Execute the (non-updating) function
  virtual Result execute(const Arguments *args, DynamicContext *context) const;
  /// Execute the updating function
  virtual PendingUpdateList executeUpdate(const Arguments *args, DynamicContext *context) const;

protected:
  ExternalFunction(const XMLCh *uri, const XMLCh *name, unsigned int numberOfArgs, XPath2MemoryManager *mm);

  const XMLCh *uri_;
  const XMLCh *name_;
  const XMLCh *uriName_;
  unsigned int numArgs_;
};

#endif


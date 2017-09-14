/*
 * Copyright (c) 2001-2008
 *     DecisionSoft Limited. All rights reserved.
 * Copyright (c) 2004-2009
 *     Oracle. All rights reserved.
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

#ifndef _DELAYEDMODULE_HPP
#define _DELAYEDMODULE_HPP

#include <xqilla/framework/XQillaExport.hpp>

#include <xercesc/framework/MemoryManager.hpp>
#include <xercesc/util/PlatformUtils.hpp>

class XQQuery;
class DynamicContext;
class ModuleCache;

class XQILLA_API DelayedModule
{
public:
  class XQILLA_API FuncDef
  {
  public:
    const char *name;
    unsigned int args;
    bool isPrivate;
    int line, column;
    const char *body;
  };

  const XMLCh *file;
  const XMLCh *prefix;
  const XMLCh *uri;
  const FuncDef *functions;

  void importModuleInto(XQQuery *importer) const;
};

#endif

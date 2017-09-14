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

#include <xqilla/utils/DelayedModule.hpp>
#include <xqilla/simple-api/XQilla.hpp>
#include <xqilla/simple-api/XQQuery.hpp>
#include <xqilla/context/DynamicContext.hpp>
#include <xqilla/functions/BuiltInModules.hpp>

XERCES_CPP_NAMESPACE_USE;

void DelayedModule::importModuleInto(XQQuery *importer) const
{
  XQQuery *module = importer->getModuleCache()->getByNamespace(uri);

  if(module == 0) {
    // Create the module
    const StaticContext *scontext = importer->getStaticContext();

    DynamicContext *context = scontext->createModuleContext();
    context->setNamespaceBinding(prefix, uri);

    module = new (scontext->getMemoryManager())
      XQQuery(context, true, importer->getModuleCache(), scontext->getMemoryManager());
    AutoDelete<XQQuery> guard(module);

    module->setIsLibraryModule(true);
    module->setModuleTargetNamespace(uri);
    module->setFile(file);

    XPath2MemoryManager *mm = context->getMemoryManager();

    for(const FuncDef *ptr = functions; ptr->name != 0; ++ptr) {
      module->addDelayedFunction(uri, mm->getPooledString(ptr->name), ptr->args,
                                 ptr->body, ptr->isPrivate,
                                 ptr->line, ptr->column);
    }

    importer->getModuleCache()->put(guard.adopt());

    BuiltInModules::core.importModuleInto(module);
    BuiltInModules::fn.importModuleInto(module);
  }

  importer->importModule(module);
}

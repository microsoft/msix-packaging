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

#include "../config/xqilla_config.h"
#include <sstream>

#include <xqilla/ast/StaticAnalysis.hpp>
#include <xqilla/context/DynamicContext.hpp>
#include <xqilla/utils/XPath2NSUtils.hpp>
#include <xqilla/utils/UTF8Str.hpp>

XERCES_CPP_NAMESPACE_USE;
using namespace std;

StaticAnalysis::StaticAnalysis(XPath2MemoryManager* memMgr)
  : _recycle(0),
    _memMgr(memMgr)
{
  memset(_dynamicVariables, 0, sizeof(_dynamicVariables));
  clear();
}

StaticAnalysis::StaticAnalysis(const StaticAnalysis &o, XPath2MemoryManager* memMgr)
  : _recycle(0),
    _memMgr(memMgr)
{
  memset(_dynamicVariables, 0, sizeof(_dynamicVariables));
  clear();
  copy(o);
}

void StaticAnalysis::copy(const StaticAnalysis &o)
{
  add(o);
  _properties = o._properties;
  _staticType = o._staticType;
}

void StaticAnalysis::release()
{
  _staticType = StaticType();
  VarEntry *tmp;
  for(int i = 0; i < HASH_SIZE; ++i) {
    while(_dynamicVariables[i]) {
      tmp = _dynamicVariables[i];
      _dynamicVariables[i] = tmp->prev;
      _memMgr->deallocate(tmp);
    }
  }
  while(_recycle) {
    tmp = _recycle;
    _recycle = tmp->prev;
    _memMgr->deallocate(tmp);
  }
}

void StaticAnalysis::clear()
{
  clearExceptType();
  _staticType = StaticType();
}

void StaticAnalysis::clearExceptType()
{
  _contextItem = false;
  _contextPosition = false;
  _contextSize = false;
  _currentTime = false;
  _implicitTimezone = false;
  _availableDocuments = false;
  _availableCollections = false;
  _forceNoFolding = false;
  _creative = false;
  _updating = false;
  _possiblyUpdating = false;

  _properties = 0;

  VarEntry *tmp;
  for(int i = 0; i < HASH_SIZE; ++i) {
    while(_dynamicVariables[i]) {
      tmp = _dynamicVariables[i];
      _dynamicVariables[i] = tmp->prev;
      tmp->prev = _recycle;
      _recycle = tmp;
    }
  }
}

void StaticAnalysis::contextItemUsed(bool value)
{
  _contextItem = value;
}

void StaticAnalysis::contextPositionUsed(bool value)
{
  _contextPosition = value;
}

void StaticAnalysis::contextSizeUsed(bool value)
{
  _contextSize = value;
}

bool StaticAnalysis::isContextItemUsed() const
{
  return _contextItem;
}

bool StaticAnalysis::isContextPositionUsed() const
{
  return _contextPosition;
}

bool StaticAnalysis::isContextSizeUsed() const
{
  return _contextSize;
}

/** Returns true if any of the context item flags have been used */
bool StaticAnalysis::areContextFlagsUsed() const
{
  return _contextItem || _contextPosition || _contextSize;
}

void StaticAnalysis::currentTimeUsed(bool value)
{
  _currentTime = value;
}

void StaticAnalysis::implicitTimezoneUsed(bool value)
{
  _implicitTimezone = value;
}

void StaticAnalysis::availableDocumentsUsed(bool value)
{
  _availableDocuments = value;
}

void StaticAnalysis::availableCollectionsUsed(bool value)
{
  _availableCollections = value;
}

bool StaticAnalysis::areDocsOrCollectionsUsed() const
{
  return _availableDocuments || _availableCollections;
}

void StaticAnalysis::forceNoFolding(bool value)
{
  _forceNoFolding = value;
}

bool StaticAnalysis::isNoFoldingForced() const
{
  return _forceNoFolding;
}

#define CREATE_VAR_ENTRY(entry) \
do { \
  if(_recycle) { \
    (entry) = _recycle; \
    _recycle = (entry)->prev; \
  } \
  else { \
    (entry) = new (_memMgr) VarEntry(); \
  } \
} while(0)


#define ADD_VAR_ENTRY(entry) \
do { \
  VarEntry *ve = _dynamicVariables[(entry)->hash]; \
  while(ve) { \
    if(ve->uri == (entry)->uri && ve->name == (entry)->name) \
      break; \
    ve = ve->prev; \
  } \
  if(ve) { \
    (entry)->prev = _recycle; \
    _recycle = (entry); \
  } \
  else { \
    (entry)->prev = _dynamicVariables[(entry)->hash]; \
    _dynamicVariables[(entry)->hash] = (entry); \
  } \
} while(0)

void StaticAnalysis::variableUsed(const XMLCh *namespaceURI, const XMLCh *name)
{
  VarEntry *entry;
  CREATE_VAR_ENTRY(entry);

  entry->set(_memMgr->getPooledString(namespaceURI), _memMgr->getPooledString(name));

  ADD_VAR_ENTRY(entry);
}

StaticAnalysis::VarEntry **StaticAnalysis::variablesUsed() const
{
  return (VarEntry**)_dynamicVariables;
}

bool StaticAnalysis::removeVariable(const XMLCh *namespaceURI, const XMLCh *name)
{
  VarEntry lookup;
  lookup.set(_memMgr->getPooledString(namespaceURI), _memMgr->getPooledString(name));

  VarEntry **parent = &_dynamicVariables[lookup.hash];
  while(*parent) {
    if((*parent)->uri == lookup.uri && (*parent)->name == lookup.name) {
      VarEntry *tmp = *parent;
      *parent = tmp->prev;
      tmp->prev = _recycle;
      _recycle = tmp;
      return true;
    } else {
      parent = &(*parent)->prev;
    }
  }

  return false;
}

bool StaticAnalysis::isVariableUsed(const XMLCh *namespaceURI, const XMLCh *name) const
{
  VarEntry lookup;
  lookup.set(_memMgr->getPooledString(namespaceURI), _memMgr->getPooledString(name));

  VarEntry *entry = _dynamicVariables[lookup.hash];
  while(entry) {
    if(entry->uri == lookup.uri && entry->name == lookup.name) {
      return true;
    }
    entry = entry->prev;
  }

  return false;
}

bool StaticAnalysis::isVariableUsed() const
{
  for(int i = 0; i < HASH_SIZE; ++i) {
    if(_dynamicVariables[i]) return true;
  }
  return false;
}

/** Sets the members of this StaticAnalysis from the given StaticAnalysis */
void StaticAnalysis::add(const StaticAnalysis &o)
{
  if(o._contextItem) _contextItem = true;
  if(o._contextPosition) _contextPosition = true;
  if(o._contextSize) _contextSize = true;
  if(o._currentTime) _currentTime = true;
  if(o._implicitTimezone) _implicitTimezone = true;
  if(o._availableDocuments) _availableDocuments = true;
  if(o._availableCollections) _availableCollections = true;
  if(o._forceNoFolding) _forceNoFolding = true;
  if(o._creative) _creative = true;
  if(o._updating) _updating = true;
  // Don't copy _possiblyUpdating

  for(int i = 0; i < HASH_SIZE; ++i) {
    VarEntry *entry = o._dynamicVariables[i];
    while(entry) {
      VarEntry *newEntry;
      CREATE_VAR_ENTRY(newEntry);
      newEntry->set(entry->uri, entry->name, entry->hash);
      ADD_VAR_ENTRY(newEntry);
      entry = entry->prev;
    }
  }
}

void StaticAnalysis::addExceptContextFlags(const StaticAnalysis &o)
{
  if(o._currentTime) _currentTime = true;
  if(o._implicitTimezone) _implicitTimezone = true;
  if(o._availableDocuments) _availableDocuments = true;
  if(o._availableCollections) _availableCollections = true;
  if(o._forceNoFolding) _forceNoFolding = true;
  if(o._creative) _creative = true;
  if(o._updating) _updating = true;
  // Don't copy _possiblyUpdating

  for(int i = 0; i < HASH_SIZE; ++i) {
    VarEntry *entry = o._dynamicVariables[i];
    while(entry) {
      VarEntry *newEntry;
      CREATE_VAR_ENTRY(newEntry);
      newEntry->set(entry->uri, entry->name, entry->hash);
      ADD_VAR_ENTRY(newEntry);
      entry = entry->prev;
    }
  }
}

void StaticAnalysis::addExceptVariable(const XMLCh *namespaceURI, const XMLCh *name, const StaticAnalysis &o)
{
  namespaceURI = _memMgr->getPooledString(namespaceURI);
  name = _memMgr->getPooledString(name);

  if(o._contextItem) _contextItem = true;
  if(o._contextPosition) _contextPosition = true;
  if(o._contextSize) _contextSize = true;
  if(o._currentTime) _currentTime = true;
  if(o._implicitTimezone) _implicitTimezone = true;
  if(o._availableDocuments) _availableDocuments = true;
  if(o._availableCollections) _availableCollections = true;
  if(o._forceNoFolding) _forceNoFolding = true;
  if(o._creative) _creative = true;
  if(o._updating) _updating = true;
  // Don't copy _possiblyUpdating

  for(int i = 0; i < HASH_SIZE; ++i) {
    VarEntry *entry = o._dynamicVariables[i];
    while(entry) {
      if(namespaceURI != entry->uri || name != entry->name) {
        VarEntry *newEntry;
        CREATE_VAR_ENTRY(newEntry);
        newEntry->set(entry->uri, entry->name, entry->hash);
        ADD_VAR_ENTRY(newEntry);
      }
      entry = entry->prev;
    }
  }
}

/** Returns true if flags are set, or variables have been used */
bool StaticAnalysis::isUsed() const
{
  return _contextItem || _contextPosition || _contextSize
    || _currentTime || _implicitTimezone || _availableCollections
    || _availableDocuments || _forceNoFolding || _creative
    || isVariableUsed();
}

bool StaticAnalysis::isUsedExceptContextFlags() const
{
  return _currentTime || _implicitTimezone || _availableCollections
    || _availableDocuments || _forceNoFolding || _creative
    || isVariableUsed();
}

void StaticAnalysis::creative(bool value)
{
  _creative = value;
}

bool StaticAnalysis::isCreative() const
{
  return _creative;
}

void StaticAnalysis::updating(bool value)
{
  _updating = value;
}

bool StaticAnalysis::isUpdating() const
{
  return _updating;
}

void StaticAnalysis::possiblyUpdating(bool value)
{
  _possiblyUpdating = value;
}

bool StaticAnalysis::isPossiblyUpdating() const
{
  return _possiblyUpdating;
}

unsigned int StaticAnalysis::getProperties() const
{
	return _properties;
}

void StaticAnalysis::setProperties(unsigned int props)
{
	_properties = props;
}

const StaticType &StaticAnalysis::getStaticType() const
{
	return _staticType;
}

StaticType &StaticAnalysis::getStaticType()
{
	return _staticType;
}

std::string StaticAnalysis::toString() const
{
  std::ostringstream s;

  s << "Context Item:          " << (_contextItem ? "true" : "false") << std::endl;
  s << "Context Position:      " << (_contextPosition ? "true" : "false") << std::endl;
  s << "Context Size:          " << (_contextSize ? "true" : "false") << std::endl;
  s << "Current Time:          " << (_currentTime ? "true" : "false") << std::endl;
  s << "Implicit Timezone:     " << (_implicitTimezone ? "true" : "false") << std::endl;
  s << "Available Documents:   " << (_availableDocuments ? "true" : "false") << std::endl;
  s << "Available Collections: " << (_availableCollections ? "true" : "false") << std::endl;
  s << "Force No Folding:      " << (_forceNoFolding ? "true" : "false") << std::endl;
  s << "Creative:              " << (_creative ? "true" : "false") << std::endl;
  s << "Updating:              " << (_updating ? "true" : "false") << std::endl;
  s << "Possibly Updating:     " << (_possiblyUpdating ? "true" : "false") << std::endl;

  s << "Variables Used:        [";
  bool first = true;
  for(int i = 0; i < HASH_SIZE; ++i) {
    VarEntry *entry = _dynamicVariables[i];
    while(entry) {
      if(first) {
        first = false;
      }
      else {
        s << ", ";
      }

      s << "{" << UTF8(entry->uri) << "}" << UTF8(entry->name);
      entry = entry->prev;
    }
  }
  s << "]" << std::endl;


  XMLBuffer buf;
  _staticType.typeToBuf(buf);
  s << "Static Type:           " << UTF8(buf.getRawBuffer()) << std::endl;

  return s.str();
}

void StaticAnalysis::VarEntry::set(const XMLCh *u, const XMLCh *n)
{
  uri = u;
  name = n;
  hash = 5381;
  prev = 0;

  if(u) {
    while(*u++)
      hash = ((hash << 5) + hash) + *u;
  }
  if(n) {
    while(*n++)
      hash = ((hash << 5) + hash) + *n;
  }

  hash %= HASH_SIZE;
}

void StaticAnalysis::VarEntry::set(const XMLCh *u, const XMLCh *n, size_t h)
{
  uri = u;
  name = n;
  hash = h;
  prev = 0;
}

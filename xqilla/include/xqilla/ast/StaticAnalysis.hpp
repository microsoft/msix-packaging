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

#ifndef _STATICANALYSIS_HPP
#define _STATICANALYSIS_HPP

#include <string>

#include <xqilla/framework/XQillaExport.hpp>
#include <xqilla/ast/StaticType.hpp>

#include <vector>

class XPath2MemoryManager;

/**
 * Records access to various parts of the context during static resolution.
 */
class XQILLA_API StaticAnalysis
{
public:
  static const int HASH_SIZE = 13;

  class XQILLA_API VarEntry
  {
  public:
    VarEntry() : uri(0), name(0), hash(0), prev(0) {}
    void set(const XMLCh *u, const XMLCh *n);
    void set(const XMLCh *u, const XMLCh *n, size_t h);

    const XMLCh *uri, *name;
    size_t hash;
    VarEntry *prev;
  };

  StaticAnalysis(XPath2MemoryManager* memMgr);
  StaticAnalysis(const StaticAnalysis &o, XPath2MemoryManager* memMgr);

  void copy(const StaticAnalysis &o);
  void release();

  /// Clears all the information in this StaticAnalysis
  void clear();
  void clearExceptType();

  /** Overrides all the other flags, and never allows this sub-expression
      to be constant folded. */
  void forceNoFolding(bool value);
  bool isNoFoldingForced() const;

  void contextItemUsed(bool value);
  void contextPositionUsed(bool value);
  void contextSizeUsed(bool value);
  bool isContextItemUsed() const;
  bool isContextPositionUsed() const;
  bool isContextSizeUsed() const;
  /** Returns true if any of the context item flags have been used */
  bool areContextFlagsUsed() const;

  void currentTimeUsed(bool value);
  void implicitTimezoneUsed(bool value);

  void availableDocumentsUsed(bool value);
  void availableCollectionsUsed(bool value);
  bool areDocsOrCollectionsUsed() const;

  void variableUsed(const XMLCh *namespaceURI, const XMLCh *name);
  bool removeVariable(const XMLCh *namespaceURI, const XMLCh *name);
  bool isVariableUsed(const XMLCh *namespaceURI, const XMLCh *name) const;
  bool isVariableUsed() const;
  VarEntry **variablesUsed() const;

  /** Sets the members of this StaticAnalysis from the given StaticAnalysis */
  void add(const StaticAnalysis &o);
  void addExceptContextFlags(const StaticAnalysis &o);
  void addExceptVariable(const XMLCh *namespaceURI, const XMLCh *name, const StaticAnalysis &o);

  /** Returns true if flags are set, or variables have been used */
  bool isUsed() const;
  bool isUsedExceptContextFlags() const;

  void creative(bool value);
  bool isCreative() const;

  void updating(bool value);
  bool isUpdating() const;
  void possiblyUpdating(bool value);
  bool isPossiblyUpdating() const;

  /**
   * Properties that allow optimisation regarding sorting or not.
   * The values are OR'd as flags, so they must be distinct bits
   */
  enum Properties {
    DOCORDER     = 0x001, ///< Results are returned in document order
    PEER         = 0x002, ///< Results do not appear in the descendants of other results
    SUBTREE      = 0x004, ///< Results are members of the set of descendants of the context node
    GROUPED      = 0x008, ///< Results are grouped by the document they come from
    SAMEDOC      = 0x010, ///< Results are from the same document as the context node
    ONENODE      = 0x020, ///< Only ever returns one node
    SELF         = 0x040, ///< Only ever returns the context node
    UNDEFINEDVAR = 0x080  ///< This is a variable that has been undefined
  };

  unsigned int getProperties() const;
  void setProperties(unsigned int props);

  const StaticType &getStaticType() const;
  StaticType &getStaticType();

  std::string toString() const;

private:
  StaticAnalysis(const StaticAnalysis &o);
  StaticAnalysis &operator=(const StaticAnalysis &o);

  bool _contextItem;
  bool _contextPosition;
  bool _contextSize;
  bool _currentTime;
  bool _implicitTimezone;
  bool _availableDocuments;
  bool _availableCollections;
  bool _forceNoFolding;
  bool _creative;
  bool _updating;
  bool _possiblyUpdating;

  unsigned int _properties;
  StaticType _staticType;

  VarEntry *_dynamicVariables[HASH_SIZE];
  VarEntry *_recycle;
  XPath2MemoryManager *_memMgr;
};

#endif

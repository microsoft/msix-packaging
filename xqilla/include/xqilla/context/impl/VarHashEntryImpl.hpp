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

#ifndef _VARHASHENTRYIMPL_HPP
#define _VARHASHENTRYIMPL_HPP

#include <xqilla/framework/XQillaExport.hpp>
#include <xqilla/runtime/Sequence.hpp>
#include <xqilla/context/VarHashEntry.hpp>

/** The class that stores the values of the variables. */
template<class TYPE>
class VarHashEntryImpl : public VarHashEntry<TYPE>
{
public:
  /// Meaningfull constructor
  VarHashEntryImpl(const TYPE &value);

  /** Gets the value of the variable (overload in derived classes for
      special behaviour) */
  virtual const TYPE &getValue() const;

  /** Sets the value of the variable (overload in derived classes for
      special behaviour) */
  virtual void setValue(const TYPE &value);

protected:
  TYPE _value;
};

template<class TYPE>
VarHashEntryImpl<TYPE>::VarHashEntryImpl(const TYPE &value)
  : _value(value)
{
}

template<class TYPE>
const TYPE &VarHashEntryImpl<TYPE>::getValue() const
{
  return _value;
}

template<class TYPE>
void VarHashEntryImpl<TYPE>::setValue(const TYPE &value)
{
  _value = value;
}

#endif


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

#ifndef _TZONE_HPP
#define _TZONE_HPP

#include <xqilla/framework/XQillaExport.hpp>
#include <xqilla/framework/ReferenceCounted.hpp>
#include <xqilla/items/ATDurationOrDerived.hpp>
#include <xqilla/mapm/m_apm.h>

#include <xercesc/util/XercesDefs.hpp>

class DynamicContext;

class XQILLA_API Timezone : public ReferenceCounted
{
public :
  typedef RefCountPointer<const Timezone> Ptr;

  /**
   * Constructor. Creates a Timezone representing the given number of seconds.
   */
  Timezone(const MAPM &seconds);

  /**
   * Constructor. Creates a Timezone representing the given duration.
   */
  Timezone(const ATDurationOrDerived::Ptr &duration, const DynamicContext* context);

  /** Returns true if other is equal to this, 
   * false otherwise 
   */
  bool equals(const Timezone::Ptr &other) const;

  ATDurationOrDerived::Ptr asDayTimeDuration(const DynamicContext* context) const;
  
  /**
   * Returns a string representation of this Timezone.  If positive, does
   * print leading '+' sign, and if UTC returns "Z".
   */
  const XMLCh* asString(const DynamicContext* context) const;

  /**
   * Returns the whole timezone expressed in seconds
   */
  const MAPM &asSeconds() const { return seconds_; }

  static MAPM convert(bool positive, int hour, int minute);

  static void validate(const MAPM &tz);

private:
  MAPM seconds_;
};

#endif // _TZONE_HPP



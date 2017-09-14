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
#include <assert.h>

#include <xqilla/items/Timezone.hpp>
#include <xqilla/exceptions/XPath2TypeCastException.hpp>
#include <xqilla/utils/XPath2Utils.hpp>
#include <xqilla/context/DynamicContext.hpp>
#include <xqilla/items/DatatypeFactory.hpp>
#include "../utils/DateUtils.hpp"
#include <xqilla/items/ATDecimalOrDerived.hpp>
#include <xqilla/items/ATDurationOrDerived.hpp>
#include <xqilla/context/ItemFactory.hpp>

#include <xercesc/util/XMLString.hpp>
#include <xercesc/util/XMLUniDefs.hpp>
#include <xercesc/framework/XMLBuffer.hpp>

static const int g_maxHour = 14;

Timezone::Timezone(const MAPM &seconds)
  : seconds_(seconds)
{
  validate(seconds_);
}

Timezone::Timezone(const ATDurationOrDerived::Ptr &duration, const DynamicContext* context)
  : seconds_(duration->asSeconds(context)->asMAPM())
{
  validate(seconds_);
}

MAPM Timezone::convert(bool positive, int hour, int minute)
{
  return (hour * DateUtils::g_secondsPerHour + minute * DateUtils::g_secondsPerMinute) * (positive ? +1 : -1);
}

void Timezone::validate(const MAPM &tz) {
  // Check that we have a valid timezone
  if(tz.abs() > (g_maxHour * DateUtils::g_secondsPerHour)) {
    XQThrow2(XPath2TypeCastException ,X("Timezone::Timezone"),
             X("Timezone outside of valid range created [err:FODT0003]."));
  }
  if(tz.rem(DateUtils::g_secondsPerMinute).sign() != 0) {
    XQThrow2(XPath2TypeCastException ,X("Timezone::Timezone"),
             X("Timezone must have an integral number of minutes [err:FODT0003]."));
  }
}

bool Timezone::equals(const Timezone::Ptr &other) const
{
  return seconds_ == other->seconds_;
}

ATDurationOrDerived::Ptr Timezone::asDayTimeDuration(const DynamicContext* context) const
{
  return context->getItemFactory()->createDayTimeDuration(seconds_, context);
}

const XMLCh* Timezone::asString(const DynamicContext* context) const {
  if(seconds_.sign() == 0)
    return XPath2Utils::asStr(XERCES_CPP_NAMESPACE_QUALIFIER chLatin_Z,context->getMemoryManager());

  MAPM hours, minutes;
  seconds_.integer_div_rem(DateUtils::g_secondsPerHour, hours, minutes);
  minutes = minutes.integer_divide(DateUtils::g_secondsPerMinute);

  XERCES_CPP_NAMESPACE_QUALIFIER XMLBuffer buffer(1023, context->getMemoryManager());
  if(seconds_.sign() > 0)
    buffer.append(XERCES_CPP_NAMESPACE_QUALIFIER chPlus);
  else {
    buffer.append(XERCES_CPP_NAMESPACE_QUALIFIER chDash);
    hours = hours.abs();
    minutes = minutes.abs();
  }
  DateUtils::formatNumber(hours,2,buffer);
  buffer.append(XERCES_CPP_NAMESPACE_QUALIFIER chColon);
  DateUtils::formatNumber(minutes,2,buffer);
  
  return context->getMemoryManager()->getPooledString(buffer.getRawBuffer());
}

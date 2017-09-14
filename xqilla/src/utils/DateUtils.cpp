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

#include <math.h>
#include <climits>
#include <stdio.h> // for sprintf
#if defined(WIN32) && !defined(__CYGWIN__)
#define snprintf _snprintf
#endif

#include "DateUtils.hpp"
#include <xercesc/framework/XMLBuffer.hpp>
#include <xercesc/util/XMLUniDefs.hpp>
#include <xqilla/framework/XPath2MemoryManager.hpp>
#include <xqilla/items/ATDateOrDerived.hpp>
#include <xqilla/items/ATDateTimeOrDerived.hpp>
#include <xqilla/items/Timezone.hpp>
#include <xqilla/items/DatatypeFactory.hpp>
#include <xqilla/context/ItemFactory.hpp>
#include <xqilla/utils/ContextUtils.hpp>
#include <xqilla/utils/XPath2Utils.hpp>
#include <xqilla/utils/NumUtils.hpp>
#include <xqilla/exceptions/XPath2TypeCastException.hpp>

#include <xercesc/util/Mutexes.hpp>

XERCES_CPP_NAMESPACE_USE;

const int DateUtils::g_secondsPerMinute = 60;
const int DateUtils::g_minutesPerHour = 60;
const int DateUtils::g_hoursPerDay = 24;
const int DateUtils::g_secondsPerHour = DateUtils::g_secondsPerMinute*DateUtils::g_minutesPerHour;
const int DateUtils::g_secondsPerDay = DateUtils::g_secondsPerHour*DateUtils::g_hoursPerDay;

void DateUtils::formatNumber(int value, int minDigits, XMLBuffer& buffer)
{
  bool bIsNegative=false;
  if(value<0)
  {
    bIsNegative=true;
    value=-value;
  }
  XMLCh tmpBuff[19];
  XMLString::binToText(value,tmpBuff,18,10);
  if(bIsNegative)
    buffer.append(chDash);
  for(int len = XPath2Utils::intStrlen(tmpBuff);len<minDigits;len++)
    buffer.append(chDigit_0);
  buffer.append(tmpBuff);
}

void DateUtils::formatNumber(const MAPM &value, int minDigits, XMLBuffer &buffer)
{
  char obuf[1024];
  value.toIntegerString(obuf);

  char *str = obuf;
  if(value.sign() < 0) {
    ++str;
    buffer.append(chDash);
  }

  size_t length = strlen(str);
  for(int i = (int)length; i < minDigits; ++i) {
    buffer.append(chDigit_0);
  }

  buffer.append(X(str));
}


int DateUtils::fQuotient( int a, int b) {
  if (b == 0)
    return 0;

  return (int)floor(a/(double)b);
}

int DateUtils::fQuotient( double a, double b) {
  if (b == 0)
    return 0;

  return (int)floor(a/(double)b);
}

int DateUtils::modulo( int a, int b)  {
   return a - fQuotient(a, b)*b;
}

double DateUtils::modulo( double a, double b) {
   return a - fQuotient(a, b)*b;
}

int DateUtils::fQuotient(int a, int low, int high ) {
    return fQuotient(a-low,high-low);
}

int DateUtils::modulo(int a, int low, int high ) {
    return modulo(a - low, high-low) + low;
}

// modulo(a,b) = a - (floor(a / b) * b)
MAPM DateUtils::modulo( MAPM a, MAPM b)  {
  return a - ((a/b).floor() * b);
}

int DateUtils::maximumDayInMonthFor(MAPM yearValue, MAPM monthValue) {
  return maximumDayInMonthFor(asInt(yearValue), asInt(monthValue));
}

int DateUtils::maximumDayInMonthFor(int year, int month) {
    int M = modulo(month,1,13);
    int Y = year + fQuotient(month,1,13);
  if ( M == 1 || M == 3 || M == 5 || M == 7 || M == 8 || M == 10 || M == 12 )
    return 31;
  else if ( M == 4 || M == 6 || M == 9 || M == 11 )
    return 30;
  else if ( M == 2 && modulo(Y,4) == 0 && ( modulo(Y,400) == 0 || modulo(Y,100) != 0 ) )
    return 29;
  else
    return 28;
}

bool DateUtils::isLeapYear(MAPM year)
{
  return modulo(year,4) == 0 && ( modulo(year,400) == 0 || modulo(year,100) != 0 );
}

int DateUtils::asInt(MAPM num) 
{
  if(num < INT_MIN || num > INT_MAX) {
    XQThrow2(XPath2TypeCastException, X("DateUtils::asInt"), X("Invalid representation of an int [err:FORG0001]"));
  } else {
    return NumUtils::MAPMtoInt(num);
  }
}

static int days_before_month[] =  { 0, 
                                    0+31, 
                                    0+31+28, 
                                    0+31+28+31, 
                                    0+31+28+31+30, 
                                    0+31+28+31+30+31, 
                                    0+31+28+31+30+31+30, 
                                    0+31+28+31+30+31+30+31, 
                                    0+31+28+31+30+31+30+31+31, 
                                    0+31+28+31+30+31+30+31+31+30, 
                                    0+31+28+31+30+31+30+31+31+30+31, 
                                    0+31+28+31+30+31+30+31+31+30+31+30
                                  };
static int days_before_month_leap[] = { 0, 
                                        0+31, 
                                        0+31+29, 
                                        0+31+29+31, 
                                        0+31+29+31+30, 
                                        0+31+29+31+30+31, 
                                        0+31+29+31+30+31+30, 
                                        0+31+29+31+30+31+30+31, 
                                        0+31+29+31+30+31+30+31+31, 
                                        0+31+29+31+30+31+30+31+31+30, 
                                        0+31+29+31+30+31+30+31+31+30+31, 
                                        0+31+29+31+30+31+30+31+31+30+31+30
                                      };

MAPM DateUtils::convertDMY2Absolute(MAPM day, MAPM month, MAPM year)
{
  MAPM prevYear = year - 1;
  if(year.sign() < 0) ++prevYear;
  MAPM absolute = ( prevYear * 365 ) + prevYear.integer_divide(4) - prevYear.integer_divide(100) +
    prevYear.integer_divide(400);

  if(isLeapYear(year))
    absolute+=days_before_month_leap[asInt(month)-1];
  else
    absolute+=days_before_month[asInt(month)-1];
  absolute+= day;
  return absolute - 1;
}

static int days_in_400_years = 400*365 + 400/4 - 400/100 + 400/400;
static int days_in_100_years = 100*365 + 100/4 - 100/100;
static int days_in_4_years   = 4*365 + 4/4;
static int days_in_1_years   = 1*365;

static inline int daysInYear(const MAPM &year)
{
  return DateUtils::isLeapYear(year) ? 366 : 365;
}

void DateUtils::convertAbsolute2DMY(MAPM absolute, MAPM& day, MAPM& month, MAPM& year)
{
  absolute += 1;

  bool bc = absolute.sign() <= 0;
  bool fix = false;

  MAPM div, rem;
  absolute.integer_div_rem(days_in_400_years, div, rem);
  year = div * 400;
  absolute = rem;

  absolute.integer_div_rem(days_in_100_years, div, rem);
  if(div <= -4) fix = true;
  if(div >= 4) {
    div -= 1;
    rem += days_in_100_years;
  }
  year += div * 100;
  absolute = rem;

  absolute.integer_div_rem(days_in_4_years, div, rem);
  year += div * 4;
  absolute = rem;

  absolute.integer_div_rem(days_in_1_years, div, rem);
  if(div <= -4) fix = true;
  if(div >= 4) {
    div -= 1;
    rem += days_in_1_years;
  }
  year += div;
  absolute = rem;

  if(bc) {
    if(fix && absolute.sign() == 0) {
      // Correct off by one error in year calculations
      // due to negative leap years
      absolute += 1;
    }
    else {
      --year;
      absolute += daysInYear(year);
    }
  }
  else {
    if(absolute.sign() != 0) {
      ++year;
    }
  }

  month = 12;
  day = 31;
  int *days = isLeapYear(year) ? days_before_month_leap : days_before_month;
  for(int i = 11; i >= 0; --i) {
    if(absolute > days[i]) {
      month = i + 1;
      day = absolute - days[i];
      break;
    }
  }
}

const ATDateOrDerived::Ptr DateUtils::getCurrentDate(const DynamicContext* context) 
{
  // We get the current time and adjust it to our timezone.  We then set
  // this timezone in the Date object.
  time_t curDate = context->getCurrentTime();
  // Note using localtime uses the tzset() function used by
  // DateUtils::getImplicitTimezone.  This function and getImplicitTimezone
  // MUST get the same value in order for the correct time to be stored.
  struct tm curLocalDate;
  threadsafe_localtime(&curDate, &curLocalDate);

  char szDate[256];
  snprintf(szDate, 256,"%04d-%02d-%02d",
           curLocalDate.tm_year+1900,
           curLocalDate.tm_mon+1,
           curLocalDate.tm_mday);

  const ATDateOrDerived::Ptr date = context->getItemFactory()->
    createDate(context->getMemoryManager()->getPooledString(szDate), context);
  return date->setTimezone(new Timezone(ContextUtils::getTimezone()), context);
}

const ATDateTimeOrDerived::Ptr DateUtils::getCurrentDateTime(const DynamicContext* context) 
{
  // We get the current time and adjust it to our timezone.  We then set
  // this timezone in the DateTime object.
  time_t curDate = context->getCurrentTime();
  // Note using localtime uses the tzset() function used by
  // DateUtils::getImplicitTimezone.  This function and getImplicitTimezone
  // MUST get the same value in order for the correct time to be stored.
  struct tm curLocalDate;
  threadsafe_localtime(&curDate, &curLocalDate);

  char szDate[256];
  snprintf(szDate, 256,"%04d-%02d-%02dT%02d:%02d:%02d",
           curLocalDate.tm_year+1900,
           curLocalDate.tm_mon+1,
           curLocalDate.tm_mday,
           curLocalDate.tm_hour,
           curLocalDate.tm_min,
           curLocalDate.tm_sec);

  const ATDateTimeOrDerived::Ptr dateTime = context->getItemFactory()->
    createDateTime(context->getMemoryManager()->getPooledString(szDate), context);
  return dateTime->setTimezone(new Timezone(ContextUtils::getTimezone()), context);
}

const ATTimeOrDerived::Ptr DateUtils::getCurrentTime(const DynamicContext* context) 
{
  time_t curDate = context->getCurrentTime();

  struct tm curLocalDate;
  threadsafe_localtime(&curDate, &curLocalDate);

  char szDate[256];
  snprintf(szDate, 256,"%02d:%02d:%02d",
           curLocalDate.tm_hour,
           curLocalDate.tm_min,
           curLocalDate.tm_sec);

  const ATTimeOrDerived::Ptr time = context->getItemFactory()->
    createTime(context->getMemoryManager()->getPooledString(szDate), context);
  return time->setTimezone(new Timezone(ContextUtils::getTimezone()), context);
}

static XMLMutex *time_mutex = 0;

void DateUtils::initialize()
{
  if(time_mutex == 0) {
    time_mutex = new XMLMutex();
  }
}

void DateUtils::terminate()
{
  delete time_mutex;
  time_mutex = 0;
}

struct tm *DateUtils::threadsafe_localtime(const time_t *timep, struct tm *result)
{
  XMLMutexLock lock(time_mutex);

  struct tm *tmp = ::localtime(timep);
  memcpy(result, tmp, sizeof(struct tm));
  return result;
}

struct tm *DateUtils::threadsafe_gmtime(const time_t *timep, struct tm *result)
{
  XMLMutexLock lock(time_mutex);

  struct tm *tmp = ::gmtime(timep);
  memcpy(result, tmp, sizeof(struct tm));
  return result;
}

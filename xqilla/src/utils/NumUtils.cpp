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
#include <xqilla/utils/NumUtils.hpp>
#include <stdlib.h>

int NumUtils::MAPMtoInt(MAPM m)
{
  /*
   * In theory no float unit can provide absolutely accuracy
   * (http://en.wikipedia.org/wiki/Floating_point#Accuracy_problems). Says the 
   * double value is 3, in computer the actual value might be 2.9999999996.
   * In this case, if we convert it to integer directly, some compilers would 
   * conver the value to "2", not expected "3".
   *
   * This phenomenon is not a bug but just due to float unit/arithmetic
   * accuracy limitation, especially on those imprecise float units like
   * embedded systems and float unit emulators. 
   *
   * To solve this accuracy issue, we round the double value with 0.5 before 
   * converting it to integer. This method is a common solution for safely
   * converting double value to integer. 
   */
  double x = m.toDouble();
  return (x >= 0 ? (int)(x + 0.5) : (int)(x - 0.5));
}   
    
double NumUtils::MAPMtoDouble(MAPM m)
{
  return m.toDouble();
}   

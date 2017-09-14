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
#include <xqilla/items/ATDurationOrDerived.hpp>
#include <xercesc/util/XMLUniDefs.hpp>

/* dayTimeDuration */
const XMLCh ATDurationOrDerived::fgDT_DAYTIMEDURATION[] =
{
    XERCES_CPP_NAMESPACE_QUALIFIER chLatin_d, XERCES_CPP_NAMESPACE_QUALIFIER chLatin_a, XERCES_CPP_NAMESPACE_QUALIFIER chLatin_y,
    XERCES_CPP_NAMESPACE_QUALIFIER chLatin_T, XERCES_CPP_NAMESPACE_QUALIFIER chLatin_i, XERCES_CPP_NAMESPACE_QUALIFIER chLatin_m, 
    XERCES_CPP_NAMESPACE_QUALIFIER chLatin_e, XERCES_CPP_NAMESPACE_QUALIFIER chLatin_D, XERCES_CPP_NAMESPACE_QUALIFIER chLatin_u, 
    XERCES_CPP_NAMESPACE_QUALIFIER chLatin_r, XERCES_CPP_NAMESPACE_QUALIFIER chLatin_a, XERCES_CPP_NAMESPACE_QUALIFIER chLatin_t, 
    XERCES_CPP_NAMESPACE_QUALIFIER chLatin_i, XERCES_CPP_NAMESPACE_QUALIFIER chLatin_o, XERCES_CPP_NAMESPACE_QUALIFIER chLatin_n, 
    XERCES_CPP_NAMESPACE_QUALIFIER chNull
};

/* 
 <xs:pattern value="[^YM]*[DT].*"/>
*/

const XMLCh ATDurationOrDerived::pattern_DT_DAYTIMEDURATION[] =
{
    XERCES_CPP_NAMESPACE_QUALIFIER chOpenSquare, XERCES_CPP_NAMESPACE_QUALIFIER chCaret,        XERCES_CPP_NAMESPACE_QUALIFIER chLatin_Y, 
    XERCES_CPP_NAMESPACE_QUALIFIER chLatin_M,    XERCES_CPP_NAMESPACE_QUALIFIER chCloseSquare,  XERCES_CPP_NAMESPACE_QUALIFIER chAsterisk, 
    XERCES_CPP_NAMESPACE_QUALIFIER chOpenSquare, XERCES_CPP_NAMESPACE_QUALIFIER chLatin_D,      XERCES_CPP_NAMESPACE_QUALIFIER chLatin_T, 
    XERCES_CPP_NAMESPACE_QUALIFIER chCloseSquare,XERCES_CPP_NAMESPACE_QUALIFIER chPeriod,       XERCES_CPP_NAMESPACE_QUALIFIER chAsterisk, 
    XERCES_CPP_NAMESPACE_QUALIFIER chNull
};

/* yearMonthDuration */
const XMLCh ATDurationOrDerived::fgDT_YEARMONTHDURATION[] = {
    XERCES_CPP_NAMESPACE_QUALIFIER chLatin_y, XERCES_CPP_NAMESPACE_QUALIFIER chLatin_e, XERCES_CPP_NAMESPACE_QUALIFIER chLatin_a, 
    XERCES_CPP_NAMESPACE_QUALIFIER chLatin_r, XERCES_CPP_NAMESPACE_QUALIFIER chLatin_M, XERCES_CPP_NAMESPACE_QUALIFIER chLatin_o, 
    XERCES_CPP_NAMESPACE_QUALIFIER chLatin_n, XERCES_CPP_NAMESPACE_QUALIFIER chLatin_t, XERCES_CPP_NAMESPACE_QUALIFIER chLatin_h,
    XERCES_CPP_NAMESPACE_QUALIFIER chLatin_D, XERCES_CPP_NAMESPACE_QUALIFIER chLatin_u, XERCES_CPP_NAMESPACE_QUALIFIER chLatin_r, 
    XERCES_CPP_NAMESPACE_QUALIFIER chLatin_a, XERCES_CPP_NAMESPACE_QUALIFIER chLatin_t, XERCES_CPP_NAMESPACE_QUALIFIER chLatin_i,
    XERCES_CPP_NAMESPACE_QUALIFIER chLatin_o, XERCES_CPP_NAMESPACE_QUALIFIER chLatin_n, XERCES_CPP_NAMESPACE_QUALIFIER chNull
};

/* 
 <xs:pattern value="[^DT]*"/>
*/

const XMLCh ATDurationOrDerived::pattern_DT_YEARMONTHDURATION[] =
{
    XERCES_CPP_NAMESPACE_QUALIFIER chOpenSquare, XERCES_CPP_NAMESPACE_QUALIFIER chCaret,        XERCES_CPP_NAMESPACE_QUALIFIER chLatin_D, 
    XERCES_CPP_NAMESPACE_QUALIFIER chLatin_T,    XERCES_CPP_NAMESPACE_QUALIFIER chCloseSquare,  XERCES_CPP_NAMESPACE_QUALIFIER chAsterisk, 
    XERCES_CPP_NAMESPACE_QUALIFIER chNull
};

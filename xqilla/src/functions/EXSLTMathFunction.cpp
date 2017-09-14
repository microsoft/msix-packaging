/*
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
#include <xqilla/functions/EXSLTMathFunction.hpp>

XERCES_CPP_NAMESPACE_USE

/* http://exslt.org/math */
const XMLCh EXSLTMathFunction::XMLChFunctionURI[] = {
  chLatin_h,
  chLatin_t,
  chLatin_t,
  chLatin_p,
  chColon,
  chForwardSlash,
  chForwardSlash,
  chLatin_e,
  chLatin_x,
  chLatin_s,
  chLatin_l,
  chLatin_t,
  chPeriod,
  chLatin_o,
  chLatin_r,
  chLatin_g,
  chForwardSlash,
  chLatin_m,
  chLatin_a,
  chLatin_t,
  chLatin_h,
  chNull
};

EXSLTMathFunction::EXSLTMathFunction(const XMLCh* name, const char *signature, const VectorOfASTNodes &args, XPath2MemoryManager* memMgr)
  : NumericFunction(name, signature, args, memMgr)
{ 
  uri_ = XMLChFunctionURI;
}


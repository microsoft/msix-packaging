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
#include <xqilla/functions/XQillaFunction.hpp>

#include <xercesc/validators/schema/SchemaSymbols.hpp>

#if defined(XERCES_HAS_CPP_NAMESPACE)
XERCES_CPP_NAMESPACE_USE
#endif

// xqilla
const XMLCh XQillaFunction::XQillaPrefix[] = {
	chLatin_x,
	chLatin_q,
	chLatin_i,
	chLatin_l,
	chLatin_l,
	chLatin_a,
	chNull
};

/* http://xqilla.sourceforge.net/Functions */
const XMLCh XQillaFunction::XMLChFunctionURI[] = {
	chLatin_h,
	chLatin_t,
	chLatin_t,
	chLatin_p,
	chColon,
	chForwardSlash,
	chForwardSlash,
	chLatin_x,
	chLatin_q,
	chLatin_i,
	chLatin_l,
	chLatin_l,
	chLatin_a,
	chPeriod,
	chLatin_s,
	chLatin_o,
	chLatin_u,
	chLatin_r,
	chLatin_c,
	chLatin_e,
	chLatin_f,
	chLatin_o,
	chLatin_r,
	chLatin_g,
	chLatin_e,
	chPeriod,
	chLatin_n,
	chLatin_e,
	chLatin_t,
	chForwardSlash,
	chLatin_F,
	chLatin_u,
	chLatin_n,
	chLatin_c,
	chLatin_t,
	chLatin_i,
	chLatin_o,
	chLatin_n,
	chLatin_s,
	chNull
};

XQillaFunction::XQillaFunction(const XMLCh* name, const char *signature, const VectorOfASTNodes &args, XPath2MemoryManager* memMgr)
  : XQFunction(name, signature, args, memMgr)
{
  uri_ = XMLChFunctionURI;
}


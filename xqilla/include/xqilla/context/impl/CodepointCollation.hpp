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

#ifndef CODEPOINTCOLLATION_HPP
#define CODEPOINTCOLLATION_HPP

#include <xqilla/framework/XQillaExport.hpp>
#include <xqilla/context/impl/CollationImpl.hpp>

class XPath2MemoryManager;

class XQILLA_API CodepointCollation : public CollationHelper {
public:
	CodepointCollation();

  static const XMLCh* getCodepointCollationName();
	virtual const XMLCh* getCollationName() const;
	virtual int compare(const XMLCh* string1, const XMLCh* string2) const;
};

#endif

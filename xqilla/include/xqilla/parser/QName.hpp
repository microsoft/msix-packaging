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

#ifndef _QNAME_HPP
#define _QNAME_HPP

#include <xqilla/framework/XQillaExport.hpp>
#include <xercesc/util/XercesDefs.hpp>
#include <xqilla/framework/XPath2MemoryManager.hpp>
#include <vector>

/*
Represents a Qualified name. Built in the parser.
 */
class XQILLA_API QualifiedName {

public:
	QualifiedName(const XMLCh* qualifiedName, XPath2MemoryManager* memMgr);
	// This constructor should be used on persistent strings (no memory manager involved)
	QualifiedName(const XMLCh* prefix, const XMLCh* name);
	QualifiedName(const XMLCh* fullName);
	~QualifiedName();

	const XMLCh* getPrefix() const;
	const XMLCh* getName() const;

	const XMLCh* getFullName(XPath2MemoryManager* memMgr) const;

private:
	const XMLCh* _prefix,*_name;
  bool _bDeleteStrings;
};

#endif


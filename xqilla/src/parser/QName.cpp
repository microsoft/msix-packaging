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
#include <xqilla/parser/QName.hpp>
#include <xqilla/utils/XPath2Utils.hpp>
#include <xqilla/framework/XPath2MemoryManager.hpp>
#include <xercesc/util/XMLUniDefs.hpp>
#include <xercesc/util/XMLString.hpp>
XERCES_CPP_NAMESPACE_USE;

//Parse qualifiedName into prefix and name
QualifiedName::QualifiedName(const XMLCh* qualifiedName, XPath2MemoryManager* memMgr) : _bDeleteStrings(false)
{
  if(qualifiedName && *qualifiedName)
  {
    int colonPos = XMLString::indexOf(qualifiedName, chColon, 0);
    if(colonPos == -1) {
      _name = memMgr->getPooledString(qualifiedName);
      _prefix = 0;
    }
    else {
      XMLCh* tempPrefix = new XMLCh[colonPos + 1];
      XMLString::subString(tempPrefix, qualifiedName, 0, colonPos);
      tempPrefix[colonPos] = 0;
      _prefix = memMgr->getPooledString(tempPrefix);
      delete [] tempPrefix;

      unsigned int length = XPath2Utils::uintStrlen(qualifiedName);
      XMLCh* tempName = new XMLCh[length - colonPos];
      XMLString::subString(tempName, qualifiedName, colonPos + 1, length);
      tempName[length - colonPos - 1] = 0;
      _name = memMgr->getPooledString(tempName);
      delete [] tempName;
    }
  }
  else
  {
    _name = XMLUni::fgZeroLenString;
    _prefix = 0;
  }
}

QualifiedName::QualifiedName(const XMLCh* qualifiedName) : _bDeleteStrings(true)
{
  if(qualifiedName && *qualifiedName)
  {
    int colonPos=XMLString::indexOf(qualifiedName, chColon, 0);
    if(colonPos == -1) {
      _name = XMLString::replicate(qualifiedName);
      _prefix = 0;
    }
    else
    {
      XMLCh* tempPrefix = new XMLCh[colonPos + 1];
      XMLString::subString(tempPrefix, qualifiedName, 0, colonPos);
      tempPrefix[colonPos] = 0;
      _prefix = XMLString::replicate(tempPrefix);
      delete [] tempPrefix;

      unsigned int length = XPath2Utils::uintStrlen(qualifiedName);
      XMLCh* tempName = new XMLCh[length - colonPos];
      XMLString::subString(tempName, qualifiedName, colonPos + 1, length);
      tempName[length - colonPos - 1] = 0;
      _name = XMLString::replicate(tempName);
      delete [] tempName;
    }
  }
  else
  {
    _name = XMLUni::fgZeroLenString;
    _prefix = 0;
    _bDeleteStrings = false;
  }
}

QualifiedName::QualifiedName(const XMLCh* prefix, const XMLCh* name) : _bDeleteStrings(false)
{
  _prefix = prefix;
  _name = name;
}

QualifiedName::~QualifiedName()
{
  if(_bDeleteStrings)
  {
      XMLString::release((XMLCh**)&_prefix);
      XMLString::release((XMLCh**)&_name);
  }
}

const XMLCh* QualifiedName::getPrefix() const {
	return _prefix;
}

const XMLCh* QualifiedName::getName() const {
	return _name;
}

const XMLCh* QualifiedName::getFullName(XPath2MemoryManager* memMgr) const {
	if(_prefix != NULL) {
		XMLCh szColon[2] = {chColon, chNull};
		return XPath2Utils::concatStrings(_prefix, szColon, _name, memMgr);
	}

	return memMgr->getPooledString(_name);
}

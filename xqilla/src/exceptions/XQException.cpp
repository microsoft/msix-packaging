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
#include <xqilla/exceptions/XQException.hpp>

#include <xercesc/util/XMLString.hpp>
#include <xqilla/utils/UTF8Str.hpp>
#include <xqilla/ast/LocationInfo.hpp>
#include <iostream>

// This is only for Windows, to force export of
// symbols for the two exceptions -- they are not used
// elsewhere within XQilla at this time
#include <xqilla/exceptions/QueryTimeoutException.hpp>
#include <xqilla/exceptions/QueryInterruptedException.hpp>
void __exceptionDummy() {
	try {
		// do something that won't be optimized away
		::strcmp("a","b");
	}
	catch (QueryTimeoutException &qte) {}
	catch (QueryInterruptedException &qie) {}
}

#if defined(XERCES_HAS_CPP_NAMESPACE)
XERCES_CPP_NAMESPACE_USE
#endif

XQException::XQException(const XMLCh *reason, const XMLCh* file, unsigned int line, unsigned int column, const char *cppFile, unsigned int cppLine)
  : type_(XMLString::transcode("XQException")),
    error_(XMLString::replicate(reason)),
    cppFunction_(XMLString::transcode("XQuery")),
    cppFile_(cppFile),
    cppLine_(cppLine),
    xqLine_(line),
    xqColumn_(column),
    xqFile_(XMLString::replicate(file))
{
}

XQException::XQException(const XMLCh* const type, const XMLCh* const functionName, const XMLCh* const reason, const LocationInfo *info, const char *cppFile, unsigned int cppLine)
  : type_(XMLString::replicate(type)),
    error_(XMLString::replicate(reason)),
    cppFunction_(XMLString::replicate(functionName)),
    cppFile_(cppFile),
    cppLine_(cppLine),
    xqLine_(0),
    xqColumn_(0),
    xqFile_(0)
{
  if(info != 0) setXQueryPosition(info);
}

XQException::XQException(const XQException &o)
  : type_(XMLString::replicate(o.type_)),
    error_(XMLString::replicate(o.error_)),
    cppFunction_(XMLString::replicate(o.cppFunction_)),
    cppFile_(o.cppFile_),
    cppLine_(o.cppLine_),
    xqLine_(o.xqLine_),
    xqColumn_(o.xqColumn_),
    xqFile_(XMLString::replicate(o.xqFile_))
{
}

XQException::~XQException()
{
  XMLString::release(&type_);
  XMLString::release(&error_);
  XMLString::release(&cppFunction_);
  XMLString::release(&xqFile_);
}

void XQException::setXQueryPosition(const XMLCh *file, unsigned int line, unsigned int column)
{
  XMLString::release(&xqFile_);
  xqFile_ = XMLString::replicate(file);
  xqLine_ = line;
  xqColumn_ = column;
}

void XQException::setXQueryPosition(const LocationInfo *info)
{
  XMLString::release(&xqFile_);
  xqFile_ = XMLString::replicate(info->getFile());
  xqLine_ = info->getLine();
  xqColumn_ = info->getColumn();
}

void XQException::printDebug(const XMLCh* const context) const
{
  std::cerr << std::endl;
  std::cerr << "===================" << std::endl;
  std::cerr << UTF8(context) << std::endl << std::endl;
  std::cerr << "Type: " << UTF8(type_) << std::endl;
  std::cerr << "Reason: " << UTF8(error_) << std::endl;
  std::cerr << "XQuery Location: " << UTF8(xqFile_) << ":" << xqLine_ << ":" << xqColumn_ << std::endl;
  std::cerr << "C++ Location: " << UTF8(cppFunction_) << ", " << cppFile_ << ":" << cppLine_ << std::endl;
  std::cerr << "===================" << std::endl;

}

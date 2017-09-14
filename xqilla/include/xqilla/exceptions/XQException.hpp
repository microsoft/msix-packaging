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

#ifndef XQEXCEPTION_HPP
#define XQEXCEPTION_HPP

#include <xqilla/framework/XQillaExport.hpp>
#include <xqilla/utils/XStr.hpp>

#define XQThrow(type, function, reason) throw type(function, reason, this, __FILE__, __LINE__)
#define XQThrow2(type, function, reason) throw type(function, reason, 0, __FILE__, __LINE__)
#define XQThrow3(type, function, reason, location) throw type(function, reason, location, __FILE__, __LINE__)
#define XQSimpleThrow(reason, xqfile, xqline, xqcolumn) throw XQException(reason, xqfile, xqline, (unsigned int)xqcolumn, __FILE__, __LINE__)

class LocationInfo;

class XQILLA_API XQException
{
public:
  XQException(const XMLCh *reason, const XMLCh* file, unsigned int line, unsigned int column, const char *cppFile, unsigned int cppLine);
  XQException(const XQException &);
  virtual ~XQException();

  const XMLCh *getType() const { return type_; }
  const XMLCh *getError() const { return error_; }

  const XMLCh *getXQueryFile() const { return xqFile_; }
  unsigned int getXQueryLine() const { return xqLine_; }
  unsigned int getXQueryColumn() const { return xqColumn_; }

  void setXQueryPosition(const XMLCh *file, unsigned int line, unsigned int column);
  void setXQueryPosition(const LocationInfo *info);

  const XMLCh *getCppFunction() const { return cppFunction_; }
  const char *getCppFile() const { return cppFile_; }
  unsigned int getCppLine() const { return cppLine_; }

  void printDebug(const XMLCh* const context) const;

protected:
  XQException(const XMLCh* const type, const XMLCh* const functionName, const XMLCh* const reason, const LocationInfo *info, const char *cppFile, unsigned int cppLine);

private:
  XQException &operator=(const XQException &);
  
  XMLCh* type_;
  XMLCh* error_;

  XMLCh* cppFunction_;
  const char *cppFile_;
  unsigned int cppLine_;

  unsigned int xqLine_, xqColumn_;
  XMLCh* xqFile_;
};

#endif


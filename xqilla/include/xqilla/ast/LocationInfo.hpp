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

#ifndef _LOCATIONINFO_HPP
#define _LOCATIONINFO_HPP


#include <xqilla/framework/XQillaExport.hpp>

#include <xercesc/util/XercesDefs.hpp>

/** A class that gives records a location in the query */
class XQILLA_API LocationInfo
{
public:
	/** Constructor that sets the information to 0 */
  LocationInfo()
    : file_(0), line_(0), column_(0) {}
  /** Constructor that takes the initial information */
  LocationInfo(const XMLCh *file, unsigned int line, unsigned int column)
    : file_(file), line_(line), column_(column) {}

  /** The file that the query was parsed from. This could be null. */
  const XMLCh *getFile() const { return file_; }
  /** The line in the query */
  unsigned int getLine() const { return line_; }
  /** The column in the query */
  unsigned int getColumn() const { return column_; }

  /** Sets the location to the given information */
  void setLocationInfo(const XMLCh *file, unsigned int line, unsigned int column)
  {
    file_ = file;
    line_ = line;
    column_ = column;
  }
  /** Sets the location with the information in the given LocationInfo object */
  void setLocationInfo(const LocationInfo *o)
  {
    if(o != 0) {
      file_ = o->file_;
      line_ = o->line_;
      column_ = o->column_;
    }
  }

private:
  const XMLCh *file_;
  unsigned int line_, column_;
};

#endif

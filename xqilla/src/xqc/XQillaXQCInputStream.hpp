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

#ifndef _XQILLAXQCINPUTSTREAM_HPP
#define _XQILLAXQCINPUTSTREAM_HPP

#include "XQillaXQCImplementation.hpp"

#include <xqilla/utils/XStr.hpp>

#include <xercesc/sax/InputSource.hpp>
#include <xercesc/util/BinInputStream.hpp>
#include "../config/xqilla_config.h"

class CharInputStream : public XERCES_CPP_NAMESPACE_QUALIFIER BinInputStream
{
public :
  CharInputStream(const char *string)
    : string_(string),
      len_(strlen(string)),
      pos_(0)
  {
  }

  virtual XercesFilePos curPos() const
  {
    return pos_;
  }

  virtual XercesSizeUint readBytes(XMLByte* const toFill, const XercesSizeUint maxToRead)
  {
    XercesSizeUint read = maxToRead;
    if(read > len_) read = len_;

    memcpy(toFill, string_, read);
    len_ -= read;
    pos_ += read;
    string_ += read;

    return read;
  }

  virtual const XMLCh* getContentType() const
  {
    return 0;
  }

private:
  const char *string_;
  XMLSize_t len_;
  XercesFilePos pos_;
};

class CharInputSource : public XERCES_CPP_NAMESPACE_QUALIFIER InputSource
{
public :
  CharInputSource(const char *string)
    : string_(string)
  {
    setEncoding(X("UTF-8"));
  }

  virtual XERCES_CPP_NAMESPACE_QUALIFIER BinInputStream* makeStream() const
  {
    return new CharInputStream(string_);
  }

private:
  const char *string_;
};

class FileInputStream : public XERCES_CPP_NAMESPACE_QUALIFIER BinInputStream
{
public :
  FileInputStream(FILE *file)
    : file_(file),
      pos_(0)
  {
  }

  virtual XercesFilePos curPos() const
  {
    return pos_;
  }

  virtual XercesSizeUint readBytes(XMLByte* const toFill, const XercesSizeUint maxToRead)
  {
    XercesSizeUint read = fread(toFill, 1, maxToRead, file_);
    pos_ += read;
    return read;
  }

  virtual const XMLCh* getContentType() const
  {
    return 0;
  }

private:
  FILE *file_;
  XercesFilePos pos_;
};

class FileInputSource : public XERCES_CPP_NAMESPACE_QUALIFIER InputSource
{
public :
  FileInputSource(FILE *file)
    : file_(file)
  {
  }

  virtual XERCES_CPP_NAMESPACE_QUALIFIER BinInputStream* makeStream() const
  {
    return new FileInputStream(file_);
  }

private:
  FILE *file_;
};

class BinXQCInputStream : public XERCES_CPP_NAMESPACE_QUALIFIER BinInputStream
{
public :
  BinXQCInputStream(XQC_InputStream *stream)
    : stream_(stream),
      pos_(0)
  {
  }

  virtual XercesFilePos curPos() const
  {
    return pos_;
  }

  virtual XercesSizeUint readBytes(XMLByte* const toFill, const XercesSizeUint maxToRead)
  {
    unsigned int read = stream_->read(stream_, (void *)toFill, (unsigned int)maxToRead);
    pos_ += read;
    return read;
  }

  virtual const XMLCh* getContentType() const
  {
    return 0;
  }

private:
  XQC_InputStream *stream_;
  XercesFilePos pos_;
};

class XQCInputSource : public XERCES_CPP_NAMESPACE_QUALIFIER InputSource
{
public :
  XQCInputSource(XQC_InputStream *stream)
    : stream_(stream)
  {
    if(stream_->encoding != 0)
      setEncoding(X(stream_->encoding));
  }

  ~XQCInputSource()
  {
    stream_->free(stream_);
  }

  virtual XERCES_CPP_NAMESPACE_QUALIFIER BinInputStream* makeStream() const
  {
    return new BinXQCInputStream(stream_);
  }

private:
  XQC_InputStream *stream_;
};

#endif

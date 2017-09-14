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
#include <xqilla/framework/XQillaExport.hpp>
#include <xqilla/exceptions/XQillaException.hpp>
#include <xqilla/exceptions/XQException.hpp>
#include <xqilla/utils/XPath2Utils.hpp>

#include <xercesc/framework/XMLBuffer.hpp>

bool XQillaException::_debug = false;

XERCES_CPP_NAMESPACE_USE;

#if _XERCES_VERSION >= 30000
XQillaException::XQillaException(short code, const XMLCh* message)
  : DOMXPathException(code)
{
    if(msg) fMemoryManager->deallocate((void*)msg);
    msg = XMLString::replicate(message, fMemoryManager);
}

XQillaException::XQillaException(const XQException &ex)
  : DOMXPathException(INVALID_EXPRESSION_ERR)
{
  XMLBuffer buffer(1023);
  buffer.append(ex.getError());

  if(ex.getXQueryLine() != 0) {
    buffer.append(' ');
    buffer.append('a');
    buffer.append('t');
    buffer.append(' ');
    buffer.append(ex.getXQueryFile());
    buffer.append(':');
    XPath2Utils::numToBuf(ex.getXQueryLine(), buffer);
    buffer.append(':');
    XPath2Utils::numToBuf(ex.getXQueryColumn(), buffer);
  }

  if(msg) fMemoryManager->deallocate((void*)msg);
  msg = XMLString::replicate(buffer.getRawBuffer(), fMemoryManager);
}

XQillaException::XQillaException(const XQillaException &other)
  : DOMXPathException(other)
{
}

XQillaException::~XQillaException()
{
}
#else
XQillaException::XQillaException(short code, const XMLCh* message)
  : DOMXPathException(code, XMLString::replicate(message))
{
}

XQillaException::XQillaException(const XQException &ex)
  : DOMXPathException(INVALID_EXPRESSION_ERR, 0)
{
  XMLBuffer buffer(1023);
  buffer.append(ex.getError());

  if(ex.getXQueryLine() != 0) {
    buffer.append(' ');
    buffer.append('a');
    buffer.append('t');
    buffer.append(' ');
    buffer.append(ex.getXQueryFile());
    buffer.append(':');
    XPath2Utils::numToBuf(ex.getXQueryLine(), buffer);
    buffer.append(':');
    XPath2Utils::numToBuf(ex.getXQueryColumn(), buffer);
  }

  msg = XMLString::replicate(buffer.getRawBuffer());
}

XQillaException::XQillaException(const XQillaException &other)
  : DOMXPathException(other.code, XMLString::replicate(other.msg))
{
}

XQillaException::~XQillaException()
{
  XMLString::release(const_cast<XMLCh**>(&msg));
}
#endif

void XQillaException::setDebug(bool flag)
{
  _debug = flag;
}

bool XQillaException::getDebug(void)
{
  return _debug;
}

DOMXPathException::ExceptionCode XQillaException::getCode(void) const 
{
  return (DOMXPathException::ExceptionCode)code;
}

const XMLCh* XQillaException::getString(void) const 
{
  return msg;
}

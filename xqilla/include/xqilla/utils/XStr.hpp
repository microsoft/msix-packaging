#ifndef _XSTR_HPP
#define _XSTR_HPP

#include <xqilla/framework/XQillaExport.hpp>
#include <xercesc/util/XercesDefs.hpp>
#include <xercesc/framework/MemoryManager.hpp>

// From Xerces 2.0

class XQILLA_API XStr
{
public :
  XStr(const char* const toTranscode, XERCES_CPP_NAMESPACE_QUALIFIER MemoryManager *mm = 0);
  ~XStr();

  const XMLCh *str() const
  {
    return str_;
  }

  XMLCh *adopt()
  {
    XMLCh *result = (XMLCh*)str_;
    str_ = 0;
    return result;
  }

private :
  XMLCh *str_;
  XERCES_CPP_NAMESPACE_QUALIFIER MemoryManager *mm_;
};

#define X(strg) XStr(strg).str()

#endif

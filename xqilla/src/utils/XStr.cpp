#include "../config/xqilla_config.h"
#include <xqilla/utils/XStr.hpp>
#include <xqilla/framework/XPath2MemoryManager.hpp>

#include <xercesc/util/XMLString.hpp>
#include <xercesc/util/XMLUTF8Transcoder.hpp>

XERCES_CPP_NAMESPACE_USE;

XStr::XStr(const char* const toTranscode, MemoryManager *mm)
  : str_(0),
    mm_(mm)
{
  if(toTranscode == 0) {
    str_ = mm_ ? (XMLCh*)mm_->allocate(1 * sizeof(XMLCh)) : new XMLCh[1];
    str_[0] = 0;
  }
  else {
    XMLUTF8Transcoder t(0, 512);
    size_t l = XMLString::stringLen(toTranscode);
    const size_t needed = l * 2 + 1; // 2 chars per byte is the worst case, + '\0'
    str_ = mm_ ? (XMLCh*)mm_->allocate(needed * sizeof(XMLCh)) : new XMLCh[needed];

    AutoDeleteArray<unsigned char> charSizes(new unsigned char[needed]);

#if _XERCES_VERSION >= 30000
    XMLSize_t bytesEaten = 0;
    t.transcodeFrom((const XMLByte*)toTranscode, l+1, str_,
                    needed, bytesEaten, charSizes);
#else
    unsigned int bytesEaten = 0;
    t.transcodeFrom((const XMLByte*)toTranscode, (unsigned int)l+1, str_,
                    (unsigned int)needed, bytesEaten, charSizes);
#endif
  }
}

XStr::~XStr()
{
  if(str_) {
    if(mm_) mm_->deallocate(str_);
    else delete [] str_;
  }
}

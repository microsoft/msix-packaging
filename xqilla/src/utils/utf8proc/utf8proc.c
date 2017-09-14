/*
 *  Copyright (c) 2006-2007 Jan Behrens, FlexiGuided GmbH, Berlin
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a
 *  copy of this software and associated documentation files (the "Software"),
 *  to deal in the Software without restriction, including without limitation
 *  the rights to use, copy, modify, merge, publish, distribute, sublicense,
 *  and/or sell copies of the Software, and to permit persons to whom the
 *  Software is furnished to do so, subject to the following conditions:
 *
 *  The above copyright notice and this permission notice shall be included in
 *  all copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 *  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 *  DEALINGS IN THE SOFTWARE.
 *
 *
 *  *** A ADDITIONAL DIFFERENT LICENCE FOR UTF8PROC ***
 *
 *  Copyright (c) 2004, 2015 Oracle and/or its affiliates. All rights reserved.
 *      
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 *  Enhancement by Oracle:
 *  1. Support UTF16.
 *  2. Support Uppercase and Lowercase mapping.
 *  3. Support WordBounding and SentenceBounding.
 *  4. Support diacritic chars differentiating.
 *
 *  For upon enhancement, there are some modifications:
 *  1. Extend the "int lastbounding" as "struct bound_attr_t last_bound_attr"
 *  2. Rename the "utf8proc_data.c" as "utf8_proc_data.h"
 *  3. Extend the data tables in utf8proc_data.h
 *  4. Optimize the sequences arithmetic:
 *       positive number : sequences with single element
 *       zero            : no sequences mapping
 *       negative number : (-x) is the offset to sequences table
 *
 */

/*
 *  This library contains derived data from a modified version of the
 *  Unicode data files.
 *
 *  The original data files are available at
 *  http://www.unicode.org/Public/UNIDATA/
 *
 *  Please notice the copyright statement in the file "utf8proc_data.c".
 */


/*
 *  File name:    utf8proc.c
 *  Version:      1.1.1
 *  Last changed: 2007-07-22
 *
 *  Description:
 *  Implementation of libutf8proc.
 */


#include <xqilla/utils/utf8proc.h>
#include "utf8proc_data.h"

const int8_t utf8proc_utf8class[256] = {
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
  2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
  3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
  4, 4, 4, 4, 4, 4, 4, 4, 0, 0, 0, 0, 0, 0, 0, 0 };

// http://unicode.org/Public/UNIDATA/auxiliary/WordBreakTest.html
// "-1" means: the word bound marks int TBD_buf[] should be deleted.
// The last value WB_START, is added for detecting the begining of stream.
static const int8_t utf8proc_wb_table[][14] = {
  {1, 1, 1, 1, 1,  1, 1, 1,  1,  1, 1,  0,  0, 1},
  {1, 1, 0, 1, 1,  1, 1, 1,  1,  1, 1,  1,  1, 1},
  {1, 1, 1, 1, 1,  1, 1, 1,  1,  1, 1,  1,  1, 1},
  {1, 1, 1, 1, 1,  1, 1, 1,  1,  1, 1,  1,  1, 1},
  {1, 1, 1, 1, 0,  1, 1, 1,  1,  1, 0,  0,  0, 1},
  {1, 1, 1, 1, 1,  0, 1, 1,  1,  0, 0,  0,  0, 1},
  {1, 1, 1, 1, 1,  1, 1, 1,  1,  1, 1,  0,  0, 1},
  {1, 1, 1, 1, 1,  1, 1, 1,  1,  1, 1,  0,  0, 1},
  {1, 1, 1, 1, 1,  1, 1, 1,  1,  1, 1,  0,  0, 1},
  {1, 1, 1, 1, 1,  0, 1, 1,  1,  0, 0,  0,  0, 1},
  {1, 1, 1, 1, 0,  0, 1, 1,  1,  0, 0,  0,  0, 1},
  {1, 1, 1, 1, 1,  1, 1, 1,  1,  1, 1,  0,  0, 1},
  {1, 1, 1, 1, 1,  1, 1, 1,  1,  1, 1,  0,  0, 1},
  {1, 1, 1, 1, 1,  0, 1, 1,  1,  0, 0,  0,  0, 1},
  {1, 1, 1, 1, 1, -1, 1, 1,  1,  1, 1, -1, -1, 1},
  {1, 1, 1, 1, 1, -1, 1, 1,  1,  1, 1, -1, -1, 1},
  {1, 1, 1, 1, 1, -1, 1, 1,  1,  1, 1, -1, -1, 1},
  {1, 1, 1, 1, 1,  1, 1, 1,  1,  1, 1,  0,  0, 1},
  {1, 1, 1, 1, 1,  1, 1, 1,  1,  1, 1,  0,  0, 1},
  {1, 1, 1, 1, 1,  1, 1, 1,  1, -1, 1, -1, -1, 1},
  {1, 1, 1, 1, 1,  1, 1, 1,  1, -1, 1,  0,  0, 1},
  {1, 1, 1, 1, 1,  1, 1, 1,  1, -1, 1,  0,  0, 1},
  {1, 1, 1, 1, 1,  1, 1, 1,  1,  1, 1,  1,  1, 1}
};

// http://unicode.org/Public/UNIDATA/auxiliary/SentenceBreakTest.html
// The last value SB_START, is added for detecting the begining of stream.
static const int8_t utf8proc_sb_table[][16] = {
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
  {1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
  {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
  {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
  {1, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1},
  {1, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 1},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
  {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}
};

#define UTF8PROC_HANGUL_SBASE     0xAC00
#define UTF8PROC_HANGUL_LBASE     0x1100
#define UTF8PROC_HANGUL_VBASE     0x1161
#define UTF8PROC_HANGUL_TBASE     0x11A7
#define UTF8PROC_HANGUL_LCOUNT    19
#define UTF8PROC_HANGUL_VCOUNT    21
#define UTF8PROC_HANGUL_TCOUNT    28
#define UTF8PROC_HANGUL_NCOUNT    588
#define UTF8PROC_HANGUL_SCOUNT    11172
// END is exclusive
#define UTF8PROC_HANGUL_L_START   0x1100
#define UTF8PROC_HANGUL_L_END     0x115A
#define UTF8PROC_HANGUL_L_FILLER  0x115F
#define UTF8PROC_HANGUL_V_START   0x1160
#define UTF8PROC_HANGUL_V_END     0x11A3
#define UTF8PROC_HANGUL_T_START   0x11A8
#define UTF8PROC_HANGUL_T_END     0x11FA
#define UTF8PROC_HANGUL_S_START   0xAC00
#define UTF8PROC_HANGUL_S_END     0xD7A4

#define UTF8PROC_BOUNDCLASS_START       0
#define UTF8PROC_BOUNDCLASS_OTHER       1
#define UTF8PROC_BOUNDCLASS_CR          2
#define UTF8PROC_BOUNDCLASS_LF          3
#define UTF8PROC_BOUNDCLASS_CONTROL     4
#define UTF8PROC_BOUNDCLASS_EXTEND      5
#define UTF8PROC_BOUNDCLASS_L           6
#define UTF8PROC_BOUNDCLASS_V           7
#define UTF8PROC_BOUNDCLASS_T           8
#define UTF8PROC_BOUNDCLASS_LV          9
#define UTF8PROC_BOUNDCLASS_LVT        10

// WB means WordBreak. The value is defined by
//   http://unicode.org/Public/UNIDATA/auxiliary/WordBreakTest.html
#define WB_OTHER                        0
#define WB_CR                           1
#define WB_LF                           2
#define WB_NEWLINE                      3
#define WB_KATAKANA                     4
#define WB_ALETTER                      5
#define WB_MIDLETTER                    6
#define WB_MIDNUM                       7
#define WB_MIDNUMLET                    8
#define WB_NUMERIC                      9
#define WB_EXTENDNUMLET                10
#define WB_FORMATFE                    11
#define WB_EXTENDFE                    12
#define WB_ALETTER_FORMATFE            13
#define WB_ALETTER_MIDLETTER           14
#define WB_ALETTER_MIDNUMLET           15
#define WB_ALETTER_MIDNUMLET_FORMATFE  16
#define WB_ALETTER_MIDNUM              17
#define WB_NUMERIC_MIDLETTER           18
#define WB_NUMERIC_MIDNUMLET           19
#define WB_NUMERIC_MIDNUM              20
#define WB_NUMERIC_MIDNUMLET_FORMATFE  21
#define WB_START                       22

// SB means SentenceBreak. The value is defined by
//   http://unicode.org/Public/UNIDATA/auxiliary/SentenceBreakTest.html
#define SB_OTHER                        0
#define SB_CR                           1
#define SB_LF                           2
#define SB_SEP                          3
#define SB_SP                           4
#define SB_LOWER                        5
#define SB_UPPER                        6
#define SB_OLETTER                      7
#define SB_NUMERIC                      8
#define SB_ATERM                        9
#define SB_STERM                       10
#define SB_CLOSE                       11
#define SB_SCONTINUE                   12
#define SB_FORMATFE                    13
#define SB_EXTENDFE                    14
#define SB_START                       15

// Following three BOUD definition are related to upon wb_table/sb_table.
#define CANCEL_BOUND                   -1 // Cancel TBD bounds
#define IS_BOUND                        1 // Confirmed BOUND
#define NOT_BOUND                       0

#define UTF8PROC_INVALID_CODE          -1

#define PUT(buf, pos, bufsize, ch)    {if (pos < bufsize) buf[pos] = ch; pos++;}


const char *utf8proc_version(void) {
  return "1.1.3";
}

const char *utf8proc_errmsg(ssize_t errcode) {
  switch (errcode) {
    case UTF8PROC_ERROR_NOMEM:
    return "Memory for processing UTF-8 data could not be allocated.";
    case UTF8PROC_ERROR_OVERFLOW:
    return "UTF-8 string is too long to be processed.";
    case UTF8PROC_ERROR_INVALIDUTF8:
    return "Invalid UTF-8 string";
    case UTF8PROC_ERROR_NOTASSIGNED:
    return "Unassigned Unicode code point found in UTF-8 string.";
    case UTF8PROC_ERROR_INVALIDOPTS:
    return "Invalid options for UTF-8 processing chosen.";
    default:
    return "An unknown error occured while processing UTF-8 data.";
  }
}

ssize_t utf8proc_iterate(const uint8_t *str, ssize_t strlen, int32_t *dst)
{
  ssize_t length;
  ssize_t i;
  int32_t uc = -1;
  *dst = -1;
  if (!strlen) return 0;
  length = utf8proc_utf8class[str[0]];
  if (!length) return UTF8PROC_ERROR_INVALIDUTF8;
  if (strlen >= 0 && length > strlen) return UTF8PROC_ERROR_INVALIDUTF8;
  for (i=1; i<length; i++) {
    if ((str[i] & 0xC0) != 0x80) return UTF8PROC_ERROR_INVALIDUTF8;
  }
  switch (length) {
    case 1:
    uc = str[0];
    break;
    case 2:
    uc = ((str[0] & 0x1F) <<  6) + (str[1] & 0x3F);
    if (uc < 0x80) uc = -1;
    break;
    case 3:
    uc = ((str[0] & 0x0F) << 12) + ((str[1] & 0x3F) <<  6)
      + (str[2] & 0x3F);
    if (uc < 0x800 || (uc >= 0xD800 && uc < 0xE000) ||
      (uc >= 0xFDD0 && uc < 0xFDF0)) uc = -1;
    break;
    case 4:
    uc = ((str[0] & 0x07) << 18) + ((str[1] & 0x3F) << 12)
      + ((str[2] & 0x3F) <<  6) + (str[3] & 0x3F);
    if (uc < 0x10000 || uc >= 0x110000) uc = -1;
    break;
  }
  if (uc < 0 || ((uc & 0xFFFF) >= 0xFFFE))
    return UTF8PROC_ERROR_INVALIDUTF8;
  *dst = uc;
  return length;
}

ssize_t utf16proc_iterate(const uint16_t *str, ssize_t strlen, int32_t *dst)
{
  ssize_t length;
  int32_t uc, w2; // w2 means "the second word"
  *dst = -1;
  if (strlen == 0)
    return 0;

  uc = *str;
  if (uc >= 0xD800 && uc <= 0xDBFF) {// two words
    if (strlen >=0 && strlen < 2)   // prevent visiting illegal memory
      return UTF8PROC_ERROR_INVALIDUTF8;

    length = 2;
    w2 = *(str+1);
    if (w2 < 0xDC00 || w2 > 0xDFFF)
      return UTF8PROC_ERROR_INVALIDUTF8;

    uc = ((uc & 0x3ff) << 10) + (w2 & 0x3ff);
    uc += 0x10000;

  } else { // single word
    length = 1;
  }

  *dst = uc;
  return length;
}

int utf8proc_codepoint_valid(int32_t uc) {
  if (uc < 0 || uc >= 0x110000 ||
    ((uc & 0xFFFF) >= 0xFFFE) || (uc >= 0xD800 && uc < 0xE000) ||
    (uc >= 0xFDD0 && uc < 0xFDF0)) return false;
  else return true;
}

ssize_t utf8proc_encode_char(int32_t uc, uint8_t *dst) {
  if (uc < 0x00) {
    return 0;
  } else if (uc < 0x80) {
    dst[0] = uc;
    return 1;
  } else if (uc < 0x800) {
    dst[0] = 0xC0 + (uc >> 6);
    dst[1] = 0x80 + (uc & 0x3F);
    return 2;
  } else if (uc == 0xFFFF) {
    dst[0] = 0xFF;
    return 1;
  } else if (uc == 0xFFFE) {
    dst[0] = 0xFE;
    return 1;
  } else if (uc < 0x10000) {
    dst[0] = 0xE0 + (uc >> 12);
    dst[1] = 0x80 + ((uc >> 6) & 0x3F);
    dst[2] = 0x80 + (uc & 0x3F);
    return 3;
  } else if (uc < 0x110000) {
    dst[0] = 0xF0 + (uc >> 18);
    dst[1] = 0x80 + ((uc >> 12) & 0x3F);
    dst[2] = 0x80 + ((uc >> 6) & 0x3F);
    dst[3] = 0x80 + (uc & 0x3F);
    return 4;
  } else return 0;
}

ssize_t utf16proc_encode_char(int32_t uc, uint16_t *dst) {
  if(uc < 0)
    return 0;
  if(uc < 0x10000) {
    *dst = uc;
    return 1;
  } else {
    // ASSERT uc <= 0x10FFFF;
    uc -= 0x10000;
    dst[0] = ((uc >> 10) | 0xD800);
    dst[1] = ((uc & 0x3FF) | 0xDC00);
    return 2;
  }
  return 0;
}

const utf8proc_property_t *utf8proc_get_property(int32_t uc) {
  // ASSERT: uc >= 0 && uc < 0x110000
  return utf8proc_properties + (
    utf8proc_stage2table[
      utf8proc_stage1table[uc >> 8] + (uc & 0xFF)
    ]
  );
}

static void utf8proc_reset_bound_attr(bound_attr_t* attr)
{
  // Free the TBD heap as need
  if (attr->TBD_heap)
    free (attr->TBD_heap);

  attr->cluster      = UTF8PROC_BOUNDCLASS_START;
  attr->word         = WB_START;
  attr->TBD_stack[0] = UTF8PROC_INVALID_CODE;
  attr->TBD_heap     = NULL;
  attr->TBD_buf_size = sizeof(attr->TBD_stack) / sizeof(attr->TBD_stack[0]);
  memset(attr->sb_attr_queue, SB_START, sizeof(attr->sb_attr_queue));

  return;
}

void utf8proc_init_bound_attr(bound_attr_t* attr)
{
  if (attr == NULL)
    return;

  attr->TBD_heap = NULL; // Must be set to NULL before reseting.
  attr->filter_callback = NULL;
  utf8proc_reset_bound_attr(attr);
  return;
}

ssize_t utf8proc_flush(int32_t *dst, ssize_t bufsize,
                       int options, bound_attr_t *last_bound_attr)
{
  ssize_t written = 0;
  if (options & (UTF8PROC_WORDBOUND | UTF8PROC_SENTENCEBOUND)) {
    int output_sb = true;
    // At the end of the stream, we have to fetch the chars in TBD_buf if any.
    int32_t *p = last_bound_attr->TBD_heap ? \
                 last_bound_attr->TBD_heap : last_bound_attr->TBD_stack;

    for (; *p != UTF8PROC_INVALID_CODE; p++) {
      if (*p == UTF8PROC_SB_MARK && output_sb) {
        // We only output the first confirmed sentence bound.
        PUT(dst, written, bufsize, UTF8PROC_SB_MARK);
        output_sb = false;
      } else
        PUT(dst, written, bufsize, *p);
    }

    // Unicode Standard say that a Word/Sentence Bound Mark should be placed
    // at the end of stream
    if (options & UTF8PROC_WORDBOUND)
      PUT(dst, written, bufsize, UTF8PROC_WB_MARK);
    if (options & UTF8PROC_SENTENCEBOUND)
      PUT(dst, written, bufsize, UTF8PROC_SB_MARK);
  }

  utf8proc_reset_bound_attr(last_bound_attr);
  return written;
}

#define utf8proc_decompose_lump(replacement_uc) \
  return utf8proc_decompose_char((replacement_uc), dst, bufsize, \
  options & ~UTF8PROC_LUMP, last_bound_attr)

ssize_t utf8proc_decompose_char(int32_t uc, int32_t *dst, ssize_t bufsize,
                                int options, bound_attr_t *last_bound_attr) {
  // ASSERT: uc >= 0 && uc < 0x110000
  const utf8proc_property_t *property;
  utf8proc_propval_t category;
  int32_t hangul_sindex;
  property = utf8proc_get_property(uc);
  category = property->category;
  hangul_sindex = uc - UTF8PROC_HANGUL_SBASE;
  if (options & (UTF8PROC_COMPOSE|UTF8PROC_DECOMPOSE)) {
    if (hangul_sindex >= 0 && hangul_sindex < UTF8PROC_HANGUL_SCOUNT) {
      int32_t hangul_tindex;
      if (bufsize >= 1) {
        dst[0] = UTF8PROC_HANGUL_LBASE +
          hangul_sindex / UTF8PROC_HANGUL_NCOUNT;
        if (bufsize >= 2) dst[1] = UTF8PROC_HANGUL_VBASE +
          (hangul_sindex % UTF8PROC_HANGUL_NCOUNT) / UTF8PROC_HANGUL_TCOUNT;
      }
      hangul_tindex = hangul_sindex % UTF8PROC_HANGUL_TCOUNT;
      if (!hangul_tindex) return 2;
      if (bufsize >= 3) dst[2] = UTF8PROC_HANGUL_TBASE + hangul_tindex;
      return 3;
    }
  }
  if (last_bound_attr->filter_callback) {
    // Skip current codepoint if user want to filter it out.
    if (last_bound_attr->filter_callback(uc) == 0)
      return 0;
  }
  if (options & UTF8PROC_REJECTNA) {
    if (!category) return UTF8PROC_ERROR_NOTASSIGNED;
  }
  if (options & UTF8PROC_IGNORE) {
    if (property->ignorable) return 0;
  }
  if (options & UTF8PROC_REMOVE_DIACRITIC) {
    if (property->is_diacritic) return 0;
  }
  if (options & UTF8PROC_LUMP) {
    if (category == UTF8PROC_CATEGORY_ZS) utf8proc_decompose_lump(0x0020);
    if (uc == 0x2018 || uc == 0x2019 || uc == 0x02BC || uc == 0x02C8)
      utf8proc_decompose_lump(0x0027);
    if (category == UTF8PROC_CATEGORY_PD || uc == 0x2212)
      utf8proc_decompose_lump(0x002D);
    if (uc == 0x2044 || uc == 0x2215) utf8proc_decompose_lump(0x002F);
    if (uc == 0x2236) utf8proc_decompose_lump(0x003A);
    if (uc == 0x2039 || uc == 0x2329 || uc == 0x3008)
      utf8proc_decompose_lump(0x003C);
    if (uc == 0x203A || uc == 0x232A || uc == 0x3009)
      utf8proc_decompose_lump(0x003E);
    if (uc == 0x2216) utf8proc_decompose_lump(0x005C);
    if (uc == 0x02C4 || uc == 0x02C6 || uc == 0x2038 || uc == 0x2303)
      utf8proc_decompose_lump(0x005E);
    if (category == UTF8PROC_CATEGORY_PC || uc == 0x02CD)
      utf8proc_decompose_lump(0x005F);
    if (uc == 0x02CB) utf8proc_decompose_lump(0x0060);
    if (uc == 0x2223) utf8proc_decompose_lump(0x007C);
    if (uc == 0x223C) utf8proc_decompose_lump(0x007E);
    if ((options & UTF8PROC_NLF2LS) && (options & UTF8PROC_NLF2PS)) {
      if (category == UTF8PROC_CATEGORY_ZL ||
          category == UTF8PROC_CATEGORY_ZP)
        utf8proc_decompose_lump(0x000A);
    }
  }
  if (options & UTF8PROC_STRIPMARK) {
    if (category == UTF8PROC_CATEGORY_MN ||
      category == UTF8PROC_CATEGORY_MC ||
      category == UTF8PROC_CATEGORY_ME) return 0;
  }
  if (options & UTF8PROC_CASEFOLD) {
    const int32_t mapping = property->casefold_mapping;
    if (mapping > 0) {
      return utf8proc_decompose_char(mapping, dst, bufsize, options,
                                     last_bound_attr);
    } else if (mapping < 0) {
      ssize_t written = 0;
      const int32_t *entry = utf8proc_sequences + (-mapping);
      for (; *entry >= 0; entry++) {
        written += utf8proc_decompose_char(*entry, dst+written,
                         (bufsize > written) ? (bufsize - written) : 0, options,
                         last_bound_attr);
        if (written < 0) return UTF8PROC_ERROR_OVERFLOW;
      }
      return written;
    }
  }
  if (options & (UTF8PROC_UPPERCASE|UTF8PROC_LOWERCASE)) {
    int32_t value;
    if (options & UTF8PROC_UPPERCASE) {
      options &= (~UTF8PROC_UPPERCASE);
      value = property->uppercase_mapping;
    } else {
      options &= (~UTF8PROC_LOWERCASE);
      value = property->lowercase_mapping;
    }

    if (value > 0) {         // a single codepoint mapping to x
       return utf8proc_decompose_char(value, dst, bufsize, options, last_bound_attr);
    } else if (value == 0) { // no mapping
       return utf8proc_decompose_char(uc, dst, bufsize, options, last_bound_attr);
    } else {                 // -value is the index to special case mapping
       ssize_t written = 0;
       const int32_t *p = utf8proc_sequences + (-value);
       for (; *p>0; p++) {
         written += utf8proc_decompose_char(*p, dst+written, bufsize-written,
                                            options, last_bound_attr);
       }
       return written;
    }
  }
  if (options & (UTF8PROC_COMPOSE|UTF8PROC_DECOMPOSE)) {
    if (property->decomp_mapping != 0 &&
        (!property->decomp_type || (options & UTF8PROC_COMPAT))) {
      int32_t mapping = property->decomp_mapping;
      ssize_t written = 0;
      const int32_t *entry = utf8proc_sequences + (-mapping);
      // If the mapping is not sequence
      if (mapping > 0)
        return utf8proc_decompose_char(mapping, dst, bufsize,
                                       options, last_bound_attr);
      // If the mapping is sequence
      for (; *entry >= 0; entry++) {
        written += utf8proc_decompose_char(*entry, dst+written,
                         (bufsize > written) ? (bufsize - written) : 0, options,
                         last_bound_attr);
        if (written < 0) return UTF8PROC_ERROR_OVERFLOW;
      }
      return written;
    }
  }
  if (options & UTF8PROC_CHARBOUND) {
    int boundary;
    int tbc, lbc;
    bound_attr_t* bp = (bound_attr_t *)last_bound_attr;
    tbc =
      (uc == 0x000D) ? UTF8PROC_BOUNDCLASS_CR :
      (uc == 0x000A) ? UTF8PROC_BOUNDCLASS_LF :
      ((category == UTF8PROC_CATEGORY_ZL ||
        category == UTF8PROC_CATEGORY_ZP ||
        category == UTF8PROC_CATEGORY_CC ||
        category == UTF8PROC_CATEGORY_CF) &&
        !(uc == 0x200C || uc == 0x200D)) ? UTF8PROC_BOUNDCLASS_CONTROL :
      property->extend ? UTF8PROC_BOUNDCLASS_EXTEND :
      ((uc >= UTF8PROC_HANGUL_L_START && uc < UTF8PROC_HANGUL_L_END) ||
        uc == UTF8PROC_HANGUL_L_FILLER) ? UTF8PROC_BOUNDCLASS_L :
      (uc >= UTF8PROC_HANGUL_V_START && uc < UTF8PROC_HANGUL_V_END) ?
        UTF8PROC_BOUNDCLASS_V :
      (uc >= UTF8PROC_HANGUL_T_START && uc < UTF8PROC_HANGUL_T_END) ?
        UTF8PROC_BOUNDCLASS_T :
      (uc >= UTF8PROC_HANGUL_S_START && uc < UTF8PROC_HANGUL_S_END) ? (
        ((uc-UTF8PROC_HANGUL_SBASE) % UTF8PROC_HANGUL_TCOUNT == 0) ?
          UTF8PROC_BOUNDCLASS_LV : UTF8PROC_BOUNDCLASS_LVT
      ) :
      UTF8PROC_BOUNDCLASS_OTHER;
    lbc = bp->cluster;
    boundary =
      (tbc == UTF8PROC_BOUNDCLASS_EXTEND) ? false :
      (lbc == UTF8PROC_BOUNDCLASS_START) ? true :
      (lbc == UTF8PROC_BOUNDCLASS_CR &&
       tbc == UTF8PROC_BOUNDCLASS_LF) ? false :
      (lbc == UTF8PROC_BOUNDCLASS_CONTROL) ? true :
      (tbc == UTF8PROC_BOUNDCLASS_CONTROL) ? true :
      (lbc == UTF8PROC_BOUNDCLASS_L &&
       (tbc == UTF8PROC_BOUNDCLASS_L ||
        tbc == UTF8PROC_BOUNDCLASS_V ||
        tbc == UTF8PROC_BOUNDCLASS_LV ||
        tbc == UTF8PROC_BOUNDCLASS_LVT)) ? false :
      ((lbc == UTF8PROC_BOUNDCLASS_LV ||
        lbc == UTF8PROC_BOUNDCLASS_V) &&
       (tbc == UTF8PROC_BOUNDCLASS_V ||
        tbc == UTF8PROC_BOUNDCLASS_T)) ? false :
      ((lbc == UTF8PROC_BOUNDCLASS_LVT ||
        lbc == UTF8PROC_BOUNDCLASS_T) &&
       tbc == UTF8PROC_BOUNDCLASS_T) ? false :
       true;
    bp->cluster = tbc;
    if (boundary) {
      if (bufsize >= 1) dst[0] = 0xFFFF;
      if (bufsize >= 2) dst[1] = uc;
      return 2;
    }
  }
  if (options & (UTF8PROC_WORDBOUND | UTF8PROC_SENTENCEBOUND)) {
    int word_boundary     = NOT_BOUND;
    int sentence_boundary = NOT_BOUND;

    int TBD          = false; // bool value
    int TBD_size     = last_bound_attr->TBD_buf_size;
    int32_t *TBD_buf = last_bound_attr->TBD_heap ? \
                       last_bound_attr->TBD_heap : last_bound_attr->TBD_stack;

    // Declare some vars for Word/Sentence boundary
    ssize_t i, written, size;
    int32_t *write;
    int output_sb;

    if (options & UTF8PROC_WORDBOUND) {
      int tbc = property->word_bound_attr;
      int lbc = last_bound_attr->word;
      int old_tbc;
      // WB_FORMATFE and WB_EXTENDFE are the same in bounding.
      if (tbc == WB_EXTENDFE) tbc = WB_FORMATFE;
      if (lbc == WB_EXTENDFE) lbc = WB_FORMATFE;
      word_boundary = utf8proc_wb_table[lbc][tbc];

      switch(lbc) {
        case WB_ALETTER_FORMATFE: lbc = WB_ALETTER; break;
        case WB_ALETTER_MIDNUMLET_FORMATFE: lbc = WB_ALETTER_MIDNUMLET; break;
        case WB_NUMERIC_MIDNUMLET_FORMATFE: lbc = WB_NUMERIC_MIDNUMLET; break;
      }

      old_tbc = tbc;
      switch(lbc) {
      case WB_ALETTER:
        switch(tbc) {
        case WB_FORMATFE:  tbc = WB_ALETTER_FORMATFE;  break;
        case WB_MIDLETTER: tbc = WB_ALETTER_MIDLETTER; break;
        case WB_MIDNUMLET: tbc = WB_ALETTER_MIDNUMLET; break;
        case WB_MIDNUM:    tbc = WB_ALETTER_MIDNUM;    break;
        } break;
      case WB_ALETTER_MIDNUMLET:
        switch(tbc) {
        case WB_FORMATFE: tbc = WB_ALETTER_MIDNUMLET_FORMATFE;break;
        } break;
      case WB_NUMERIC:
        switch(tbc) {
        case WB_MIDLETTER: tbc = WB_NUMERIC_MIDLETTER; break;
        case WB_MIDNUMLET: tbc = WB_NUMERIC_MIDNUMLET; break;
        case WB_MIDNUM:    tbc = WB_NUMERIC_MIDNUM;    break;
        } break;
      case WB_NUMERIC_MIDNUMLET:
        switch(tbc) {
        case WB_FORMATFE: tbc = WB_NUMERIC_MIDNUMLET_FORMATFE;break;
        } break;
      }
      if (lbc != WB_START) {
        if (old_tbc == tbc && tbc == WB_FORMATFE && word_boundary != IS_BOUND)
          tbc = lbc;
        if (tbc >= WB_ALETTER_FORMATFE)
          TBD = true;
      }
      last_bound_attr->word = tbc;
    }
    if (options & UTF8PROC_SENTENCEBOUND) {
      int do_not_insert_property = false;
      int8_t* sb_attr = last_bound_attr->sb_attr_queue;
      int tbc = property->sentence_bound_attr;
      int lbc = sb_attr[1];
      // WB_FORMATFE and WB_EXTENDFE are the same in bounding.
      if (tbc == SB_EXTENDFE) tbc = SB_FORMATFE;
      if (lbc == SB_EXTENDFE) lbc = SB_FORMATFE;
      sentence_boundary = utf8proc_sb_table[lbc][tbc];
      if (tbc == SB_FORMATFE) {
        TBD = true;
        if (lbc != SB_SEP && lbc != SB_CR && lbc != SB_LF)
          do_not_insert_property = true;
      }

      // The queue depth is fixed as "2". Rule numbers are defined by:
      // http://unicode.org/Public/UNIDATA/auxiliary/SentenceBreakTest.html
      // Rule 4.0
      if (lbc == SB_SEP || lbc == SB_CR || lbc == SB_LF) {
        do_not_insert_property = false;
        TBD = false;
      }
      else if (!do_not_insert_property) {
        // Rule 7.0
        if (sb_attr[0] == SB_UPPER && sb_attr[1] == SB_ATERM && tbc == SB_UPPER) {
          sentence_boundary = NOT_BOUND;
        } // Rule 8.0 9.0 and 11.0
        else if ((sb_attr[0] == SB_ATERM || sb_attr[0] == SB_STERM) &&
                 (sb_attr[1] == SB_CLOSE || sb_attr[1] == SB_SP)) {
          int determined = false;
          if (sb_attr[0] == SB_ATERM) { // Rule 8.0
            if (tbc != SB_OLETTER && tbc != SB_UPPER &&
                tbc != SB_LOWER   && tbc != SB_SEP) {
              TBD = true;
              do_not_insert_property = true;
            } else if (tbc == SB_LOWER) {
              sentence_boundary = CANCEL_BOUND;
              determined = true;
            } else if (tbc == SB_OLETTER || tbc == SB_UPPER) {
              sentence_boundary = IS_BOUND;
              determined = true;
            }
          }
          if (!determined) { // Rule 9.0 and 11.0
            if (tbc == SB_CLOSE || tbc == SB_SP) {
              do_not_insert_property = true;
              if (tbc != sb_attr[1]) {
                sb_attr[1] = tbc; // Rule 9.0
                if (tbc == SB_SP) {
                  sentence_boundary = NOT_BOUND;
                } else {
                  TBD = true;
                  sentence_boundary = IS_BOUND;
                }
              } else {
                sentence_boundary = NOT_BOUND;
              }
            } else { // Rule 11.0
              sentence_boundary = utf8proc_sb_table[sb_attr[0]][tbc];
            }
          }
        }
      }
      if (sb_attr[0] == SB_START) {
        do_not_insert_property = false;
        TBD = false;
      }
      if (sentence_boundary == IS_BOUND) {
        if (!do_not_insert_property && !TBD) {
          // clear sentence bound property queue.
          sb_attr[0] = SB_OTHER;
          sb_attr[1] = tbc;
        }
      } else if (!do_not_insert_property) {
        // put current property into sentence bound proberty queue.
        sb_attr[0] = sb_attr[1];
        sb_attr[1] = tbc;
      }
    }
    // output result data of word/sentence bounding.
    written = 0;
    size = TBD_size;
    output_sb = true;
    for (i = 0; i < TBD_size; i++) {
      if (TBD_buf[i] == UTF8PROC_INVALID_CODE) {
        write = TBD_buf + i;
        break;
      }
    }

    // If the buffer is about to be used out, use heap to create a bigger one.
    // The threshold value is (TBD_size-8), because the max size of writting
    // a codepoint with a bouding is sure to less then 8.
    if (i >= (TBD_size - 8)) {
      TBD_size = TBD_size * 2; // double the size.
      if (last_bound_attr->TBD_heap) {
        // If we are using heap already, just realloc a bigger one.
        TBD_buf = realloc(TBD_buf, TBD_size * sizeof(TBD_buf[0]));
        if (TBD_buf == NULL) {
          free(last_bound_attr->TBD_heap);
          return UTF8PROC_ERROR_NOMEM;
        }
      } else {
        // If using stack, create a heap with double size and copy the TBD data.
        TBD_buf = malloc(TBD_size * sizeof(TBD_buf[0]));
        if (TBD_buf == NULL)
          return UTF8PROC_ERROR_NOMEM;
        memcpy(TBD_buf, last_bound_attr->TBD_stack, size * sizeof(TBD_buf[0]));
      }

      size  = TBD_size;
      write = TBD_buf + i;
      last_bound_attr->TBD_buf_size = TBD_size;
      last_bound_attr->TBD_heap     = TBD_buf;
    }

    if (!TBD) {  // Output all confirmed content
      write = dst;
      size = bufsize;
      for (i = 0; i < TBD_size && TBD_buf[i] > 0; i++) {
        if ((word_boundary == CANCEL_BOUND && TBD_buf[i] == UTF8PROC_WB_MARK))
          ; // Do not output canceled WB_MARK
        else if (TBD_buf[i] == UTF8PROC_SB_MARK) {
          if (sentence_boundary == CANCEL_BOUND) {
            ; // Do not output canceled SB_MARK
          } else if(output_sb) {
            PUT(write, written, size, UTF8PROC_SB_MARK);
            output_sb = false; // Only output the first confirmed SB_MARK
          }
        } else
          PUT(write, written, size, TBD_buf[i]);
      }
    }
    if (word_boundary == IS_BOUND)
      PUT(write, written, size, UTF8PROC_WB_MARK);
    if (sentence_boundary == IS_BOUND && output_sb)
      PUT(write, written, size, UTF8PROC_SB_MARK);
    PUT(write, written, size, uc);
    if (!TBD) {
      TBD_buf[0] = UTF8PROC_INVALID_CODE; // reset buffer
      return written;
    } else {
      write[written] = UTF8PROC_INVALID_CODE; // Mark the end
      return 0; // do not return any thing if the char is "TBD"
    }
  }

  if (bufsize >= 1) *dst = uc;
  return 1;
}

ssize_t utf8proc_decompose(
  const uint8_t *str, ssize_t strlen,
  int32_t *buffer, ssize_t bufsize, int options)
{
  return utf8proc_decompose_with_filter(str, strlen, buffer, bufsize, options, NULL);
}

ssize_t utf8proc_decompose_with_filter(
  const uint8_t *str, ssize_t strlen,
  int32_t *buffer, ssize_t bufsize, int options,
  int (*filter_callback)(int32_t codepoint)
) {
  // strlen will be ignored, if UTF8PROC_NULLTERM is set in options
  ssize_t wpos = 0;
  if ((options & UTF8PROC_COMPOSE) && (options & UTF8PROC_DECOMPOSE))
    return UTF8PROC_ERROR_INVALIDOPTS;
  if ((options & UTF8PROC_STRIPMARK) &&
      !(options & UTF8PROC_COMPOSE) && !(options & UTF8PROC_DECOMPOSE))
    return UTF8PROC_ERROR_INVALIDOPTS;
  if ((options & UTF8PROC_CHARBOUND) &&
      (options & (UTF8PROC_SENTENCEBOUND | UTF8PROC_WORDBOUND)))
    return UTF8PROC_ERROR_INVALIDOPTS;
  {
    int32_t uc;
    ssize_t rpos = 0;
    ssize_t decomp_result;
    bound_attr_t boundclass;
    utf8proc_init_bound_attr(&boundclass);
    boundclass.filter_callback = filter_callback;
    while (1) {
      if (options & UTF8PROC_NULLTERM) {
        if (options & UTF8PROC_UTF16)
          rpos += utf16proc_iterate((uint16_t*)(str + rpos), -1, &uc) * \
                  sizeof(uint16_t);
        else
          rpos += utf8proc_iterate(str + rpos, -1, &uc);
        // checking of return value is not neccessary,
        // as 'uc' is < 0 in case of error
        if (uc < 0) return UTF8PROC_ERROR_INVALIDUTF8;
        if (rpos < 0) return UTF8PROC_ERROR_OVERFLOW;
        if (uc == 0) break;
      } else {
        if (rpos >= strlen) break;
        if (options & UTF8PROC_UTF16)
          rpos += utf16proc_iterate((uint16_t*)(str + rpos), strlen - rpos, &uc) \
                  * sizeof(uint16_t);
        else
          rpos += utf8proc_iterate(str + rpos, strlen - rpos, &uc);
        if (uc < 0) return UTF8PROC_ERROR_INVALIDUTF8;
      }
      decomp_result = utf8proc_decompose_char(
        uc, buffer + wpos, (bufsize > wpos) ? (bufsize - wpos) : 0, options,
        &boundclass);
      if (decomp_result < 0) return decomp_result;
      wpos += decomp_result;
      // prohibiting integer overflows due to too long strings:
      if (wpos < 0 || wpos > SSIZE_MAX/sizeof(int32_t)/2)
        return UTF8PROC_ERROR_OVERFLOW;
    }
    wpos += utf8proc_flush(buffer + wpos, (bufsize > wpos) ? (bufsize - wpos) : 0,
                           options, &boundclass);
  }
  if ((options & (UTF8PROC_COMPOSE|UTF8PROC_DECOMPOSE)) && bufsize >= wpos) {
    ssize_t pos = 0;
    while (pos < wpos-1) {
      int32_t uc1, uc2;
      const utf8proc_property_t *property1, *property2;
      uc1 = buffer[pos];
      uc2 = buffer[pos+1];
      property1 = utf8proc_get_property(uc1);
      property2 = utf8proc_get_property(uc2);
      if (property1->combining_class > property2->combining_class &&
          property2->combining_class > 0) {
        buffer[pos] = uc2;
        buffer[pos+1] = uc1;
        if (pos > 0) pos--; else pos++;
      } else {
        pos++;
      }
    }
  }
  return wpos;
}

ssize_t utf8proc_reencode(int32_t *buffer, ssize_t length, int options) {
  // UTF8PROC_NULLTERM option will be ignored, 'length' is never ignored
  // ASSERT: 'buffer' has one spare byte of free space at the end!
  if (options & (UTF8PROC_NLF2LS | UTF8PROC_NLF2PS | UTF8PROC_STRIPCC)) {
    ssize_t rpos;
    ssize_t wpos = 0;
    int32_t uc;
    for (rpos = 0; rpos < length; rpos++) {
      uc = buffer[rpos];
      if (uc == 0x000D && rpos < length-1 && buffer[rpos+1] == 0x000A) rpos++;
      if (uc == 0x000A || uc == 0x000D || uc == 0x0085 ||
          ((options & UTF8PROC_STRIPCC) && (uc == 0x000B || uc == 0x000C))) {
        if (options & UTF8PROC_NLF2LS) {
          if (options & UTF8PROC_NLF2PS) {
            buffer[wpos++] = 0x000A;
          } else {
            buffer[wpos++] = 0x2028;
          }
        } else {
          if (options & UTF8PROC_NLF2PS) {
            buffer[wpos++] = 0x2029;
          } else {
            buffer[wpos++] = 0x0020;
          }
        }
      } else if ((options & UTF8PROC_STRIPCC) &&
          (uc < 0x0020 || (uc >= 0x007F && uc < 0x00A0))) {
        if (uc == 0x0009) buffer[wpos++] = 0x0020;
      } else {
        buffer[wpos++] = uc;
      }
    }
    length = wpos;
  }
  if (options & UTF8PROC_COMPOSE) {
    int32_t *starter = NULL;
    int32_t current_char;
    const utf8proc_property_t *starter_property = NULL, *current_property;
    utf8proc_propval_t max_combining_class = -1;
    ssize_t rpos;
    ssize_t wpos = 0;
    int32_t composition;
    for (rpos = 0; rpos < length; rpos++) {
      current_char = buffer[rpos];
      current_property = utf8proc_get_property(current_char);
      if (starter && current_property->combining_class > max_combining_class) {
        // combination perhaps possible
        int32_t hangul_lindex;
        int32_t hangul_sindex;
        hangul_lindex = *starter - UTF8PROC_HANGUL_LBASE;
        if (hangul_lindex >= 0 && hangul_lindex < UTF8PROC_HANGUL_LCOUNT) {
          int32_t hangul_vindex;
          hangul_vindex = current_char - UTF8PROC_HANGUL_VBASE;
          if (hangul_vindex >= 0 && hangul_vindex < UTF8PROC_HANGUL_VCOUNT) {
            *starter = UTF8PROC_HANGUL_SBASE +
              (hangul_lindex * UTF8PROC_HANGUL_VCOUNT + hangul_vindex) *
              UTF8PROC_HANGUL_TCOUNT;
            starter_property = NULL;
            continue;
          }
        }
        hangul_sindex = *starter - UTF8PROC_HANGUL_SBASE;
        if (hangul_sindex >= 0 && hangul_sindex < UTF8PROC_HANGUL_SCOUNT &&
            (hangul_sindex % UTF8PROC_HANGUL_TCOUNT) == 0) {
          int32_t hangul_tindex;
          hangul_tindex = current_char - UTF8PROC_HANGUL_TBASE;
          if (hangul_tindex >= 0 && hangul_tindex < UTF8PROC_HANGUL_TCOUNT) {
            *starter += hangul_tindex;
            starter_property = NULL;
            continue;
          }
        }
        if (!starter_property) {
          starter_property = utf8proc_get_property(*starter);
        }
        if (!starter_property->excl_compression &&
            starter_property->comb1st_index >= 0 &&
            current_property->comb2nd_index >= 0) {
          composition = utf8proc_combinations[
            starter_property->comb1st_index +
            current_property->comb2nd_index
          ];
          if (composition >= 0 && (!(options & UTF8PROC_STABLE) ||
              !(utf8proc_get_property(composition)->comp_exclusion))) {
            *starter = composition;
            starter_property = NULL;
            continue;
          }
        }
      }
      buffer[wpos] = current_char;
      if (current_property->combining_class) {
        if (current_property->combining_class > max_combining_class) {
          max_combining_class = current_property->combining_class;
        }
      } else {
        starter = buffer + wpos;
        starter_property = NULL;
        max_combining_class = -1;
      }
      wpos++;
    }
    length = wpos;
  }
  {
    ssize_t rpos, wpos = 0;
    int32_t uc;
    for (rpos = 0; rpos < length; rpos++) {
      uc = buffer[rpos];
      if (options & UTF8PROC_UTF16)
        wpos += sizeof(uint16_t) * \
                utf16proc_encode_char(uc, (uint16_t*)((uint8_t *)buffer + wpos));
      else
        wpos += utf8proc_encode_char(uc, ((uint8_t *)buffer) + wpos);
    }
    ((uint8_t *)buffer)[wpos] = 0;
    return wpos;
  }
}

ssize_t utf8proc_map(
  const uint8_t *str, ssize_t strlen, uint8_t **dstptr, int options
) {
  int32_t *buffer;
  ssize_t result;
  *dstptr = NULL;
  result = utf8proc_decompose(str, strlen, NULL, 0, options);
  if (result < 0) return result;
  buffer = malloc(result * sizeof(int32_t) + 1);
  if (!buffer) return UTF8PROC_ERROR_NOMEM;
  result = utf8proc_decompose(str, strlen, buffer, result, options);
  if (result < 0) {
    free(buffer);
    return result;
  }
  result = utf8proc_reencode(buffer, result, options);
  if (result < 0) {
    free(buffer);
    return result;
  }
  {
    int32_t *newptr;
    newptr = realloc(buffer, result+1);
    if (newptr) buffer = newptr;
  }
  *dstptr = (uint8_t *)buffer;
  return result;
}

uint8_t *utf8proc_NFD(const uint8_t *str) {
  uint8_t *retval;
  utf8proc_map(str, 0, &retval, UTF8PROC_NULLTERM | UTF8PROC_STABLE |
    UTF8PROC_DECOMPOSE);
  return retval;
}

uint8_t *utf8proc_NFC(const uint8_t *str) {
  uint8_t *retval;
  utf8proc_map(str, 0, &retval, UTF8PROC_NULLTERM | UTF8PROC_STABLE |
    UTF8PROC_COMPOSE);
  return retval;
}

uint8_t *utf8proc_NFKD(const uint8_t *str) {
  uint8_t *retval;
  utf8proc_map(str, 0, &retval, UTF8PROC_NULLTERM | UTF8PROC_STABLE |
    UTF8PROC_DECOMPOSE | UTF8PROC_COMPAT);
  return retval;
}

uint8_t *utf8proc_NFKC(const uint8_t *str) {
  uint8_t *retval;
  utf8proc_map(str, 0, &retval, UTF8PROC_NULLTERM | UTF8PROC_STABLE |
    UTF8PROC_COMPOSE | UTF8PROC_COMPAT);
  return retval;
}

ssize_t utf16proc_reencode(int32_t *buffer, ssize_t length, int options) {
  ssize_t retval;
  options |= UTF8PROC_UTF16;
  retval = utf8proc_reencode(buffer, length, options);
  return retval < 0 ? retval : (retval/sizeof(uint16_t));
}

ssize_t utf16proc_decompose_char(int32_t uc, int32_t *dst, ssize_t bufsize,
                                 int options, bound_attr_t *last_bound_attr)
{
  options |= UTF8PROC_UTF16;
  return utf8proc_decompose_char(uc, dst, bufsize, options, last_bound_attr);
}

ssize_t utf16proc_decompose(const uint16_t *str, ssize_t strlen,
                            int32_t *buffer, ssize_t bufsize, int options)
{
  options |= UTF8PROC_UTF16;
  return utf8proc_decompose((uint8_t*)str, strlen*2, buffer, bufsize, options);
}

ssize_t utf16proc_map(const uint16_t *str, ssize_t strlen, uint16_t **dstptr,
                      int options)
{
  ssize_t retval;
  options |= UTF8PROC_UTF16;
  retval = utf8proc_map((const uint8_t *)str, strlen * sizeof(uint16_t),
                        (uint8_t **)dstptr, options);
  return retval < 0 ? retval : (retval/sizeof(uint16_t));
}

uint16_t *utf16proc_NFD(const uint16_t *str) {
  uint16_t *retval;
  utf16proc_map(str, 0, &retval,
                UTF8PROC_NULLTERM | UTF8PROC_STABLE | UTF8PROC_DECOMPOSE);
  return retval;
}

uint16_t *utf16proc_NFC(const uint16_t *str) {
  uint16_t *retval;
  utf16proc_map(str, 0, &retval,
                UTF8PROC_NULLTERM | UTF8PROC_STABLE | UTF8PROC_COMPOSE);
  return retval;
}

uint16_t *utf16proc_NFKD(const uint16_t *str) {
  uint16_t *retval;
  utf16proc_map(str, 0, &retval, UTF8PROC_NULLTERM | UTF8PROC_STABLE |
                                 UTF8PROC_DECOMPOSE | UTF8PROC_COMPAT);
  return retval;
}

uint16_t *utf16proc_NFKC(const uint16_t *str) {
  uint16_t *retval;
  utf16proc_map(str, 0, &retval, UTF8PROC_NULLTERM | UTF8PROC_STABLE |
                                 UTF8PROC_COMPOSE | UTF8PROC_COMPAT);
  return retval;
}


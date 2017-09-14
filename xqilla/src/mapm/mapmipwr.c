
/* 
 *  M_APM  -  mapmipwr.c
 *
 *  Copyright (C) 1999 - 2002   Michael C. Ring
 *
 *  Permission to use, copy, and distribute this software and its
 *  documentation for any purpose with or without fee is hereby granted, 
 *  provided that the above copyright notice appear in all copies and 
 *  that both that copyright notice and this permission notice appear 
 *  in supporting documentation.
 *
 *	Permission to modify the software is granted. Permission to distribute
 *	the modified code is granted. Modifications are to be distributed
 *	by using the file 'license.txt' as a template to modify the file header.
 *	'license.txt' is available in the official MAPM distribution.
 *
 *	This software is provided "as is" without express or implied warranty.
 *
 *	THIS FILE HAS BEEN MODIFIED FROM THE OFFICIAL MAPM DISTRIBUTION BY
 *	'XQilla project' on 2005/11/03.
 *	THIS FILE IS ORIGINALLY FROM MAPM VERSION 4.6.1.
 */

#include "m_apm_lc.h"

/****************************************************************************/
void	m_apm_integer_pow(M_APM rr, int places, M_APM aa, int mexp)
{
M_APM   tmp0, tmpy, tmpz;
int	nexp, ii, signflag, local_precision;

if (mexp == 0)
  {
   m_apm_copy(rr, MM_One);
   return;
  }
else
  {
   if (mexp > 0)
     {
      signflag = 0;
      nexp     = mexp;
     }
   else
     {
      signflag = 1;
      nexp     = -mexp;
     }
  }

if (aa->m_apm_sign == 0)
  {
   M_set_to_zero(rr);
   return;
  }

tmp0 = M_get_stack_var();
tmpy = M_get_stack_var();
tmpz = M_get_stack_var();

local_precision = places + 8;

m_apm_copy(tmpy, MM_One);
m_apm_copy(tmpz, aa);

while (TRUE)
  {
   ii   = nexp & 1;
   nexp = nexp >> 1;

   if (ii != 0)                       /* exponent -was- odd */
     {
      m_apm_multiply(tmp0, tmpy, tmpz);
      m_apm_round(tmpy, local_precision, tmp0);

      if (nexp == 0)
        break;
     }

   m_apm_multiply(tmp0, tmpz, tmpz);
   m_apm_round(tmpz, local_precision, tmp0);
  }

if (signflag)
  {
   m_apm_reciprocal(rr, places, tmpy);
  }
else
  {
   m_apm_round(rr, places, tmpy);
  }

M_restore_stack(3);
}
/****************************************************************************/

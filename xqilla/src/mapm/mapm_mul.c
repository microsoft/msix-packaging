
/* 
 *  M_APM  -  mapm_mul.c
 *
 *  Copyright (C) 1999 - 2003   Michael C. Ring
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

extern void M_fast_multiply(M_APM, M_APM, M_APM);

/****************************************************************************/
void	m_apm_multiply(M_APM r, M_APM a, M_APM b)
{
int	ai, itmp, sign, nexp, ii, jj, index1, index2, 
	indexr, index0, numdigits;
UCHAR   *cp, *cp_div, *cp_rem;
void	*vp;

sign = a->m_apm_sign * b->m_apm_sign;
nexp = a->m_apm_exponent + b->m_apm_exponent;

if (sign == 0)      /* one number is zero, result is zero */
  {
   M_set_to_zero(r);
   return;
  }

numdigits = a->m_apm_datalength + b->m_apm_datalength;
index1 = (a->m_apm_datalength + 1) >> 1;
index2 = (b->m_apm_datalength + 1) >> 1;

/* 
 *	If we are multiplying 2 'big' numbers, use the fast algorithm. 
 *
 *	This is a **very** approx break even point between this algorithm
 *      and the FFT multiply. Note that different CPU's, operating systems, 
 *      and compiler's may yield a different break even point. This point
 *      (~96 decimal digits) is how the experiment came out on the 
 *      author's system. 
 */

if (index1 >= 48 && index2 >= 48)
  {
   M_fast_multiply(r, a, b);
   return;
  }

ii = (numdigits + 1) >> 1;     /* required size of result, in bytes */

if (ii > r->m_apm_malloclength)
  {
   if ((vp = MAPM_REALLOC(r->m_apm_data, (ii + 32))) == NULL)
     {
      /* fatal, this does not return */

      M_apm_log_error_msg(M_APM_EXIT, "\'m_apm_multiply\', Out of memory");
     }
  
   r->m_apm_malloclength = ii + 28;
   r->m_apm_data = (UCHAR *)vp;
  }

M_get_div_rem_addr(&cp_div, &cp_rem);

index0 = index1 + index2;
cp = r->m_apm_data;
memset(cp, 0, index0);
ii = index1;

while (TRUE)
  {
   indexr = --index0;
   jj = index2;
   ai = (int)a->m_apm_data[--ii];

   while (TRUE)
     {
      itmp = ai * b->m_apm_data[--jj];

      /*
       *  M_get_div_rem(itmp, &numdiv, &numrem);
       *  cp[indexr]   += numrem;
       *  cp[indexr-1] += numdiv;
       */

      cp[indexr]   += cp_rem[itmp];
      cp[indexr-1] += cp_div[itmp];

      if (cp[indexr] >= 100)
        {
         cp[indexr] -= 100;
         cp[indexr-1] += 1;
	}

      indexr--;

      if (cp[indexr] >= 100)
        {
         cp[indexr] -= 100;
         cp[indexr-1] += 1;
	}

      if (jj == 0)
        break;
     }

   if (ii == 0)
     break;
  }

r->m_apm_sign       = sign;
r->m_apm_exponent   = nexp;
r->m_apm_datalength = numdigits;

M_apm_normalize(r);
}
/****************************************************************************/

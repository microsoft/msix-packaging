
/* 
 *  M_APM  -  mapmistr.c
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

/****************************************************************************/
void	m_apm_to_integer_string(char *s, M_APM mtmp)
{
void    *vp;
UCHAR	*ucp, numdiv, numrem;
char	*cp, *p, sbuf[128];
int	ct, dl, numb, ii;

vp = NULL;
ct = mtmp->m_apm_exponent;
dl = mtmp->m_apm_datalength;

/*
 *  if |input| < 1, result is "0"
 */

if (ct <= 0 || mtmp->m_apm_sign == 0)
  {
   s[0] = '0';
   s[1] = '\0';
   return;
  }

if (ct > 112)
  {
   if ((vp = (void *)MAPM_MALLOC((ct + 32) * sizeof(char))) == NULL)
     {
      /* fatal, this does not return */

      M_apm_log_error_msg(M_APM_EXIT, 
                          "\'m_apm_to_integer_string\', Out of memory");
     }

   cp = (char *)vp;
  }
else
  {
   cp = sbuf;
  }

p  = cp;
ii = 0;

/* handle a negative number */

if (mtmp->m_apm_sign == -1)
  {
   ii = 1;
   *p++ = '-';
  }

/* get num-bytes of data (#digits / 2) to use in the string */

if (ct > dl)
  numb = (dl + 1) >> 1;
else
  numb = (ct + 1) >> 1;

ucp = mtmp->m_apm_data;

while (TRUE)
  {
   M_get_div_rem_10((int)(*ucp++), &numdiv, &numrem);

   *p++ = numdiv + '0';
   *p++ = numrem + '0';

   if (--numb == 0)
     break;
  }

/* pad with trailing zeros if the exponent > datalength */
 
if (ct > dl)
  memset(p, '0', (ct + 1 - dl));

cp[ct + ii] = '\0';
strcpy(s, cp);

if (vp != NULL)
  MAPM_FREE(vp);
}
/****************************************************************************/

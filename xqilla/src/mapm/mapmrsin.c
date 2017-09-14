
/* 
 *  M_APM  -  mapmrsin.c
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
/*
                                x^3     x^5     x^7     x^9
		 sin(x) == x -  ---  +  ---  -  ---  +  ---  ...
                                 3!      5!      7!      9!
*/
void	M_raw_sin(M_APM rr, int places, M_APM xx)
{
M_APM	sum, term, tmp7, tmp8, tmp9;
int     tolerance, flag, local_precision, prev_exp;
long	m1, m2;

sum  = M_get_stack_var();
term = M_get_stack_var();
tmp7 = M_get_stack_var();
tmp8 = M_get_stack_var();
tmp9 = M_get_stack_var();

m_apm_copy(sum, xx);
m_apm_copy(term, xx);
m_apm_multiply(tmp8, xx, xx);
m_apm_round(tmp9, (places + 6), tmp8);

local_precision = places + 8;
tolerance       = -(places + 4);
prev_exp        = 0;

m1   = 2L;
flag = 0;

while (TRUE)
  {
   m2 = m1 * (m1 + 1);
   m_apm_set_long(tmp7, m2);

   m_apm_multiply(tmp8, term, tmp9);
   m_apm_divide(term, local_precision, tmp8, tmp7);

   if (flag == 0)
     {
      m_apm_subtract(tmp7, sum, term);
      m_apm_copy(sum, tmp7);
     }
   else
     {
      m_apm_add(tmp7, sum, term);
      m_apm_copy(sum, tmp7);
     }

   if ((term->m_apm_exponent < tolerance) || (term->m_apm_sign == 0))
     break;

   if (m1 != 2L)
     {
      local_precision = local_precision + term->m_apm_exponent - prev_exp;

      if (local_precision < 20)
        local_precision = 20;
     }

   prev_exp = term->m_apm_exponent;

   m1 += 2;
   flag = 1 - flag;
  }

m_apm_round(rr, places, sum);
M_restore_stack(5);
}
/****************************************************************************/
/*
                                x^2     x^4     x^6     x^8
		 cos(x) == 1 -  ---  +  ---  -  ---  +  ---  ...
                                 2!      4!      6!      8!
*/
void	M_raw_cos(M_APM rr, int places, M_APM xx)
{
M_APM	sum, term, tmp7, tmp8, tmp9;
int     tolerance, flag, local_precision, prev_exp;
long	m1, m2;

sum  = M_get_stack_var();
term = M_get_stack_var();
tmp7 = M_get_stack_var();
tmp8 = M_get_stack_var();
tmp9 = M_get_stack_var();

m_apm_copy(sum, MM_One);
m_apm_copy(term, MM_One);

m_apm_multiply(tmp8, xx, xx);
m_apm_round(tmp9, (places + 6), tmp8);

local_precision = places + 8;
tolerance       = -(places + 4);
prev_exp        = 0;

m1   = 1L;
flag = 0;

while (TRUE)
  {
   m2 = m1 * (m1 + 1);
   m_apm_set_long(tmp7, m2);

   m_apm_multiply(tmp8, term, tmp9);
   m_apm_divide(term, local_precision, tmp8, tmp7);

   if (flag == 0)
     {
      m_apm_subtract(tmp7, sum, term);
      m_apm_copy(sum, tmp7);
     }
   else
     {
      m_apm_add(tmp7, sum, term);
      m_apm_copy(sum, tmp7);
     }

   if ((term->m_apm_exponent < tolerance) || (term->m_apm_sign == 0))
     break;

   if (m1 != 1L)
     {
      local_precision = local_precision + term->m_apm_exponent - prev_exp;

      if (local_precision < 20)
        local_precision = 20;
     }

   prev_exp = term->m_apm_exponent;

   m1 += 2;
   flag = 1 - flag;
  }

m_apm_round(rr, places, sum);
M_restore_stack(5);
}
/****************************************************************************/

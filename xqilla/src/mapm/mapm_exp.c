
/* 
 *  M_APM  -  mapm_exp.c
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

static  M_APM  MM_exp_log2R;
static  M_APM  MM_exp_1024R;
static	int    MM_firsttime1 = TRUE;

/****************************************************************************/
void	M_free_all_exp()
{
if (MM_firsttime1 == FALSE)
  {
   m_apm_free(MM_exp_log2R);
   m_apm_free(MM_exp_1024R);

   MM_firsttime1 = TRUE;
  }
}
/****************************************************************************/
void	m_apm_exp(M_APM r, int places, M_APM x)
{
M_APM   tmp7, tmp8, tmp9;
char    outstr[64];
int	dplaces, nn, ii;

if (MM_firsttime1)
  {
   MM_firsttime1 = FALSE;

   MM_exp_log2R = m_apm_init();
   MM_exp_1024R = m_apm_init();

   m_apm_set_string(MM_exp_log2R, "1.44269504089");   /* ~ 1 / log(2) */
   m_apm_set_string(MM_exp_1024R, "9.765625E-4");     /*   1 / 1024   */
  }

tmp7 = M_get_stack_var();
tmp8 = M_get_stack_var();
tmp9 = M_get_stack_var();

/*
    From David H. Bailey's MPFUN Fortran package :

    exp (t) =  (1 + r + r^2 / 2! + r^3 / 3! + r^4 / 4! ...) ^ q * 2 ^ n

    where q = 256, r = t' / q, t' = t - n Log(2) and where n is chosen so
    that -0.5 Log(2) < t' <= 0.5 Log(2).  Reducing t mod Log(2) and
    dividing by 256 insures that -0.001 < r <= 0.001, which accelerates
    convergence in the above series.

    we won't let n == 1 or -1. if n computes out to be 1, we will set
    n = 0. this is so we can still calculate log(2). if we let n = 1, 
    then we need log(2) in order to compute log(2), which is no good ...

    if the new 'n' == 0, we will then use q = 1024 instead of 256 so
    the series still converges as fast.

    since the value of log(2) is critical to this function, log, and
    log10, we will keep our own local copy of it. if we need more
    precision in our log(2), we will re-compute it on the fly to get
    the desired precision.
*/

/* find 'n' and convert it to a normal C int            */
/* we just need an approx 1/log(2) for this calculation */

m_apm_multiply(tmp8, x, MM_exp_log2R);

/* round to the nearest int */

if (x->m_apm_sign >= 0)
  m_apm_add(tmp9, tmp8, MM_0_5);
else
  m_apm_subtract(tmp9, tmp8, MM_0_5);

m_apm_to_integer_string(outstr, tmp9);
nn = atoi(outstr);

if (abs(nn) == 1)
  nn = 0;

if (nn == 0)
  {
   ii = 10;
   dplaces = places + 8;
   m_apm_multiply(tmp9, x, MM_exp_1024R);
  }
else
  {
   ii = 8;
   dplaces = places + 6;

   /* check to make sure our log(2) is accurate enough */

   M_check_log_places(dplaces);

   m_apm_set_long(tmp7, (long)nn);
   m_apm_multiply(tmp8, tmp7, MM_lc_log2);
   m_apm_subtract(tmp7, x, tmp8);

   /*
    *     guarantee that |tmp7| < 1.
    *     the normal case will break out immediately.
    *	  this is just here to plan for the future.
    *     when sizeof(int) gets real big, it may
    *     be possible for the 'nn' calculation from
    *     above to not be optimized due to the inexact
    *     constant used for 'MM_exp_log2R'.
    */

   while (TRUE)
     {
      if (tmp7->m_apm_exponent <= 0)
        break;
     
      if (tmp7->m_apm_sign == 0)
        break;
     
      if (tmp7->m_apm_sign == 1)
        {
	 nn++;
         m_apm_subtract(tmp8, tmp7, MM_lc_log2);
	 m_apm_copy(tmp7, tmp8);
	}
      else
        {
	 nn--;
         m_apm_add(tmp8, tmp7, MM_lc_log2);
	 m_apm_copy(tmp7, tmp8);
	}
     }

   m_apm_multiply(tmp9, tmp7, MM_5x_256R);
  }

/* perform the series expansion ... */

M_raw_exp(tmp8, dplaces, tmp9);

/*
 *   raise result to the 256 OR 1024 power (q = 256 OR 1024) 
 *
 *   note : x ^ 256    == (((x ^ 2) ^ 2) ^ 2) ... 8  times
 *        : x ^ 1024   == (((x ^ 2) ^ 2) ^ 2) ... 10 times
 */

while (--ii >= 0)   
  {
   m_apm_multiply(tmp9, tmp8, tmp8);
   m_apm_round(tmp8, dplaces, tmp9);
  }

/* now compute 2 ^ N */

m_apm_integer_pow(tmp7, dplaces, MM_Two, nn);

m_apm_multiply(tmp9, tmp7, tmp8);
m_apm_round(r, places, tmp9);
M_restore_stack(3);                    /* restore the 3 locals we used here */
}
/****************************************************************************/
/*
	calculate the exponential function using the following series :

                              x^2     x^3     x^4     x^5
	exp(x) == 1  +  x  +  ---  +  ---  +  ---  +  ---  ...
                               2!      3!      4!      5!

*/
void	M_raw_exp(M_APM rr, int places, M_APM xx)
{
M_APM   tmp0, digit, term;
int	tolerance,  local_precision, prev_exp;
long    m1;

tmp0  = M_get_stack_var();
term  = M_get_stack_var();
digit = M_get_stack_var();

local_precision = places + 8;
tolerance       = -(places + 4);
prev_exp        = 0;

m_apm_add(rr, MM_One, xx);
m_apm_copy(term, xx);

m1 = 2L;

while (TRUE)
  {
   m_apm_set_long(digit, m1);
   m_apm_multiply(tmp0, term, xx);
   m_apm_divide(term, local_precision, tmp0, digit);
   m_apm_add(tmp0, rr, term);
   m_apm_copy(rr, tmp0);

   if ((term->m_apm_exponent < tolerance) || (term->m_apm_sign == 0))
     break;

   if (m1 != 2L)
     {
      local_precision = local_precision + term->m_apm_exponent - prev_exp;

      if (local_precision < 20)
        local_precision = 20;
     }

   prev_exp = term->m_apm_exponent;
   m1++;
  }

M_restore_stack(3);                    /* restore the 3 locals we used here */
}
/****************************************************************************/

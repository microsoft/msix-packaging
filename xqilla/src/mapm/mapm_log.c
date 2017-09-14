
/* 
 *  M_APM  -  mapm_log.c
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
#include <math.h>

/****************************************************************************/
/*
        Calls the LOG function. The formula used is :

        log10(x)  =  A * log(x) where A = log  (e)  [0.43429448190325...]
                                             10
*/
void	m_apm_log10(M_APM rr, int places, M_APM aa)
{
int     dplaces;
M_APM   tmp8, tmp9;

tmp8 = M_get_stack_var();
tmp9 = M_get_stack_var();

dplaces = places + 4;
M_check_log_places(dplaces + 45);

m_apm_log(tmp9, dplaces, aa);
m_apm_multiply(tmp8, tmp9, MM_lc_log10R);
m_apm_round(rr, places, tmp8);
M_restore_stack(2);                    /* restore the 2 locals we used here */
}
/****************************************************************************/
void	m_apm_log(M_APM r, int places, M_APM a)
{
M_APM   tmp0, tmp1, tmp2;
int	mexp, dplaces;

if (a->m_apm_sign <= 0)
  {
   M_apm_log_error_msg(M_APM_RETURN, 
                       "Warning! ... \'m_apm_log\', Negative argument");
   M_set_to_zero(r);
   return;
  }

tmp0 = M_get_stack_var();
tmp1 = M_get_stack_var();
tmp2 = M_get_stack_var();

dplaces = places + 8;

/*
 *    if the input is real close to 1, use the series expansion
 *    to compute the log.
 *    
 *    0.9999 < a < 1.0001
 */

m_apm_subtract(tmp0, a, MM_One);

if (tmp0->m_apm_sign == 0)    /* is input exactly 1 ?? */
  {                           /* if so, result is 0    */
   M_set_to_zero(r);
   M_restore_stack(3);   
   return;
  }

if (tmp0->m_apm_exponent <= -4)
  {
   M_log_near_1(r, places, tmp0);
   M_restore_stack(3);   
   return;
  }

/* make sure our log(10) is accurate enough for this calculation */
/* (and log(2) which is called from M_log_basic_iteration) */

M_check_log_places(dplaces + 25);

mexp = a->m_apm_exponent;
if (mexp >= -4 && mexp <= 4)
  {
   M_log_basic_iteration(r, places, a);
  }
else
  {
   /*
    *  use log (x * y) = log(x) + log(y)
    *
    *  here we use y = exponent of our base 10 number.
    *
    *  let 'C' = log(10) = 2.3025850929940....
    *
    *  then log(x * y) = log(x) + ( C * base_10_exponent )
    */

   m_apm_copy(tmp2, a);
   
   mexp = tmp2->m_apm_exponent - 2;
   tmp2->m_apm_exponent = 2;              /* force number between 10 & 100 */
   
   M_log_basic_iteration(tmp0, dplaces, tmp2);
   
   m_apm_set_long(tmp1, (long)mexp);
   m_apm_multiply(tmp2, tmp1, MM_lc_log10);
   m_apm_add(tmp1, tmp2, tmp0);
   
   m_apm_round(r, places, tmp1);
  }

M_restore_stack(3);                    /* restore the 3 locals we used here */
}
/****************************************************************************/
/*
	calculate log (1 + x) with the following series:

              x
	y = -----      ( |y| < 1 )
	    x + 2


            [ 1 + y ]                 y^3     y^5     y^7
	log [-------]  =  2 * [ y  +  ---  +  ---  +  ---  ... ] 
            [ 1 - y ]                  3       5       7 

*/
void	M_log_near_1(M_APM rr, int places, M_APM xx)
{
M_APM   tmp0, tmp1, tmp2, tmpS, term;
int	tolerance,  local_precision;
long    m1;

tmp0 = M_get_stack_var();
tmp1 = M_get_stack_var();
tmp2 = M_get_stack_var();
tmpS = M_get_stack_var();
term = M_get_stack_var();

tolerance       = xx->m_apm_exponent - places - 4;
local_precision = places + 8 - xx->m_apm_exponent;

m_apm_add(tmp0, xx, MM_Two);
m_apm_divide(tmpS, (local_precision + 6), xx, tmp0);

m_apm_copy(term, tmpS);
m_apm_multiply(tmp0, tmpS, tmpS);
m_apm_round(tmp2, (local_precision + 6), tmp0);

m1 = 3;

while (TRUE)
  {
   m_apm_set_long(tmp1, m1);
   m_apm_multiply(tmp0, term, tmp2);
   m_apm_round(term, local_precision, tmp0);
   m_apm_divide(tmp0, local_precision, term, tmp1);
   m_apm_add(tmp1, tmpS, tmp0);

   if ((tmp0->m_apm_exponent < tolerance) || (tmp0->m_apm_sign == 0))
     break;

   m_apm_copy(tmpS, tmp1);
   m1 += 2;
  }

m_apm_multiply(tmp0, MM_Two, tmp1);
m_apm_round(rr, places, tmp0);

M_restore_stack(5);                    /* restore the 5 locals we used here */
}
/****************************************************************************/

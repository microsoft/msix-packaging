
/* 
 *  M_APM  -  mapmasn0.c
 *
 *  Copyright (C) 2000 - 2002   Michael C. Ring
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
 * 	THIS FILE IS ORIGINALLY FROM MAPM VERSION 4.6.1.
 */

#include "m_apm_lc.h"

/****************************************************************************/
/*
        Calculate arcsin using the identity :

                                      x
        arcsin (x) == arctan [ --------------- ]
                                sqrt(1 - x^2)

*/
void	M_arcsin_near_0(M_APM rr, int places, M_APM aa)
{
M_APM   tmp5, tmp6;

tmp5 = M_get_stack_var();
tmp6 = M_get_stack_var();

M_cos_to_sin(tmp5, (places + 8), aa);
m_apm_divide(tmp6, (places + 8), aa, tmp5);
M_arctan_near_0(rr, places, tmp6);

M_restore_stack(2);
}
/****************************************************************************/
/*
        Calculate arccos using the identity :

        arccos (x) == PI / 2 - arcsin (x)

*/
void	M_arccos_near_0(M_APM rr, int places, M_APM aa)
{
M_APM   tmp1, tmp2;

tmp1 = M_get_stack_var();
tmp2 = M_get_stack_var();

M_check_PI_places(places);
M_arcsin_near_0(tmp1, (places + 4), aa);
m_apm_subtract(tmp2, MM_lc_HALF_PI, tmp1);
m_apm_round(rr, places, tmp2);

M_restore_stack(2);
}
/****************************************************************************/
/*
	calculate arctan (x) with the following series:

                               x^3     x^5     x^7     x^9
	arctan (x)  ==   x  -  ---  +  ---  -  ---  +  ---  ...
                                3       5       7       9

*/
void	M_arctan_near_0(M_APM rr, int places, M_APM aa)
{
M_APM   tmp0, tmpR, tmp2, tmpS, digit, term;
int	tolerance,  local_precision;
long    m1;

tmp0  = M_get_stack_var();
tmp2  = M_get_stack_var();
tmpR  = M_get_stack_var();
tmpS  = M_get_stack_var();
term  = M_get_stack_var();
digit = M_get_stack_var();

tolerance       = aa->m_apm_exponent - places - 4;
local_precision = places + 8 - aa->m_apm_exponent;

m_apm_copy(term, aa);
m_apm_copy(tmpS, aa);
m_apm_multiply(tmp0, aa, aa);
m_apm_round(tmp2, (local_precision + 8), tmp0);

m1 = 1;

while (TRUE)
  {
   m1 += 2;
   m_apm_set_long(digit, m1);
   m_apm_multiply(tmp0, term, tmp2);
   m_apm_round(term, local_precision, tmp0);
   m_apm_divide(tmp0, local_precision, term, digit);
   m_apm_subtract(tmpR, tmpS, tmp0);

   if ((tmp0->m_apm_exponent < tolerance) || (tmp0->m_apm_sign == 0))
     {
      m_apm_round(rr, places, tmpR);
      break;
     }

   m1 += 2;
   m_apm_set_long(digit, m1);
   m_apm_multiply(tmp0, term, tmp2);
   m_apm_round(term, local_precision, tmp0);
   m_apm_divide(tmp0, local_precision, term, digit);
   m_apm_add(tmpS, tmpR, tmp0);

   if ((tmp0->m_apm_exponent < tolerance) || (tmp0->m_apm_sign == 0))
     {
      m_apm_round(rr, places, tmpS);
      break;
     }
  }

M_restore_stack(6);                    /* restore the 6 locals we used here */
}
/****************************************************************************/

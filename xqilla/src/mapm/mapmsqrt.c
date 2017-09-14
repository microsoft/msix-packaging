
/* 
 *  M_APM  -  mapmsqrt.c
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
void	m_apm_sqrt(M_APM rr, int places, M_APM aa)
{
M_APM   last_x, guess, tmpN, tmp7, tmp8, tmp9;
int	ii, bflag, nexp, tolerance, dplaces;

if (aa->m_apm_sign <= 0)
  {
   if (aa->m_apm_sign == -1)
     {
      M_apm_log_error_msg(M_APM_RETURN,
                "Warning! ... \'m_apm_sqrt\', Negative argument");
     }

   M_set_to_zero(rr);
   return;
  }

last_x = M_get_stack_var();
guess  = M_get_stack_var();
tmpN   = M_get_stack_var();
tmp7   = M_get_stack_var();
tmp8   = M_get_stack_var();
tmp9   = M_get_stack_var();

m_apm_copy(tmpN, aa);
nexp = 0;

/* 
    normalize the input number (make the exponent near 0) so
    the 'guess' function will not over/under flow on large
    magnitude exponents.
*/

if (aa->m_apm_exponent > 2)
  {
   nexp = aa->m_apm_exponent / 2;
   tmpN->m_apm_exponent -= 2 * nexp;
  }

if (aa->m_apm_exponent < -2)
  {
   nexp = aa->m_apm_exponent / 2 - 1;
   tmpN->m_apm_exponent -= 2 * nexp;
  }

M_get_sqrt_guess(guess, tmpN);    /* actually gets 1/sqrt guess */

tolerance = places + 4;
dplaces   = places + 16;
bflag     = FALSE;

m_apm_negate(last_x, MM_Ten);

/*   Use the following iteration to calculate 1 / sqrt(N) :

         X    =  0.5 * X * [ 3 - N * X^2 ]
          n+1                    
*/

ii = 0;

while (TRUE)
  {
   m_apm_multiply(tmp9, tmpN, guess);
   m_apm_multiply(tmp8, tmp9, guess);
   m_apm_round(tmp7, dplaces, tmp8);
   m_apm_subtract(tmp9, MM_Three, tmp7);
   m_apm_multiply(tmp8, tmp9, guess);
   m_apm_multiply(tmp9, tmp8, MM_0_5);

   if (bflag)
     break;

   m_apm_round(guess, dplaces, tmp9);

   /* force at least 2 iterations so 'last_x' has valid data */

   if (ii != 0)
     {
      m_apm_subtract(tmp7, guess, last_x);

      if (tmp7->m_apm_sign == 0)
        break;

      /* 
       *   if we are within a factor of 4 on the error term,
       *   we will be accurate enough after the *next* iteration
       *   is complete.  (note that the sign of the exponent on 
       *   the error term will be a negative number).
       */

      if ((-4 * tmp7->m_apm_exponent) > tolerance)
        bflag = TRUE;
     }

   m_apm_copy(last_x, guess);
   ii++;
  }

/*
 *    multiply by the starting number to get the final
 *    sqrt and then adjust the exponent since we found
 *    the sqrt of the normalized number.
 */

m_apm_multiply(tmp8, tmp9, tmpN);
m_apm_round(rr, places, tmp8);
rr->m_apm_exponent += nexp;

M_restore_stack(6);
}
/****************************************************************************/

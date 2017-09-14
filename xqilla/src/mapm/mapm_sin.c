
/* 
 *  M_APM  -  mapm_sin.c
 *
 *  Copyright (C) 1999 - 2002  Michael C. Ring
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
void	m_apm_sin(M_APM r, int places, M_APM a)
{
M_APM	tmp3;

tmp3 = M_get_stack_var();
M_limit_angle_to_pi(tmp3, (places + 6), a);
M_5x_sin(r, places, tmp3);
M_restore_stack(1);
}
/****************************************************************************/
void	m_apm_cos(M_APM r, int places, M_APM a)
{
M_APM	tmp3;

tmp3 = M_get_stack_var();
M_limit_angle_to_pi(tmp3, (places + 6), a);
M_4x_cos(r, places, tmp3);
M_restore_stack(1);
}
/****************************************************************************/
void	m_apm_sin_cos(M_APM sinv, M_APM cosv, int places, M_APM aa)
{
M_APM	tmp5, tmp6, tmp7;

tmp5 = M_get_stack_var();
tmp6 = M_get_stack_var();
tmp7 = M_get_stack_var();

M_limit_angle_to_pi(tmp5, (places + 6), aa);
M_4x_cos(tmp7, (places + 6), tmp5);

/*
 *   compute sin(x) = sqrt(1 - cos(x) ^ 2).
 *
 *   note that the sign of 'sin' will always be positive after the
 *   sqrt call. we need to adjust the sign based on what quadrant
 *   the original angle is in.
 */

M_cos_to_sin(tmp6, (places + 6), tmp7);
if (tmp6->m_apm_sign != 0)
  tmp6->m_apm_sign = tmp5->m_apm_sign;
 
m_apm_round(sinv, places, tmp6);
m_apm_round(cosv, places, tmp7);
M_restore_stack(3);
}
/****************************************************************************/
void	m_apm_tan(M_APM r, int places, M_APM a)
{
M_APM	tmps, tmpc, tmp0;

tmps = M_get_stack_var();
tmpc = M_get_stack_var();
tmp0 = M_get_stack_var();

m_apm_sin_cos(tmps, tmpc, (places + 4), a);
 
/* tan(x) = sin(x) / cos(x) */

m_apm_divide(tmp0, (places + 4), tmps, tmpc);
m_apm_round(r, places, tmp0);
M_restore_stack(3);
}
/****************************************************************************/
void	M_limit_angle_to_pi(M_APM rr, int places, M_APM aa)
{
M_APM	tmp7, tmp8, tmp9;

M_check_PI_places(places);

tmp9 = M_get_stack_var();
m_apm_copy(tmp9, MM_lc_PI);

if (m_apm_compare(aa, tmp9) == 1)       /*  > PI  */
  {
   tmp7 = M_get_stack_var();
   tmp8 = M_get_stack_var();

   m_apm_add(tmp7, aa, tmp9);
   m_apm_integer_divide(tmp9, tmp7, MM_lc_2_PI);
   m_apm_multiply(tmp8, tmp9, MM_lc_2_PI);
   m_apm_subtract(tmp9, aa, tmp8);
   m_apm_round(rr, places, tmp9);

   M_restore_stack(3);
   return;
  }

tmp9->m_apm_sign = -1;
if (m_apm_compare(aa, tmp9) == -1)       /*  < -PI  */
  {
   tmp7 = M_get_stack_var();
   tmp8 = M_get_stack_var();

   m_apm_add(tmp7, aa, tmp9);
   m_apm_integer_divide(tmp9, tmp7, MM_lc_2_PI);
   m_apm_multiply(tmp8, tmp9, MM_lc_2_PI);
   m_apm_subtract(tmp9, aa, tmp8);
   m_apm_round(rr, places, tmp9);

   M_restore_stack(3);
   return;
  }

m_apm_copy(rr, aa);
M_restore_stack(1);
}
/****************************************************************************/


/* 
 *  M_APM  -  mapmhsin.c
 *
 *  Copyright (C) 2000 - 2002  Michael C. Ring
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
 *      sinh(x) == 0.5 * [ exp(x) - exp(-x) ]
 */
void	m_apm_sinh(M_APM rr, int places, M_APM aa)
{
M_APM	tmp1, tmp2, tmp3;
int     local_precision;

tmp1 = M_get_stack_var();
tmp2 = M_get_stack_var();
tmp3 = M_get_stack_var();

local_precision = places + 4;

m_apm_exp(tmp1, local_precision, aa);
m_apm_reciprocal(tmp2, local_precision, tmp1);
m_apm_subtract(tmp3, tmp1, tmp2);
m_apm_multiply(tmp1, tmp3, MM_0_5);
m_apm_round(rr, places, tmp1);

M_restore_stack(3);
}
/****************************************************************************/
/*
 *      cosh(x) == 0.5 * [ exp(x) + exp(-x) ]
 */
void	m_apm_cosh(M_APM rr, int places, M_APM aa)
{
M_APM	tmp1, tmp2, tmp3;
int     local_precision;

tmp1 = M_get_stack_var();
tmp2 = M_get_stack_var();
tmp3 = M_get_stack_var();

local_precision = places + 4;

m_apm_exp(tmp1, local_precision, aa);
m_apm_reciprocal(tmp2, local_precision, tmp1);
m_apm_add(tmp3, tmp1, tmp2);
m_apm_multiply(tmp1, tmp3, MM_0_5);
m_apm_round(rr, places, tmp1);

M_restore_stack(3);
}
/****************************************************************************/
/*
 *      tanh(x) == [ exp(x) - exp(-x) ]  /  [ exp(x) + exp(-x) ]
 */
void	m_apm_tanh(M_APM rr, int places, M_APM aa)
{
M_APM	tmp1, tmp2, tmp3, tmp4;
int     local_precision;

tmp1 = M_get_stack_var();
tmp2 = M_get_stack_var();
tmp3 = M_get_stack_var();
tmp4 = M_get_stack_var();

local_precision = places + 4;

m_apm_exp(tmp1, local_precision, aa);
m_apm_reciprocal(tmp2, local_precision, tmp1);
m_apm_subtract(tmp3, tmp1, tmp2);
m_apm_add(tmp4, tmp1, tmp2);
m_apm_divide(tmp1, local_precision, tmp3, tmp4);
m_apm_round(rr, places, tmp1);

M_restore_stack(4);
}
/****************************************************************************/


/* 
 *  M_APM  -  mapmgues.c
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
 *	'XQilla project' on 2005/11/03 and 2015/12/04.
 *	THIS FILE IS ORIGINALLY FROM MAPM VERSION 4.6.1.
 */

#include "m_apm_lc.h"
#include <math.h>

/****************************************************************************/
void	M_get_sqrt_guess(M_APM r, M_APM a)
{
  /* sqrt algorithm actually finds 1/sqrt */
  m_apm_set_double(r, (1.0 / sqrt(m_apm_get_double(a))));
}
/****************************************************************************/
void	M_get_cbrt_guess(M_APM r, M_APM a)
{
  m_apm_set_double(r, exp(log(m_apm_get_double(a)) / 3.0));
}
/****************************************************************************/
/*
 *	for log, asin, and acos we induce an error of 10 ^ -5.
 *	this enables the iterative routine to be more efficient
 *	by knowing exactly how accurate the initial guess is.
 *
 *	but the real reason ... this also prevents some corner
 *	conditions where the iterative functions may terminate too soon.
 */
void	M_get_log_guess(M_APM r, M_APM a)
{
  /* induce error of 10 ^ -5 */
  m_apm_set_double(r, (1.00001 * log(m_apm_get_double(a))));
}
/****************************************************************************/
/*
 *	the implementation of the asin & acos functions 
 *	guarantee that 'a' is always < 0.85, so it is 
 *	safe to multiply by a number > 1
 */
void	M_get_asin_guess(M_APM r, M_APM a)
{
  /* induce error of 10 ^ -5 */
  m_apm_set_double(r, (1.00001 * asin(m_apm_get_double(a))));
}
/****************************************************************************/
void	M_get_acos_guess(M_APM r, M_APM a)
{
  /* induce error of 10 ^ -5 */
  m_apm_set_double(r, (1.00001 * acos(m_apm_get_double(a))));
}
/****************************************************************************/
/*
	convert a C 'double' into an M_APM value. 
*/
void	m_apm_set_double(M_APM atmp, double dd)
{
char	*cp, *p, *ps, buf[64];

if (dd == 0.0)                     /* special case for 0 exactly */
   m_apm_set_string(atmp, "0");
else
  {
   sprintf(buf,"%.14E",dd);
   
   cp = strstr(buf, "E");
   if (atoi(cp + sizeof(char)) == 0)
     *cp = '\0';
   
   p = cp;
   
   while (TRUE)
     {
      p--;
      if (*p == '0' || *p == '.' || *p == ',')
        *p = ' ';
      else
        break;
     }
   
   ps = buf;
   p  = buf;
   
   while (TRUE)
     {
      if ((*p = *ps) == '\0')
        break;
   
      if (*ps++ != ' ')
        p++;
     }

   m_apm_set_string(atmp, buf);
  }
}
/****************************************************************************/
/*
	convert a M_APM value into a C 'double'
*/
double m_apm_get_double(M_APM atmp)
{
  UCHAR	numdiv, numrem;

  double result = 0;
  int index=0;

  int max_i = (atmp->m_apm_datalength + 1) >> 1;
  if(max_i > ((17 + 1) >> 1)) {
    // We only need to deal with the top 17 digits, by which point we've
    // maxed out the double's mantissa.
    max_i = ((17 + 1) >> 1);
  }

  for(; index < max_i; ++index) {
    M_get_div_rem_10((int)atmp->m_apm_data[index], &numdiv, &numrem);
    result = result * 100 + (double)numdiv * 10 + numrem;
  }

  result *= pow(10, atmp->m_apm_exponent - (max_i << 1));

  if(atmp->m_apm_sign == -1)
    result *= -1;
  
  return result;
}
/****************************************************************************/

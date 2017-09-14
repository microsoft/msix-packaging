
/* 
 *  M_APM  -  mapm_set.c
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
void	m_apm_set_long(M_APM atmp, long mm)
{
int     ii, nbytes, len;
char	*p, *buf, ch, buf2[64];

/* if zero, return right away */

if (mm == 0)
  {
   M_set_to_zero(atmp);
   return;
  }

M_long_2_ascii(buf2, mm);     /* convert long -> ascii in base 10 */
buf = buf2;

if (mm < 0)
  {
   atmp->m_apm_sign = -1;
   buf++;                     /* get past '-' sign */
  }
else
  {
   atmp->m_apm_sign = 1;
  }

len = (int) strlen(buf);
atmp->m_apm_exponent = len;

/* least significant nibble of ODD data-length must be 0 */

if ((len & 1) != 0)
  {
   buf[len] = '0';
  }

/* remove any trailing '0' ... */

while (TRUE)
  {
   if (buf[--len] != '0')
     break;
  }

atmp->m_apm_datalength = ++len;

nbytes = (len + 1) >> 1;
p = buf;

for (ii=0; ii < nbytes; ii++)
  {
   ch = *p++ - '0';
   atmp->m_apm_data[ii] = 10 * ch + *p++ - '0';
  }
}
/****************************************************************************/
void	m_apm_set_string(M_APM ctmp, char *s_in)
{
char *M_buf;

char	ch, *cp, *s, *p;
void	*vp;
int	i, j, zflag, exponent, sign;

   if ((M_buf = (char *)MAPM_MALLOC(strlen(s_in) + 32)) == NULL)
     {
      /* fatal, this does not return */

      M_apm_log_error_msg(M_APM_EXIT, "\'m_apm_set_string\', Out of memory");
     }

s = M_buf;
strcpy(s,s_in);

/* default == zero ... */

M_set_to_zero(ctmp);

p = s;

while (TRUE)
  {
   if (*p == ' ' || *p == '\t')
     p++;
   else
     break;
  }

if (*p == '\0')
  return;

sign = 1;             /* assume number is positive */

if (*p == '+')        /* scan by optional '+' sign */
  p++;
else
  {
   if (*p == '-')     /* check if number negative */
     {
      sign = -1;
      p++;
     }
  }

M_lowercase(p);       /* convert string to lowercase */
exponent = 0;         /* default */
   
if ((cp = strstr(p,"e")) != NULL)
  {
   exponent = atoi(cp + sizeof(char));
   *cp = '\0';          /* erase the exponent now */
  }

j = M_strposition(p,".");        /* is there a decimal point ?? */
if (j == -1)
  {
   strcat(p,".");                /* if not, append one */
   j = M_strposition(p,".");     /* now find it ... */
  }

if (j > 0)                       /* normalize number and adjust exponent */
  {
   exponent += j;
   memmove((p+1),p,(j * sizeof(char)));
  }

p++;        /* scan past implied decimal point now in column 1 (index 0) */

i = (int) strlen(p);
ctmp->m_apm_datalength = i;

if ((i & 1) != 0)   /* if odd number of digits, append a '0' to make it even */
  strcat(p,"0");    

j = (int) strlen(p) >> 1;  /* number of bytes in encoded M_APM number */

/* do we need more memory to hold this number */

if (j > ctmp->m_apm_malloclength)
  {
   if ((vp = MAPM_REALLOC(ctmp->m_apm_data, (j + 32))) == NULL)
     {
      /* fatal, this does not return */

      M_apm_log_error_msg(M_APM_EXIT, "\'m_apm_set_string\', Out of memory");
     }
  
   ctmp->m_apm_malloclength = j + 28;
   ctmp->m_apm_data = (UCHAR *)vp;
  }

zflag = TRUE;

for (i=0; i < j; i++)
  {
   ch = *p++ - '0';
   if ((ch = (10 * ch + *p++ - '0')) != 0)
     zflag = FALSE;

   if (((int)ch & 0xFF) >= 100)
     {
      M_apm_log_error_msg(M_APM_RETURN,
      "Warning! \'m_apm_set_string\' : Non-digit char found in parse");

      M_apm_log_error_msg(M_APM_RETURN, "Text =");
      M_apm_log_error_msg(M_APM_RETURN, s_in);

      M_set_to_zero(ctmp);
      return;
     }

   ctmp->m_apm_data[i]   = ch;
   ctmp->m_apm_data[i+1] = 0;
  }

ctmp->m_apm_exponent = exponent;
ctmp->m_apm_sign     = sign;

if (zflag)
  {
   ctmp->m_apm_exponent   = 0;
   ctmp->m_apm_sign       = 0;
   ctmp->m_apm_datalength = 1;
  }
else
  {
   M_apm_normalize(ctmp);
  }

/*
 *  if our local temp string is getting too big,
 *  release it's memory and start over next time.
 *  (this 1000 byte threshold is quite arbitrary,
 *  it may be more efficient in your app to make
 *  this number bigger).
 */

   MAPM_FREE(M_buf);
}
/****************************************************************************/
void	m_apm_to_string(char *s, int places, M_APM mtmp)
{
M_APM   ctmp;
char	*cp;
int	i, index, first, max_i, num_digits, dec_places;
UCHAR	numdiv, numrem;

ctmp = M_get_stack_var();
dec_places = places;

if (dec_places < 0)
  m_apm_copy(ctmp, mtmp);
else
  m_apm_round(ctmp, dec_places, mtmp);

if (ctmp->m_apm_sign == 0)
  {
   if (dec_places < 0)
     strcpy(s,"0.0E0");  /* crioux: positive, no sign */
   else
     {
      strcpy(s,"0");

      if (dec_places > 0)
        strcat(s,".");

      for (i=0; i < dec_places; i++)
        strcat(s,"0");

      strcat(s,"E0");  /* crioux: positive, no sign */
     }

   M_restore_stack(1);
   return;
  }

max_i = (ctmp->m_apm_datalength + 1) >> 1;

if (dec_places < 0)
  num_digits = ctmp->m_apm_datalength;
else
  num_digits = dec_places + 1;

cp = s;

if (ctmp->m_apm_sign == -1)
  *cp++ = '-';

first = TRUE;

i = 0;
index = 0;

while (TRUE)
  {
   if (index >= max_i)
     {
      numdiv = 0;
      numrem = 0;
     }
   else
      M_get_div_rem_10((int)ctmp->m_apm_data[index],&numdiv,&numrem);

   index++;

   *cp++ = numdiv + '0';

   if (++i == num_digits)
     break;

   if (first)
     {
      first = FALSE;
      *cp++ = '.';
     }

   *cp++ = numrem + '0';

   if (++i == num_digits)
     break;
  }

i = ctmp->m_apm_exponent - 1;
if (i >= 0)
  sprintf(cp,"E%d",i);  /* crioux: positive, no sign (could remove branching here) */
else
  sprintf(cp,"E%d",i);

M_restore_stack(1);
}
/****************************************************************************/

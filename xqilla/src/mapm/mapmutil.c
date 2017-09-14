
/* 
 *  M_APM  -  mapmutil.c
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

static  UCHAR	*M_mul_div = NULL;
static  UCHAR   *M_mul_rem = NULL;

static  UCHAR   M_mul_div_10[100];
static	UCHAR   M_mul_rem_10[100];

static	int	M_util_firsttime = TRUE;
static	int     M_firsttime3 = TRUE;

static	M_APM	M_work_0_5;

/****************************************************************************/
M_APM	m_apm_init()
{
M_APM	atmp;

if ((atmp = (M_APM)MAPM_MALLOC(sizeof(M_APM_struct))) == NULL)
  {
   /* fatal, this does not return */

   M_apm_log_error_msg(M_APM_EXIT, "\'m_apm_init\', Out of memory");
  }

atmp->m_apm_id           = M_APM_IDENT;
atmp->m_apm_malloclength = 80;
atmp->m_apm_datalength   = 1;
atmp->m_apm_refcount     = 1;           /* not for us, for MAPM C++ class */
atmp->m_apm_exponent     = 0;
atmp->m_apm_sign         = 0;

if ((atmp->m_apm_data = (UCHAR *)MAPM_MALLOC(84)) == NULL)
  {
   /* fatal, this does not return */

   M_apm_log_error_msg(M_APM_EXIT, "\'m_apm_init\', Out of memory");
  }

atmp->m_apm_data[0] = 0;
return(atmp);
}
/****************************************************************************/
void	m_apm_free(M_APM atmp)
{
if (atmp->m_apm_id == M_APM_IDENT)
  {
   atmp->m_apm_id = 0x0FFFFFF0L;
   MAPM_FREE(atmp->m_apm_data);
   MAPM_FREE(atmp);
  }
else
  {
   M_apm_log_error_msg(M_APM_RETURN, 
                       "Invalid M_APM variable passed to \'m_apm_free\'");
  }
}
/****************************************************************************/
void	M_alloc_all_util()
{

  if (M_firsttime3) {
    M_firsttime3 = FALSE;
    M_init_util_data();
    M_init_trig_globals();
  }

if (M_util_firsttime)
  {
   M_util_firsttime = FALSE;

   M_work_0_5 = m_apm_init();
   m_apm_set_string(M_work_0_5, "5");
  }
}
/****************************************************************************/
void	M_free_all_util()
{
if (M_util_firsttime == FALSE)
  {
   m_apm_free(M_work_0_5);
   M_util_firsttime = TRUE;
  }

if (M_firsttime3 == FALSE)
  {
   MAPM_FREE(M_mul_div);
   MAPM_FREE(M_mul_rem);
  
   M_mul_div    = NULL;
   M_mul_rem    = NULL;
   M_firsttime3 = TRUE;
  }
}
/****************************************************************************/
/*
 *      just a dummy wrapper to keep some compilers from complaining
 */
int 	M_get_sizeof_int()
{
return(sizeof(int));
}
/****************************************************************************/
void	M_init_util_data()
{
int	k;
UCHAR   ndiv, nrem;

if (M_mul_div != NULL)
  return;

M_mul_div = (UCHAR *)MAPM_MALLOC(10000 * sizeof(UCHAR));
M_mul_rem = (UCHAR *)MAPM_MALLOC(10000 * sizeof(UCHAR));

if (M_mul_div == NULL || M_mul_rem == NULL)
  {
   /* fatal, this does not return */

   M_apm_log_error_msg(M_APM_EXIT,
                       "M_init_util_data() could not allocate memory");
  }

ndiv = 0;
nrem = 0;

for (k=0; k < 100; k++)
  {
   M_mul_div_10[k] = ndiv;
   M_mul_rem_10[k] = nrem;

   if (++nrem == 10)
     {
      nrem = 0;
      ndiv++;
     }
  }

ndiv = 0;
nrem = 0;

for (k=0; k < 10000; k++)
  {
   M_mul_div[k] = ndiv;
   M_mul_rem[k] = nrem;

   if (++nrem == 100)
     {
      nrem = 0;
      ndiv++;
     }
  }
}
/****************************************************************************/
void	M_get_div_rem_addr(UCHAR **ndivp, UCHAR **nremp)
{
*ndivp = M_mul_div;
*nremp = M_mul_rem;
}
/****************************************************************************/
void	M_get_div_rem(int tbl_lookup, UCHAR *ndiv, UCHAR *nrem)
{
*ndiv = M_mul_div[tbl_lookup];
*nrem = M_mul_rem[tbl_lookup];
}
/****************************************************************************/
void	M_get_div_rem_10(int tbl_lookup, UCHAR *ndiv, UCHAR *nrem)
{
*ndiv = M_mul_div_10[tbl_lookup];
*nrem = M_mul_rem_10[tbl_lookup];
}
/****************************************************************************/
void	m_apm_round(M_APM btmp, int places, M_APM atmp) 
{
int	ii;

ii = places + 1;

if (atmp->m_apm_datalength <= ii)
  {
   m_apm_copy(btmp,atmp);
   return;
  }

M_work_0_5->m_apm_exponent = atmp->m_apm_exponent - ii;

if (atmp->m_apm_sign > 0)
  m_apm_add(btmp, atmp, M_work_0_5);
else
  m_apm_subtract(btmp, atmp, M_work_0_5);

btmp->m_apm_datalength = ii;
M_apm_normalize(btmp);
}
/****************************************************************************/
void	M_apm_normalize(M_APM atmp)
{
int	i, index, datalength, exponent;
UCHAR   *ucp, numdiv, numrem, numrem2;

if (atmp->m_apm_sign == 0)
  return;

datalength = atmp->m_apm_datalength;
exponent   = atmp->m_apm_exponent;

/* make sure trailing bytes/chars are 0                */
/* the following function will adjust the 'datalength' */
/* we want the original value and will fix it later    */

M_apm_pad(atmp, (datalength + 3));

while (TRUE)			/* remove lead-in '0' if any */
  {
   M_get_div_rem_10((int)atmp->m_apm_data[0], &numdiv, &numrem);

   if (numdiv >= 1)      /* number is normalized, done here */
     break;

   index = (datalength + 1) >> 1;

   if (numrem == 0)      /* both nibbles are 0, we can move full bytes */
     {
      i = 0;
      ucp = atmp->m_apm_data;

      while (TRUE)	 /* find out how many '00' bytes we can move */
        {
	 if (*ucp != 0)
	   break;

         ucp++;
	 i++;
	}

      memmove(atmp->m_apm_data, ucp, (index + 1 - i));
      datalength -= 2 * i;
      exponent -= 2 * i;
     }
   else
     {
      for (i=0; i < index; i++)
        {
         M_get_div_rem_10((int)atmp->m_apm_data[i+1], &numdiv, &numrem2);
         atmp->m_apm_data[i] = 10 * numrem + numdiv;
	 numrem = numrem2;
        }
   
      datalength--;
      exponent--;
     }
  }

while (TRUE)			/* remove trailing '0' if any */
  {
   index = ((datalength + 1) >> 1) - 1;

   if ((datalength & 1) == 0)   /* back-up full bytes at a time if the */
     {				/* current length is an even number    */
      ucp = atmp->m_apm_data + index;
      if (*ucp == 0)
        {
	 while (TRUE)
	   {
	    datalength -= 2;
	    index--;
	    ucp--;

	    if (*ucp != 0)
	      break;
	   }
	}
     }

   M_get_div_rem_10((int)atmp->m_apm_data[index], &numdiv, &numrem);

   if (numrem != 0)		/* last digit non-zero, all done */
     break;

   if ((datalength & 1) != 0)   /* if odd, then first char must be non-zero */
     {
      if (numdiv != 0)
        break;
     }

   if (datalength == 1)
     {
      atmp->m_apm_sign = 0;
      exponent = 0;
      break;
     }
     
   datalength--;
  }

atmp->m_apm_datalength = datalength;
atmp->m_apm_exponent   = exponent;
}
/****************************************************************************/
void	M_apm_scale(M_APM ctmp, int count)
{
int	ii, numb, ct;
UCHAR	*chp, numdiv, numdiv2, numrem;
void	*vp;

ct = count;

ii = (ctmp->m_apm_datalength + ct + 1) >> 1;
if (ii > ctmp->m_apm_malloclength)
  {
   if ((vp = MAPM_REALLOC(ctmp->m_apm_data, (ii + 32))) == NULL)
     {
      /* fatal, this does not return */

      M_apm_log_error_msg(M_APM_EXIT, "\'M_apm_scale\', Out of memory");
     }
   
   ctmp->m_apm_malloclength = ii + 28;
   ctmp->m_apm_data = (UCHAR *)vp;
  }

if ((ct & 1) != 0)          /* move odd number first */
  {
   ct--;
   chp = ctmp->m_apm_data;
   ii  = ((ctmp->m_apm_datalength + 1) >> 1) - 1;

   if ((ctmp->m_apm_datalength & 1) == 0)
     {
      /*
       *   original datalength is even:
       *
       *   uv  wx  yz   becomes  -->   0u  vw  xy  z0
       */

      numdiv = 0;

      while (TRUE)
        {
         M_get_div_rem_10((int)chp[ii], &numdiv2, &numrem);

	 chp[ii + 1] = 10 * numrem + numdiv;
	 numdiv = numdiv2;

	 if (ii == 0)
	   break;

         ii--;
	}

      chp[0] = numdiv2;
     }
   else
     {
      /*
       *   original datalength is odd:
       *
       *   uv  wx  y0   becomes  -->   0u  vw  xy
       */

      M_get_div_rem_10((int)chp[ii], &numdiv2, &numrem);

      if (ii == 0)
        {
         chp[0] = numdiv2;
        }
      else
        {
         while (TRUE)
           {
            M_get_div_rem_10((int)chp[ii - 1], &numdiv, &numrem);

	    chp[ii] = 10 * numrem + numdiv2;
	    numdiv2 = numdiv;

	    if (--ii == 0)
	      break;
	   }

         chp[0] = numdiv;
        }
     }

   ctmp->m_apm_exponent++;
   ctmp->m_apm_datalength++;
  }

/* ct is even here */

if (ct > 0)
  {
   numb = (ctmp->m_apm_datalength + 1) >> 1;
   ii   = ct >> 1;
   
   memmove((ctmp->m_apm_data + ii), ctmp->m_apm_data, numb);
   memset(ctmp->m_apm_data, 0, ii);
   
   ctmp->m_apm_datalength += ct;
   ctmp->m_apm_exponent += ct;
  }
}
/****************************************************************************/
void	M_apm_pad(M_APM ctmp, int new_length)
{
int	num1, numb, ct;
UCHAR	numdiv, numrem;
void	*vp;

ct = new_length;
if (ctmp->m_apm_datalength >= ct)
  return;
  
numb = (ct + 1) >> 1;
if (numb > ctmp->m_apm_malloclength)
  {
   if ((vp = MAPM_REALLOC(ctmp->m_apm_data, (numb + 32))) == NULL)
     {
      /* fatal, this does not return */

      M_apm_log_error_msg(M_APM_EXIT, "\'M_apm_pad\', Out of memory");
     }
   
   ctmp->m_apm_malloclength = numb + 28;
   ctmp->m_apm_data = (UCHAR *)vp;
  }

num1 = (ctmp->m_apm_datalength + 1) >> 1;

if ((ctmp->m_apm_datalength & 1) != 0)
  {
   M_get_div_rem_10((int)ctmp->m_apm_data[num1 - 1], &numdiv, &numrem);
   ctmp->m_apm_data[num1 - 1] = 10 * numdiv;
  }

memset((ctmp->m_apm_data + num1), 0, (numb - num1));
ctmp->m_apm_datalength = ct;
}
/****************************************************************************/

/*
      debug_dsp(cc)
      M_APM cc;
      {
static char buffer[8192];

m_apm_to_string(buffer, -1, cc);
printf("(dsp func) = [%s]\n",buffer);

      }
*/


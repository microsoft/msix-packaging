
/* 
 *  M_APM  -  mapmfmul.c
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

static int M_firsttimef = TRUE;

/*
 *      specify the max size the FFT routine can handle 
 *      (in MAPM, #digits = 2 * #bytes)
 *
 *      this number *must* be an exact power of 2.
 *
 *      **WORST** case input numbers (all 9's) has shown that
 *	the FFT math will overflow if the #define here is 
 *      >= 1048576. On my system, 524,288 worked OK. I will
 *      factor down another factor of 2 to safeguard against 
 *	other computers have less precise floating point math. 
 *	If you are confident in your system, 524288 will 
 *	theoretically work fine.
 *
 *      the define here allows the FFT algorithm to multiply two
 *      524,288 digit numbers yielding a 1,048,576 digit result.
 */

#define MAX_FFT_BYTES 262144

/*
 *      the Divide-and-Conquer multiplication kicks in when the size of
 *	the numbers exceed the capability of the FFT (#define just above).
 *
 *	#bytes    D&C call depth
 *	------    --------------
 *       512K           1
 *        1M            2
 *        2M            3
 *        4M            4
 *       ...           ...
 *    2.1990E+12       23 
 *
 *	the following stack sizes are sized to meet the 
 *      above 2.199E+12 example, though I wouldn't want to
 *	wait for it to finish...
 *
 *      Each call requires 7 stack variables to be saved so 
 *	we need a stack depth of 23 * 7 + PAD.  (we use 164)
 *
 *      For 'exp_stack', 3 integers also are required to be saved 
 *      for each recursive call so we need a stack depth of 
 *      23 * 3 + PAD. (we use 72)
 *
 *
 *      If the FFT multiply is disabled, resize the arrays
 *	as follows:
 *
 *      the following stack sizes are sized to meet the 
 *      worst case expected assuming we are multiplying 
 *      numbers with 2.14E+9 (2 ^ 31) digits. 
 *
 *      For sizeof(int) == 4 (32 bits) there may be up to 32 recursive
 *      calls. Each call requires 7 stack variables so we need a
 *      stack depth of 32 * 7 + PAD.  (we use 240)
 *
 *      For 'exp_stack', 3 integers also are required to be saved 
 *      for each recursive call so we need a stack depth of 
 *      32 * 3 + PAD. (we use 100)
 */

#ifdef NO_FFT_MULTIPLY
#define M_STACK_SIZE 240
#define M_ISTACK_SIZE 100
#else
#define M_STACK_SIZE 164
#define M_ISTACK_SIZE 72
#endif

static int    exp_stack[M_ISTACK_SIZE];
static int    exp_stack_ptr;

static UCHAR  *mul_stack_data[M_STACK_SIZE];
static int    mul_stack_data_size[M_STACK_SIZE];
static int    M_mul_stack_ptr;

static UCHAR  *fmul_a1, *fmul_a0, *fmul_a9, *fmul_b1, *fmul_b0, 
	      *fmul_b9, *fmul_t0;

static int    size_flag, bit_limit, stmp, itmp, mii;

static M_APM  M_ain;
static M_APM  M_bin;

extern void   M_fast_multiply(M_APM, M_APM, M_APM);
extern void   M_fmul_div_conq(UCHAR *, UCHAR *, UCHAR *, int);
extern void   M_fmul_add(UCHAR *, UCHAR *, int, int);
extern int    M_fmul_subtract(UCHAR *, UCHAR *, UCHAR *, int);
extern void   M_fmul_split(UCHAR *, UCHAR *, UCHAR *, int);
extern int    M_next_power_of_2(int);
extern int    M_get_stack_ptr(int);
extern void   M_push_mul_int(int);
extern int    M_pop_mul_int(void);

#ifdef NO_FFT_MULTIPLY
extern void   M_4_byte_multiply(UCHAR *, UCHAR *, UCHAR *);
#else
extern void   M_fast_mul_fft(UCHAR *, UCHAR *, UCHAR *, int);
#endif

/*
 *      the following algorithm is used in this fast multiply routine
 *	(sometimes called the divide-and-conquer technique.)
 *
 *	assume we have 2 numbers (a & b) with 2N digits. 
 *
 *      let : a = (2^N) * A1 + A0 , b = (2^N) * B1 + B0      
 *
 *	where 'A1' is the 'most significant half' of 'a' and 
 *      'A0' is the 'least significant half' of 'a'. Same for 
 *	B1 and B0.
 *
 *	Now use the identity :
 *
 *               2N   N            N                    N
 *	ab  =  (2  + 2 ) A1B1  +  2 (A1-A0)(B0-B1)  + (2 + 1)A0B0
 *
 *
 *      The original problem of multiplying 2 (2N) digit numbers has
 *	been reduced to 3 multiplications of N digit numbers plus some
 *	additions, subtractions, and shifts.
 *
 *	The fast multiplication algorithm used here uses the above 
 *	identity in a recursive process. This algorithm results in
 *	O(n ^ 1.585) growth.
 */


/****************************************************************************/
void	M_free_all_fmul()
{
int	k;

if (M_firsttimef == FALSE)
  {
   m_apm_free(M_ain);
   m_apm_free(M_bin);

   for (k=0; k < M_STACK_SIZE; k++)
     {
      if (mul_stack_data_size[k] != 0)
        {
         MAPM_FREE(mul_stack_data[k]);
	}
     }

   M_firsttimef = TRUE;
  }
}
/****************************************************************************/
void	M_push_mul_int(int val)
{
exp_stack[++exp_stack_ptr] = val;
}
/****************************************************************************/
int	M_pop_mul_int()
{
return(exp_stack[exp_stack_ptr--]);
}
/****************************************************************************/
void   	M_fmul_split(UCHAR *x1, UCHAR *x0, UCHAR *xin, int nbytes)
{
memcpy(x1, xin, nbytes);
memcpy(x0, (xin + nbytes), nbytes);
}
/****************************************************************************/
void	M_fast_multiply(M_APM rr, M_APM aa, M_APM bb)
{
void	*vp;
int	ii, k, nexp, sign;

if (M_firsttimef)
  {
   M_firsttimef = FALSE;

   for (k=0; k < M_STACK_SIZE; k++)
     mul_stack_data_size[k] = 0;

   size_flag = M_get_sizeof_int();
   bit_limit = 8 * size_flag + 1;

   M_ain = m_apm_init();
   M_bin = m_apm_init();
  }

exp_stack_ptr   = -1;
M_mul_stack_ptr = -1;

m_apm_copy(M_ain, aa);
m_apm_copy(M_bin, bb);

sign = M_ain->m_apm_sign * M_bin->m_apm_sign;
nexp = M_ain->m_apm_exponent + M_bin->m_apm_exponent;

if (M_ain->m_apm_datalength >= M_bin->m_apm_datalength)
  ii = M_ain->m_apm_datalength;
else
  ii = M_bin->m_apm_datalength;

ii = (ii + 1) >> 1;
ii = M_next_power_of_2(ii);

/* Note: 'ii' must be >= 4 here. this is guaranteed 
   by the caller: m_apm_multiply
*/

k = 2 * ii;                   /* required size of result, in bytes  */

M_apm_pad(M_ain, k);          /* fill out the data so the number of */
M_apm_pad(M_bin, k);          /* bytes is an exact power of 2       */

if (k > rr->m_apm_malloclength)
  {
   if ((vp = MAPM_REALLOC(rr->m_apm_data, (k + 32))) == NULL)
     {
      /* fatal, this does not return */

      M_apm_log_error_msg(M_APM_EXIT, "\'M_fast_multiply\', Out of memory");
     }
  
   rr->m_apm_malloclength = k + 28;
   rr->m_apm_data = (UCHAR *)vp;
  }

#ifdef NO_FFT_MULTIPLY

M_fmul_div_conq(rr->m_apm_data, M_ain->m_apm_data, 
                                M_bin->m_apm_data, ii);
#else

/*
 *     if the numbers are *really* big, use the divide-and-conquer
 *     routine first until the numbers are small enough to be handled 
 *     by the FFT algorithm. If the numbers are already small enough,
 *     call the FFT multiplication now.
 *
 *     Note that 'ii' here is (and must be) an exact power of 2.
 */

if (size_flag == 2)   /* if still using 16 bit compilers .... */
  {
   M_fast_mul_fft(rr->m_apm_data, M_ain->m_apm_data, 
                                  M_bin->m_apm_data, ii);
  }
else                  /* >= 32 bit compilers */
  {
   if (ii > (MAX_FFT_BYTES + 2))
     {
      M_fmul_div_conq(rr->m_apm_data, M_ain->m_apm_data, 
                                      M_bin->m_apm_data, ii);
     }
   else
     {
      M_fast_mul_fft(rr->m_apm_data, M_ain->m_apm_data, 
                                     M_bin->m_apm_data, ii);
     }
  }

#endif

rr->m_apm_sign       = sign;
rr->m_apm_exponent   = nexp;
rr->m_apm_datalength = 4 * ii;

M_apm_normalize(rr);
}
/****************************************************************************/
/*
 *      This is the recursive function to perform the multiply. The 
 *      design intent here is to have no local variables. Any local
 *      data that needs to be saved is saved on one of the two stacks.
 */
void	M_fmul_div_conq(UCHAR *rr, UCHAR *aa, UCHAR *bb, int sz)
{

#ifdef NO_FFT_MULTIPLY

if (sz == 4)                /* multiply 4x4 yielding an 8 byte result */
  {
   M_4_byte_multiply(rr, aa, bb);
   return;
  }

#else

/*
 *  if the numbers are now small enough, let the FFT algorithm
 *  finish up.
 */

if (sz == MAX_FFT_BYTES)     
  {
   M_fast_mul_fft(rr, aa, bb, sz);
   return;
  }

#endif

memset(rr, 0, (2 * sz));    /* zero out the result */
mii = sz >> 1;

itmp = M_get_stack_ptr(mii);
M_push_mul_int(itmp);

fmul_a1 = mul_stack_data[itmp];

itmp    = M_get_stack_ptr(mii);
fmul_a0 = mul_stack_data[itmp];

itmp    = M_get_stack_ptr(2 * sz);
fmul_a9 = mul_stack_data[itmp];

itmp    = M_get_stack_ptr(mii);
fmul_b1 = mul_stack_data[itmp];

itmp    = M_get_stack_ptr(mii);
fmul_b0 = mul_stack_data[itmp];

itmp    = M_get_stack_ptr(2 * sz);
fmul_b9 = mul_stack_data[itmp];

itmp    = M_get_stack_ptr(2 * sz);
fmul_t0 = mul_stack_data[itmp];

M_fmul_split(fmul_a1, fmul_a0, aa, mii);
M_fmul_split(fmul_b1, fmul_b0, bb, mii);

stmp  = M_fmul_subtract(fmul_a9, fmul_a1, fmul_a0, mii);
stmp *= M_fmul_subtract(fmul_b9, fmul_b0, fmul_b1, mii);

M_push_mul_int(stmp);
M_push_mul_int(mii);

M_fmul_div_conq(fmul_t0, fmul_a0, fmul_b0, mii);

mii  = M_pop_mul_int();
stmp = M_pop_mul_int();
itmp = M_pop_mul_int();

M_push_mul_int(itmp);
M_push_mul_int(stmp);
M_push_mul_int(mii);

/*   to restore all stack variables ...
fmul_a1 = mul_stack_data[itmp];
fmul_a0 = mul_stack_data[itmp+1];
fmul_a9 = mul_stack_data[itmp+2];
fmul_b1 = mul_stack_data[itmp+3];
fmul_b0 = mul_stack_data[itmp+4];
fmul_b9 = mul_stack_data[itmp+5];
fmul_t0 = mul_stack_data[itmp+6];
*/

fmul_a1 = mul_stack_data[itmp];
fmul_b1 = mul_stack_data[itmp+3];
fmul_t0 = mul_stack_data[itmp+6];

memcpy((rr + sz), fmul_t0, sz);    /* first 'add', result is now zero */
				   /* so we just copy in the bytes    */
M_fmul_add(rr, fmul_t0, mii, sz);

M_fmul_div_conq(fmul_t0, fmul_a1, fmul_b1, mii);

mii  = M_pop_mul_int();
stmp = M_pop_mul_int();
itmp = M_pop_mul_int();

M_push_mul_int(itmp);
M_push_mul_int(stmp);
M_push_mul_int(mii);

fmul_a9 = mul_stack_data[itmp+2];
fmul_b9 = mul_stack_data[itmp+5];
fmul_t0 = mul_stack_data[itmp+6];

M_fmul_add(rr, fmul_t0, 0, sz);
M_fmul_add(rr, fmul_t0, mii, sz);

if (stmp != 0)
  M_fmul_div_conq(fmul_t0, fmul_a9, fmul_b9, mii);

mii  = M_pop_mul_int();
stmp = M_pop_mul_int();
itmp = M_pop_mul_int();

fmul_t0 = mul_stack_data[itmp+6];

/*
 *  if the sign of (A1 - A0)(B0 - B1) is positive, ADD to
 *  the result. if it is negative, SUBTRACT from the result.
 */

if (stmp < 0)
  {
   fmul_a9 = mul_stack_data[itmp+2];
   fmul_b9 = mul_stack_data[itmp+5];

   memset(fmul_b9, 0, (2 * sz)); 
   memcpy((fmul_b9 + mii), fmul_t0, sz); 
   M_fmul_subtract(fmul_a9, rr, fmul_b9, (2 * sz));
   memcpy(rr, fmul_a9, (2 * sz));
  }

if (stmp > 0)
  M_fmul_add(rr, fmul_t0, mii, sz);

M_mul_stack_ptr -= 7;
}
/****************************************************************************/
/*
 *	special addition function for use with the fast multiply operation
 */
void    M_fmul_add(UCHAR *r, UCHAR *a, int offset, int sz)
{
int	i, j;
UCHAR   carry;

carry = 0;
j = offset + sz;
i = sz;

while (TRUE)
  {
   r[--j] += carry + a[--i];

   if (r[j] >= 100)
     {
      r[j] -= 100;
      carry = 1;
     }
   else
      carry = 0;

   if (i == 0)
     break;
  }

if (carry)
  {
   while (TRUE)
     {
      r[--j] += 1;
   
      if (r[j] < 100)
        break;

      r[j] -= 100;
     }
  }
}
/****************************************************************************/
/*
 *	special subtraction function for use with the fast multiply operation
 */
int     M_fmul_subtract(UCHAR *r, UCHAR *a, UCHAR *b, int sz)
{
int	k, jtmp, sflag, nb, borrow;

nb    = sz;
sflag = 0;      /* sign flag: assume the numbers are equal */

/*
 *   find if a > b (so we perform a-b)
 *   or      a < b (so we perform b-a)
 */

for (k=0; k < nb; k++)
  {
   if (a[k] < b[k])
     {
      sflag = -1;
      break;
     }

   if (a[k] > b[k])
     {
      sflag = 1;
      break;
     }
  }

if (sflag == 0)
  {
   memset(r, 0, nb);            /* zero out the result */
  }
else
  {
   k = nb;
   borrow = 0;

   while (TRUE)
     {
      k--;

      if (sflag == 1)
        jtmp = (int)a[k] - (int)b[k] - borrow;
      else
        jtmp = (int)b[k] - (int)a[k] - borrow;

      if (jtmp >= 0)
        {
         r[k] = (UCHAR)jtmp;
         borrow = 0;
        }
      else
        {
         r[k] = (UCHAR)(100 + jtmp);
         borrow = 1;
        }

      if (k == 0)
        break;
     }
  }

return(sflag);
}
/****************************************************************************/
int     M_next_power_of_2(int n)
{
int     ct, k;

if (n <= 2)
  return(n);

k  = 2;
ct = 0;

while (TRUE)
  {
   if (k >= n)
     break;

   k = k << 1;

   if (++ct == bit_limit)
     {
      /* fatal, this does not return */

      M_apm_log_error_msg(M_APM_EXIT, 
               "\'M_next_power_of_2\', ERROR :sizeof(int) too small ??");
     }
  }

return(k);
}
/****************************************************************************/
int	M_get_stack_ptr(int sz)
{
int	i, k;
UCHAR   *cp;

k = ++M_mul_stack_ptr;

/* if size is 0, just need to malloc and return */
if (mul_stack_data_size[k] == 0)
  {
   if ((i = sz) < 16)
     i = 16;

   if ((cp = (UCHAR *)MAPM_MALLOC(i + 4)) == NULL)
     {
      /* fatal, this does not return */

      M_apm_log_error_msg(M_APM_EXIT, "\'M_get_stack_ptr\', Out of memory");
     }

   mul_stack_data[k]      = cp;
   mul_stack_data_size[k] = i;
  }
else        /* it has been malloc'ed, see if it's big enough */
  {
   if (sz > mul_stack_data_size[k])
     {
      cp = mul_stack_data[k];

      if ((cp = (UCHAR *)MAPM_REALLOC(cp, (sz + 4))) == NULL)
        {
         /* fatal, this does not return */
   
         M_apm_log_error_msg(M_APM_EXIT, "\'M_get_stack_ptr\', Out of memory");
        }
   
      mul_stack_data[k]      = cp;
      mul_stack_data_size[k] = sz;
     }
  }

return(k);
}
/****************************************************************************/

#ifdef NO_FFT_MULTIPLY

/*
 *      multiply a 4 byte number by a 4 byte number
 *      yielding an 8 byte result. each byte contains
 *      a base 100 'digit', i.e.: range from 0-99.
 *
 *             MSB         LSB
 *
 *      a,b    [0] [1] [2] [3]
 *   result    [0]  .....  [7]
 */

void	M_4_byte_multiply(UCHAR *r, UCHAR *a, UCHAR *b)
{
int	b0, jj;
UCHAR   *cp1, *cp2, numdiv, numrem;

memset(r, 0, 8);               /* zero out 8 byte result */
jj = 3;

/* loop for one number [b], un-roll the inner 'loop' [a] */

while (1)
  {
   b0  = (int)b[jj];
   cp1 = r + (3 + jj);
   cp2 = cp1 + 1;

   M_get_div_rem((b0 * a[3]), &numdiv, &numrem);
   
   *cp2 += numrem;
   *cp1 += numdiv;
   
   if (*cp2 >= 100)
     {
      *cp2 -= 100;
      *cp1 += 1;
     }
   
   cp1--;
   cp2--;
   
   if (*cp2 >= 100)
     {
      *cp2 -= 100;
      *cp1 += 1;
     }
   
   M_get_div_rem((b0 * a[2]), &numdiv, &numrem);
   
   *cp2 += numrem;
   *cp1 += numdiv;
   
   if (*cp2 >= 100)
     {
      *cp2 -= 100;
      *cp1 += 1;
     }
   
   cp1--;
   cp2--;
   
   if (*cp2 >= 100)
     {
      *cp2 -= 100;
      *cp1 += 1;
     }
   
   M_get_div_rem((b0 * a[1]), &numdiv, &numrem);
   
   *cp2 += numrem;
   *cp1 += numdiv;
   
   if (*cp2 >= 100)
     {
      *cp2 -= 100;
      *cp1 += 1;
     }
   
   cp1--;
   cp2--;
   
   if (*cp2 >= 100)
     {
      *cp2 -= 100;
      *cp1 += 1;
     }
   
   M_get_div_rem((b0 * a[0]), &numdiv, &numrem);
   
   *cp2 += numrem;
   *cp1 += numdiv;
   
   if (*cp2 >= 100)
     {
      *cp2 -= 100;
      *cp1 += 1;
     }
   
   if (jj-- == 0)
     break;
  }
}

#endif

/****************************************************************************/

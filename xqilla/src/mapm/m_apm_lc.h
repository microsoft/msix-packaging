
/* 
 *  M_APM  -  m_apm_lc.h
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

#ifndef M__APM_LOCAL_INC
#define M__APM_LOCAL_INC

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <xqilla/mapm/m_apm.h>

/* 
 *   this supports older (and maybe newer?) Borland compilers.
 *   these Borland compilers define __MSDOS__
 */

#ifndef MSDOS
#ifdef __MSDOS__
#define MSDOS
#endif
#endif

/* 
 *   this supports some newer Borland compilers (i.e., v5.5).
 */

#ifndef MSDOS
#ifdef __BORLANDC__
#define MSDOS
#endif
#endif

/* 
 *   this supports the LCC-WIN32 compiler
 */

#ifndef MSDOS
#ifdef __LCC__
#define MSDOS
#endif
#endif

/* 
 *   this supports Micro$oft Visual C++ and also possibly older
 *   straight C compilers as well.
 */

#ifndef MSDOS
#ifdef _MSC_VER
#define MSDOS
#endif
#endif

/* 
 *   this supports Metrowerks CodeWarrior 7.0 compiler (I think...)
 */

#ifndef MSDOS
#ifdef __MWERKS__
#define MSDOS
#endif
#endif

/* 
 *   this supports MINGW 32 compiler
 */

#ifndef MSDOS
#ifdef __MINGW_H
#define MSDOS
#endif
#endif


/*
 *  If for some reason (RAM limitations, slow floating point, whatever) 
 *  you do NOT want to use the FFT multiply algorithm, un-comment the 
 *  #define below, delete mapm_fft.c and remove mapm_fft from the build.
 */

/*  #define NO_FFT_MULTIPLY  */

/*
 *      use your own memory management functions if desired.
 *      re-define MAPM_* below to point to your functions.
 *      an example is shown below.
 */

/*
extern   void   *memory_allocate(unsigned int);
extern   void   *memory_reallocate(void *, unsigned int);
extern   void   memory_free(void *);

#define  MAPM_MALLOC memory_allocate
#define  MAPM_REALLOC memory_reallocate
#define  MAPM_FREE memory_free
*/

/* default: use the standard C library memory functions ... */

#define  MAPM_MALLOC malloc
#define  MAPM_REALLOC realloc
#define  MAPM_FREE free

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#define	M_APM_IDENT 0x6BCC9AE5
#define	M_APM_RETURN 1
#define	M_APM_EXIT 2

/* number of digits in the global constants, PI, E, etc */

#define	VALID_DECIMAL_PLACES 128

extern  int     MM_lc_PI_digits;
extern  int     MM_lc_log_digits;

/*
 *   constants not in m_apm.h
 */

extern	M_APM	MM_0_5;
extern	M_APM	MM_0_85;
extern	M_APM	MM_5x_125R;
extern	M_APM	MM_5x_64R;
extern	M_APM	MM_5x_256R;
extern	M_APM	MM_5x_Eight;
extern	M_APM	MM_5x_Sixteen;
extern	M_APM	MM_5x_Twenty;
extern	M_APM	MM_lc_PI;
extern	M_APM	MM_lc_HALF_PI;
extern	M_APM	MM_lc_2_PI;
extern	M_APM	MM_lc_log2;
extern	M_APM	MM_lc_log10;
extern	M_APM	MM_lc_log10R;

/*
 *   prototypes for internal functions
 */

#ifndef NO_FFT_MULTIPLY
extern	void	M_free_all_fft(void);
#endif

extern	void	M_init_trig_globals(void);
extern	void	M_free_all_div(void);
extern	void	M_free_all_exp(void);
extern	void	M_free_all_pow(void);
extern	void	M_free_all_rnd(void);
extern	void	M_free_all_cnst(void);
extern	void	M_free_all_fmul(void);
extern	void	M_free_all_stck(void);
extern	void	M_free_all_util(void);

extern	void	M_raw_exp(M_APM, int, M_APM);
extern	void	M_raw_sin(M_APM, int, M_APM);
extern	void	M_raw_cos(M_APM, int, M_APM);
extern	void	M_5x_sin(M_APM, int, M_APM);
extern	void	M_4x_cos(M_APM, int, M_APM);
extern	void	M_5x_do_it(M_APM, int, M_APM);
extern	void	M_4x_do_it(M_APM, int, M_APM);

extern	M_APM	M_get_stack_var(void);
extern	void	M_restore_stack(int);
extern	int 	M_get_sizeof_int(void);

extern	void	M_apm_sdivide(M_APM, int, M_APM, M_APM);
extern	void	M_cos_to_sin(M_APM, int, M_APM);
extern	void	M_limit_angle_to_pi(M_APM, int, M_APM);
extern	void	M_log_near_1(M_APM, int, M_APM);
extern	void	M_get_sqrt_guess(M_APM, M_APM);
extern	void	M_get_cbrt_guess(M_APM, M_APM);
extern	void	M_get_log_guess(M_APM, M_APM);
extern	void	M_get_asin_guess(M_APM, M_APM);
extern	void	M_get_acos_guess(M_APM, M_APM);
extern	void	M_arcsin_near_0(M_APM, int, M_APM);
extern	void	M_arccos_near_0(M_APM, int, M_APM);
extern	void	M_arctan_near_0(M_APM, int, M_APM);
extern	void	M_log_basic_iteration(M_APM, int, M_APM);
extern	void	M_check_log_places(int);
extern	void	M_log_AGM_R_func(M_APM, int, M_APM, M_APM);
extern	void	M_init_util_data(void);
extern	void	M_get_div_rem_addr(UCHAR **, UCHAR **);
extern	void	M_get_div_rem(int,UCHAR *, UCHAR *);
extern	void	M_get_div_rem_10(int, UCHAR *, UCHAR *);
extern	void	M_apm_normalize(M_APM);
extern	void	M_apm_scale(M_APM, int);
extern	void	M_apm_pad(M_APM, int);
extern  void    M_long_2_ascii(char *, long);
extern	void	M_check_PI_places(int);
extern  void    M_calculate_PI_AGM(M_APM, int);
extern  void    M_set_to_zero(M_APM);
extern	int	M_strposition(char *, char *);
extern	char	*M_lowercase(char *);
extern  void    M_apm_log_error_msg(int, char *);
extern  void	M_apm_round_fixpt(M_APM, int, M_APM);

#endif



/* 
 *  M_APM  -  m_apm.h
 *
 *  Copyright (C) 1999 - 2002   Michael C. Ring
 *
 *  Permission to use, copy, and distribute this software and its
 *  documentation for any purpose with or without fee is hereby granted, 
 *  provided that the above copyright notice appear in all copies and 
 *  that both that copyright notice and this permission notice appear 
 *  in supporting documentation.
 *
 *  Permission to modify the software is granted. Permission to distribute
 *	the modified code is granted. Modifications are to be distributed 
 *	by using the file 'license.txt' as a template to modify the file header.
 *	'license.txt' is available in the official MAPM distribution.
 *
 *	This software is provided "as is" without express or implied warranty.
 *
 *	THIS FILE HAS BEEN MODIFIED FROM THE OFFICIAL MAPM DISTRIBUTION BY
 *	'XQilla project' on 2005/11/01, 2005/11/03, 2006/08/09, 2006/08/21,
 *	2007/09/20.
 *	THIS FILE IS ORIGINALLY FROM MAPM VERSION 4.6.1.
 */

#ifndef M__APM__INCLUDED
#define M__APM__INCLUDED

#include <xqilla/framework/XQillaExport.hpp>

#ifdef __cplusplus
/* Comment this line out if you've compiled the library as C++. */
#define APM_CONVERT_FROM_C
#endif

#ifdef APM_CONVERT_FROM_C
extern "C" {
#endif

typedef unsigned char UCHAR;

typedef struct  {
	UCHAR	*m_apm_data;
	long	m_apm_id;
	int     m_apm_refcount;       /* <- used only by C++ MAPM class */
	int	m_apm_malloclength;
	int	m_apm_datalength;
	int	m_apm_exponent;
	int	m_apm_sign;
} M_APM_struct;

typedef M_APM_struct *M_APM;

/*
 *	convienient predefined constants
 *  (Not thread safe in C++ API)
 */

extern	M_APM	MM_Zero;
extern	M_APM	MM_One;
extern	M_APM	MM_Two;
extern	M_APM	MM_Three;
extern	M_APM	MM_Four;
extern	M_APM	MM_Five;
extern	M_APM	MM_Ten;

extern	M_APM	MM_PI;
extern	M_APM	MM_HALF_PI;
extern	M_APM	MM_2_PI;
extern	M_APM	MM_E;

extern	M_APM	MM_LOG_E_BASE_10;
extern	M_APM	MM_LOG_10_BASE_E;
extern	M_APM	MM_LOG_2_BASE_E;
extern	M_APM	MM_LOG_3_BASE_E;


/*
 *	function prototypes
 */
extern void m_apm_library_initialize(void);
extern void m_apm_library_terminate(void);

extern void m_apm_mt_initialize(void);
extern void m_apm_mt_terminate(void);

extern	void	m_apm_enter(void);
extern	void	m_apm_leave(void);

extern	XQILLA_API M_APM	m_apm_init(void);
extern	void	m_apm_free(M_APM);
extern	void	m_apm_free_all_mem(void);
extern	void	m_apm_trim_mem_usage(void);

extern	void	m_apm_set_string(M_APM, char *);
extern	void	m_apm_set_double(M_APM, double);
extern	void	m_apm_set_long(M_APM, long);

extern double m_apm_get_double(M_APM atmp);

extern	void	m_apm_to_string(char *, int, M_APM);
extern  void	m_apm_to_fixpt_string(char *, int, M_APM);
extern  void	m_apm_to_fixpt_stringex(char *, int, M_APM, char, char, int);
extern  char	*m_apm_to_fixpt_stringexp(int, M_APM, char, char, int);
extern  void    m_apm_to_integer_string(char *, M_APM);

extern	void	m_apm_absolute_value(M_APM, M_APM);
extern	void	m_apm_negate(M_APM, M_APM);
extern	XQILLA_API void	m_apm_copy(M_APM, M_APM);
extern	void	m_apm_round(M_APM, int, M_APM);
extern	XQILLA_API int	m_apm_compare(M_APM, M_APM);
extern	int	m_apm_sign(M_APM);
extern	int	m_apm_exponent(M_APM);
extern	int	m_apm_significant_digits(M_APM);
extern	int	m_apm_is_integer(M_APM);
extern	int	m_apm_is_even(M_APM);
extern	int	m_apm_is_odd(M_APM);

extern	void	m_apm_gcd(M_APM, M_APM, M_APM);
extern	void	m_apm_lcm(M_APM, M_APM, M_APM);

extern	void	m_apm_add(M_APM, M_APM, M_APM);
extern	void	m_apm_subtract(M_APM, M_APM, M_APM);
extern	void	m_apm_multiply(M_APM, M_APM, M_APM);
extern	void	m_apm_divide(M_APM, int, M_APM, M_APM);
extern	void	m_apm_integer_divide(M_APM, M_APM, M_APM);
extern	void	m_apm_integer_div_rem(M_APM, M_APM, M_APM, M_APM);
extern	void	m_apm_reciprocal(M_APM, int, M_APM);
extern	void	m_apm_factorial(M_APM, M_APM);
extern	void	m_apm_floor(M_APM, M_APM);
extern	void	m_apm_ceil(M_APM, M_APM);
extern	void	m_apm_get_random(M_APM);
extern	void	m_apm_set_random_seed(char *);

extern	void	m_apm_sqrt(M_APM, int, M_APM);
extern	void	m_apm_cbrt(M_APM, int, M_APM);
extern	void	m_apm_log(M_APM, int, M_APM);
extern	void	m_apm_log10(M_APM, int, M_APM);
extern	void	m_apm_exp(M_APM, int, M_APM);
extern	void	m_apm_pow(M_APM, int, M_APM, M_APM);
extern  void	m_apm_integer_pow(M_APM, int, M_APM, int);
extern  void	m_apm_integer_pow_nr(M_APM, M_APM, int);

extern	void	m_apm_sin_cos(M_APM, M_APM, int, M_APM);
extern	void	m_apm_sin(M_APM, int, M_APM);
extern	void	m_apm_cos(M_APM, int, M_APM);
extern	void	m_apm_tan(M_APM, int, M_APM);
extern	void	m_apm_arcsin(M_APM, int, M_APM);
extern	void	m_apm_arccos(M_APM, int, M_APM);
extern	void	m_apm_arctan(M_APM, int, M_APM);
extern	void	m_apm_arctan2(M_APM, int, M_APM, M_APM);

extern  void    m_apm_sinh(M_APM, int, M_APM);
extern  void    m_apm_cosh(M_APM, int, M_APM);
extern  void    m_apm_tanh(M_APM, int, M_APM);
extern  void    m_apm_arcsinh(M_APM, int, M_APM);
extern  void    m_apm_arccosh(M_APM, int, M_APM);
extern  void    m_apm_arctanh(M_APM, int, M_APM);

extern  void    m_apm_cpp_precision(int);   /* only for C++ wrapper */

extern	M_APM	m_apm_init_mt(void);
extern	void	m_apm_free_mt(M_APM);
extern	void	m_apm_free_all_mem_mt(void);
extern	void	m_apm_trim_mem_usage_mt(void);

extern	void	m_apm_set_string_mt(M_APM, char *);
extern	void	m_apm_set_double_mt(M_APM, double);
extern	void	m_apm_set_long_mt(M_APM, long);

extern	void	m_apm_to_string_mt(char *, int, M_APM);
extern  void	m_apm_to_fixpt_string_mt(char *, int, M_APM);
extern  void	m_apm_to_fixpt_stringex_mt(char *, int, M_APM, char, 
					   char, int);

extern  char	*m_apm_to_fixpt_stringexp_mt(int, M_APM, char, char, int);
extern  void    m_apm_to_integer_string_mt(char *, M_APM);

extern	void	m_apm_absolute_value_mt(M_APM, M_APM);
extern	void	m_apm_negate_mt(M_APM, M_APM);
extern	void	m_apm_copy_mt(M_APM, M_APM);
extern	void	m_apm_round_mt(M_APM, int, M_APM);
extern	int	m_apm_compare_mt(M_APM, M_APM);
extern	int	m_apm_sign_mt(M_APM);
extern	int	m_apm_exponent_mt(M_APM);
extern	int	m_apm_significant_digits_mt(M_APM);
extern	int	m_apm_is_integer_mt(M_APM);
extern	int	m_apm_is_even_mt(M_APM);
extern	int	m_apm_is_odd_mt(M_APM);

extern	void	m_apm_gcd_mt(M_APM, M_APM, M_APM);
extern	void	m_apm_lcm_mt(M_APM, M_APM, M_APM);

extern	void	m_apm_add_mt(M_APM, M_APM, M_APM);
extern	void	m_apm_subtract_mt(M_APM, M_APM, M_APM);
extern	void	m_apm_multiply_mt(M_APM, M_APM, M_APM);
extern	void	m_apm_divide_mt(M_APM, int, M_APM, M_APM);
extern	void	m_apm_integer_divide_mt(M_APM, M_APM, M_APM);
extern	void	m_apm_integer_div_rem_mt(M_APM, M_APM, M_APM, M_APM);
extern	void	m_apm_reciprocal_mt(M_APM, int, M_APM);
extern	void	m_apm_factorial_mt(M_APM, M_APM);
extern	void	m_apm_floor_mt(M_APM, M_APM);
extern	void	m_apm_ceil_mt(M_APM, M_APM);
extern	void	m_apm_get_random_mt(M_APM);
extern	void	m_apm_set_random_seed_mt(char *);

extern	void	m_apm_sqrt_mt(M_APM, int, M_APM);
extern	void	m_apm_cbrt_mt(M_APM, int, M_APM);
extern	void	m_apm_log_mt(M_APM, int, M_APM);
extern	void	m_apm_log10_mt(M_APM, int, M_APM);
extern	void	m_apm_exp_mt(M_APM, int, M_APM);
extern	void	m_apm_pow_mt(M_APM, int, M_APM, M_APM);
extern  void	m_apm_integer_pow_mt(M_APM, int, M_APM, int);
extern  void	m_apm_integer_pow_nr_mt(M_APM, M_APM, int);

extern	void	m_apm_sin_cos_mt(M_APM, M_APM, int, M_APM);
extern	void	m_apm_sin_mt(M_APM, int, M_APM);
extern	void	m_apm_cos_mt(M_APM, int, M_APM);
extern	void	m_apm_tan_mt(M_APM, int, M_APM);
extern	void	m_apm_arcsin_mt(M_APM, int, M_APM);
extern	void	m_apm_arccos_mt(M_APM, int, M_APM);
extern	void	m_apm_arctan_mt(M_APM, int, M_APM);
extern	void	m_apm_arctan2_mt(M_APM, int, M_APM, M_APM);

extern  void    m_apm_sinh_mt(M_APM, int, M_APM);
extern  void    m_apm_cosh_mt(M_APM, int, M_APM);
extern  void    m_apm_tanh_mt(M_APM, int, M_APM);
extern  void    m_apm_arcsinh_mt(M_APM, int, M_APM);
extern  void    m_apm_arccosh_mt(M_APM, int, M_APM);
extern  void    m_apm_arctanh_mt(M_APM, int, M_APM);

extern  void    m_apm_cpp_precision_mt(int);   /* only for C++ wrapper */

/* more intuitive alternate names for the ARC functions ... */

#define m_apm_asin m_apm_arcsin
#define m_apm_acos m_apm_arccos
#define m_apm_atan m_apm_arctan
#define m_apm_atan2 m_apm_arctan2

#define m_apm_asinh m_apm_arcsinh
#define m_apm_acosh m_apm_arccosh
#define m_apm_atanh m_apm_arctanh

#define m_apm_asin_mt m_apm_arcsin_mt
#define m_apm_acos_mt m_apm_arccos_mt
#define m_apm_atan_mt m_apm_arctan_mt
#define m_apm_atan2_mt m_apm_arctan2_mt

#define m_apm_asinh_mt m_apm_arcsinh_mt
#define m_apm_acosh_mt m_apm_arccosh_mt
#define m_apm_atanh_mt m_apm_arctanh_mt

#ifdef APM_CONVERT_FROM_C
}      /* End extern "C" bracket */
#endif

#ifdef __cplusplus   /*<- Hides the class below from C compilers */

/*
    This class lets you use M_APM's a bit more intuitively with
    C++'s operator and function overloading, constructors, etc.

    Added 3/24/2000 by Orion Sky Lawlor, olawlor@acm.org
*/

extern 
#ifdef APM_CONVERT_FROM_C
"C" 
#endif
int MM_cpp_min_precision;


class XQILLA_API MAPM {
protected:

/*
The M_APM structure here is implemented as a reference-
counted, copy-on-write data structure-- this makes copies
very fast, but that's why it's so ugly.  A MAPM object is 
basically just a wrapper around a (possibly shared) 
M_APM_struct myVal.
*/

	M_APM myVal;  /* My M_APM structure */
	void create(void);
	void destroy(void);
	void copyFrom(M_APM Nval);
	static M_APM makeNew(void);
	static void ref(M_APM val);
	static void unref(M_APM val);
	
	/* This routine is called to get a private (mutable)
	   copy of our current value. */
	M_APM val(void);
	
	/*BAD: C M_APM routines doesn't use "const" where they should--
	  hence we have to cast to a non-const type here (FIX THIS!).

	  (in due time.... MCR)
	*/
	M_APM cval(void) const;
	/* This is the default number of digits to use for 
	   1-ary functions like sin, cos, tan, etc.
	   It's the larger of my digits and cpp_min_precision.
        */
	int myDigits(void) const;
	/* This is the default number of digits to use for 
	   2-ary functions like divide, atan2, etc.
	   It's the larger of my digits, his digits, and cpp_min_precision.
        */
	int digits(const MAPM &otherVal) const;
public:
	/* Constructors: */
	MAPM(void); /* Default constructor (takes no value) */
	MAPM(const MAPM &m); /* Copy constructor */
	MAPM(M_APM m, bool copy=true); /* M_APM constructor (refcount starts at one) */
	MAPM(const char *s); /* Constructor from string */
	MAPM(double d); /* Constructor from double-precision float */
	MAPM(int l); /* Constructor from int */
	MAPM(long l); /* Constructor from long int */
	/* Destructor */
	~MAPM();

	M_APM c_struct() const { return myVal; }
	
	/* Extracting string descriptions: */
	void toString(char *dest,int decimalPlaces) const;
	void toFixPtString(char *dest,int decimalPlaces) const;
	void toFixPtStringEx(char *dest,int dp,char a,char b,int c) const;
	char *toFixPtStringExp(int dp,char a,char b,int c) const;
	void toIntegerString(char *dest) const;

  double toDouble() const;
	
	/* Basic operators: */
	MAPM &operator=(const MAPM &m); /* Assigment operator */
	MAPM &operator=(const char *s); /* Assigment operator */
	MAPM &operator=(double d); /* Assigment operator */
	MAPM &operator=(int l); /* Assigment operator */
	MAPM &operator=(long l); /* Assigment operator */
	MAPM operator++(); /* Prefix increment operator */
	MAPM operator--(); /* Prefix decrement operator */
	const MAPM operator++(int);  /* Postfix increment operator */
	const MAPM operator--(int);  /* Postfix decrement operator */
	
	/* Comparison operators */
	bool operator==(const MAPM &m) const; /* Equality operator */
	bool operator!=(const MAPM &m) const; /* Inequality operator */
	bool operator<(const MAPM &m) const;
	bool operator<=(const MAPM &m) const;
	bool operator>(const MAPM &m) const;
	bool operator>=(const MAPM &m) const;
	int compare(const MAPM &m) const;
	
	/* Basic arithmetic operators */
	friend MAPM operator+(const MAPM &a,const MAPM &b);
	friend MAPM operator-(const MAPM &a,const MAPM &b);
	friend MAPM operator*(const MAPM &a,const MAPM &b);

	/* Default division keeps larger of cpp_min_precision, numerator 
	   digits of precision, or denominator digits of precision. */
	friend MAPM operator/(const MAPM &a,const MAPM &b);
	
	MAPM divide(const MAPM &m,int toDigits) const;
	MAPM divide(const MAPM &m) const;
	
	/* Assignment arithmetic operators */
	MAPM &operator+=(const MAPM &m);
	MAPM &operator-=(const MAPM &m);
	MAPM &operator*=(const MAPM &m);
	MAPM &operator/=(const MAPM &m);
	
	/* Extracting/setting simple information: */
	int sign(void) const;
	int exponent(void) const ;
	int significant_digits(void) const;
	int is_integer(void) const;
	int is_even(void) const;
	int is_odd(void) const;

	/* Functions: */
	MAPM abs(void) const;
	MAPM neg(void) const;
	MAPM round(int toDigits) const;

/* I got tired of typing the various declarations for a simple 
   1-ary real-to-real function on MAPM's; hence this define:
   The digits-free versions return my digits of precision or 
   cpp_min_precision, whichever is bigger.
*/

#define MAPM_1aryFunc(func) \
	MAPM func(int toDigits) const;\
	MAPM func(void) const;

	MAPM_1aryFunc(sqrt)
	MAPM_1aryFunc(cbrt)
	MAPM_1aryFunc(log)
	MAPM_1aryFunc(exp)
	MAPM_1aryFunc(log10)
	MAPM_1aryFunc(sin)
	MAPM_1aryFunc(asin)
	MAPM_1aryFunc(cos)
	MAPM_1aryFunc(acos)
	MAPM_1aryFunc(tan)
	MAPM_1aryFunc(atan)
	MAPM_1aryFunc(sinh)
	MAPM_1aryFunc(asinh)
	MAPM_1aryFunc(cosh)
	MAPM_1aryFunc(acosh)
	MAPM_1aryFunc(tanh)
	MAPM_1aryFunc(atanh)
#undef MAPM_1aryFunc
	
	void sincos(MAPM &sinR,MAPM &cosR,int toDigits);
	void sincos(MAPM &sinR,MAPM &cosR);
	MAPM pow(const MAPM &m,int toDigits) const;
	MAPM pow(const MAPM &m) const;
	MAPM atan2(const MAPM &x,int toDigits) const;
	MAPM atan2(const MAPM &x) const;

	MAPM gcd(const MAPM &m) const;

	MAPM lcm(const MAPM &m) const;

	static MAPM random(void);

	MAPM floor(void) const;
	MAPM ceil(void) const;

	/* Functions defined only on integers: */
	MAPM factorial(void) const;
	MAPM ipow_nr(int p) const;
	MAPM ipow(int p,int toDigits) const;
	MAPM ipow(int p) const;
	MAPM integer_divide(const MAPM &denom) const;
	void integer_div_rem(const MAPM &denom,MAPM &quot,MAPM &rem) const;
	MAPM div(const MAPM &denom) const;
	MAPM rem(const MAPM &denom) const;
};

/* math.h-style functions: */

inline MAPM fabs(const MAPM &m) {return m.abs();}
inline MAPM factorial(const MAPM &m) {return m.factorial();}
inline MAPM floor(const MAPM &m) {return m.floor();}
inline MAPM ceil(const MAPM &m) {return m.ceil();}
inline MAPM get_random(void) {return MAPM::random();}

/* I got tired of typing the various declarations for a simple 
   1-ary real-to-real function on MAPM's; hence this define:
*/
#define MAPM_1aryFunc(func) \
	inline MAPM func(const MAPM &m) {return m.func();} \
	inline MAPM func(const MAPM &m,int toDigits) {return m.func(toDigits);}

/* Define a big block of simple functions: */
	MAPM_1aryFunc(sqrt)
	MAPM_1aryFunc(cbrt)
	MAPM_1aryFunc(log)
	MAPM_1aryFunc(exp)
	MAPM_1aryFunc(log10)
	MAPM_1aryFunc(sin)
	MAPM_1aryFunc(asin)
	MAPM_1aryFunc(cos)
	MAPM_1aryFunc(acos)
	MAPM_1aryFunc(tan)
	MAPM_1aryFunc(atan)
	MAPM_1aryFunc(sinh)
	MAPM_1aryFunc(asinh)
	MAPM_1aryFunc(cosh)
	MAPM_1aryFunc(acosh)
	MAPM_1aryFunc(tanh)
	MAPM_1aryFunc(atanh)
#undef MAPM_1aryFunc

/* Computes x to the power y */
inline MAPM pow(const MAPM &x,const MAPM &y,int toDigits)
		{return x.pow(y,toDigits);}
inline MAPM pow(const MAPM &x,const MAPM &y)
		{return x.pow(y);}
inline MAPM atan2(const MAPM &y,const MAPM &x,int toDigits)
		{return y.atan2(x,toDigits);}
inline MAPM atan2(const MAPM &y,const MAPM &x)
		{return y.atan2(x);}
inline MAPM gcd(const MAPM &u,const MAPM &v)
		{return u.gcd(v);}
inline MAPM lcm(const MAPM &u,const MAPM &v)
		{return u.lcm(v);}
#endif
#endif


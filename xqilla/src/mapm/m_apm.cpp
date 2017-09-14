
#include "../config/xqilla_config.h"
#include <xqilla/mapm/m_apm.h>

void MAPM::create(void) 
{
    myVal=makeNew();
}

void MAPM::destroy(void) 
{
    unref(myVal);
    myVal=0;
}

void MAPM::copyFrom(M_APM Nval) 
{
	 M_APM oldVal=myVal;
	 myVal=Nval;
	 ref(myVal);
	 unref(oldVal);
}

M_APM MAPM::makeNew(void) 
{
	M_APM val=m_apm_init_mt();
	/* refcount initialized to 1 by 'm_apm_init' */
	return val;
}

void MAPM::ref(M_APM val) 
{
	val->m_apm_refcount++;
}

void MAPM::unref(M_APM val) 
{
	val->m_apm_refcount--;
	if (val->m_apm_refcount==0)
		m_apm_free_mt(val);
}
	
M_APM MAPM::val(void) 
{
	if (myVal->m_apm_refcount==1) 
	/* Return my private myVal */
		return myVal;

	/* Otherwise, our copy of myVal is shared--
	   we need to make a new private copy.
            */
	M_APM oldVal=myVal;
	myVal=makeNew();
	m_apm_copy(myVal,oldVal);
	unref(oldVal);
	return myVal;
}

M_APM MAPM::cval(void) const 
{
	return (M_APM)myVal;
}

int MAPM::myDigits(void) const 
{
	int maxd=m_apm_significant_digits_mt(cval());
	if (maxd<MM_cpp_min_precision) maxd=MM_cpp_min_precision;
	return maxd;
}

int MAPM::digits(const MAPM &otherVal) const 
{
	int maxd=myDigits();
	int his=m_apm_significant_digits_mt(otherVal.cval());
	if (maxd<his) maxd=his;
	return maxd;
}

MAPM::MAPM(void)
{
    create();
}

MAPM::MAPM(const MAPM &m)
{
    myVal=(M_APM)m.cval();
    ref(myVal);
}

MAPM::MAPM(M_APM m, bool copy)
{
  if(copy) {
    create();
    m_apm_copy(myVal, m);
  }
  else {
    myVal=(M_APM)m;
    ref(myVal);
  }
}

MAPM::MAPM(const char *s)
{
    create();
    m_apm_set_string_mt(val(),(char *)s);
}

MAPM::MAPM(double d)
{
    create();
    m_apm_set_double_mt(val(),d);
}

MAPM::MAPM(int l)
{
    create();
    m_apm_set_long(val(),l);
}

MAPM::MAPM(long l)
{
    create();
    m_apm_set_long(val(),l);
}

MAPM::~MAPM() 
{
    destroy();
}

void MAPM::toString(char *dest,int decimalPlaces) const
{
    m_apm_to_string_mt(dest,decimalPlaces,cval());
}

void MAPM::toFixPtString(char *dest,int decimalPlaces) const
{
    m_apm_to_fixpt_string_mt(dest,decimalPlaces,cval());
}

void MAPM::toFixPtStringEx(char *dest,int dp,char a,char b,int c) const
{
    m_apm_to_fixpt_stringex_mt(dest,dp,cval(),a,b,c);
}

char* MAPM::toFixPtStringExp(int dp,char a,char b,int c) const
{
    return(m_apm_to_fixpt_stringexp_mt(dp,cval(),a,b,c));
}

void MAPM::toIntegerString(char *dest) const
{
    m_apm_to_integer_string(dest,cval());
}

double MAPM::toDouble() const
{
  return m_apm_get_double(cval());
}
	
MAPM & MAPM::operator=(const MAPM &m)
{
//     m_apm_enter();
    copyFrom((M_APM)m.cval());
//     m_apm_leave();
    return *this;
}

MAPM & MAPM::operator=(const char *s)
{
    m_apm_set_string_mt(val(),(char *)s);
    return *this;
}

MAPM & MAPM::operator=(double d)
{
    m_apm_set_double_mt(val(),d);
    return *this;
}

MAPM & MAPM::operator=(int l)
{
    m_apm_set_long(val(),l);
    return *this;
}

MAPM & MAPM::operator=(long l)
{
    m_apm_set_long(val(),l);
    return *this;
}

MAPM MAPM::operator++()
{
//     m_apm_enter();
    MAPM ret;
    m_apm_add_mt(ret.val(),cval(),MM_One);
    *this = ret;
//     m_apm_leave();
    return *this;
}

MAPM MAPM::operator--()
{
//     m_apm_enter();
    MAPM ret;
    m_apm_subtract_mt(ret.val(),cval(),MM_One);
    *this = ret;
//     m_apm_leave();
    return *this;
}

const MAPM MAPM::operator++(int)
{
// 	m_apm_enter();
	MAPM old = *this;
	++(*this);          /* Call prefix increment */
// 	m_apm_leave();
	return old;
}

const MAPM MAPM::operator--(int)
{
// 	m_apm_enter();
	MAPM old = *this;
	--(*this);          /* Call prefix decrement */
// 	m_apm_leave();
	return old;
}
	
bool MAPM::operator==(const MAPM &m) const
{
    return m_apm_compare(cval(),m.cval())==0;
}

bool MAPM::operator!=(const MAPM &m) const
{
    return m_apm_compare(cval(),m.cval())!=0;
}

bool MAPM::operator<(const MAPM &m) const
{
    return m_apm_compare(cval(),m.cval())<0;
}

bool MAPM::operator<=(const MAPM &m) const
{
    return m_apm_compare(cval(),m.cval())<=0;
}

bool MAPM::operator>(const MAPM &m) const
{
    return m_apm_compare(cval(),m.cval())>0;
}

bool MAPM::operator>=(const MAPM &m) const
{
    return m_apm_compare(cval(),m.cval())>=0;
}

int MAPM::compare(const MAPM &m) const
{
    return m_apm_compare(cval(),m.cval());
}

MAPM operator+(const MAPM &a,const MAPM &b)
{
    MAPM ret;
    m_apm_add_mt(ret.val(),a.cval(),b.cval());
    return ret;
}

MAPM operator-(const MAPM &a,const MAPM &b)
{
    MAPM ret;
    m_apm_subtract_mt(ret.val(),a.cval(),b.cval());
    return ret;
}

MAPM operator*(const MAPM &a,const MAPM &b)
{
    MAPM ret;
    m_apm_multiply_mt(ret.val(),a.cval(),b.cval());
    return ret;
}

MAPM operator/(const MAPM &a,const MAPM &b) 
{
    return a.divide(b,a.digits(b));
}
	
MAPM MAPM::divide(const MAPM &m,int toDigits) const
{
    MAPM ret;
    m_apm_divide_mt(ret.val(),toDigits,cval(),m.cval());
    return ret;
}

MAPM MAPM::divide(const MAPM &m) const 
{
    return divide(m,digits(m));
}
	
MAPM & MAPM::operator+=(const MAPM &m) 
{
    *this = *this+m;
    return *this;
}

MAPM & MAPM::operator-=(const MAPM &m) 
{
    *this = *this-m;
    return *this;
}

MAPM & MAPM::operator*=(const MAPM &m) 
{
    *this = *this*m;
    return *this;
}

MAPM & MAPM::operator/=(const MAPM &m) 
{
    *this = *this/m;
    return *this;
}

int MAPM::sign(void) const
{
    return m_apm_sign(cval());
}

int MAPM::exponent(void) const 
{
    return m_apm_exponent(cval());
}

int MAPM::significant_digits(void) const 
{
    return m_apm_significant_digits_mt(cval());
}

int MAPM::is_integer(void) const 
{
    return m_apm_is_integer(cval());
}

int MAPM::is_even(void) const 
{
    return m_apm_is_even(cval());
}

int MAPM::is_odd(void) const 
{
    return m_apm_is_odd(cval());
}

MAPM MAPM::abs(void) const
{
    MAPM ret;
    m_apm_absolute_value(ret.val(),cval());
    return ret;
}

MAPM MAPM::neg(void) const
{
    MAPM ret;
    m_apm_negate(ret.val(),cval());
    return ret;
}

MAPM MAPM::round(int toDigits) const
{
    MAPM ret;
    m_apm_round_mt(ret.val(),toDigits,cval());
    return ret;
}


#define MAPM_1aryFuncImpl(func) \
MAPM MAPM::func(int toDigits) const\
	{MAPM ret;m_apm_##func##_mt(ret.val(),toDigits,cval());return ret;}\
MAPM MAPM::func(void) const {return func(myDigits());}

MAPM_1aryFuncImpl(sqrt)
MAPM_1aryFuncImpl(cbrt)
MAPM_1aryFuncImpl(log)
MAPM_1aryFuncImpl(exp)
MAPM_1aryFuncImpl(log10)
MAPM_1aryFuncImpl(sin)
MAPM_1aryFuncImpl(asin)
MAPM_1aryFuncImpl(cos)
MAPM_1aryFuncImpl(acos)
MAPM_1aryFuncImpl(tan)
MAPM_1aryFuncImpl(atan)
MAPM_1aryFuncImpl(sinh)
MAPM_1aryFuncImpl(asinh)
MAPM_1aryFuncImpl(cosh)
MAPM_1aryFuncImpl(acosh)
MAPM_1aryFuncImpl(tanh)
MAPM_1aryFuncImpl(atanh)
#undef MAPM_1aryFuncImpl
	
void MAPM::sincos(MAPM &sinR,MAPM &cosR,int toDigits)
{
    m_apm_sin_cos_mt(sinR.val(),cosR.val(),toDigits,cval());
}

void MAPM::sincos(MAPM &sinR,MAPM &cosR)
{
    sincos(sinR,cosR,myDigits());
}

MAPM MAPM::pow(const MAPM &m,int toDigits) const
{
    MAPM ret;
    m_apm_pow_mt(ret.val(),toDigits,cval(), m.cval());
    return ret;
}

MAPM MAPM::pow(const MAPM &m) const 
{
    return pow(m,digits(m));
}

MAPM MAPM::atan2(const MAPM &x,int toDigits) const
{
    MAPM ret;
    m_apm_arctan2_mt(ret.val(),toDigits,cval(),x.cval());
    return ret;
}

MAPM MAPM::atan2(const MAPM &x) const
{
    return atan2(x,digits(x));
}

MAPM MAPM::gcd(const MAPM &m) const
{
    MAPM ret;
    m_apm_gcd_mt(ret.val(),cval(),m.cval());
    return ret;
}

MAPM MAPM::lcm(const MAPM &m) const
{
    MAPM ret;
    m_apm_lcm_mt(ret.val(),cval(),m.cval());
    return ret;
}

MAPM MAPM::random(void) 
{
    MAPM ret;
    m_apm_get_random_mt(ret.val());
    return ret;
}

MAPM MAPM::floor(void) const
{
    MAPM ret;
    m_apm_floor_mt(ret.val(),cval());
    return ret;
}

MAPM MAPM::ceil(void) const
{
    MAPM ret;
    m_apm_ceil_mt(ret.val(),cval());
    return ret;
}

MAPM MAPM::factorial(void) const
{
    MAPM ret;
    m_apm_factorial_mt(ret.val(),cval());
    return ret;
}

MAPM MAPM::ipow_nr(int p) const
{
    MAPM ret;
    m_apm_integer_pow_nr_mt(ret.val(),cval(),p);
    return ret;
}

MAPM MAPM::ipow(int p,int toDigits) const
{
    MAPM ret;
    m_apm_integer_pow_mt(ret.val(),toDigits,cval(),p);
    return ret;
}

MAPM MAPM::ipow(int p) const
{
    return ipow(p,myDigits());
}

MAPM MAPM::integer_divide(const MAPM &denom) const
{
    MAPM ret;
    m_apm_integer_divide_mt(ret.val(),cval(),denom.cval());
    return ret;
}

void MAPM::integer_div_rem(const MAPM &denom,MAPM &quot,MAPM &rem) const
{
    m_apm_integer_div_rem_mt(quot.val(),rem.val(),cval(),denom.cval());
}

MAPM MAPM::div(const MAPM &denom) const 
{
    return integer_divide(denom);
}

MAPM MAPM::rem(const MAPM &denom) const 
{
    MAPM ret,ignored;
	integer_div_rem(denom,ignored,ret);return ret;
}


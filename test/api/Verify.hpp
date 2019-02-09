//  Copyright (C) 2019 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.

#include "MSIXWindows.hpp"

#include <string>
#include <cstdarg>
#include <iostream>

template <typename T1, typename T2 = T1>
class VerifyCompareTraits
{
public:
    static bool AreEqual(const T1& expected, const T2& actual) { return (expected == actual); }
    static bool AreSame(const T1& expected, const T2& actual) { return reinterpret_cast<const void*>(&expected) == reinterpret_cast<const void*>(&actual); }
};

template <typename T1, typename T2>
class VerifyCompareTraits<T1*, T2*>
{
public:
    static bool AreEqual(T1* const& expected, T2* const& actual) { return (reinterpret_cast<const void*>(expected) == reinterpret_cast<const void*>(actual)); }
    static bool AreSame(T1* const& expected, T2* const& actual) { return (reinterpret_cast<const void* const&>(expected) == reinterpret_cast<const void* const&>(actual)); }
};

template<typename T>
class VerifyException : public std::exception
{
public:

    template<typename U = T, typename std::enable_if<std::is_integral<U>::value>::type* = nullptr>
    VerifyException(U e, U a, const int l) :  line(l)
    {
        message = "expected: " + std::to_string(e) + " got: " + std::to_string(a) + " on line " + std::to_string(line);
    }

    template<typename U = T, typename std::enable_if<!std::is_integral<U>::value>::type* = nullptr>
    VerifyException(U e, U a, const int l) : line(l)
    {
        std::string exp(e);
        std::string act(a);
        message = "expected: " + exp + " got: " + a + " on line " + std::to_string(line);
    }

    const char *what() const noexcept override { return message.c_str(); }

protected:
    std::string message;
    int line;
};

class Verify
{
    private:
        static void LogSucceed(int nLogs...)
        {
            va_list logs;
            va_start(logs, nLogs);
            std::cout << "SUCCEEDED ";
            for (int i = 0; i < nLogs; i++)
            {
                std::cout << va_arg(logs, const char*);
            }
            std::cout << std::endl;
            va_end(logs);
        }

    public:
        static void Succeeded(HRESULT hr, const char* expression, const int line)
        {
            if (SUCCEEDED(hr))
            {
                LogSucceed(1, expression);
            }
            else
            {
                throw VerifyException<HRESULT>(S_OK, hr, line);
            }
        }

        template<typename T>
        static void AreEqual(T expectedValue, const char* expected, T actualValue, const char* actual, const int line)
        { 
            if (VerifyCompareTraits<T>::AreEqual(expectedValue, actualValue))
            {
                LogSucceed(3, expected, " == ", actual);
            }
            else
            {
                throw VerifyException<T>(expectedValue, actualValue, line);
            }
        }

        static void BoolCheck(const char* expected, bool actualValue, const char* actual, const int line)
        {
            if (actualValue)
            {
                LogSucceed(2, expected, actual);
            }
            else
            {
                throw VerifyException<const char*>(expected, actual, line);
            }
        }

        template<typename T>
        static void AreSame(T expectedObj, const char* expected, T actualObj, const char* actual, const int line)
        {
            if (VerifyCompareTraits<T, T>::AreSame(expectedObj, actualObj))
            {
                LogSucceed(3, expected, " == ", actual);
            }
            else
            {
                throw VerifyException<const char*>(expected, actual, line);
            }
        }

        template<typename T>
        static void IsNull(T* object, const char* exp, const int line)
        {
            if (object == nullptr)
            {
                LogSucceed(2, exp, " is null");
            }
            else
            {
                throw VerifyException<const char*>(exp, " expect object to be null, but it isn't", line);
            }
        }

        template<typename T>
        static void IsNotNull(T* object, const char* exp, const int line)
        {
            if (object != nullptr)
            {
                LogSucceed(2, exp, " is not null");
            }
            else
            {
                throw VerifyException<const char*>(exp, " expect object to be not null, but is null", line);
            }
        }
};

#define VERIFY_SUCCEEDED(__hr) { Verify::Succeeded(__hr, #__hr, __LINE__); }
#define VERIFY_ARE_EQUAL(__exp, __act) { Verify::AreEqual(__exp, #__exp, __act, #__act, __LINE__); }
#define VERIFY_ARE_SAME(__exp, __act) { Verify::AreSame(__exp, #__exp, __act, #__act, __LINE__); }
#define VERIFY_IS_NULL(__act) { Verify::IsNull(__act, #__act, __LINE__); }
#define VERIFY_NOT_NULL(__act) { Verify::IsNotNull(__act, #__act, __LINE__); }
#define VERIFY_HR(__exp, __hr) { Verify::AreEqual(__exp, #__exp, __hr, #__hr, __LINE__); }
#define VERIFY_IS_TRUE(__act) { Verify::BoolCheck("IsTrue ", __act, #__act, __LINE__); }
#define VERIFY_IS_FALSE(__act) { Verify::BoolCheck("IsFalse ", !(__act), #__act, __LINE__); }

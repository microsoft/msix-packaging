//
//  Copyright (C) 2017 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
//

#include "MSIXWindows.hpp"
#include "AppxPackaging.hpp"
#include "ExpectedValues.hpp"

#include <iostream>
#include <exception>
#include <codecvt>
#include <string>
#include <locale>

class Exception : public std::exception
{
public:
    Exception(std::string actual, std::string expected, const int line) : 
        m_actual(actual),
        m_expected(expected),
        m_line(line)
    {}

    Exception(const char* actual, const char* expected, const int line) : 
        m_actual(actual),
        m_expected(expected),
        m_line(line)
    {}

    Exception(HRESULT actual, HRESULT expected, const int line) : 
        m_line(line)
    {
        m_actual = std::to_string(static_cast<std::uint32_t>(actual));
        m_expected = std::to_string(static_cast<std::uint32_t>(actual));
    }

    std::string& Actual()   { return m_actual; }
    std::string& Expected() { return m_expected; }
    int         Line()     { return m_line; }
protected:
    std::string m_actual;
    std::string m_expected;
    int         m_line;
};

LPVOID STDMETHODCALLTYPE MyAllocate(SIZE_T cb) { return std::malloc(cb); }
void STDMETHODCALLTYPE   MyFree(LPVOID pv)     { std::free(pv); }

// Helper class to free string buffers obtained from the packaging APIs.
class Text
{
public:
    wchar_t** operator&() { return &content; }
    ~Text() { Cleanup(); }
    wchar_t* Get() { return content; }
protected:
    wchar_t* content = nullptr;
    void Cleanup() { if (content) { MyFree(content); content = nullptr; } }
};

// Stripped down ComPtr provided for those platforms that do not already have a ComPtr class.
template <class T>
class ComPtr
{
public:
    // default ctor
    ComPtr() = default;
    ComPtr(T* ptr) : m_ptr(ptr) { InternalAddRef(); }

    ~ComPtr() { InternalRelease(); }
    inline T* operator->() const { return m_ptr; }
    inline T* Get() const { return m_ptr; }

    inline T** operator&()
    {   InternalRelease();
        return &m_ptr;
    }

protected:
    T* m_ptr = nullptr;

    inline void InternalAddRef() { if (m_ptr) { m_ptr->AddRef(); } }
    inline void InternalRelease()
    {   
        T* temp = m_ptr;
        if (temp)
        {   m_ptr = nullptr;
            temp->Release();
        }
    }
};

std::string utf16_to_utf8(const std::wstring& utf16string)
{
    auto converted = std::wstring_convert<std::codecvt_utf8<wchar_t>>{}.to_bytes(utf16string.data());
    std::string result(converted.begin(), converted.end());
    return result;
}

// Compares the string without caring about the \\ or / depending on the platform
bool Compare(std::wstring actual, std::wstring expected)
{   // actual always use the current platform path separator
    std::wstring actualCopy = actual;
    for(auto& c : actualCopy)
    {
        if(c == '/')
        {
            c = '\\';
        }
    }
    return actualCopy == expected;
}

void LogSucceed(int nLogs...)
{
    va_list logs;
    va_start(logs, nLogs);
    std::cout << "Success ";
    for(int i = 0; i < nLogs; i++)
    {
        std::cout << va_arg(logs, const char*);
    }
    std::cout << std::endl;
    va_end(logs);
}

// Macros used for testing
#define VERIFY_SUCCEEDED(__hr)         { if (FAILED(__hr)) { throw Exception(__hr, S_OK, __LINE__); } else { LogSucceed(1, #__hr); } }
#define VERIFY_HR(__exp, __hr)         { if (__exp != __hr) { throw Exception(__hr, __exp, __LINE__); } else { LogSucceed(3, #__hr, " == ", #__exp); } }
#define VERIFY_WSTR(__res, __exp)      { if(!Compare(__res, __exp)) { throw Exception(utf16_to_utf8(__res), utf16_to_utf8(__exp), __LINE__); } else { LogSucceed(3, #__res, " == ", #__exp); } }
#define VERIFY_ARE_EQUAL(__res, __exp) { if(__res != __exp) { throw Exception(std::to_string(__res), std::to_string(__exp), __LINE__); } else { LogSucceed(3, #__res, " == ", #__exp); } }
#define VERIFY_ARE_SAME(__res, __exp)  { if(__res != __exp) { throw Exception(#__res, #__exp, __LINE__); } else { LogSucceed(3, #__res, " == ", #__exp); } }
#define VERIFY_NOT_NULL(__res)         { if(__res == nullptr) { throw Exception(#__res, "pointer is null", __LINE__); } else { LogSucceed(2, #__res, " not null"); } }
#define VERIFY_IS_NULL(__res)          { if(__res != nullptr) { throw Exception(#__res, "pointer is not null", __LINE__); } else { LogSucceed(2, #__res, " is null"); } }
#define VERIFY_IS_TRUE(__res)          { if(!__res) { throw Exception(#__res, "is false", __LINE__); } else { LogSucceed(2, #__res, " is true"); } }

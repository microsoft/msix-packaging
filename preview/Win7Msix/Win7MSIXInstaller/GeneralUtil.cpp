#include "GeneralUtil.h"
#include <string>
#include <codecvt>
#include <iostream>

//
// Converts a wstring from utf16 to utf8
//
// Parameters:
// utf16string - A utf16 wstring
// 
std::string utf16_to_utf8(const std::wstring& utf16string)
{
	auto converted = std::wstring_convert<std::codecvt_utf8<wchar_t>>{}.to_bytes(utf16string.data());
	std::string result(converted.begin(), converted.end());
	return result;
}

//
// Converts a string from utf8 to utf16
//
// Parameters:
// utf8string - A utf8 string
// 
std::wstring utf8_to_utf16(const std::string& utf8string)
{
	// see https://connect.microsoft.com/VisualStudio/feedback/details/1403302/unresolved-external-when-using-codecvt-utf8
	auto converted = std::wstring_convert<std::codecvt_utf8_utf16<unsigned short>, unsigned short>{}.from_bytes(utf8string.data());
	std::wstring result(converted.begin(), converted.end());
	return result;
}



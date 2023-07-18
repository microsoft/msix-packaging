#pragma once

#include<windows.h>

namespace ErrorMessageHelper
{
    std::wstring GetErrorMessageFromHRESULT(
        HRESULT hresult);

}
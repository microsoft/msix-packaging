//
//  Copyright (C) 2019 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
// 
#pragma once

#include <string>

namespace MSIX {

    // Validates payload file name is correct
    struct FileNameValidation
    {
        static bool IsFileNameValid(const std::string& name);
        static bool IsIdentifierValid(const std::string& name);
        static bool IsFootPrintFile(const std::string& fileName, bool isBundle);
        static bool IsReservedFolder(const std::string& fileName);
    };
}

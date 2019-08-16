//
//  Copyright (C) 2017 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
// 
#pragma once
#include <string>

namespace MSIX {
    namespace Global { 
        namespace Log {
            void Append(const std::string& comment);
            std::string Text();
            void Clear();
        }
    }
}
#pragma once
#include <string>

namespace MSIX {
    namespace Global { 
        namespace Log {
            void Append(const std::string& comment);
            const std::string& Text();
            void Clear();
        }
    }
}
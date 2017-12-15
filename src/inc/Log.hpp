#pragma once
#include <string>

namespace xPlat {
    namespace Global { 
        namespace Log {
            void Append(const std::string& comment);
            const std::string& Text();
        }
    }
}
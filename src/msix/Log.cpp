//
//  Copyright (C) 2017 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
// 
#include "Log.hpp"
#include <sstream>

namespace MSIX { namespace Global { namespace Log {
static std::stringstream g_content;

void Append(const std::string& comment) { ((!comment.empty()) ? g_content << '\n' : g_content) << comment; }
std::string Text() { return g_content.str(); }
void Clear() { g_content.clear(); }

} /* log */ } /* Global */ } /* msix */
#include "Log.hpp"
#include <string>

namespace MSIX { namespace Global { namespace Log {

static std::string g_content;

void Append(const std::string& comment)
{
    if (g_content.empty())
    {
        g_content = comment;
    }
    else
    {
        g_content = g_content + "\n" + comment;
    }
}

const std::string& Text()
{
    return g_content;
}

void Clear()
{
    g_content.clear();
}

} /* log */ } /* Global */ } /* msix */
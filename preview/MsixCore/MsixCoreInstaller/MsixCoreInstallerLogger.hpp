#pragma once
#include <windows.h>
#include <TraceLoggingProvider.h>
TRACELOGGING_DECLARE_PROVIDER(g_MsixUITraceLoggingProvider);


// Definition of function to return error if failed
#define __FILENAME__ (strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__)
#define RETURN_IF_FAILED(a) \
{                       \
    HRESULT __hr = a;   \
    if (FAILED(__hr))   \
    {   \
        TraceLoggingWrite(g_MsixUITraceLoggingProvider, \
            "RETURN_IF_FAILED", \
            TraceLoggingLevel(WINEVENT_LEVEL_ERROR), \
            TraceLoggingValue(#a, "Code"), \
            TraceLoggingHResult(__hr, "HR"), \
            TraceLoggingUInt32(__LINE__, "Line"), \
            TraceLoggingValue(__FILENAME__, "Filename")); \
        return __hr; \
    }  \
}
#include "MsixCoreInstallerLogger.hpp"

// Define the GUID to use in TraceLoggingProviderRegister 
// {033321d3-d599-48e0-868d-c59f15901637}
// One way to enable:
// logman create trace <nameoftrace> -p "{033321d3-d599-48e0-868d-c59f15901637}" -o <filename>
// i.e. logman create trace MsixTrace -p "{033321d3-d599-48e0-868d-c59f15901637}" -o c:\msixtrace.etl
// logman start MsixTrace
// logman stop MsixTrace
// tracerpt.exe, Windows Performance Analyzer or other tools can be used to view the etl file.
TRACELOGGING_DEFINE_PROVIDER(
    g_MsixTraceLoggingProvider,
    "MsixTraceLoggingProvider",
    (0x033321d3, 0xd599, 0x48e0, 0x86, 0x8d, 0xc5, 0x9f, 0x15, 0x90, 0x16, 0x37));

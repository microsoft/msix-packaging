#include "MsixCoreInstallerLogger.hpp"
// Define the GUID to use in TraceLoggingProviderRegister 
// {db5b779e-2dcf-41bc-ab0e-40a6e02f1438}
// One way to enable:
// logman create trace <nameoftrace> -p "{db5b779e-2dcf-41bc-ab0e-40a6e02f1438}" -o <filename>
// i.e. logman create trace MsixTrace -p "{db5b779e-2dcf-41bc-ab0e-40a6e02f1438}" -o c:\msixtrace.etl
// logman start MsixTrace
// logman stop MsixTrace
// tracerpt.exe, Windows Performance Analyzer or other tools can be used to view the etl file.
TRACELOGGING_DEFINE_PROVIDER(
    g_MsixUITraceLoggingProvider,
    "MsixInstallerUITraceLoggingProvider",
    (0xdb5b779e, 0x2dcf, 0x41bc, 0xab, 0x0e, 0x40, 0xa6, 0xe0, 0x2f, 0x14, 0x38));

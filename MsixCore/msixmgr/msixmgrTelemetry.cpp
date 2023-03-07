#include "msixmgrTelemetry.hpp"
// Define the GUID to use in TraceLoggingProviderRegister 
// {db5b779e-2dcf-41bc-ab0e-40a6e02f1438}
// One way to enable:
// logman create trace <nameoftrace> -p "{db5b779e-2dcf-41bc-ab0e-40a6e02f1438}" -o <filename>
// i.e. logman create trace MsixTrace -p "{db5b779e-2dcf-41bc-ab0e-40a6e02f1438}" -o c:\msixtrace.etl
// logman start MsixTrace
// logman stop MsixTrace
// tracerpt.exe, Windows Performance Analyzer or other tools can be used to view the etl file.
TRACELOGGING_DEFINE_PROVIDER(
    g_MsixMgrTelemetryProvider,
    "Microsoft.Windows.MsixMgr",
    (0x9e1b67a1, 0xdd1c, 0x5d8c, 0xd7, 0x71, 0xd2, 0xc4, 0x5c, 0xd8, 0x84, 0x65),
    TraceLoggingOptionMicrosoftTelemetry());
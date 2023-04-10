#include "msixmgrTelemetry.hpp"

// Defines the Provider to be used for Telemetry Logging for MsixMgr Tool Dashboard

TRACELOGGING_DEFINE_PROVIDER(
    g_MsixMgrTelemetryProvider,
    "Microsoft.Windows.MsixMgr",
    (0x9e1b67a1, 0xdd1c, 0x5d8c, 0xd7, 0x71, 0xd2, 0xc4, 0x5c, 0xd8, 0x84, 0x65),
    TraceLoggingOptionMicrosoftTelemetry());
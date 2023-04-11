#pragma once
#include <windows.h>
#include <TraceLoggingProvider.h>

// Declares the Provider to be used for Telemetry Logging for MsixMgr Tool Telemetry
// Defines the Provider Option Group to which the the telemetry will be logged
// Defines the Privacy Tags and Data tags

TRACELOGGING_DECLARE_PROVIDER(g_MsixMgrTelemetryProvider);

#define TraceLoggingOptionMicrosoftTelemetry() \
    TraceLoggingOptionGroup(0000000000, 00000, 00000, 0000, 0000, 0000, 0000, 0000, 000, 0000, 0000)

#define TelemetryPrivacyDataTag(tag) TraceLoggingUInt64((tag), "PartA_PrivTags")

#define PDT_ProductAndServiceUsage             0x0000000002000000u

#define MICROSOFT_KEYWORD_CRITICAL_DATA 0x0000800000000000 // Bit 47
#pragma once
#include <windows.h>
#include "AppxPackaging.hpp"
#include <winmeta.h>
#include <string.h>

/// Helper to get string resource
///
/// @param resourceId - resource ID, these should be listed in resource.h
/// @return string for the resource, resolved from the stringtable defined in MsixCoreInstaller.rc
std::wstring GetStringResource(UINT resourceId);


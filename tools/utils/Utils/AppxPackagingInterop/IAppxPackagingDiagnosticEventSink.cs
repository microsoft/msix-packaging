﻿// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

namespace Microsoft.Msix.Utils.AppxPackagingInterop
{
    using System;
    using System.Runtime.InteropServices;

    [Flags]
    public enum APPX_PACKAGING_CONTEXT_CHANGE_TYPE
    {
        APPX_PACKAGING_CONTEXT_CHANGE_TYPE_START = 0,
        APPX_PACKAGING_CONTEXT_CHANGE_TYPE_CHANGE = 1,
        APPX_PACKAGING_CONTEXT_CHANGE_TYPE_DETAILS = 2,
        APPX_PACKAGING_CONTEXT_CHANGE_TYPE_END = 3
    }

    [Guid("17239D47-6ADB-45D2-80F6-F9CBC3BF059D"), InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
    public interface IAppxPackagingDiagnosticEventSink : IDisposable
    {
        void ReportContextChange(
            [In] APPX_PACKAGING_CONTEXT_CHANGE_TYPE changeType,
            [In] int contextId,
            [In, MarshalAs(UnmanagedType.LPStr)] string contextName,
            [In, MarshalAs(UnmanagedType.LPWStr)] string contextMessage,
            [In, MarshalAs(UnmanagedType.LPWStr)] string detailsMessage);

        void ReportError(
            [In, MarshalAs(UnmanagedType.LPWStr)] string errorMessage);
    }
}

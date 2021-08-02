﻿// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

namespace Microsoft.Msix.Utils.AppxPackagingInterop
{
    using System;
    using System.Runtime.InteropServices;

    [Guid("369648FA-A7EB-4909-A15D-6954A078F18A"), InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
    public interface IAppxPackagingDiagnosticEventSinkManager : IDisposable
    {
        void SetSinkForProcess(
            [In] IAppxPackagingDiagnosticEventSink sink);

        void SetSinkForThread(
            [In] IAppxPackagingDiagnosticEventSink sink);
    }
}

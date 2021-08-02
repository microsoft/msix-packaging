// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

namespace Microsoft.Msix.Utils.AppxPackagingInterop
{
    using System;
    using System.Runtime.InteropServices;

    [Guid("9091B09B-C8D5-4F31-8687-A338259FAEFB"), InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
    public interface IAppxManifestTargetDeviceFamily : IDisposable
    {
        [return: MarshalAs(UnmanagedType.LPWStr)]
        string GetName();

        ulong GetMinVersion();

        ulong GetMaxVersionTested();
    }
}

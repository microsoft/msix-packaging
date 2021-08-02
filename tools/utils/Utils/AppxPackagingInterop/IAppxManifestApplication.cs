// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

namespace Microsoft.Msix.Utils.AppxPackagingInterop
{
    using System;

    using System.Runtime.InteropServices;

    [Guid("5da89bf4-3773-46be-b650-7e744863b7e8"), InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
    public interface IAppxManifestApplication : IDisposable
    {
        void GetStringValue(string name, [Out, MarshalAs(UnmanagedType.LPWStr)] out string value);

        [return: MarshalAs(UnmanagedType.LPWStr)]
        string GetAppUserModelId();
    }
}

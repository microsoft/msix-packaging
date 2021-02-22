// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

namespace Microsoft.Msix.Utils.AppxPackagingInterop
{
    using System;

    using System.Runtime.InteropServices;

    [Guid("03faf64d-f26f-4b2c-aaf7-8fe7789b8bca"), InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
    public interface IAppxManifestProperties : IDisposable
    {
        bool GetBoolValue(string name);

        void GetStringValue(string name, [Out, MarshalAs(UnmanagedType.LPWStr)] out string value);
    }
}
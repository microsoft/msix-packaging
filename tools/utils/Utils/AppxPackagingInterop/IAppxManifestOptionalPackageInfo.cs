// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

namespace Microsoft.Msix.Utils.AppxPackagingInterop
{
    using System;

    using System.Runtime.InteropServices;

    [Guid("2634847D-5B5D-4FE5-A243-002FF95EDC7E"), InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
    public interface IAppxManifestOptionalPackageInfo : IDisposable
    {
        bool GetIsOptionalPackage();

        [return: MarshalAs(UnmanagedType.LPWStr)]
        string GetMainPackageName();
    }
}
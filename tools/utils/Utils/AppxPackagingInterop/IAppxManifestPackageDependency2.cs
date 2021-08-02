// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

namespace Microsoft.Msix.Utils.AppxPackagingInterop
{
    using System;

    using System.Runtime.InteropServices;

    // Note: The AppxPackaging.idl definition of this interface implements IAppxManifestPackageDependency.
    // Therefore the functions in IAppxManifestPackageDependency should be re-declared here.
    [Guid("DDA0B713-F3FF-49D3-898A-2786780C5D98"), InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
    public interface IAppxManifestPackageDependency2 : IDisposable
    {
        // IAppxManifestPackageDependency functions
        [return: MarshalAs(UnmanagedType.LPWStr)]
        string GetName();

        [return: MarshalAs(UnmanagedType.LPWStr)]
        string GetPublisher();

        ulong GetMinVersion();

        // IAppxManifestPackageDependency2 functions
        ushort GetMaxMajorVersionTested();
    }
}
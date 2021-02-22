// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

namespace Microsoft.Msix.Utils.AppxPackagingInterop
{
    using System;

    using System.Runtime.InteropServices;

    [Guid("54CD06C1-268F-40BB-8ED2-757A9EBAEC8D"), InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
    public interface IAppxBundleManifestPackageInfo : IDisposable
    {
        APPX_BUNDLE_PAYLOAD_PACKAGE_TYPE GetPackageType();

        IAppxManifestPackageId GetPackageId();

        [return: MarshalAs(UnmanagedType.LPWStr)]
        string GetFileName();

        ulong GetOffset();

        ulong GetSize();

        IAppxManifestQualifiedResourcesEnumerator GetResources();
    }
}

// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

namespace Microsoft.Msix.Utils.AppxPackagingInterop
{
    using System;

    using System.Runtime.InteropServices;

    // Note: The AppxPackaging.idl definition of this interface implements IAppxManifestPackageId.
    // Therefore the functions in IAppxManifestPackageId should be re-declared here.
    [Guid("2256999d-d617-42f1-880e-0ba4542319d5"), InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
    public interface IAppxManifestPackageId2 : IDisposable
    {
        // IAppxManifestPackageId functions
        [return: MarshalAs(UnmanagedType.LPWStr)]
        string GetName();

        APPX_PACKAGE_ARCHITECTURE GetArchitecture();

        [return: MarshalAs(UnmanagedType.LPWStr)]
        string GetPublisher();

        ulong GetVersion();

        [return: MarshalAs(UnmanagedType.LPWStr)]
        string GetResourceId();

        bool ComparePublisher([In, MarshalAs(UnmanagedType.LPWStr)] string otherPublisher);

        [return: MarshalAs(UnmanagedType.LPWStr)]
        string GetPackageFullName();

        [return: MarshalAs(UnmanagedType.LPWStr)]
        string GetPackageFamilyName();

        // IAppxManifestPackageId2 functions
        APPX_PACKAGE_ARCHITECTURE2 GetArchitecture2();
    }
}

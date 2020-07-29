//-----------------------------------------------------------------------
// <copyright file="IAppxBundleManifestPackageInfo.cs" company="Microsoft">
//     Copyright (c) Microsoft. All rights reserved.
// </copyright>
//-----------------------------------------------------------------------

namespace Microsoft.Packaging.Utils.AppxPackagingInterop
{
    using System;
    using System.Diagnostics.CodeAnalysis;

    using System.Runtime.InteropServices;

    [SuppressMessage("StyleCop.CSharp.DocumentationRules", "*", Justification = "Interop")]
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

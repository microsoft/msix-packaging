//-----------------------------------------------------------------------
// <copyright file="IAppxBundleManifestOptionalBundleInfo.cs" company="Microsoft">
//     Copyright (c) Microsoft. All rights reserved.
// </copyright>
//-----------------------------------------------------------------------

namespace Microsoft.Msix.Utils.AppxPackagingInterop
{
    using System;
    using System.Diagnostics.CodeAnalysis;

    using System.Runtime.InteropServices;

    [SuppressMessage("StyleCop.CSharp.DocumentationRules", "*", Justification = "Interop")]
    [Guid("515BF2E8-BCB0-4D69-8C48-E383147B6E12"), InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
    public interface IAppxBundleManifestOptionalBundleInfo : IDisposable
    {
        IAppxManifestPackageId GetPackageId();

        [return: MarshalAs(UnmanagedType.LPWStr)]
        string GetFileName();

        IAppxBundleManifestPackageInfoEnumerator GetPackageInfoItems();
    }
}

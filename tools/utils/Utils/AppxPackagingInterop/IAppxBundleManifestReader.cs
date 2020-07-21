//-----------------------------------------------------------------------
// <copyright file="IAppxBundleManifestReader.cs" company="Microsoft">
//     Copyright (c) Microsoft. All rights reserved.
// </copyright>
//-----------------------------------------------------------------------

namespace Microsoft.Packaging.SDKUtils.AppxPackagingInterop
{
    using System;
    using System.Diagnostics.CodeAnalysis;

    using System.Runtime.InteropServices;
    using System.Runtime.InteropServices.ComTypes;

    [SuppressMessage("StyleCop.CSharp.DocumentationRules", "*", Justification = "Interop")]
    [Guid("CF0EBBC1-CC99-4106-91EB-E67462E04FB0"), InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
    public interface IAppxBundleManifestReader : IDisposable
    {
        IAppxManifestPackageId GetPackageId();

        IAppxBundleManifestPackageInfoEnumerator GetPackageInfoItems();

        IStream GetStream();
    }
}

// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

namespace Microsoft.Msix.Utils.AppxPackagingInterop
{
    using System;

    using System.Runtime.InteropServices;
    using System.Runtime.InteropServices.ComTypes;

    [Guid("BBA65864-965F-4A5F-855F-F074BDBF3A7B"), InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
    public interface IAppxBundleFactory : IDisposable
    {
        IAppxBundleWriter CreateBundleWriter([In] IStream outputStream, [In] ulong bundleVersion);

        IAppxBundleReader CreateBundleReader([In] IStream inputStream);

        IAppxBundleManifestReader CreateBundleManifestReader([In] IStream inputStream);
    }
}

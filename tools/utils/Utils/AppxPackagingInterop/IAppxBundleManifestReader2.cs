// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

namespace Microsoft.Msix.Utils.AppxPackagingInterop
{
    using System;

    using System.Runtime.InteropServices;

    // Note: The AppxPackaging.idl definition of this interface implements IUnknown, not IAppxBundleManifestReader.
    // Therefore the functions in IAppxBundleManifestReader should not be declared here.
    [Guid("5517DF70-033F-4AF2-8213-87D766805C02"), InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
    public interface IAppxBundleManifestReader2 : IDisposable
    {
        IAppxBundleManifestOptionalBundleInfoEnumerator GetOptionalBundles();
    }
}

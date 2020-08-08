//-----------------------------------------------------------------------
// <copyright file="IAppxBundleManifestReader2.cs" company="Microsoft">
//     Copyright (c) Microsoft. All rights reserved.
// </copyright>
//-----------------------------------------------------------------------

namespace Microsoft.Msix.Utils.AppxPackagingInterop
{
    using System;
    using System.Diagnostics.CodeAnalysis;

    using System.Runtime.InteropServices;

    // Note: The AppxPackaging.idl definition of this interface implements IUnknown, not IAppxBundleManifestReader.
    // Therefore the functions in IAppxBundleManifestReader should not be declared here.
    [SuppressMessage("StyleCop.CSharp.DocumentationRules", "*", Justification = "Interop")]
    [Guid("5517DF70-033F-4AF2-8213-87D766805C02"), InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
    public interface IAppxBundleManifestReader2 : IDisposable
    {
        IAppxBundleManifestOptionalBundleInfoEnumerator GetOptionalBundles();
    }
}

//-----------------------------------------------------------------------
// <copyright file="IAppxBundleReader.cs" company="Microsoft">
//     Copyright (c) Microsoft. All rights reserved.
// </copyright>
//-----------------------------------------------------------------------

namespace Microsoft.Packaging.SDKUtils.AppxPackagingInterop
{
    using System;
    using System.Diagnostics.CodeAnalysis;

    using System.Runtime.InteropServices;

    [SuppressMessage("StyleCop.CSharp.DocumentationRules", "*", Justification = "Interop")]
    [Guid("DD75B8C0-BA76-43B0-AE0F-68656A1DC5C8"), InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
    public interface IAppxBundleReader : IDisposable
    {
        IAppxFile GetFootprintFile([In] APPX_BUNDLE_FOOTPRINT_FILE_TYPE fileType);

        IAppxBlockMapReader GetBlockMap();

        IAppxBundleManifestReader GetManifest();

        IAppxFilesEnumerator GetPayloadPackages();

        IAppxFile GetPayloadPackage([In, MarshalAs(UnmanagedType.LPWStr)] string fileName);
    }
}

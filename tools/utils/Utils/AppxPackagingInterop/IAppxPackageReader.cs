//-----------------------------------------------------------------------
// <copyright file="IAppxPackageReader.cs" company="Microsoft">
//     Copyright (c) Microsoft. All rights reserved.
// </copyright>
//-----------------------------------------------------------------------

namespace Microsoft.Msix.Utils.AppxPackagingInterop
{
    using System;
    using System.Diagnostics.CodeAnalysis;

    using System.Runtime.InteropServices;

    [SuppressMessage("StyleCop.CSharp.DocumentationRules", "*", Justification = "Interop")]
    [Guid("b5c49650-99bc-481c-9a34-3d53a4106708"), InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
    public interface IAppxPackageReader : IDisposable
    {
        IAppxBlockMapReader GetBlockMap();

        IAppxFile GetFootprintFile([In] APPX_FOOTPRINT_FILE_TYPE type);

        IAppxFile GetPayloadFile([In, MarshalAs(UnmanagedType.LPWStr)] string fileName);

        IAppxFilesEnumerator GetPayloadFiles();

        IAppxManifestReader GetManifest();
    }
}

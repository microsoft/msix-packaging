// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

namespace Microsoft.Msix.Utils.AppxPackagingInterop
{
    using System;

    using System.Runtime.InteropServices;

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

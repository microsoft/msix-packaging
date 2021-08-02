﻿// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

namespace Microsoft.Msix.Utils.AppxPackagingInterop
{
    using System;

    using System.Runtime.InteropServices;
    using System.Runtime.InteropServices.ComTypes;

    [Guid("277672ac-4f63-42c1-8abc-beae3600eb59"), InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
    public interface IAppxBlockMapFile : IDisposable
    {
        IAppxBlockMapBlocksEnumerator GetBlocks();

        uint GetLocalFileHeaderSize();

        [return: MarshalAs(UnmanagedType.LPWStr)]
        string GetName();

        ulong GetUncompressedSize();

        bool ValidateFileHash([In] IStream fileStream);
    }
}

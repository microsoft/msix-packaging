// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

namespace Microsoft.Msix.Utils.AppxPackagingInterop
{
    using System;

    using System.Runtime.InteropServices;

    [Guid("75cf3930-3244-4fe0-a8c8-e0bcb270b889"), InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
    public interface IAppxBlockMapBlock : IDisposable
    {
        IntPtr GetHash([Out] out uint bufferSize);

        uint GetCompressedSize();
    }
}

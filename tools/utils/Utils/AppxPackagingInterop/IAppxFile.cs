// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

namespace Microsoft.Msix.Utils.AppxPackagingInterop
{
    using System;

    using System.Runtime.InteropServices;
    using System.Runtime.InteropServices.ComTypes;

    [Guid("91df827b-94fd-468f-827b-57f41b2f6f2e"), InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
    public interface IAppxFile : IDisposable
    {
        APPX_COMPRESSION_OPTION GetCompressionOption();

        [return: MarshalAs(UnmanagedType.LPWStr)]
        string GetContentType();

        [return: MarshalAs(UnmanagedType.LPWStr)]
        string GetName();

        ulong GetSize();

        IStream GetStream();
    }
}

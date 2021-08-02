// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

namespace Microsoft.Msix.Utils.AppxPackagingInterop
{
    using System.Runtime.InteropServices;
    using System.Runtime.InteropServices.ComTypes;

    [Guid("9099e33b-246f-41e4-881a-008eb613f858"), InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
    public interface IAppxPackageWriter
    {
        void AddPayloadFile(
            [In, MarshalAs(UnmanagedType.LPWStr)] string fileName,
            [In, MarshalAs(UnmanagedType.LPWStr)] string contentType,
            [In] APPX_COMPRESSION_OPTION compressionOption,
            [In] IStream inputStream);

        void Close(
            [In] IStream manifest);
    }
}

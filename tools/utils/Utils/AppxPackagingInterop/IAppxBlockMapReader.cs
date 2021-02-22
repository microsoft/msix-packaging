// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

namespace Microsoft.Msix.Utils.AppxPackagingInterop
{
    using System;

    using System.Runtime.InteropServices;
    using System.Runtime.InteropServices.ComTypes;

    [Guid("5efec991-bca3-42d1-9ec2-e92d609ec22a"), InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
    public interface IAppxBlockMapReader : IDisposable
    {
        IAppxBlockMapFile GetFile([In, MarshalAs(UnmanagedType.LPWStr)] string filename);

        IAppxBlockMapFilesEnumerator GetFiles();

        IUri GetHashMethod();

        IStream GetStream();
    }
}

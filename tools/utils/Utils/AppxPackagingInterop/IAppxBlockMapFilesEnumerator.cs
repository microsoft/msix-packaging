// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

namespace Microsoft.Msix.Utils.AppxPackagingInterop
{
    using System;

    using System.Runtime.InteropServices;

    [Guid("02b856a2-4262-4070-bacb-1a8cbbc42305"), InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
    public interface IAppxBlockMapFilesEnumerator : IDisposable
    {
        IAppxBlockMapFile GetCurrent();

        bool GetHasCurrent();

        bool MoveNext();
    }
}

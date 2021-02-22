// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

namespace Microsoft.Msix.Utils.AppxPackagingInterop
{
    using System;

    using System.Runtime.InteropServices;

    [Guid("6b429b5b-36ef-479e-b9eb-0c1482b49e16"), InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
    public interface IAppxBlockMapBlocksEnumerator : IDisposable
    {
        IAppxBlockMapBlock GetCurrent();

        bool GetHasCurrent();

        bool MoveNext();
    }
}

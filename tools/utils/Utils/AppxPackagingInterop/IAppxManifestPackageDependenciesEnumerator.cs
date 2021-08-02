// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

namespace Microsoft.Msix.Utils.AppxPackagingInterop
{
    using System;

    using System.Runtime.InteropServices;

    [Guid("b43bbcf9-65a6-42dd-bac0-8c6741e7f5a4"), InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
    public interface IAppxManifestPackageDependenciesEnumerator : IDisposable
    {
        IAppxManifestPackageDependency GetCurrent();

        bool GetHasCurrent();

        bool MoveNext();
    }
}

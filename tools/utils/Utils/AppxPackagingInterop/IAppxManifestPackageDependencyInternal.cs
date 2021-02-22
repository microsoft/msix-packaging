// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

namespace Microsoft.Msix.Utils.AppxPackagingInterop
{
    using System;

    using System.Runtime.InteropServices;

    [Guid("6A41FF03-BFA5-4866-BFA0-293F220E43B0"), InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
    public interface IAppxManifestPackageDependencyInternal : IDisposable
    {
        // IAppxManifestPackageDependencyInternal functions
        bool GetIsOptional();
    }
}
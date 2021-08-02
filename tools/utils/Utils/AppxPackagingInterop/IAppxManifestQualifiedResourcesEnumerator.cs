// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

namespace Microsoft.Msix.Utils.AppxPackagingInterop
{
    using System;

    using System.Runtime.InteropServices;

    [Guid("8ef6adfe-3762-4a8f-9373-2fc5d444c8d2"), InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
    public interface IAppxManifestQualifiedResourcesEnumerator : IDisposable
    {
        IAppxManifestQualifiedResource GetCurrent();

        bool GetHasCurrent();

        bool MoveNext();
    }
}

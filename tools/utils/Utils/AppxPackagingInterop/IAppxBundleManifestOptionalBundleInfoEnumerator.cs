// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

namespace Microsoft.Msix.Utils.AppxPackagingInterop
{
    using System;

    using System.Runtime.InteropServices;

    [Guid("9A178793-F97E-46AC-AACA-DD5BA4C177C8"), InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
    public interface IAppxBundleManifestOptionalBundleInfoEnumerator : IDisposable
    {
        IAppxBundleManifestOptionalBundleInfo GetCurrent();

        bool GetHasCurrent();

        bool MoveNext();
    }
}

// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

namespace Microsoft.Msix.Utils.AppxPackagingInterop
{
    using System;

    using System.Runtime.InteropServices;

    [Guid("F9B856EE-49A6-4E19-B2B0-6A2406D63A32"), InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
    public interface IAppxBundleManifestPackageInfoEnumerator : IDisposable
    {
        IAppxBundleManifestPackageInfo GetCurrent();

        bool GetHasCurrent();

        bool MoveNext();
    }
}

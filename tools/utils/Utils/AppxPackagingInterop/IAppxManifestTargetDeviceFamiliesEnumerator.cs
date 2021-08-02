// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

namespace Microsoft.Msix.Utils.AppxPackagingInterop
{
    using System;
    using System.Runtime.InteropServices;

    [Guid("36537F36-27A4-4788-88C0-733819575017"), InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
    public interface IAppxManifestTargetDeviceFamiliesEnumerator : IDisposable
    {
        IAppxManifestTargetDeviceFamily GetCurrent();

        bool GetHasCurrent();

        bool MoveNext();
    }
}

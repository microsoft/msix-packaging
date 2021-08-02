// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

namespace Microsoft.Msix.Utils.AppxPackagingInterop
{
    using System;
    using System.Runtime.InteropServices;

    [Guid("11D22258-F470-42C1-B291-8361C5437E41"), InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
    public interface IAppxManifestCapabilitiesEnumerator : IDisposable
    {
        [return: MarshalAs(UnmanagedType.LPWStr)]
        string GetCurrent();

        bool GetHasCurrent();

        bool MoveNext();
    }
}

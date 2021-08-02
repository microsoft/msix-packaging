﻿// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

namespace Microsoft.Msix.Utils.AppxPackagingInterop
{
    using System;
    using System.Runtime.InteropServices;

    [Guid("9eb8a55a-f04b-4d0d-808d-686185d4847a"), InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
    public interface IAppxManifestApplicationsEnumerator : IDisposable
    {
        IAppxManifestApplication GetCurrent();

        bool GetHasCurrent();

        bool MoveNext();
    }
}

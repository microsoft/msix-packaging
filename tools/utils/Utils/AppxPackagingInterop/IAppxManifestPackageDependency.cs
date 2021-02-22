// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

namespace Microsoft.Msix.Utils.AppxPackagingInterop
{
    using System;
    using System.Diagnostics.CodeAnalysis;

    using System.Runtime.InteropServices;

    [SuppressMessage("StyleCop.CSharp.DocumentationRules", "*", Justification = "Interop")]
    [Guid("e4946b59-733e-43f0-a724-3bde4c1285a0"), InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
    public interface IAppxManifestPackageDependency : IDisposable
    {
        [return: MarshalAs(UnmanagedType.LPWStr)]
        string GetName();

        [return: MarshalAs(UnmanagedType.LPWStr)]
        string GetPublisher();

        ulong GetMinVersion();
    }
}
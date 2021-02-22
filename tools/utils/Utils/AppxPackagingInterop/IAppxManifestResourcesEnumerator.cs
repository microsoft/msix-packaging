// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

namespace Microsoft.Msix.Utils.AppxPackagingInterop
{
    using System;
    using System.Diagnostics.CodeAnalysis;

    using System.Runtime.InteropServices;

    [SuppressMessage("StyleCop.CSharp.DocumentationRules", "*", Justification = "Interop")]
    [Guid("de4dfbbd-881a-48bb-858c-d6f2baeae6ed"), InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
    public interface IAppxManifestResourcesEnumerator : IDisposable
    {
        [return: MarshalAs(UnmanagedType.LPWStr)]
        string GetCurrent();

        bool GetHasCurrent();

        bool MoveNext();
    }
}

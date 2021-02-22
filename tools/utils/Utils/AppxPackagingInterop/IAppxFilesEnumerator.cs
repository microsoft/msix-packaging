// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

namespace Microsoft.Msix.Utils.AppxPackagingInterop
{
    using System;
    using System.Diagnostics.CodeAnalysis;

    using System.Runtime.InteropServices;

    [SuppressMessage("StyleCop.CSharp.DocumentationRules", "*", Justification = "Interop")]
    [Guid("f007eeaf-9831-411c-9847-917cdc62d1fe"), InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
    public interface IAppxFilesEnumerator : IDisposable
    {
        IAppxFile GetCurrent();

        bool GetHasCurrent();

        bool MoveNext();
    }
}

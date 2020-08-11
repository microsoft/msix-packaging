//-----------------------------------------------------------------------
// <copyright file="IAppxManifestApplicationsEnumerator.cs" company="Microsoft">
//     Copyright (c) Microsoft. All rights reserved.
// </copyright>
//-----------------------------------------------------------------------

namespace Microsoft.Msix.Utils.AppxPackagingInterop
{
    using System;
    using System.Diagnostics.CodeAnalysis;
    using System.Runtime.InteropServices;

    [SuppressMessage("StyleCop.CSharp.DocumentationRules", "*", Justification = "Interop")]
    [Guid("9eb8a55a-f04b-4d0d-808d-686185d4847a"), InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
    public interface IAppxManifestApplicationsEnumerator : IDisposable
    {
        IAppxManifestApplication GetCurrent();

        bool GetHasCurrent();

        bool MoveNext();
    }
}

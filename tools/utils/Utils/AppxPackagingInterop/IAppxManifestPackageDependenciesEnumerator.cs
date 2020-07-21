//-----------------------------------------------------------------------
// <copyright file="IAppxManifestPackageDependenciesEnumerator.cs" company="Microsoft">
//     Copyright (c) Microsoft. All rights reserved.
// </copyright>
//-----------------------------------------------------------------------

namespace Microsoft.Packaging.Utils.AppxPackagingInterop
{
    using System;
    using System.Diagnostics.CodeAnalysis;

    using System.Runtime.InteropServices;

    [SuppressMessage("StyleCop.CSharp.DocumentationRules", "*", Justification = "Interop")]
    [Guid("b43bbcf9-65a6-42dd-bac0-8c6741e7f5a4"), InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
    public interface IAppxManifestPackageDependenciesEnumerator : IDisposable
    {
        IAppxManifestPackageDependency GetCurrent();

        bool GetHasCurrent();

        bool MoveNext();
    }
}

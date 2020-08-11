//-----------------------------------------------------------------------
// <copyright file="IAppxManifestPackageDependencyInternal.cs" company="Microsoft">
//     Copyright (c) Microsoft. All rights reserved.
// </copyright>
//-----------------------------------------------------------------------

namespace Microsoft.Msix.Utils.AppxPackagingInterop
{
    using System;
    using System.Diagnostics.CodeAnalysis;

    using System.Runtime.InteropServices;

    [SuppressMessage("StyleCop.CSharp.DocumentationRules", "*", Justification = "Interop")]
    [Guid("6A41FF03-BFA5-4866-BFA0-293F220E43B0"), InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
    public interface IAppxManifestPackageDependencyInternal : IDisposable
    {
        // IAppxManifestPackageDependencyInternal functions
        bool GetIsOptional();
    }
}
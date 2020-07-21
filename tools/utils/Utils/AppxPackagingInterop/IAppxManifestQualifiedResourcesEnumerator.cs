//-----------------------------------------------------------------------
// <copyright file="IAppxManifestQualifiedResourcesEnumerator.cs" company="Microsoft">
//     Copyright (c) Microsoft. All rights reserved.
// </copyright>
//-----------------------------------------------------------------------

namespace Microsoft.Packaging.Utils.AppxPackagingInterop
{
    using System;
    using System.Diagnostics.CodeAnalysis;

    using System.Runtime.InteropServices;

    [SuppressMessage("StyleCop.CSharp.DocumentationRules", "*", Justification = "Interop")]
    [Guid("8ef6adfe-3762-4a8f-9373-2fc5d444c8d2"), InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
    public interface IAppxManifestQualifiedResourcesEnumerator : IDisposable
    {
        IAppxManifestQualifiedResource GetCurrent();

        bool GetHasCurrent();

        bool MoveNext();
    }
}

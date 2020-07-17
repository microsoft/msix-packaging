//-----------------------------------------------------------------------
// <copyright file="IAppxBundleManifestOptionalBundleInfoEnumerator.cs" company="Microsoft">
//     Copyright (c) Microsoft. All rights reserved.
// </copyright>
//-----------------------------------------------------------------------

namespace Microsoft.Packaging.SDKUtils.AppxPackagingInterop
{
    using System;
    using System.Diagnostics.CodeAnalysis;

    using System.Runtime.InteropServices;

    [SuppressMessage("StyleCop.CSharp.DocumentationRules", "*", Justification = "Interop")]
    [Guid("9A178793-F97E-46AC-AACA-DD5BA4C177C8"), InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
    public interface IAppxBundleManifestOptionalBundleInfoEnumerator : IDisposable
    {
        IAppxBundleManifestOptionalBundleInfo GetCurrent();

        bool GetHasCurrent();

        bool MoveNext();
    }
}

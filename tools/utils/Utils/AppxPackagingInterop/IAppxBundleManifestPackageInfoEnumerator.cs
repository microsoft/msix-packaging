//-----------------------------------------------------------------------
// <copyright file="IAppxBundleManifestPackageInfoEnumerator.cs" company="Microsoft">
//     Copyright (c) Microsoft. All rights reserved.
// </copyright>
//-----------------------------------------------------------------------

namespace Microsoft.Packaging.SDKUtils.AppxPackagingInterop
{
    using System;
    using System.Diagnostics.CodeAnalysis;

    using System.Runtime.InteropServices;

    [SuppressMessage("StyleCop.CSharp.DocumentationRules", "*", Justification = "Interop")]
    [Guid("F9B856EE-49A6-4E19-B2B0-6A2406D63A32"), InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
    public interface IAppxBundleManifestPackageInfoEnumerator : IDisposable
    {
        IAppxBundleManifestPackageInfo GetCurrent();

        bool GetHasCurrent();

        bool MoveNext();
    }
}

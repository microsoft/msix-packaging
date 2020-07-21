//-----------------------------------------------------------------------
// <copyright file="IAppxManifestTargetDeviceFamiliesEnumerator.cs" company="Microsoft">
//     Copyright (c) Microsoft. All rights reserved.
// </copyright>
//-----------------------------------------------------------------------

namespace Microsoft.Packaging.SDKUtils.AppxPackagingInterop
{
    using System;
    using System.Diagnostics.CodeAnalysis;
    using System.Runtime.InteropServices;

    [SuppressMessage("StyleCop.CSharp.DocumentationRules", "*", Justification = "Interop")]
    [Guid("36537F36-27A4-4788-88C0-733819575017"), InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
    public interface IAppxManifestTargetDeviceFamiliesEnumerator : IDisposable
    {
        IAppxManifestTargetDeviceFamily GetCurrent();

        bool GetHasCurrent();

        bool MoveNext();
    }
}

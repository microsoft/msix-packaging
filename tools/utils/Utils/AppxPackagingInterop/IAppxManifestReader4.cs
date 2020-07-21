//-----------------------------------------------------------------------
// <copyright file="IAppxManifestReader4.cs" company="Microsoft">
//     Copyright (c) Microsoft. All rights reserved.
// </copyright>
//-----------------------------------------------------------------------

namespace Microsoft.Packaging.SDKUtils.AppxPackagingInterop
{
    using System;
    using System.Diagnostics.CodeAnalysis;
    using System.Runtime.InteropServices;
    using System.Runtime.InteropServices.ComTypes;

    // Note: The AppxPackaging.idl definition of this interface implements IAppxManifestReader3.
    // Therefore the functions in IAppxManifestReader, IAppxManifestReader2, and IAppxManifestReader3 should be re-declared here.
    [SuppressMessage("StyleCop.CSharp.DocumentationRules", "*", Justification = "Interop")]
    [Guid("4579BB7C-741D-4161-B5A1-47BD3B78AD9B"), InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
    public interface IAppxManifestReader4 : IDisposable
    {
        // IAppxManifestReader functions
        IAppxManifestPackageId GetPackageId();

        IAppxManifestProperties GetProperties();

        IAppxManifestPackageDependenciesEnumerator GetPackageDependencies();

        APPX_CAPABILITIES GetCapabilities();

        IAppxManifestResourcesEnumerator GetResources();

        IAppxManifestDeviceCapabilitiesEnumerator GetDeviceCapabilities();

        ulong GetPrerequisite([In, MarshalAs(UnmanagedType.LPWStr)] string name);

        IAppxManifestApplicationsEnumerator GetApplications();

        IStream GetStream();

        // IAppxManifestReader2 functions
        IAppxManifestQualifiedResourcesEnumerator GetQualifiedResources();

        // IAppxManifestReader3 functions
        IAppxManifestCapabilitiesEnumerator GetCapabilitiesByCapabilityClass(APPX_CAPABILITY_CLASS_TYPE capabilityClass);

        IAppxManifestTargetDeviceFamiliesEnumerator GetTargetDeviceFamilies();

        // IAppxManifestReader4 functions
        IAppxManifestOptionalPackageInfo GetOptionalPackageInfo();
    }
}

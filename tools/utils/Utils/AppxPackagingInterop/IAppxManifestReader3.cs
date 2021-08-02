// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

namespace Microsoft.Msix.Utils.AppxPackagingInterop
{
    using System;
    using System.Runtime.InteropServices;
    using System.Runtime.InteropServices.ComTypes;

    // Note: The AppxPackaging.idl definition of this interface implements IAppxManifestReader2.
    // Therefore the functions in IAppxManifestReader and IAppxManifestReader2 should be re-declared here.
    [Guid("C43825AB-69B7-400A-9709-CC37F5A72D24"), InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
    public interface IAppxManifestReader3 : IDisposable
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
    }
}

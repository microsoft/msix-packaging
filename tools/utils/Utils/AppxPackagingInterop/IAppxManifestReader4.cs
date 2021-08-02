// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

namespace Microsoft.Msix.Utils.AppxPackagingInterop
{
    using System;
    using System.Runtime.InteropServices;
    using System.Runtime.InteropServices.ComTypes;

    // Note: The AppxPackaging.idl definition of this interface implements IAppxManifestReader3.
    // Therefore the functions in IAppxManifestReader, IAppxManifestReader2, and IAppxManifestReader3 should be re-declared here.
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

// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

namespace Microsoft.Msix.Utils.AppxPackagingInterop
{
    using System;
    using System.Diagnostics.CodeAnalysis;

    using System.Runtime.InteropServices;
    using System.Runtime.InteropServices.ComTypes;

    // Note: The AppxPackaging.idl definition of this interface implements IAppxManifestReader.
    // Therefore the functions in IAppxManifestReader should be re-declared here.
    [SuppressMessage("StyleCop.CSharp.DocumentationRules", "*", Justification = "Interop")]
    [Guid("d06f67bc-b31d-4eba-a8af-638e73e77b4d"), InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
    public interface IAppxManifestReader2 : IDisposable
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
    }
}

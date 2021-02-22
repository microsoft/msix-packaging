// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

namespace Microsoft.Msix.Utils.AppxPackagingInterop
{
    using System;
    using System.Runtime.InteropServices;
    using System.Runtime.InteropServices.ComTypes;

    // Empty declarations of unused types for building project
    public enum APPX_CAPABILITIES
    {
    }

    public enum APPX_COMPRESSION_OPTION
    {
        APPX_COMPRESSION_OPTION_NONE = 0,
        APPX_COMPRESSION_OPTION_NORMAL = 1,
        APPX_COMPRESSION_OPTION_MAXIMUM = 2,
        APPX_COMPRESSION_OPTION_FAST = 3,
        APPX_COMPRESSION_OPTION_SUPERFAST = 4
    }

    [Flags]
    public enum STGM
    {
        STGM_READ = 0x0,
        STGM_WRITE = 0x1,
        STGM_READWRITE = 0x2
    }

    public enum APPX_BUNDLE_PAYLOAD_PACKAGE_TYPE
    {
        APPX_BUNDLE_PAYLOAD_PACKAGE_TYPE_APPLICATION = 0,
        APPX_BUNDLE_PAYLOAD_PACKAGE_TYPE_RESOURCE = 1
    }

    public enum APPX_PACKAGE_ARCHITECTURE
    {
        APPX_PACKAGE_ARCHITECTURE_X86 = 0,
        APPX_PACKAGE_ARCHITECTURE_ARM = 5,
        APPX_PACKAGE_ARCHITECTURE_X64 = 9,
        APPX_PACKAGE_ARCHITECTURE_NEUTRAL = 11,
        APPX_PACKAGE_ARCHITECTURE_ARM64 = 12,
        APPX_PACKAGE_ARCHITECTURE_X86A64 = 14
    }

    // Note from idl: Must match APPX_PACKAGE_ARCHITECTURE
    public enum APPX_PACKAGE_ARCHITECTURE2
    {
        APPX_PACKAGE_ARCHITECTURE2_X86 = 0,
        APPX_PACKAGE_ARCHITECTURE2_ARM = 5,
        APPX_PACKAGE_ARCHITECTURE2_X64 = 9,
        APPX_PACKAGE_ARCHITECTURE2_NEUTRAL = 11,
        APPX_PACKAGE_ARCHITECTURE2_ARM64 = 12,
        APPX_PACKAGE_ARCHITECTURE2_X86_ON_ARM64 = 14,
        APPX_PACKAGE_ARCHITECTURE2_UNKNOWN = 0xFFFF,
    }

    public enum APPX_BUNDLE_FOOTPRINT_FILE_TYPE
    {
        APPX_BUNDLE_FOOTPRINT_FILE_TYPE_FIRST = 0,
        APPX_BUNDLE_FOOTPRINT_FILE_TYPE_MANIFEST = 0,
        APPX_BUNDLE_FOOTPRINT_FILE_TYPE_BLOCKMAP = 1,
        APPX_BUNDLE_FOOTPRINT_FILE_TYPE_SIGNATURE = 2,
        APPX_BUNDLE_FOOTPRINT_FILE_TYPE_LAST = 2
    }

    public enum DX_FEATURE_LEVEL
    {
        DX_FEATURE_LEVEL_UNSPECIFIED = 0,
        DX_FEATURE_LEVEL_9 = 1,
        DX_FEATURE_LEVEL_10 = 2,
        DX_FEATURE_LEVEL_11 = 3
    }

    public enum APPX_FOOTPRINT_FILE_TYPE
    {
        APPX_FOOTPRINT_FILE_TYPE_MANIFEST = 0,
        APPX_FOOTPRINT_FILE_TYPE_BLOCKMAP = 1,
        APPX_FOOTPRINT_FILE_TYPE_SIGNATURE = 2,
        APPX_FOOTPRINT_FILE_TYPE_CODEINTEGRITY = 3,
        APPX_FOOTPRINT_FILE_TYPE_CONTENTGROUPMAP = 4
    }

    public enum APPX_CAPABILITY_CLASS_TYPE
    {
        APPX_CAPABILITY_CLASS_DEFAULT = 0x00000000,
        APPX_CAPABILITY_CLASS_GENERAL = 0x00000001,
        APPX_CAPABILITY_CLASS_RESTRICTED = 0x00000002,
        APPX_CAPABILITY_CLASS_WINDOWS = 0x00000004,
        APPX_CAPABILITY_CLASS_ALL = 0x00000007,
        APPX_CAPABILITY_CLASS_CUSTOM = 0x00000008
    }

    [Guid("beb94909-e451-438b-b5a7-d79e767b75d8"), InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
    public interface IAppxFactory : IDisposable
    {
        IAppxPackageWriter CreatePackageWriter([In] IStream outputStream, [In] ref APPX_PACKAGE_SETTINGS settings);

        IAppxPackageReader CreatePackageReader([In] IStream inputStream);

        IAppxManifestReader CreateManifestReader([In] IStream inputStream);

        IAppxBlockMapReader CreateBlockMapReader([In] IStream inputStream);

        IAppxBlockMapReader CreateValidatedBlockMapReader([In] IStream blockMapStream, [In] string signatureFileName);
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct APPX_PACKAGE_SETTINGS
    {
        [MarshalAs(UnmanagedType.Bool)]
        public bool ForceZip32;
        [MarshalAs(UnmanagedType.Interface)]
        public IUri HashMethod;
    }
}

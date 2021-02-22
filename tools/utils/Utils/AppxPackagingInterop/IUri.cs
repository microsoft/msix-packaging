// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

namespace Microsoft.Msix.Utils.AppxPackagingInterop
{
    using System;
    using System.Diagnostics.CodeAnalysis;
    using System.Runtime.InteropServices;

    [SuppressMessage("StyleCop.CSharp.DocumentationRules", "*", Justification = "Interop")]
    public enum UriProperty
    {
        ABSOLUTE_URI = 0,
        STRING_START = ABSOLUTE_URI,
        AUTHORITY = 1,
        DISPLAY_URI = 2,
        DOMAIN = 3,
        EXTENSION = 4,
        FRAGMENT = 5,
        HOST = 6,
        PASSWORD = 7,
        PATH = 8,
        PATH_AND_QUERY = 9,
        QUERY = 10,
        RAW_URI = 11,
        SCHEME_NAME = 12,
        USER_INFO = 13,
        USER_NAME = 14,
        STRING_LAST = USER_NAME,
        HOST_TYPE = 15,
        DWORD_START = HOST_TYPE,
        PORT = 16,
        SCHEME = 17,
        ZONE = 18,
        DWORD_LAST = ZONE
    }

    [SuppressMessage("StyleCop.CSharp.DocumentationRules", "*", Justification = "Interop")]
    [ComImport, Guid("A39EE748-6A27-4817-A6F2-13914BEF5890"), InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
    public interface IUri
    {
        void GetPropertyBSTR([In] UriProperty uriProp, [Out] out string property, [In] uint flags);

        void GetPropertyLength([In] UriProperty uriProp, [Out] out uint propLen, [In] uint flags);

        void GetPropertyDWORD([In] UriProperty uriProp, [Out] out uint propValue, [In] uint flags);

        void HasProperty([In] UriProperty uriProp, [Out] out bool hasProperty);

        void GetAbsoluteUri([MarshalAs(UnmanagedType.BStr), Out] out string absoluteUri);

        void GetAuthority([MarshalAs(UnmanagedType.BStr), Out] out string authority);

        void GetDisplayUri([MarshalAs(UnmanagedType.BStr), Out] out string displayString);

        void GetDomain([MarshalAs(UnmanagedType.BStr), Out] out string domain);

        void GetExtension([MarshalAs(UnmanagedType.BStr), Out] out string extension);

        void GetFragment([MarshalAs(UnmanagedType.BStr), Out] out string fragment);

        void GetHost([MarshalAs(UnmanagedType.BStr), Out] out string host);

        void GetPassword([MarshalAs(UnmanagedType.BStr), Out] out string password);

        void GetPath([MarshalAs(UnmanagedType.BStr), Out] out string path);

        void GetPathAndQuery([MarshalAs(UnmanagedType.BStr), Out] out string pathAndQuery);

        void GetQuery([MarshalAs(UnmanagedType.BStr), Out] out string query);

        void GetRawUri([MarshalAs(UnmanagedType.BStr), Out] out string rawUri);

        void GetSchemeName([MarshalAs(UnmanagedType.BStr), Out] out string schemeName);

        void GetUserInfo([MarshalAs(UnmanagedType.BStr), Out] out string userInfo);

        void GetUserName([MarshalAs(UnmanagedType.BStr), Out] out string userName);

        void GetHostType([Out] uint hostType);

        void GetPort([Out] uint port);

        void GetScheme([Out] uint scheme);

        void GetZone([Out] uint zone);

        void GetProperties([Out] uint flags);

        void IsEqual([In] IUri uri, [Out] bool equal);
    }
}

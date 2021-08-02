﻿// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

namespace Microsoft.Msix.Utils.AppxPackagingInterop
{
    using System;

    using System.Runtime.InteropServices;
    using System.Runtime.InteropServices.ComTypes;

    // Note: The IDL definition of this interface implements IAppxBlockMapFile.
    // Therefore the functions in IAppxBlockMapFile should be re-declared here.
    [Guid("04D9AAB2-7957-41D3-8CFB-594525F37966"), InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
    public interface IAppxEncryptedBlockMapFile : IDisposable
    {
        // IAppxBlockMapFile
        IAppxBlockMapBlocksEnumerator GetBlocks();

        uint GetLocalFileHeaderSize();

        [return: MarshalAs(UnmanagedType.LPWStr)]
        string GetName();

        ulong GetUncompressedSize();

        bool ValidateFileHash([In] IStream fileStream);

        // IAppxEncryptedBlockMapFile
        [return: MarshalAs(UnmanagedType.LPArray, ArraySubType = UnmanagedType.U8, SizeParamIndex = 0)]
        byte[] GetFileHash(out uint fileHashSize);

        ulong GetId();

        ulong GetEncryptedSize();

        bool GetIsEncrypted();
    }
}

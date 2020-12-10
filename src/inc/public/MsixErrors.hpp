//
//  Copyright (C) 2017 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
// 
#pragma once
#ifndef MSIX_MSIX_ERRORS__H
#define MSIX_MSIX_ERRORS__H

namespace MSIX {

    static const std::uint32_t ERROR_FACILITY = 0x8BAD0000;              // Facility 2989
    static const std::uint32_t XML_FACILITY   = ERROR_FACILITY + 0x1000; // XML exceptions: 0x8BAD1000 + XMLException error code

    // defines error codes
    enum class Error : std::uint32_t
    {
        //
        // Win32 error codes
        //
        OK                          = 0x00000000,
        NotImplemented              = 0x80004001,
        NoInterface                 = 0x80004002,
        Unexpected                  = 0x8000ffff,
        FileNotFound                = 0x80070002,
        OutOfMemory                 = 0x8007000E,
        NotSupported                = 0x80070032,
        InvalidParameter            = 0x80070057,
        Stg_E_Invalidpointer        = 0x80030009,
        InvalidState                = 0x804d0003,
        BadFormat                   = 0x8007000b,
        InvalidData                 = 0x8007000d,
        OutOfBounds                 = 0x8000000b,
        PackagingErrorInternal      = 0x80080200,

        //
        // msix specific error codes
        //

        // Basic file errors
        FileOpen                    = ERROR_FACILITY + 0x0001,
        FileSeek                    = ERROR_FACILITY + 0x0002,
        FileRead                    = ERROR_FACILITY + 0x0003,
        FileWrite                   = ERROR_FACILITY + 0x0003,
        FileCreateDirectory         = ERROR_FACILITY + 0x0004,
        FileSeekOutOfRange          = ERROR_FACILITY + 0x0005,

        // Zip format errors
        ZipCentralDirectoryHeader   = ERROR_FACILITY + 0x0011,
        ZipLocalFileHeader          = ERROR_FACILITY + 0x0012,
        Zip64EOCDRecord             = ERROR_FACILITY + 0x0013,
        Zip64EOCDLocator            = ERROR_FACILITY + 0x0014,
        ZipEOCDRecord               = ERROR_FACILITY + 0x0015,
        ZipHiddenData               = ERROR_FACILITY + 0x0016,
        ZipBadExtendedData          = ERROR_FACILITY + 0x0017,

        // Inflate errors
        InflateInitialize           = ERROR_FACILITY + 0x0021,
        InflateRead                 = ERROR_FACILITY + 0x0022,
        InflateCorruptData          = ERROR_FACILITY + 0x0023,

        // Package format errors
        MissingAppxSignatureP7X     = ERROR_FACILITY + 0x0031,
        MissingContentTypesXML      = ERROR_FACILITY + 0x0032,
        MissingAppxBlockMapXML      = ERROR_FACILITY + 0x0033,
        MissingAppxManifestXML      = ERROR_FACILITY + 0x0034,
        DuplicateFootprintFile      = ERROR_FACILITY + 0x0035,
        UnknownFileNameEncoding     = ERROR_FACILITY + 0x0036,
        DuplicateFile               = ERROR_FACILITY + 0x0037,

        // Signature errors
        SignatureInvalid            = ERROR_FACILITY + 0x0041,
        CertNotTrusted              = ERROR_FACILITY + 0x0042,
        PublisherMismatch           = ERROR_FACILITY + 0x0043,

        // Blockmap semantic errors
        BlockMapSemanticError       = ERROR_FACILITY + 0x0051,
        BlockMapInvalidData         = ERROR_FACILITY + 0x0052,

        // AppxManifest semantic errors
        AppxManifestSemanticError   = ERROR_FACILITY + 0x0061,

        // Bundle errors
        PackageIsBundle             = ERROR_FACILITY + 0x0071,

        // Deflate errors
        DeflateInitialize           = ERROR_FACILITY + 0x0081,
        DeflateWrite                = ERROR_FACILITY + 0x0082,
        DeflateRead                 = ERROR_FACILITY + 0x0083,

        // XML parsing errors
        XmlWarning                  = XML_FACILITY + 0x0001,
        XmlError                    = XML_FACILITY + 0x0002,
        XmlFatal                    = XML_FACILITY + 0x0003,
        XmlInvalidData              = XML_FACILITY + 0x0004,
    };
}

#endif

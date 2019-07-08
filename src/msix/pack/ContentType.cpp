//
//  Copyright (C) 2019 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
// 
#include "AppxPackaging.hpp"
#include "ContentType.hpp"
#include "Exceptions.hpp"

namespace MSIX {

    // Well-known file types to automatically select a MIME content type and compression option to use based on the file extension
    // If the extension is not in this map the default is application/octet-stream and APPX_COMPRESSION_OPTION_NORMAL
    const ContentType& ContentType::GetContentTypeByExtension(std::string& ext)
    {
        static const std::map<std::string, ContentType> extToContentType = 
        {
            { "atom",  ContentType("application/atom+xml", APPX_COMPRESSION_OPTION_NORMAL) },
            { "appx",  ContentType("application/vnd.ms-appx", APPX_COMPRESSION_OPTION_NONE) },
            { "b64",   ContentType("application/base64", APPX_COMPRESSION_OPTION_NORMAL) },
            { "cab",   ContentType("application/vnd.ms-cab-compressed", APPX_COMPRESSION_OPTION_NONE) },
            { "doc",   ContentType("application/msword", APPX_COMPRESSION_OPTION_NORMAL) },
            { "dot",   ContentType("application/msword", APPX_COMPRESSION_OPTION_NORMAL) },
            { "docm",  ContentType("application/vnd.ms-word.document.macroenabled.12", APPX_COMPRESSION_OPTION_NONE) },
            { "docx",  ContentType("application/vnd.openxmlformats-officedocument.wordprocessingml.document", APPX_COMPRESSION_OPTION_NONE) },
            { "dotm",  ContentType("application/vnd.ms-word.document.macroenabled.12", APPX_COMPRESSION_OPTION_NONE) },
            { "dotx",  ContentType("application/vnd.openxmlformats-officedocument.wordprocessingml.document", APPX_COMPRESSION_OPTION_NONE) },
            { "dll",   ContentType("application/x-msdownload", APPX_COMPRESSION_OPTION_NORMAL) },
            { "dtd",   ContentType("application/xml-dtd", APPX_COMPRESSION_OPTION_NORMAL) },
            { "exe",   ContentType("application/x-msdownload", APPX_COMPRESSION_OPTION_NORMAL) },
            { "gz",    ContentType("application/x-gzip-compressed", APPX_COMPRESSION_OPTION_NONE) },
            { "java",  ContentType("application/java", APPX_COMPRESSION_OPTION_NORMAL) },
            { "json",  ContentType("application/json", APPX_COMPRESSION_OPTION_NORMAL) },
            { "p7s",   ContentType("application/x-pkcs7-signature", APPX_COMPRESSION_OPTION_NORMAL) },
            { "pdf",   ContentType("application/pdf", APPX_COMPRESSION_OPTION_NORMAL) },
            { "ps",    ContentType("application/postscript", APPX_COMPRESSION_OPTION_NORMAL) },
            { "potm",  ContentType("application/vnd.ms-powerpoint.template.macroenabled.12", APPX_COMPRESSION_OPTION_NONE) },
            { "potx",  ContentType("application/vnd.openxmlformats-officedocument.presentationml.template", APPX_COMPRESSION_OPTION_NONE) },
            { "ppam",  ContentType("application/vnd.ms-powerpoint.addin.macroenabled.12", APPX_COMPRESSION_OPTION_NONE) },
            { "ppsm",  ContentType("application/vnd.ms-powerpoint.slideshow.macroenabled.12", APPX_COMPRESSION_OPTION_NONE) },
            { "ppsx",  ContentType("application/vnd.openxmlformats-officedocument.presentationml.slideshow", APPX_COMPRESSION_OPTION_NONE) },
            { "ppt",   ContentType("application/vnd.ms-powerpoint", APPX_COMPRESSION_OPTION_NORMAL) },
            { "pot",   ContentType("application/vnd.ms-powerpoint", APPX_COMPRESSION_OPTION_NORMAL) },
            { "pps",   ContentType("application/vnd.ms-powerpoint", APPX_COMPRESSION_OPTION_NORMAL) },
            { "ppa",   ContentType("application/vnd.ms-powerpoint", APPX_COMPRESSION_OPTION_NORMAL) },
            { "pptm",  ContentType("application/vnd.ms-powerpoint.presentation.macroenabled.12", APPX_COMPRESSION_OPTION_NONE) },
            { "pptx",  ContentType("application/vnd.openxmlformats-officedocument.presentationml.presentation", APPX_COMPRESSION_OPTION_NONE) },
            { "rar",   ContentType("application/x-rar-compressed", APPX_COMPRESSION_OPTION_NONE) },
            { "rss",   ContentType("application/rss+xml", APPX_COMPRESSION_OPTION_NORMAL) },
            { "soap",  ContentType("application/soap+xml", APPX_COMPRESSION_OPTION_NORMAL) },
            { "tar",   ContentType("application/x-tar", APPX_COMPRESSION_OPTION_NONE) },
            { "xaml",  ContentType("application/xaml+xml", APPX_COMPRESSION_OPTION_NORMAL) },
            { "xap",   ContentType("application/x-silverlight-app", APPX_COMPRESSION_OPTION_NONE) },
            { "xbap",  ContentType("application/x-ms-xbap", APPX_COMPRESSION_OPTION_NORMAL) },
            { "xhtml", ContentType("application/xhtml+xml", APPX_COMPRESSION_OPTION_NORMAL) },
            { "xlam",  ContentType("application/vnd.ms-excel.addin.macroenabled.12", APPX_COMPRESSION_OPTION_NONE) },
            { "xls",   ContentType("application/vnd.ms-excel", APPX_COMPRESSION_OPTION_NORMAL) },
            { "xlt",   ContentType("application/vnd.ms-excel", APPX_COMPRESSION_OPTION_NORMAL) },
            { "xla",   ContentType("application/vnd.ms-excel", APPX_COMPRESSION_OPTION_NORMAL) },
            { "xlsb",  ContentType("application/vnd.ms-excel.sheet.binary.macroEnabled.12", APPX_COMPRESSION_OPTION_NONE) },
            { "xlsm",  ContentType("application/vnd.ms-excel.sheet.macroEnabled.12", APPX_COMPRESSION_OPTION_NONE) },
            { "xlsx",  ContentType("application/vnd.openxmlformats-officedocument.spreadsheetml.sheet", APPX_COMPRESSION_OPTION_NONE) },
            { "xltm",  ContentType("application/vnd.ms-excel.template.macroEnabled.12", APPX_COMPRESSION_OPTION_NONE) },
            { "xltx",  ContentType("application/vnd.openxmlformats-officedocument.spreadsheetml.template", APPX_COMPRESSION_OPTION_NONE) },
            { "xsl",   ContentType("application/xslt+xml", APPX_COMPRESSION_OPTION_NORMAL) },
            { "xslt",  ContentType("application/xslt+xml", APPX_COMPRESSION_OPTION_NORMAL) },
            { "zip",   ContentType("application/x-zip-compressed", APPX_COMPRESSION_OPTION_NONE) },
            // Text types
            { "c",     ContentType("text/plain", APPX_COMPRESSION_OPTION_NORMAL) },
            { "cpp",   ContentType("text/plain", APPX_COMPRESSION_OPTION_NORMAL) },
            { "cs",    ContentType("text/plain", APPX_COMPRESSION_OPTION_NORMAL) },
            { "css",   ContentType("text/css", APPX_COMPRESSION_OPTION_NORMAL) },
            { "csv",   ContentType("text/csv", APPX_COMPRESSION_OPTION_NORMAL) },
            { "h",     ContentType("text/plain", APPX_COMPRESSION_OPTION_NORMAL) },
            { "htm",   ContentType("text/html", APPX_COMPRESSION_OPTION_NORMAL) },
            { "html",  ContentType("text/html", APPX_COMPRESSION_OPTION_NORMAL) },
            { "js",    ContentType("application/x-javascript", APPX_COMPRESSION_OPTION_NORMAL) },
            { "rtf",   ContentType("text/richtext", APPX_COMPRESSION_OPTION_NORMAL) },
            { "sct",   ContentType("text/scriptlet", APPX_COMPRESSION_OPTION_NORMAL) },
            { "txt",   ContentType("text/plain", APPX_COMPRESSION_OPTION_NORMAL) },
            { "xml",   ContentType("text/xml", APPX_COMPRESSION_OPTION_NORMAL) },
            { "xsd",   ContentType("text/xml", APPX_COMPRESSION_OPTION_NORMAL) },
            // Audio types
            { "aiff",  ContentType("audio/x-aiff", APPX_COMPRESSION_OPTION_NORMAL) },
            { "au",    ContentType("audio/basic", APPX_COMPRESSION_OPTION_NORMAL) },
            { "m4a",   ContentType("audio/mp4", APPX_COMPRESSION_OPTION_NONE) },
            { "mid",   ContentType("audio/mid", APPX_COMPRESSION_OPTION_NORMAL) },
            { "mp3",   ContentType("audio/mpeg", APPX_COMPRESSION_OPTION_NONE) },
            { "smf",   ContentType("audio/mid", APPX_COMPRESSION_OPTION_NORMAL) },
            { "wav",   ContentType("audio/wav", APPX_COMPRESSION_OPTION_NORMAL) },
            { "wma",   ContentType("audio/x-ms-wma", APPX_COMPRESSION_OPTION_NONE) },
            // Image types
            { "bmp",   ContentType("image/bmp", APPX_COMPRESSION_OPTION_NORMAL) },
            { "emf",   ContentType("image/x-emf", APPX_COMPRESSION_OPTION_NORMAL) },
            { "gif",   ContentType("image/gif", APPX_COMPRESSION_OPTION_NONE) },
            { "ico",   ContentType("image/vnd.microsoft.icon", APPX_COMPRESSION_OPTION_NORMAL) },
            { "jpg",   ContentType("image/jpeg", APPX_COMPRESSION_OPTION_NONE) },
            { "jpeg",  ContentType("image/jpeg", APPX_COMPRESSION_OPTION_NONE) },
            { "png",   ContentType("image/png", APPX_COMPRESSION_OPTION_NONE) },
            { "svg",   ContentType("image/svg+xml", APPX_COMPRESSION_OPTION_NORMAL) },
            { "tif",   ContentType("image/tiff", APPX_COMPRESSION_OPTION_NORMAL) },
            { "tiff",  ContentType("image/tiff", APPX_COMPRESSION_OPTION_NORMAL) },
            { "wmf",   ContentType("image/x-wmf", APPX_COMPRESSION_OPTION_NORMAL) },
            // Video types
            { "avi",   ContentType("video/avi", APPX_COMPRESSION_OPTION_NONE) },
            { "mpeg",  ContentType("video/mpeg", APPX_COMPRESSION_OPTION_NONE) },
            { "mpg",   ContentType("video/mpeg", APPX_COMPRESSION_OPTION_NONE) },
            { "mov",   ContentType("video/quicktime", APPX_COMPRESSION_OPTION_NONE) },
            { "wmv",   ContentType("video/x-ms-wmv", APPX_COMPRESSION_OPTION_NONE) }
        };
        // if the extension is not in the map these are the defaults
        static const ContentType defaultContentType = ContentType("application/octet-stream", APPX_COMPRESSION_OPTION_NORMAL);

        auto findExt = extToContentType.find(ext);
        if (findExt == extToContentType.end())
        {
            return defaultContentType;
        }
        return findExt->second;
    }

    const std::string ContentType::GetPayloadFileContentType(APPX_FOOTPRINT_FILE_TYPE footprintFile)
    {
        switch (footprintFile)
        {
        case APPX_FOOTPRINT_FILE_TYPE_MANIFEST:
            return "application/vnd.ms-appx.manifest+xml";
        case APPX_FOOTPRINT_FILE_TYPE_BLOCKMAP:
            return "application/vnd.ms-appx.blockmap+xml";
        case APPX_FOOTPRINT_FILE_TYPE_SIGNATURE:
            return "application/vnd.ms-appx.signature";
        case APPX_FOOTPRINT_FILE_TYPE_CODEINTEGRITY:
            return "application/vnd.ms-pkiseccat";
        }

        // TODO: add other ones if needed, otherwise throw
        ThrowErrorAndLog(Error::NotSupported, "Payload file content type not found");
    }

    const std::string ContentType::GetBundlePayloadFileContentType(APPX_BUNDLE_FOOTPRINT_FILE_TYPE footprintFile)
    {
        if (footprintFile == APPX_BUNDLE_FOOTPRINT_FILE_TYPE_MANIFEST)
        {
            return "application/vnd.ms-appx.bundlemanifest+xml";
        }
        if (footprintFile == APPX_BUNDLE_FOOTPRINT_FILE_TYPE_BLOCKMAP)
        {
            return "application/vnd.ms-appx.blockmap+xml";
        }
        if (footprintFile == APPX_BUNDLE_FOOTPRINT_FILE_TYPE_SIGNATURE)
        {
            return "application/vnd.ms-appx.signature";
        }
        // TODO: add other ones if needed, otherwise throw
        ThrowErrorAndLog(Error::NotSupported, "Bundle Payload file content type not found");
    }
}

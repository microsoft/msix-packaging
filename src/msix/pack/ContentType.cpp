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
    const ContentTypeData ContentType::GetExtensionContentTypeData(std::string& ext)
    {
        static const std::map<std::string, ContentTypeData> extToContentType = 
        {
            { "atom",  { "application/atom+xml", APPX_COMPRESSION_OPTION_NORMAL} },
            { "appx",  { "application/vnd.ms-appx", APPX_COMPRESSION_OPTION_NONE} },
            { "b64",   { "application/base64", APPX_COMPRESSION_OPTION_NORMAL} },
            { "cab",   { "application/vnd.ms-cab-compressed", APPX_COMPRESSION_OPTION_NONE} },
            { "doc",   { "application/msword", APPX_COMPRESSION_OPTION_NORMAL} },
            { "dot",   { "application/msword", APPX_COMPRESSION_OPTION_NORMAL} },
            { "docm",  { "application/vnd.ms-word.document.macroenabled.12", APPX_COMPRESSION_OPTION_NONE} },
            { "docx",  { "application/vnd.openxmlformats-officedocument.wordprocessingml.document", APPX_COMPRESSION_OPTION_NONE} },
            { "dotm",  { "application/vnd.ms-word.document.macroenabled.12", APPX_COMPRESSION_OPTION_NONE} },
            { "dotx",  { "application/vnd.openxmlformats-officedocument.wordprocessingml.document", APPX_COMPRESSION_OPTION_NONE} },
            { "dll",   { "application/x-msdownload", APPX_COMPRESSION_OPTION_NORMAL} },
            { "dtd",   { "application/xml-dtd", APPX_COMPRESSION_OPTION_NORMAL} },
            { "exe",   { "application/x-msdownload", APPX_COMPRESSION_OPTION_NORMAL} },
            { "gz",    { "application/x-gzip-compressed", APPX_COMPRESSION_OPTION_NONE} },
            { "java",  { "application/java", APPX_COMPRESSION_OPTION_NORMAL} },
            { "json",  { "application/json", APPX_COMPRESSION_OPTION_NORMAL} },
            { "p7s",   { "application/x-pkcs7-signature", APPX_COMPRESSION_OPTION_NORMAL} },
            { "pdf",   { "application/pdf", APPX_COMPRESSION_OPTION_NORMAL} },
            { "ps",    { "application/postscript", APPX_COMPRESSION_OPTION_NORMAL} },
            { "potm",  { "application/vnd.ms-powerpoint.template.macroenabled.12", APPX_COMPRESSION_OPTION_NONE} },
            { "potx",  { "application/vnd.openxmlformats-officedocument.presentationml.template", APPX_COMPRESSION_OPTION_NONE} },
            { "ppam",  { "application/vnd.ms-powerpoint.addin.macroenabled.12", APPX_COMPRESSION_OPTION_NONE} },
            { "ppsm",  { "application/vnd.ms-powerpoint.slideshow.macroenabled.12", APPX_COMPRESSION_OPTION_NONE} },
            { "ppsx",  { "application/vnd.openxmlformats-officedocument.presentationml.slideshow", APPX_COMPRESSION_OPTION_NONE} },
            { "ppt",   { "application/vnd.ms-powerpoint", APPX_COMPRESSION_OPTION_NORMAL} },
            { "pot",   { "application/vnd.ms-powerpoint", APPX_COMPRESSION_OPTION_NORMAL} },
            { "pps",   { "application/vnd.ms-powerpoint", APPX_COMPRESSION_OPTION_NORMAL} },
            { "ppa",   { "application/vnd.ms-powerpoint", APPX_COMPRESSION_OPTION_NORMAL} },
            { "pptm",  { "application/vnd.ms-powerpoint.presentation.macroenabled.12", APPX_COMPRESSION_OPTION_NONE} },
            { "pptx",  { "application/vnd.openxmlformats-officedocument.presentationml.presentation", APPX_COMPRESSION_OPTION_NONE} },
            { "rar",   { "application/x-rar-compressed", APPX_COMPRESSION_OPTION_NONE} },
            { "rss",   { "application/rss+xml", APPX_COMPRESSION_OPTION_NORMAL} },
            { "soap",  { "application/soap+xml", APPX_COMPRESSION_OPTION_NORMAL} },
            { "tar",   { "application/x-tar", APPX_COMPRESSION_OPTION_NONE} },
            { "xaml",  { "application/xaml+xml", APPX_COMPRESSION_OPTION_NORMAL} },
            { "xap",   { "application/x-silverlight-app", APPX_COMPRESSION_OPTION_NONE} },
            { "xbap",  { "application/x-ms-xbap", APPX_COMPRESSION_OPTION_NORMAL} },
            { "xhtml", { "application/xhtml+xml", APPX_COMPRESSION_OPTION_NORMAL} },
            { "xlam",  { "application/vnd.ms-excel.addin.macroenabled.12", APPX_COMPRESSION_OPTION_NONE} },
            { "xls",   { "application/vnd.ms-excel", APPX_COMPRESSION_OPTION_NORMAL} },
            { "xlt",   { "application/vnd.ms-excel", APPX_COMPRESSION_OPTION_NORMAL} },
            { "xla",   { "application/vnd.ms-excel", APPX_COMPRESSION_OPTION_NORMAL} },
            { "xlsb",  { "application/vnd.ms-excel.sheet.binary.macroEnabled.12", APPX_COMPRESSION_OPTION_NONE} },
            { "xlsm",  { "application/vnd.ms-excel.sheet.macroEnabled.12", APPX_COMPRESSION_OPTION_NONE} },
            { "xlsx",  { "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet", APPX_COMPRESSION_OPTION_NONE} },
            { "xltm",  { "application/vnd.ms-excel.template.macroEnabled.12", APPX_COMPRESSION_OPTION_NONE} },
            { "xltx",  { "application/vnd.openxmlformats-officedocument.spreadsheetml.template", APPX_COMPRESSION_OPTION_NONE} },
            { "xsl",   { "application/xslt+xml", APPX_COMPRESSION_OPTION_NORMAL} },
            { "xslt",  { "application/xslt+xml", APPX_COMPRESSION_OPTION_NORMAL} },
            { "zip",   { "application/x-zip-compressed", APPX_COMPRESSION_OPTION_NONE} },
            // Text types
            { "c",     { "text/plain", APPX_COMPRESSION_OPTION_NORMAL} },
            { "cpp",   { "text/plain", APPX_COMPRESSION_OPTION_NORMAL} },
            { "cs",    { "text/plain", APPX_COMPRESSION_OPTION_NORMAL} },
            { "css",   { "text/css", APPX_COMPRESSION_OPTION_NORMAL} },
            { "csv",   { "text/csv", APPX_COMPRESSION_OPTION_NORMAL} },
            { "h",     { "text/plain", APPX_COMPRESSION_OPTION_NORMAL} },
            { "htm",   { "text/html", APPX_COMPRESSION_OPTION_NORMAL} },
            { "html",  { "text/html", APPX_COMPRESSION_OPTION_NORMAL} },
            { "js",    { "application/x-javascript", APPX_COMPRESSION_OPTION_NORMAL} },
            { "rtf",   { "text/richtext", APPX_COMPRESSION_OPTION_NORMAL} },
            { "sct",   { "text/scriptlet", APPX_COMPRESSION_OPTION_NORMAL} },
            { "txt",   { "text/plain", APPX_COMPRESSION_OPTION_NORMAL} },
            { "xml",   { "text/xml", APPX_COMPRESSION_OPTION_NORMAL} },
            { "xsd",   { "text/xml", APPX_COMPRESSION_OPTION_NORMAL} },
            // Audio types
            { "aiff",  { "audio/x-aiff", APPX_COMPRESSION_OPTION_NORMAL} },
            { "au",    { "audio/basic", APPX_COMPRESSION_OPTION_NORMAL} },
            { "m4a",   { "audio/mp4", APPX_COMPRESSION_OPTION_NONE} },
            { "mid",   { "audio/mid", APPX_COMPRESSION_OPTION_NORMAL} },
            { "mp3",   { "audio/mpeg", APPX_COMPRESSION_OPTION_NONE} },
            { "smf",   { "audio/mid", APPX_COMPRESSION_OPTION_NORMAL} },
            { "wav",   { "audio/wav", APPX_COMPRESSION_OPTION_NORMAL} },
            { "wma",   { "audio/x-ms-wma", APPX_COMPRESSION_OPTION_NONE} },
            // Image types
            { "bmp",   { "image/bmp", APPX_COMPRESSION_OPTION_NORMAL} },
            { "emf",   { "image/x-emf", APPX_COMPRESSION_OPTION_NORMAL} },
            { "gif",   { "image/gif", APPX_COMPRESSION_OPTION_NONE} },
            { "ico",   { "image/vnd.microsoft.icon", APPX_COMPRESSION_OPTION_NORMAL} },
            { "jpg",   { "image/jpeg", APPX_COMPRESSION_OPTION_NONE} },
            { "jpeg",  { "image/jpeg", APPX_COMPRESSION_OPTION_NONE} },
            { "png",   { "image/png", APPX_COMPRESSION_OPTION_NONE} },
            { "svg",   { "image/svg+xml", APPX_COMPRESSION_OPTION_NORMAL} },
            { "tif",   { "image/tiff", APPX_COMPRESSION_OPTION_NORMAL} },
            { "tiff",  { "image/tiff", APPX_COMPRESSION_OPTION_NORMAL} },
            { "wmf",   { "image/x-wmf", APPX_COMPRESSION_OPTION_NORMAL} },
            // Video types
            { "avi",   { "video/avi", APPX_COMPRESSION_OPTION_NONE} },
            { "mpeg",  { "video/mpeg", APPX_COMPRESSION_OPTION_NONE} },
            { "mpg",   { "video/mpeg", APPX_COMPRESSION_OPTION_NONE} },
            { "mov",   { "video/quicktime", APPX_COMPRESSION_OPTION_NONE} },
            { "wmv",   { "video/x-ms-wmv", APPX_COMPRESSION_OPTION_NONE} }
        };

        auto findExt = extToContentType.find(ext);
        if (findExt == extToContentType.end())
        {
            // if it's not in the map these are the defaults
            return std::make_pair("application/octet-stream", APPX_COMPRESSION_OPTION_NORMAL);
        }
        return findExt->second;
    }

    const std::string ContentType::GetPayloadFileContentType(APPX_FOOTPRINT_FILE_TYPE footprintFile)
    {
        if (footprintFile == APPX_FOOTPRINT_FILE_TYPE_MANIFEST)
        {
            return "application/vnd.ms-appx.manifest+xml";
        }
        if (footprintFile == APPX_FOOTPRINT_FILE_TYPE_BLOCKMAP)
        {
            return "application/vnd.ms-appx.blockmap+xml";
        }
        if (footprintFile == APPX_FOOTPRINT_FILE_TYPE_SIGNATURE)
        {
            return "application/vnd.ms-appx.signature";
        }
        // TODO: add other ones if needed, otherwise throw
        NOTIMPLEMENTED
    }
}

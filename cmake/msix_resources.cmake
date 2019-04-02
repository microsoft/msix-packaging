# Copyright (C) 2017 Microsoft.  All rights reserved.
# See LICENSE file in   the project root for full license information.
# This file creates a zip file for our resources and produce MSIXResource.hpp.
# MSIXResource.hpp contains a std::uint8_t array that is the zip file as bytes. Internally,
# we create a stream from it, treat as a stream and use our own ZipObject implementation to read data from it.

set(RESOURCES_CERTS)
set(RESOURCES_BLOCKMAP)
set(RESOURCES_CONTENTTYPE)
set(RESOURCES_APPXTYPES)
set(RESOURCES_APPXMANIFEST)
set(RESOURCES_APPXBUNDLEMANIFEST)

if(CRYPTO_LIB MATCHES openssl) # Only OpenSSL needs to carry the certificates.
    list(APPEND RESOURCES_CERTS
        "certs/base64_MSFT_RCA_2010.cer"
        "certs/base64_MSFT_RCA_2011.cer"
        "certs/base64_STORE_PCA_2011.cer"
        "certs/base64_Windows_Production.cer"
        "certs/base64_Windows_Production_PCA_2011.cer"
        "certs/Microsoft_MarketPlace_PCA_2011.cer")
endif()

if(USE_VALIDATION_PARSER)

list(APPEND RESOURCES_CONTENTTYPE
    "AppxPackaging/[Content_Types]/opc-contentTypes.xsd")

list(APPEND RESOURCES_BLOCKMAP
    "AppxPackaging/BlockMap/schema/BlockMapSchema.xsd"
    "AppxPackaging/BlockMap/schema/BlockMapSchema2015.xsd"
    "AppxPackaging/BlockMap/schema/BlockMapSchema2017.xsd")

# AppxManifests. We only use the manifests schemas for Windows using msxml6
# For other parsers and platforms we only validate is valid xml.
if(XML_PARSER MATCHES msxml6)
    # Used by AppxManifest and AppxBundleManifest
    list(APPEND RESOURCES_APPXTYPES
        "AppxPackaging/Manifest/Schema/2015/AppxManifestTypes.xsd")

    list(APPEND RESOURCES_APPXMANIFEST
        "AppxPackaging/Manifest/Schema/2015/AppxPhoneManifestSchema2014.xsd"
        "AppxPackaging/Manifest/Schema/2015/ComManifestSchema.xsd"
        "AppxPackaging/Manifest/Schema/2015/DesktopManifestSchema.xsd"
        "AppxPackaging/Manifest/Schema/2015/FoundationManifestSchema.xsd"
        "AppxPackaging/Manifest/Schema/2015/FoundationManifestSchema_v2.xsd"
        "AppxPackaging/Manifest/Schema/2015/HolographicManifestSchema.xsd"
        "AppxPackaging/Manifest/Schema/2015/IotManifestSchema.xsd"
        "AppxPackaging/Manifest/Schema/2015/MobileManifestSchema.xsd"
        "AppxPackaging/Manifest/Schema/2015/RestrictedCapabilitiesManifestSchema.xsd"
        "AppxPackaging/Manifest/Schema/2015/RestrictedCapabilitiesManifestSchema_v2.xsd"
        "AppxPackaging/Manifest/Schema/2015/ServerManifestSchema.xsd"
        "AppxPackaging/Manifest/Schema/2015/UapManifestSchema.xsd"
        "AppxPackaging/Manifest/Schema/2015/UapManifestSchema_v2.xsd"
        "AppxPackaging/Manifest/Schema/2015/UapManifestSchema_v3.xsd"
        "AppxPackaging/Manifest/Schema/2015/WindowsCapabilitiesManifestSchema.xsd"
        "AppxPackaging/Manifest/Schema/2015/WindowsCapabilitiesManifestSchema_v2.xsd"
        "AppxPackaging/Manifest/Schema/2015/XboxManifestSchema.xsd"
        "AppxPackaging/Manifest/Schema/2016/DesktopManifestSchema_v2.xsd"
        "AppxPackaging/Manifest/Schema/2016/RestrictedCapabilitiesManifestSchema_v3.xsd"
        "AppxPackaging/Manifest/Schema/2016/UapManifestSchema_v4.xsd"
        "AppxPackaging/Manifest/Schema/2016/WindowsCapabilitiesManifestSchema_v3.xsd"
        "AppxPackaging/Manifest/Schema/2017/ComManifestSchema_v2.xsd"
        "AppxPackaging/Manifest/Schema/2017/DesktopManifestSchema_v3.xsd"
        "AppxPackaging/Manifest/Schema/2017/DesktopManifestSchema_v4.xsd"
        "AppxPackaging/Manifest/Schema/2017/IotManifestSchema_v2.xsd"
        "AppxPackaging/Manifest/Schema/2017/RestrictedCapabilitiesManifestSchema_v4.xsd"
        "AppxPackaging/Manifest/Schema/2017/UapManifestSchema_v5.xsd"
        "AppxPackaging/Manifest/Schema/2017/UapManifestSchema_v6.xsd"
        "AppxPackaging/Manifest/Schema/2018/DesktopManifestSchema_v5.xsd"
        "AppxPackaging/Manifest/Schema/2018/DesktopManifestSchema_v6.xsd"
        "AppxPackaging/Manifest/Schema/2018/RestrictedCapabilitiesManifestSchema_v5.xsd"
        "AppxPackaging/Manifest/Schema/2018/RestrictedCapabilitiesManifestSchema_v6.xsd"
        "AppxPackaging/Manifest/Schema/2018/UapManifestSchema_v7.xsd"
        "AppxPackaging/Manifest/Schema/2018/UapManifestSchema_v8.xsd"
    )

    list(APPEND RESOURCES_APPXBUNDLEMANIFEST
        "AppxPackaging/Manifest/Schema/2015/BundleManifestSchema2014.xsd"
        "AppxPackaging/Manifest/Schema/2016/BundleManifestSchema2016.xsd"
        "AppxPackaging/Manifest/Schema/2017/BundleManifestSchema2017.xsd"
        "AppxPackaging/Manifest/Schema/2018/BundleManifestSchema2018.xsd"
    )
elseif(XML_PARSER MATCHES xerces)
    list(APPEND RESOURCES_APPXTYPES
        "AppxPackaging/Manifest/Schema/Xerces/AppxManifestTypes.xsd")

    list(APPEND RESOURCES_APPXMANIFEST
        "AppxPackaging/Manifest/Schema/2018/RestrictedCapabilitiesManifestSchema_v5.xsd"
        "AppxPackaging/Manifest/Schema/2018/RestrictedCapabilitiesManifestSchema_v6.xsd"
        "AppxPackaging/Manifest/Schema/2015/AppxPhoneManifestSchema2014.xsd"
        "AppxPackaging/Manifest/Schema/2015/FoundationManifestSchema.xsd"
        "AppxPackaging/Manifest/Schema/2015/ComManifestSchema.xsd"
        "AppxPackaging/Manifest/Schema/2015/DesktopManifestSchema.xsd"
        "AppxPackaging/Manifest/Schema/2015/FoundationManifestSchema_v2.xsd"
        "AppxPackaging/Manifest/Schema/2015/HolographicManifestSchema.xsd"
        "AppxPackaging/Manifest/Schema/2015/IotManifestSchema.xsd"
        "AppxPackaging/Manifest/Schema/2015/MobileManifestSchema.xsd"
        "AppxPackaging/Manifest/Schema/2015/RestrictedCapabilitiesManifestSchema.xsd"
        "AppxPackaging/Manifest/Schema/2015/RestrictedCapabilitiesManifestSchema_v2.xsd"
        "AppxPackaging/Manifest/Schema/2015/ServerManifestSchema.xsd"
        "AppxPackaging/Manifest/Schema/2015/UapManifestSchema.xsd"
        "AppxPackaging/Manifest/Schema/2015/UapManifestSchema_v2.xsd"
        "AppxPackaging/Manifest/Schema/2015/UapManifestSchema_v3.xsd"
        "AppxPackaging/Manifest/Schema/2015/WindowsCapabilitiesManifestSchema.xsd"
        "AppxPackaging/Manifest/Schema/2015/WindowsCapabilitiesManifestSchema_v2.xsd"
        "AppxPackaging/Manifest/Schema/2015/XboxManifestSchema.xsd"
        "AppxPackaging/Manifest/Schema/2016/DesktopManifestSchema_v2.xsd"
        "AppxPackaging/Manifest/Schema/2016/RestrictedCapabilitiesManifestSchema_v3.xsd"
        "AppxPackaging/Manifest/Schema/2016/UapManifestSchema_v4.xsd"
        "AppxPackaging/Manifest/Schema/2016/WindowsCapabilitiesManifestSchema_v3.xsd"
        "AppxPackaging/Manifest/Schema/2017/ComManifestSchema_v2.xsd"
        "AppxPackaging/Manifest/Schema/2017/DesktopManifestSchema_v3.xsd"
        "AppxPackaging/Manifest/Schema/2017/DesktopManifestSchema_v4.xsd"
        "AppxPackaging/Manifest/Schema/2017/IotManifestSchema_v2.xsd"
        "AppxPackaging/Manifest/Schema/2017/RestrictedCapabilitiesManifestSchema_v4.xsd"
        "AppxPackaging/Manifest/Schema/2017/UapManifestSchema_v5.xsd"
        "AppxPackaging/Manifest/Schema/2017/UapManifestSchema_v6.xsd"
        "AppxPackaging/Manifest/Schema/2018/DesktopManifestSchema_v5.xsd"
        "AppxPackaging/Manifest/Schema/2018/DesktopManifestSchema_v6.xsd"
        "AppxPackaging/Manifest/Schema/2018/UapManifestSchema_v7.xsd"
        "AppxPackaging/Manifest/Schema/2018/UapManifestSchema_v8.xsd"

        )
#"AppxPackaging/Manifest/Schema/Xerces/AppxManifestSchema_Merge.xsd"
    list(APPEND RESOURCES_APPXBUNDLEMANIFEST
        "AppxPackaging/Manifest/Schema/2015/BundleManifestSchema2014.xsd"
        "AppxPackaging/Manifest/Schema/2016/BundleManifestSchema2016.xsd"
        "AppxPackaging/Manifest/Schema/2017/BundleManifestSchema2017.xsd"
        "AppxPackaging/Manifest/Schema/2018/BundleManifestSchema2018.xsd"
        )
endif()

endif(USE_VALIDATION_PARSER)

# Create zip file. Use execute_process to run the command while CMake is procesing.
message(STATUS "Resource files:")
foreach(FILE ${RESOURCES_CERTS} ${RESOURCES_BLOCKMAP} ${RESOURCES_CONTENTTYPE} ${RESOURCES_APPXTYPES} ${RESOURCES_APPXMANIFEST} ${RESOURCES_APPXBUNDLEMANIFEST})
    message(STATUS "\t${FILE}")
endforeach(FILE)

execute_process(
    COMMAND ${CMAKE_COMMAND} -E tar cvf "${CMAKE_BINARY_DIR}/resources.zip" --format=zip -- ${RESOURCES_BLOCKMAP} ${RESOURCES_CONTENTTYPE} ${RESOURCES_APPXMANIFEST} ${RESOURCES_CERTS} ${RESOURCES_APPXBUNDLEMANIFEST} ${RESOURCES_APPXTYPES}
    WORKING_DIRECTORY "${CMAKE_PROJECT_ROOT}/resources"
    OUTPUT_QUIET
)

file(READ "${CMAKE_BINARY_DIR}/resources.zip" RESOURCE_HEX HEX)
# Create a list by matching every 2 charactes. CMake separates lists with ;
string(REGEX MATCHALL ".." RESOURCE_HEX_LIST "${RESOURCE_HEX}")
list(LENGTH RESOURCE_HEX_LIST RESOURCE_LENGTH)
# The list is just a string, so change ; for ", 0x" to initialize the vector.
# Just remember the first element won't have 0x.
string(REGEX REPLACE ";" ", 0x" RESOURCE_BYTES "${RESOURCE_HEX_LIST}")

function(GetResourceHpp LIST OUTPUT)
    foreach(i ${LIST})
        string(APPEND RESULT result.push_back(std::make_pair(\"${i}\", std::move(factory->GetResource(\"${i}\")))) ";\n\t\t\t\t")
    endforeach(i)
    set(${OUTPUT} ${RESULT} PARENT_SCOPE)
endfunction()

GetResourceHpp("${RESOURCES_BLOCKMAP}"           BLOCKMAP_HPP)
GetResourceHpp("${RESOURCES_CONTENTTYPE}"        CONTENTTYPE_HPP)
GetResourceHpp("${RESOURCES_APPXMANIFEST}"       APPXMANIFEST_HPP)
GetResourceHpp("${RESOURCES_CERTS}"              CERTS_HPP)
GetResourceHpp("${RESOURCES_APPXBUNDLEMANIFEST}" APPXBUNDLEMANIFEST_HPP)
GetResourceHpp("${RESOURCES_APPXTYPES}"          APPXTYPES_HPP)

set(RESOURCE_HEADER "//
//
//  Copyright (C) 2017 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
//
//  This file is generated by CMake. Do not edit.
//
#include \"AppxPackaging.hpp\"
#include \"ComHelper.hpp\"
#include \"AppxFactory.hpp\"
#include <map>

namespace MSIX {
    namespace Resource {

        enum Type
        {
            Certificates,
            ContentType,
            BlockMap,
            AppxManifest,
            AppxBundleManifest
        };

        const size_t resourceLength = ${RESOURCE_LENGTH};
        const std::uint8_t resourceByte[resourceLength] = {0x${RESOURCE_BYTES}};
    }

    inline std::vector<std::pair<std::string, ComPtr<IStream>>> GetResources(IMsixFactory* factory, Resource::Type type)
    {
        std::vector<std::pair<std::string, ComPtr<IStream>>> result;
        switch(type)
        {
            case Resource::Type::Certificates:
                ${CERTS_HPP}
                break;
            case Resource::Type::ContentType:
                ${CONTENTTYPE_HPP}
                break;
            case Resource::Type::BlockMap:
                ${BLOCKMAP_HPP}
                break;
            case Resource::Type::AppxManifest:
                ${APPXTYPES_HPP}
                ${APPXMANIFEST_HPP}
                break;
            case Resource::Type::AppxBundleManifest:
                ${APPXTYPES_HPP}
                ${APPXBUNDLEMANIFEST_HPP}
                break;
        }
        return result;
    }
}
")
file(WRITE "${CMAKE_PROJECT_ROOT}/src/inc/MSIXResource.hpp" "${RESOURCE_HEADER}")

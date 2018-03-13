# Copyright (C) 2017 Microsoft.  All rights reserved.
# See LICENSE file in   the project root for full license information.
# This file creates a zip file for our resources and produce MSIXResource.hpp.
# MSIXResource.hpp contains a std::uint8_t array that is the zip file as bytes. Internally,
# we create a stream from it, treat as a stream and use our own ZipObject implementation to read data from it.

set(RESOURCES_CERTS)
set(RESOURCES_BLOCKMAP)
set(RESOURCES_CONTENTTYPE)
set(RESOURCES_APPXMANIFEST)

if(NOT WIN32) # Always add the certs for non-Windows.
    list(APPEND RESOURCES_CERTS
        "certs/base64_MSFT_RCA_2010.cer"
        "certs/base64_MSFT_RCA_2011.cer"
        "certs/base64_STORE_PCA_2011.cer"
        "certs/base64_Windows_Production.cer"
        "certs/base64_Windows_Production_PCA_2011.cer"
        "certs/Microsoft_MarketPlace_PCA_2011.cer")
endif()

#if(USE_VALIDATION_PARSER) turn on when there's an implementation of XmlLite/LibXml2

list(APPEND RESOURCES_CONTENTTYPE 
    "AppxPackaging/[Content_Types]/opc-contentTypes.xsd")

list(APPEND RESOURCES_BLOCKMAP
    "AppxPackaging/BlockMap/schema/BlockMapSchema.xsd"
    "AppxPackaging/BlockMap/schema/BlockMapSchema2015.xsd"
    "AppxPackaging/BlockMap/schema/BlockMapSchema2017.xsd")

# AppxManifests 
if(HAVE_MSXML6)
    list(APPEND RESOURCES_APPXMANIFEST
    "AppxPackaging/Manifest/Schema/2015/AppxManifestTypes.xsd"
    "AppxPackaging/Manifest/Schema/2015/BundleManifestSchema2013.xsd"
    "AppxPackaging/Manifest/Schema/2015/BundleManifestSchema2014.xsd"
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
    "AppxPackaging/Manifest/Schema/2016/BundleManifestSchema2016.xsd"
    "AppxPackaging/Manifest/Schema/2016/DesktopManifestSchema_v2.xsd"
    "AppxPackaging/Manifest/Schema/2016/RestrictedCapabilitiesManifestSchema_v3.xsd"
    "AppxPackaging/Manifest/Schema/2016/UapManifestSchema_v4.xsd"
    "AppxPackaging/Manifest/Schema/2016/WindowsCapabilitiesManifestSchema_v3.xsd"
    "AppxPackaging/Manifest/Schema/2017/BundleManifestSchema2017.xsd"
    "AppxPackaging/Manifest/Schema/2017/ComManifestSchema_v2.xsd"
    "AppxPackaging/Manifest/Schema/2017/DesktopManifestSchema_v3.xsd"
    "AppxPackaging/Manifest/Schema/2017/DesktopManifestSchema_v4.xsd"
    "AppxPackaging/Manifest/Schema/2017/IotManifestSchema_v2.xsd"
    "AppxPackaging/Manifest/Schema/2017/RestrictedCapabilitiesManifestSchema_v4.xsd"
    "AppxPackaging/Manifest/Schema/2017/UapManifestSchema_v5.xsd"
    "AppxPackaging/Manifest/Schema/2017/UapManifestSchema_v6.xsd")
else() # xerces
    # TODO: make changes required to make the xsds WC3 compliant.
endif()

#endif() USE_VALIDATION_PARSER

# Create zip file. Use execute_process to run the command while CMake is procesing.
message(STATUS "Resource files:")
foreach(FILE ${RESOURCES_BLOCKMAP} ${RESOURCES_CONTENTTYPE} ${RESOURCES_APPXMANIFEST} ${RESOURCES_CERTS})
    message(STATUS "\t${FILE}")
endforeach(FILE)

execute_process(
    COMMAND ${CMAKE_COMMAND} -E tar cvf "${CMAKE_BINARY_DIR}/resources.zip" --format=zip -- ${RESOURCES_BLOCKMAP} ${RESOURCES_CONTENTTYPE} ${RESOURCES_APPXMANIFEST} ${RESOURCES_CERTS}
    WORKING_DIRECTORY "${CMAKE_PROJECT_ROOT}/resources"
    OUTPUT_QUIET
)

file(READ "${CMAKE_BINARY_DIR}/resources.zip" RESOURCE_HEX HEX)
# Create a list by matching every 2 charactes. CMake separates lists with ;
string(REGEX MATCHALL ".." RESOURCE_HEX_LIST "${RESOURCE_HEX}")
list(LENGTH RESOURCE_HEX_LIST RESOURCE_LENGHT)
# The list is just a string, so change ; for ", 0x" to initialize the vector.
# Just remember the first element won't have 0x.
string(REGEX REPLACE ";" ", 0x" RESOURCE_BYTES "${RESOURCE_HEX_LIST}")

function(GetResourceHpp LIST OUTPUT)
    foreach(i ${LIST})
        string(APPEND RESULT result.push_back(ComPtr<IStream>(factory->GetResource(\"${i}\"))) ";\n\t\t\t\t")
    endforeach(i)
    set(${OUTPUT} ${RESULT} PARENT_SCOPE)
endfunction()

GetResourceHpp("${RESOURCES_BLOCKMAP}"     BLOCKMAP_HPP)
GetResourceHpp("${RESOURCES_CONTENTTYPE}"  CONTENTTYPE_HPP)
GetResourceHpp("${RESOURCES_APPXMANIFEST}" APPXMANIFEST_HPP)
GetResourceHpp("${RESOURCES_CERTS}"        CERTS_HPP)

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

namespace MSIX {
    namespace Resource {

        enum Type
        {
            Certificates,
            ContentType,
            BlockMap,
            AppxManifest
        };

        const size_t resourceLenght = ${RESOURCE_LENGHT};
        const std::uint8_t resourceByte[resourceLenght] = {0x${RESOURCE_BYTES}};
    }

    inline std::vector<ComPtr<IStream>> GetResources(IMSIXFactory* factory, Resource::Type type)
    {
        std::vector<ComPtr<IStream>> result;
        switch(type)
        {
            case Resource::Certificates:
                ${CERTS_HPP}
                break;
            case Resource::ContentType:
                ${CONTENTTYPE_HPP}
                break;
            case Resource::BlockMap:
                ${BLOCKMAP_HPP}
                break;
            case Resource::AppxManifest:
                ${APPXMANIFEST_HPP}
                break;
        }
        return result;
    }
}
")
file(WRITE "${CMAKE_PROJECT_ROOT}/src/inc/MSIXResource.hpp" "${RESOURCE_HEADER}")

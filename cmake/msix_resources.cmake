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

set(RESOURCES_DIR "${CMAKE_PROJECT_ROOT}/resources")

if(CRYPTO_LIB MATCHES openssl) # Only OpenSSL needs to carry the certificates.
    list(APPEND RESOURCES_CERTS
        "certs/base64_MSFT_RCA_2010.cer"
        "certs/base64_MSFT_RCA_2011.cer"
        "certs/base64_STORE_PCA_2011.cer"
        "certs/base64_Windows_Production.cer"
        "certs/base64_Windows_Production_PCA_2011.cer"
        "certs/Microsoft_MarketPlace_PCA_2011.cer")
endif()

if ((XML_PARSER MATCHES msxml6) OR (XML_PARSER MATCHES xerces))
    # Used by namespace manager
    if (XML_PARSER MATCHES msxml6)
        set(CHAR_TYPE  "wchar_t")
        set(STR_COMP   "wcscmp")
        set(STR_PREFIX "L")
    else() # xerces
        set(CHAR_TYPE  "char")
        set(STR_COMP   "strcmp")
        set(STR_PREFIX "u8")
    endif()

    if(USE_VALIDATION_PARSER)
        # Schemas are defined in triplets in the form of
        # <namespace> <alias> <file location relative to root/resources>
        list(APPEND CONTENT_TYPES
            "http://schemas.openxmlformats.org/package/2006/content-types" "a" "AppxPackaging/[Content_Types]/opc-contentTypes.xsd")
        list(APPEND RESOURCES_CONTENTTYPE
            CONTENT_TYPES)

        # Block map
        list(APPEND BLOCKMAP_SCHEMA
            "http://schemas.microsoft.com/appx/2010/blockmap" "a" "AppxPackaging/BlockMap/schema/BlockMapSchema.xsd")
        list(APPEND BLOCKMAP_SCHEMA_2015
            "http://schemas.microsoft.com/appx/2015/blockmap" "b" "AppxPackaging/BlockMap/schema/BlockMapSchema2015.xsd")
        list(APPEND BLOCKMAP_SCHEMA_2017
            "http://schemas.microsoft.com/appx/2017/blockmap" "c" "AppxPackaging/BlockMap/schema/BlockMapSchema2017.xsd")
        list(APPEND RESOURCES_BLOCKMAP
            BLOCKMAP_SCHEMA
            BLOCKMAP_SCHEMA_2015
            BLOCKMAP_SCHEMA_2017)

        # Used by AppxManifest and AppxBundleManifest
        list(APPEND APPX_TYPES
            "http://schemas.microsoft.com/appx/manifest/types" "t" "AppxPackaging/Manifest/Schema/2015/AppxManifestTypes.xsd")
        list(APPEND RESOURCES_APPXTYPES
            APPX_TYPES)

        # Appx Manifest
        list(APPEND MANIFEST_FOUNDATION
            "http://schemas.microsoft.com/appx/manifest/foundation/windows10"                          "win10foundation"    "AppxPackaging/Manifest/Schema/2015/FoundationManifestSchema.xsd")
        list(APPEND MANIFEST_UAP
            "http://schemas.microsoft.com/appx/manifest/uap/windows10"                                 "win10uap"           "AppxPackaging/Manifest/Schema/2015/UapManifestSchema.xsd")
        list(APPEND MANIFEST_PHONE
            "http://schemas.microsoft.com/appx/2014/phone/manifest"                                    "mp"                 "AppxPackaging/Manifest/Schema/2015/AppxPhoneManifestSchema2014.xsd")
        list(APPEND MANIFEST_FOUNDATION2
            "http://schemas.microsoft.com/appx/manifest/foundation/windows10/2"                        "foundation2"        "AppxPackaging/Manifest/Schema/2015/FoundationManifestSchema_v2.xsd")
        list(APPEND MANIFEST_UAP2
            "http://schemas.microsoft.com/appx/manifest/uap/windows10/2"                               "uap2"               "AppxPackaging/Manifest/Schema/2015/UapManifestSchema_v2.xsd")
        list(APPEND MANIFEST_UAP3
            "http://schemas.microsoft.com/appx/manifest/uap/windows10/3"                               "uap3"               "AppxPackaging/Manifest/Schema/2015/UapManifestSchema_v3.xsd")
        list(APPEND MANIFEST_UAP4
            "http://schemas.microsoft.com/appx/manifest/uap/windows10/4"                               "uap4"               "AppxPackaging/Manifest/Schema/2016/UapManifestSchema_v4.xsd")
        list(APPEND MANIFEST_WINCAP
            "http://schemas.microsoft.com/appx/manifest/foundation/windows10/windowscapabilities"      "win10wincap"        "AppxPackaging/Manifest/Schema/2015/WindowsCapabilitiesManifestSchema.xsd")
        list(APPEND MANIFEST_WINCAP2
            "http://schemas.microsoft.com/appx/manifest/foundation/windows10/windowscapabilities/2"    "wincap2"            "AppxPackaging/Manifest/Schema/2015/WindowsCapabilitiesManifestSchema_v2.xsd")
        list(APPEND MANIFEST_WINCAP3
            "http://schemas.microsoft.com/appx/manifest/foundation/windows10/windowscapabilities/3"    "wincap3"            "AppxPackaging/Manifest/Schema/2016/WindowsCapabilitiesManifestSchema_v3.xsd")
        list(APPEND MANIFEST_RESCAP
            "http://schemas.microsoft.com/appx/manifest/foundation/windows10/restrictedcapabilities"   "win10rescap"        "AppxPackaging/Manifest/Schema/2015/RestrictedCapabilitiesManifestSchema.xsd")
        list(APPEND MANIFEST_RESCAP2
            "http://schemas.microsoft.com/appx/manifest/foundation/windows10/restrictedcapabilities/2" "rescap2"            "AppxPackaging/Manifest/Schema/2015/RestrictedCapabilitiesManifestSchema_v2.xsd")
        list(APPEND MANIFEST_RESCAP3
            "http://schemas.microsoft.com/appx/manifest/foundation/windows10/restrictedcapabilities/3" "rescap3"            "AppxPackaging/Manifest/Schema/2016/RestrictedCapabilitiesManifestSchema_v3.xsd")
        list(APPEND MANIFEST_RESCAP4
            "http://schemas.microsoft.com/appx/manifest/foundation/windows10/restrictedcapabilities/4" "rescap4"            "AppxPackaging/Manifest/Schema/2017/RestrictedCapabilitiesManifestSchema_v4.xsd")
        list(APPEND MANIFEST_RESCAP5
            "http://schemas.microsoft.com/appx/manifest/foundation/windows10/restrictedcapabilities/5" "rescap5"            "AppxPackaging/Manifest/Schema/2018/RestrictedCapabilitiesManifestSchema_v5.xsd")
        list(APPEND MANIFEST_RESCAP6
            "http://schemas.microsoft.com/appx/manifest/foundation/windows10/restrictedcapabilities/6" "rescap6"            "AppxPackaging/Manifest/Schema/2018/RestrictedCapabilitiesManifestSchema_v6.xsd")
        list(APPEND MANIFEST_MOBILE
            "http://schemas.microsoft.com/appx/manifest/mobile/windows10"                              "win10mobile"        "AppxPackaging/Manifest/Schema/2015/MobileManifestSchema.xsd")
        list(APPEND MANIFEST_IOT
            "http://schemas.microsoft.com/appx/manifest/iot/windows10"                                 "win10iot"           "AppxPackaging/Manifest/Schema/2015/IotManifestSchema.xsd")
        list(APPEND MANIFEST_IOT2
            "http://schemas.microsoft.com/appx/manifest/iot/windows10/2"                               "iot2"               "AppxPackaging/Manifest/Schema/2017/IotManifestSchema_v2.xsd")
        list(APPEND MANIFEST_HOLO
            "http://schemas.microsoft.com/appx/manifest/holographic/windows10"                         "holo"               "AppxPackaging/Manifest/Schema/2015/HolographicManifestSchema.xsd")
        list(APPEND MANIFEST_SERVER
            "http://schemas.microsoft.com/appx/manifest/serverpreview/windows10"                       "win10serverpreview" "AppxPackaging/Manifest/Schema/2015/ServerManifestSchema.xsd")
        list(APPEND MANIFEST_DESK
            "http://schemas.microsoft.com/appx/manifest/desktop/windows10"                             "desktop"            "AppxPackaging/Manifest/Schema/2015/DesktopManifestSchema.xsd")
        list(APPEND MANIFEST_DESK2
            "http://schemas.microsoft.com/appx/manifest/desktop/windows10/2"                           "desktop2"           "AppxPackaging/Manifest/Schema/2016/DesktopManifestSchema_v2.xsd")
        list(APPEND MANIFEST_DESK3
            "http://schemas.microsoft.com/appx/manifest/desktop/windows10/3"                           "desktop3"           "AppxPackaging/Manifest/Schema/2017/DesktopManifestSchema_v3.xsd")
        list(APPEND MANIFEST_DESK4
            "http://schemas.microsoft.com/appx/manifest/desktop/windows10/4"                           "desktop4"           "AppxPackaging/Manifest/Schema/2017/DesktopManifestSchema_v4.xsd")
        list(APPEND MANIFEST_DESK5
            "http://schemas.microsoft.com/appx/manifest/desktop/windows10/5"                           "desktop5"           "AppxPackaging/Manifest/Schema/2018/DesktopManifestSchema_v5.xsd")
        list(APPEND MANIFEST_DESK6
            "http://schemas.microsoft.com/appx/manifest/desktop/windows10/6"                           "desktop6"           "AppxPackaging/Manifest/Schema/2018/DesktopManifestSchema_v6.xsd")
        list(APPEND MANIFEST_COM
            "http://schemas.microsoft.com/appx/manifest/com/windows10"                                 "com"                "AppxPackaging/Manifest/Schema/2015/ComManifestSchema.xsd")
        list(APPEND MANIFEST_COM2
            "http://schemas.microsoft.com/appx/manifest/com/windows10/2"                               "com2"               "AppxPackaging/Manifest/Schema/2017/ComManifestSchema_v2.xsd")
        list(APPEND MANIFEST_UAP5
            "http://schemas.microsoft.com/appx/manifest/uap/windows10/5"                               "uap5"               "AppxPackaging/Manifest/Schema/2017/UapManifestSchema_v5.xsd")
        list(APPEND MANIFEST_UAP6
            "http://schemas.microsoft.com/appx/manifest/uap/windows10/6"                               "uap6"               "AppxPackaging/Manifest/Schema/2017/UapManifestSchema_v6.xsd")
        list(APPEND MANIFEST_UAP7
            "http://schemas.microsoft.com/appx/manifest/uap/windows10/7"                               "uap7"               "AppxPackaging/Manifest/Schema/2018/UapManifestSchema_v7.xsd")
        list(APPEND MANIFEST_UAP8
            "http://schemas.microsoft.com/appx/manifest/uap/windows10/8"                               "uap8"               "AppxPackaging/Manifest/Schema/2018/UapManifestSchema_v8.xsd")
        list(APPEND RESOURCES_APPXMANIFEST
            MANIFEST_FOUNDATION
            MANIFEST_UAP
            MANIFEST_PHONE
            MANIFEST_FOUNDATION2
            MANIFEST_UAP2
            MANIFEST_UAP3
            MANIFEST_UAP4
            MANIFEST_WINCAP
            MANIFEST_WINCAP2
            MANIFEST_WINCAP3
            MANIFEST_RESCAP
            MANIFEST_RESCAP2
            MANIFEST_RESCAP3
            MANIFEST_RESCAP4
            MANIFEST_RESCAP5
            MANIFEST_RESCAP6
            MANIFEST_MOBILE
            MANIFEST_IOT
            MANIFEST_IOT2
            MANIFEST_HOLO
            MANIFEST_SERVER
            MANIFEST_DESK
            MANIFEST_DESK2
            MANIFEST_DESK3
            MANIFEST_DESK4
            MANIFEST_DESK5
            MANIFEST_DESK6
            MANIFEST_COM
            MANIFEST_COM2
            MANIFEST_UAP5
            MANIFEST_UAP6
            MANIFEST_UAP7
            MANIFEST_UAP8)

        # Bundle manifest
        list(APPEND BUNDLE_2014
            "http://schemas.microsoft.com/appx/2013/bundle" "b" "AppxPackaging/Manifest/Schema/2015/BundleManifestSchema2014.xsd")
        list(APPEND BUNDLE_2016
            "http://schemas.microsoft.com/appx/2016/bundle" "b2" "AppxPackaging/Manifest/Schema/2016/BundleManifestSchema2016.xsd")
        list(APPEND BUNDLE_2017
            "http://schemas.microsoft.com/appx/2017/bundle" "b3" "AppxPackaging/Manifest/Schema/2017/BundleManifestSchema2017.xsd")
        list(APPEND BUNDLE_2018
            "http://schemas.microsoft.com/appx/2018/bundle" "b4" "AppxPackaging/Manifest/Schema/2018/BundleManifestSchema2018.xsd")
        list(APPEND RESOURCES_APPXBUNDLEMANIFEST
            BUNDLE_2014
            BUNDLE_2016
            BUNDLE_2017
            BUNDLE_2018)

        if (XML_PARSER MATCHES xerces)
            file(COPY ${RESOURCES_DIR} DESTINATION "${CMAKE_CURRENT_BINARY_DIR}")
            set(RESOURCES_DIR "${CMAKE_CURRENT_BINARY_DIR}/resources")
            # For xerces we replace all the maxOccurs="<high number>" to maxOccurs="<unbounded>"
            # where high number is >=100. This means that it is possible to accept a manifest
            # that will fail for windows.

            # From xerces limitations:
            #   In certain complex content models specifying large values for the minOccurs or
            #   maxOccurs attributes may result in poor performance and/or large amount of
            #   memory being allocated by the parser. In such situations large values for
            #   minOccurs should be avoided, and unbounded should be used instead.
            # See: https://xerces.apache.org/xerces-c/schema-3.html
            foreach(TRIPLET ${RESOURCES_APPXMANIFEST} ${RESOURCES_APPXBUNDLEMANIFEST})
                list(GET ${TRIPLET} 2 SCHEMA)
                file(READ "${RESOURCES_DIR}/${SCHEMA}" SCHEMA_TEXT)
                string(REGEX REPLACE "maxOccurs=\"[1-9][0-9][0-9]+\"" "maxOccurs=\"unbounded\"" SCHEMA_TEXT "${SCHEMA_TEXT}")
                file(WRITE "${RESOURCES_DIR}/${SCHEMA}" "${SCHEMA_TEXT}")
            endforeach()

            # AppxManifestTypes.xsd uses some XML 1.0 non standard regex patterns
            list(GET APPX_TYPES 2 APPX_TYPES_FILE)
            file(READ "${RESOURCES_DIR}/${APPX_TYPES_FILE}" APPTYPES_TEXT)

            # The only valid characters between \x01-\x1f are x9 (tab) xA (new line) xD (carriage return)
            # ST_Description ST_FileNameCharSet
            string(REGEX REPLACE 
                [[\\x01-\\x1f]]
                [[\\t\\n\\r]]
                APPTYPES_TEXT "${APPTYPES_TEXT}")

            # Word boundaries
            # ST_ResourceReference and ST_WebAccountProviderUrl
            string(REGEX REPLACE
                [[\\bms-resource]]
                "ms-resource"
                APPTYPES_TEXT "${APPTYPES_TEXT}")
            string(REGEX REPLACE
                [[\\w]]
                [[a-zA-Z0-9_]]
                APPTYPES_TEXT "${APPTYPES_TEXT}")

            # Ilegal escaped characters
            # ST_WebAccountProviderUrl
            string(REGEX REPLACE
                [[\\/]]
                [[/]]
                APPTYPES_TEXT "${APPTYPES_TEXT}")

            # Negative lookahead. I am not sure if we can do something here... maybe a semantic check?
            # ST_Parameters
            string(REGEX REPLACE
                [[\(\(\?\!\\%\[Ii\].*\\%\[Ii\]\)\.\)\*]]
                ".*"
                APPTYPES_TEXT "${APPTYPES_TEXT}")

            file(WRITE "${RESOURCES_DIR}/${APPX_TYPES_FILE}" "${APPTYPES_TEXT}")
        endif()

        function(CreateNamespaceManager LIST OUTPUT)
            foreach(TRIPLET ${LIST})
                list(GET ${TRIPLET} 0 NAMESPACE)
                list(GET ${TRIPLET} 1 ALIAS)
                list(GET ${TRIPLET} 2 FILE)
                string(APPEND RESULT "SchemaEntry(" "${STR_PREFIX}" \" "${NAMESPACE}" \", "${STR_PREFIX}" \" "${ALIAS}" \" , u8\" "${FILE}" \" "),\n\t\t")
            endforeach()
            set(${OUTPUT} ${RESULT} PARENT_SCOPE)
        endfunction()
        CreateNamespaceManager("${RESOURCES_CONTENTTYPE}"        SCHEMAENTRY_CONTENTTYPE)
        CreateNamespaceManager("${RESOURCES_BLOCKMAP}"           SCHEMAENTRY_BLOCKMAP)
        CreateNamespaceManager("${RESOURCES_APPXTYPES}"          SCHEMAENTRY_APPXTYPES)
        CreateNamespaceManager("${RESOURCES_APPXMANIFEST}"       SCHEMAENTRY_APPXMANIFEST)
        CreateNamespaceManager("${RESOURCES_APPXBUNDLEMANIFEST}" SCHEMAENTRY_APPXBUNDLEMANIFEST)
    endif(USE_VALIDATION_PARSER)

    # Namespace manager
    set(NAMESPACE_MANAGER_HPP "
    struct SchemaEntry
    {
        const ${CHAR_TYPE}*  uri;
        const ${CHAR_TYPE}*  alias;
        const char*          schema;
    
        SchemaEntry(const ${CHAR_TYPE}* u, const ${CHAR_TYPE}* a, const char* s) : uri(u), alias(a), schema(s) {}
    
        inline bool operator==(const ${CHAR_TYPE}* otherUri) const {
            return 0 == ${STR_COMP}(uri, otherUri);
        }
    };
    
    typedef std::vector<SchemaEntry> NamespaceManager;
    
    //         ALL THE URIs MUST BE LOWER-CASE, ordering of schema entries defines order of placement of schema into schema cache.
    extern const NamespaceManager s_xmlNamespaces[];")

    set(NAMESPACE_MANAGER_CPP "
    const NamespaceManager s_xmlNamespaces[] = {
    {   // XmlContentType::ContentTypeXml
        ${SCHEMAENTRY_CONTENTTYPE}
    },
    {   // XmlContentType::AppxBlockMapXml
        ${SCHEMAENTRY_BLOCKMAP}
    },
    {   // XmlContentType::AppxManifestXml
        ${SCHEMAENTRY_APPXTYPES}
        ${SCHEMAENTRY_APPXMANIFEST}
    },
    {   // XmlContentType::AppxBundleManifestXml
        ${SCHEMAENTRY_APPXTYPES}
        ${SCHEMAENTRY_APPXBUNDLEMANIFEST}
    }};")
    string(CONFIGURE "${NAMESPACE_MANAGER_HPP}" NAMESPACE_MANAGER_HPP)
    string(CONFIGURE "${NAMESPACE_MANAGER_CPP}" NAMESPACE_MANAGER_CPP)
endif()

# Create zip file. Use execute_process to run the command while CMake is procesing.
message(STATUS "Resource files:")
set(FILES_TO_ZIP)
foreach(TRIPLET ${RESOURCES_BLOCKMAP} ${RESOURCES_CONTENTTYPE} ${RESOURCES_APPXTYPES} ${RESOURCES_APPXMANIFEST} ${RESOURCES_APPXBUNDLEMANIFEST})
    list(GET ${TRIPLET} 2 FILE)
    message(STATUS "\t${FILE}")
    list(APPEND FILES_TO_ZIP "${FILE}")
endforeach()
foreach(FILE ${RESOURCES_CERTS})
    message(STATUS "\t${FILE}")
    list(APPEND FILES_TO_ZIP "${FILE}")
endforeach()

execute_process(
    COMMAND ${CMAKE_COMMAND} -E tar cvf "${CMAKE_BINARY_DIR}/resources.zip" --format=zip -- ${FILES_TO_ZIP}
    WORKING_DIRECTORY "${RESOURCES_DIR}"
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
    foreach(TRIPLET ${LIST})
        list(GET ${TRIPLET} 2 FILE)
        string(APPEND RESULT result.push_back(std::make_pair(\"${FILE}\", std::move(factory->GetResource(\"${FILE}\")))) ";\n\t\t\t\t")
    endforeach()
    set(${OUTPUT} ${RESULT} PARENT_SCOPE)
endfunction()

GetResourceHpp("${RESOURCES_BLOCKMAP}"           BLOCKMAP_HPP)
GetResourceHpp("${RESOURCES_CONTENTTYPE}"        CONTENTTYPE_HPP)
GetResourceHpp("${RESOURCES_APPXMANIFEST}"       APPXMANIFEST_HPP)
GetResourceHpp("${RESOURCES_APPXBUNDLEMANIFEST}" APPXBUNDLEMANIFEST_HPP)
GetResourceHpp("${RESOURCES_APPXTYPES}"          APPXTYPES_HPP)

foreach(FILE ${RESOURCES_CERTS})
    string(APPEND CERTS_HPP result.push_back(std::make_pair(\"${FILE}\", std::move(factory->GetResource(\"${FILE}\")))) ";\n\t\t\t\t")
endforeach()

configure_file(${CMAKE_PROJECT_ROOT}/src/inc/MSIXResource.hpp.cmakein ${CMAKE_PROJECT_ROOT}/src/inc/MSIXResource.hpp CRLF)
configure_file(${CMAKE_PROJECT_ROOT}/src/msix/MSIXResource.cpp.cmakein ${CMAKE_PROJECT_ROOT}/src/msix/MSIXResource.cpp CRLF)

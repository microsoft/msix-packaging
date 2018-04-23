//
//  Copyright (C) 2017 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
//
#include "AppxPackaging.hpp"
#include "Exceptions.hpp"
#include "ComHelper.hpp"
#include "StreamBase.hpp"
#include "StorageObject.hpp"
#include "AppxPackageObject.hpp"
#include "UnicodeConversion.hpp"
#include "IXml.hpp"
#include "MSIXResource.hpp"
#include "Applicability.hpp"

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <limits>
#include <algorithm>
#include <array>

namespace MSIX {

    // names of footprint files.
    #define APPXBLOCKMAP_XML       "AppxBlockMap.xml"
    #define APPXMANIFEST_XML       "AppxManifest.xml"
    #define CODEINTEGRITY_CAT      "AppxMetadata/CodeIntegrity.cat"
    #define APPXSIGNATURE_P7X      "AppxSignature.p7x"
    #define CONTENT_TYPES_XML      "[Content_Types].xml"
    #define APPXBUNDLEMANIFEST_XML "AppxMetadata/AppxBundleManifest.xml"

    static const std::array<const char*, 4> footprintFiles =
    {   APPXMANIFEST_XML,
        APPXBLOCKMAP_XML,
        APPXSIGNATURE_P7X,
        CODEINTEGRITY_CAT,
    };

    static const std::array<const char*, 3> bundleFootprintFiles =
    {
        APPXBUNDLEMANIFEST_XML,
        APPXBLOCKMAP_XML,
        APPXSIGNATURE_P7X,
    };

    struct TargetDeviceFamilyEntry
    {
        const char* tdf;
        const MSIX_PLATFORMS platform;

        TargetDeviceFamilyEntry(const char* t, const MSIX_PLATFORMS p) : tdf(t), platform(p) {}

        inline bool operator==(const char* otherTdf) const {
            return 0 == strcmp(tdf, otherTdf);
        }
    };

    // ALL THE TargetDeviceFamily ENTRIES MUST BE LOWER-CASE
    static const TargetDeviceFamilyEntry targetDeviceFamilyList[] = {
        TargetDeviceFamilyEntry(u8"windows.universal",      MSIX_PLATFORM_WINDOWS10),
        TargetDeviceFamilyEntry(u8"windows.mobile",         MSIX_PLATFORM_WINDOWS10),
        TargetDeviceFamilyEntry(u8"windows.desktop",        MSIX_PLATFORM_WINDOWS10),
        TargetDeviceFamilyEntry(u8"windows.xbox",           MSIX_PLATFORM_WINDOWS10),
        TargetDeviceFamilyEntry(u8"windows.team",           MSIX_PLATFORM_WINDOWS10),
        TargetDeviceFamilyEntry(u8"windows.holographic",    MSIX_PLATFORM_WINDOWS10),
        TargetDeviceFamilyEntry(u8"windows.iot",            MSIX_PLATFORM_WINDOWS10),
        TargetDeviceFamilyEntry(u8"apple.ios.all",          MSIX_PLATFORM_IOS),
        TargetDeviceFamilyEntry(u8"apple.ios.phone",        MSIX_PLATFORM_IOS),
        TargetDeviceFamilyEntry(u8"apple.ios.tablet",       MSIX_PLATFORM_IOS),
        TargetDeviceFamilyEntry(u8"apple.ios.tv",           MSIX_PLATFORM_IOS),
        TargetDeviceFamilyEntry(u8"apple.ios.watch",        MSIX_PLATFORM_IOS),
        TargetDeviceFamilyEntry(u8"apple.macos.all",        MSIX_PLATFORM_MACOS),
        TargetDeviceFamilyEntry(u8"google.android.all",     MSIX_PLATFORM_AOSP),
        TargetDeviceFamilyEntry(u8"google.android.phone",   MSIX_PLATFORM_AOSP),
        TargetDeviceFamilyEntry(u8"google.android.tablet",  MSIX_PLATFORM_AOSP),
        TargetDeviceFamilyEntry(u8"google.android.desktop", MSIX_PLATFORM_AOSP),
        TargetDeviceFamilyEntry(u8"google.android.tv",      MSIX_PLATFORM_AOSP),
        TargetDeviceFamilyEntry(u8"google.android.watch",   MSIX_PLATFORM_AOSP),
        TargetDeviceFamilyEntry(u8"windows7.desktop",       MSIX_PLATFORM_WINDOWS7),
        TargetDeviceFamilyEntry(u8"windows8.desktop",       MSIX_PLATFORM_WINDOWS8),
        TargetDeviceFamilyEntry(u8"linux.all",              MSIX_PLATFORM_LINUX),
        TargetDeviceFamilyEntry(u8"web.edge.all",           MSIX_PLATFORM_WEB),
        TargetDeviceFamilyEntry(u8"web.blink.all",          MSIX_PLATFORM_WEB),
        TargetDeviceFamilyEntry(u8"web.chromium.all",       MSIX_PLATFORM_WEB),
        TargetDeviceFamilyEntry(u8"web.webkit.all",         MSIX_PLATFORM_WEB),
        TargetDeviceFamilyEntry(u8"web.safari.all",         MSIX_PLATFORM_WEB),
        TargetDeviceFamilyEntry(u8"web.all",                MSIX_PLATFORM_WEB),
        TargetDeviceFamilyEntry(u8"platform.all",           static_cast<MSIX_PLATFORMS>(MSIX_PLATFORM_ALL)),
    };

    static const std::size_t PercentangeEncodingTableSize = 0x7E;
    static const std::array<const char*, PercentangeEncodingTableSize> PercentangeEncoding =
    {   nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
        nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
        nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
        nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
        "%20",   "%21",   nullptr, "%23",   "%24",   "%25",   "%26",   "%27",   // [space] ! # $ % & '
        "%28",   "%29",   nullptr, "%2B",   "%2C",   nullptr, nullptr, nullptr, // ( ) + ,
        nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
        nullptr, nullptr, nullptr, "%3B",   nullptr, "%3D",   nullptr, nullptr,   // ; =
        "%40",   nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, // @
        nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
        nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
        nullptr, nullptr, nullptr, "%5B",   nullptr, "%5D",   nullptr, nullptr, // [ ]
        nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
        nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
        nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
        nullptr, nullptr, nullptr, "%7B",   nullptr, "%7D",                     // { }
    };

    // Douglas Crockford's base 32 alphabet variant is 0-9, A-Z except for i, l, o, and u.
    static const char base32DigitList[] = "0123456789abcdefghjkmnpqrstvwxyz";

    struct EncodingChar
    {
        const char* encode;
        char        decode;

        bool operator==(const std::string& rhs) const {
            return rhs == encode;
        }
        EncodingChar(const char* e, char d) : encode(e), decode(d) {}
    };

    static const EncodingChar EncodingToChar[] =
    {   EncodingChar("20", ' '), EncodingChar("21", '!'), EncodingChar("23", '#'),  EncodingChar("24", '$'),
        EncodingChar("25", '%'), EncodingChar("26", '&'), EncodingChar("27", '\''), EncodingChar("28", '('),
        EncodingChar("29", ')'), EncodingChar("25", '+'), EncodingChar("2B", '%'),  EncodingChar("2C", ','),
        EncodingChar("3B", ';'), EncodingChar("3D", '='), EncodingChar("40", '@'),  EncodingChar("5B", '['),
        EncodingChar("5D", ']'), EncodingChar("7B", '{'), EncodingChar("7D", '}')
    };

    static std::string EncodeFileName(std::string fileName)
    {
        std::ostringstream result;
        for (std::uint32_t position = 0; position < fileName.length(); ++position)
        {   std::uint8_t index = static_cast<std::uint8_t>(fileName[position]);
            if(fileName[position] < PercentangeEncodingTableSize && index < PercentangeEncoding.size() && PercentangeEncoding[index] != nullptr)
            {   result << PercentangeEncoding[index];
            }
            else if (fileName[position] == '\\') // Remove Windows file separator.
            {   result << '/';
            }
            else
            {   result << fileName[position];
            }
        }
        return result.str();
    }

    static std::string DecodeFileName(const std::string& fileName)
    {
        std::string result;
        for (std::uint32_t i = 0; i < fileName.length(); ++i)
        {   if(fileName[i] == '%')
            {   const auto& found = std::find(std::begin(EncodingToChar), std::end(EncodingToChar), fileName.substr(i+1, 2));
                ThrowErrorIf(Error::UnknownFileNameEncoding, (found == std::end(EncodingToChar)), fileName.c_str())
                result += found->decode;
                i += 2;
            }
            else
            {   result += fileName[i];
            }
        }
        return result;
    }

    static std::string Base32Encoding(const std::vector<uint8_t>& bytes)
    {
        static const size_t publisherIdSize = 13;
        static const size_t byteCount = 8;

        // Consider groups of five bytes.  This is the smallest number of bytes that has a number of bits
        // that's evenly divisible by five.
        // Every five bits starting with the most significant of the first byte are made into a base32 value.
        // Each value is used to index into the alphabet array to produce a base32 digit.
        // When out of bytes but the corresponding base32 value doesn't yet have five bits, 0 is used.
        // Normally in these cases a particular number of '=' characters are appended to the resulting base32
        // string to indicate how many bits didn't come from the actual byte value.  For our purposes no
        // such padding characters are necessary.
        //
        // Bytes:         aaaaaaaa  bbbbbbbb  cccccccc  dddddddd  eeeeeeee
        // Base32 Values: 000aaaaa  000aaabb  000bbbbb  000bcccc  000ccccd  000ddddd 000ddeee 000eeeee
        //
        // Combo of byte    a & F8    a & 07    b & 3E    b & 01    c & 0F    d & 7C   d & 03   e & 1F
        // values except              b & C0              c & F0    d & 80             e & E0
        // for shifting

        // Make sure the following math doesn't overflow.
        char output[publisherIdSize+1] = "";
        size_t outputIndex = 0;
        for(size_t byteIndex = 0; byteIndex < byteCount; byteIndex +=5)
        {
            uint8_t firstByte = bytes[byteIndex];
            uint8_t secondByte = (byteIndex + 1) < byteCount ? bytes[byteIndex + 1] : 0;
            output[outputIndex++] = base32DigitList[(firstByte & 0xF8) >> 3];
            output[outputIndex++] = base32DigitList[((firstByte & 0x07) << 2) | ((secondByte & 0xC0) >> 6)];

            if(byteIndex + 1 < byteCount)
            {
                uint8_t thirdByte = (byteIndex + 2) < byteCount ? bytes[byteIndex + 2] : 0;
                output[outputIndex++] = base32DigitList[(secondByte & 0x3E) >> 1];
                output[outputIndex++] = base32DigitList[((secondByte & 0x01) << 4) | ((thirdByte & 0xF0) >> 4)];

                if(byteIndex + 2 < byteCount)
                {
                    uint8_t fourthByte = (byteIndex + 3) < byteCount ? bytes[byteIndex + 3] : 0;
                    output[outputIndex++] = base32DigitList[((thirdByte & 0x0F) << 1) | ((fourthByte & 0x80) >> 7)];

                    if (byteIndex + 3 < byteCount)
                    {
                        uint8_t fifthByte = (byteIndex + 4) < byteCount ? bytes[byteIndex + 4] : 0;
                        output[outputIndex++] = base32DigitList[(fourthByte & 0x7C) >> 2];
                        output[outputIndex++] = base32DigitList[((fourthByte & 0x03) << 3) | ((fifthByte & 0xE0) >> 5)];

                        if (byteIndex + 4 < byteCount)
                        {
                            output[outputIndex++] = base32DigitList[fifthByte & 0x1F];
                        }
                    }
                }
            }
        }
        output[publisherIdSize] = '\0';
        return std::string(output);
    }

    std::string ComputePublisherId(const std::string& publisher)
    {
        auto wpublisher = utf8_to_u16string(publisher);
        std::vector<std::uint8_t> buffer(wpublisher.size() * sizeof(char16_t));
        memcpy(buffer.data(), &wpublisher[0], wpublisher.size() * sizeof(char16_t));

        std::vector<std::uint8_t> hash;
        ThrowErrorIfNot(Error::Unexpected, SHA256::ComputeHash(buffer.data(), buffer.size(), hash),  "Failed computing publisherId");
        return Base32Encoding(hash);
    }

    AppxManifestObject::AppxManifestObject(IXmlFactory* factory, const ComPtr<IStream>& stream) : m_stream(stream)
    {
        auto dom = factory->CreateDomFromStream(XmlContentType::AppxManifestXml, stream);
        XmlVisitor visitor(static_cast<void*>(this), [](void* s, const ComPtr<IXmlElement>& identityNode)->bool
        {
            AppxManifestObject* self = reinterpret_cast<AppxManifestObject*>(s);
            ThrowErrorIf(Error::AppxManifestSemanticError, (nullptr != self->m_packageId), "There must be only one Identity element at most in AppxManifest.xml");

            const auto& name           = identityNode->GetAttributeValue(XmlAttributeName::Name);
            const auto& architecture   = identityNode->GetAttributeValue(XmlAttributeName::Identity_ProcessorArchitecture);
            const auto& publisher      = identityNode->GetAttributeValue(XmlAttributeName::Identity_Publisher);
            const auto& version        = identityNode->GetAttributeValue(XmlAttributeName::Version);
            const auto& resourceId     = identityNode->GetAttributeValue(XmlAttributeName::ResourceId);
            ThrowErrorIf(Error::AppxManifestSemanticError, (publisher.empty()), "Invalid Identity element");
            auto publisherId = ComputePublisherId(publisher);
            self->m_packageId = std::make_unique<AppxPackageId>(name, version, resourceId, architecture, publisherId);
            return true;
        });
        dom->ForEachElementIn(dom->GetDocument(), XmlQueryName::Package_Identity, visitor);
        // Have to check for this semantically as not all validating parsers can validate this via schema
        ThrowErrorIfNot(Error::AppxManifestSemanticError, m_packageId, "No Identity element in AppxManifest.xml");

        XmlVisitor visitorTDF(static_cast<void*>(this), [](void* s, const ComPtr<IXmlElement>& tdfNode)->bool
        {
            AppxManifestObject* self = reinterpret_cast<AppxManifestObject*>(s);
            auto& name = tdfNode->GetAttributeValue(XmlAttributeName::Name);
            std::transform(name.begin(), name.end(), name.begin(), ::tolower);
            const auto& tdfEntry = std::find(std::begin(targetDeviceFamilyList), std::end(targetDeviceFamilyList), name.c_str());
            ThrowErrorIf(Error::AppxManifestSemanticError, (tdfEntry == std::end(targetDeviceFamilyList)), "Unrecognized TargetDeviceFamily");
            self->m_platform = static_cast<MSIX_PLATFORMS>(self->m_platform | (*tdfEntry).platform);
            return true;
        });
        dom->ForEachElementIn(dom->GetDocument(), XmlQueryName::Package_Dependencies_TargetDeviceFamily, visitorTDF);
        ThrowErrorIf(Error::AppxManifestSemanticError, m_platform == MSIX_PLATFORM_NONE , "Couldn't find TargetDeviceFamily element in AppxManifest.xml");
    }

    AppxBundleManifestObject::AppxBundleManifestObject(IXmlFactory* factory, const ComPtr<IStream>& stream) : m_stream(stream)
    {
        auto dom = factory->CreateDomFromStream(XmlContentType::AppxBundleManifestXml, stream);
        XmlVisitor visitorIdentity(static_cast<void*>(this), [](void* s, const ComPtr<IXmlElement>& identityNode)->bool
        {
            AppxBundleManifestObject* self = reinterpret_cast<AppxBundleManifestObject*>(s);
            ThrowErrorIf(Error::AppxManifestSemanticError, (nullptr != self->m_packageId), "There must be only one Identity element at most in AppxBundleManifest.xml");

            const auto& name           = identityNode->GetAttributeValue(XmlAttributeName::Name);
            const auto& publisher      = identityNode->GetAttributeValue(XmlAttributeName::Identity_Publisher);
            const auto& version        = identityNode->GetAttributeValue(XmlAttributeName::Version);
            ThrowErrorIf(Error::AppxManifestSemanticError, (publisher.empty()), "Invalid Identity element");
            auto publisherId = ComputePublisherId(publisher);
            self->m_packageId = std::make_unique<AppxPackageId>(name, version, "", "", publisherId);
            return true;
        });
        dom->ForEachElementIn(dom->GetDocument(), XmlQueryName::Bundle_Identity, visitorIdentity);

        struct _context
        {
            AppxBundleManifestObject* self;
            IXmlDom*                  dom;
            std::vector<std::string>  packageNames;
            size_t                    mainPackages;
        };
        _context context = { this, dom.Get(), {} , 0};

        XmlVisitor visitorPackages(static_cast<void*>(&context), [](void* c, const ComPtr<IXmlElement>& packageNode)->bool
        {
            _context* context = reinterpret_cast<_context*>(c);
            const auto& name = packageNode->GetAttributeValue(XmlAttributeName::Bundle_Package_FileName);

            std::ostringstream builder;
            builder << "Duplicate file: '" << name << "' specified in AppxBundleManifest.xml.";
            ThrowErrorIf(Error::AppxManifestSemanticError,
                std::find(std::begin(context->packageNames), std::end(context->packageNames), name) != context->packageNames.end(),
                builder.str().c_str());
            context->packageNames.push_back(name);

            const auto& version        = packageNode->GetAttributeValue(XmlAttributeName::Version);
            const auto& resourceId     = packageNode->GetAttributeValue(XmlAttributeName::ResourceId);
            const auto& architecture   = packageNode->GetAttributeValue(XmlAttributeName::Bundle_Package_Architecture);
            const auto& type           = packageNode->GetAttributeValue(XmlAttributeName::Bundle_Package_Type);
            const auto size            = GetNumber<std::uint64_t>(packageNode, XmlAttributeName::Size, 0);
            const auto offset          = GetNumber<std::uint64_t>(packageNode, XmlAttributeName::Bundle_Package_Offset, 0);

            bool isResourcePackage = (type.empty() || type == "resource"); // default value is resource
            auto package = std::make_unique<AppxPackageInBundle>(
                name, context->self->m_packageId.get()->Name, version, size, offset, resourceId,
                architecture, context->self->m_packageId.get()->PublisherId, isResourcePackage);

            std::set<std::string> languages;
            XmlVisitor visitor(static_cast<void*>(&languages), [](void* l, const ComPtr<IXmlElement>& resourceNode)->bool
            {
                std::set<std::string>* languages = reinterpret_cast<std::set<std::string>*>(l);
                const auto& language = resourceNode->GetAttributeValue(XmlAttributeName::Bundle_Package_Resources_Resource_Language);
                if (!language.empty()) { languages->insert(language); }
                return true;
            });
            context->dom->ForEachElementIn(packageNode, XmlQueryName::Bundle_Packages_Package_Resources_Resource, visitor);
            package->Languages = std::move(languages);

            if (isResourcePackage)
            {   // For now, we only support languages resource packages
                if(!package->Languages.empty())
                {
                    context->self->m_packages.push_back(std::move(package));
                }
            }
            else
            {
                context->self->m_packages.push_back(std::move(package));
                context->mainPackages++;
            }
            return true;
        });
        dom->ForEachElementIn(dom->GetDocument(), XmlQueryName::Bundle_Packages_Package, visitorPackages);
        ThrowErrorIf(Error::XmlError, (context.packageNames.size() == 0), "No packages in AppxBundleManifest.xml");
        ThrowErrorIf(Error::AppxManifestSemanticError, (context.mainPackages == 0), "Bundle contains only resource packages");
    }

    AppxPackageObject::AppxPackageObject(IMSIXFactory* factory, MSIX_VALIDATION_OPTION validation, const ComPtr<IStorageObject>& container) :
        m_factory(factory),
        m_validation(validation),
        m_container(container)
    {
        ComPtr<IXmlFactory> xmlFactory;
        ThrowHrIfFailed(factory->QueryInterface(UuidOfImpl<IXmlFactory>::iid, reinterpret_cast<void**>(&xmlFactory)));

        // 1. Get the appx signature from the container and parse it
        // TODO: pass validation flags and other necessary goodness through.
        auto file = m_container->GetFile(APPXSIGNATURE_P7X);
        if ((validation & MSIX_VALIDATION_OPTION_SKIPSIGNATURE) == 0)
        {   ThrowErrorIfNot(Error::MissingAppxSignatureP7X, file, "AppxSignature.p7x not in archive!");
        }

        m_appxSignature = ComPtr<IVerifierObject>::Make<AppxSignatureObject>(factory, validation, file);

        // 2. Get content type using signature object for validation
        file = m_container->GetFile(CONTENT_TYPES_XML);
        ThrowErrorIfNot(Error::MissingContentTypesXML, file, "[Content_Types].xml not in archive!");
        ComPtr<IStream> stream = m_appxSignature->GetValidationStream(CONTENT_TYPES_XML, file);
        auto contentType = xmlFactory->CreateDomFromStream(XmlContentType::ContentTypeXml, stream);

        // 3. Get blockmap object using signature object for validation
        file = m_container->GetFile(APPXBLOCKMAP_XML);
        ThrowErrorIfNot(Error::MissingAppxBlockMapXML, file, "AppxBlockMap.xml not in archive!");
        stream = m_appxSignature->GetValidationStream(APPXBLOCKMAP_XML, file);
        m_appxBlockMap = ComPtr<IVerifierObject>::Make<AppxBlockMapObject>(factory, stream);

        // 4. Get manifest object using blockmap object for validation
        // TODO: pass validation flags and other necessary goodness through.
        auto appxManifestInContainer = m_container->GetFile(APPXMANIFEST_XML);
        auto appxBundleManifestInContainer = m_container->GetFile(APPXBUNDLEMANIFEST_XML);

        ThrowErrorIfNot(Error::MissingAppxManifestXML, (appxManifestInContainer || appxBundleManifestInContainer) ,
            "AppxManifest.xml or AppxBundleManifest.xml not in archive!");
        ThrowErrorIf(Error::MissingAppxManifestXML, (appxManifestInContainer && appxBundleManifestInContainer) ,
            "AppxManifest.xml and AppxBundleManifest.xml in archive!");
        // We already validate that there's at least one and not both
        if(appxManifestInContainer)
        {
            stream = m_appxBlockMap->GetValidationStream(APPXMANIFEST_XML, appxManifestInContainer);
            m_appxManifest = ComPtr<IVerifierObject>::Make<AppxManifestObject>(xmlFactory.Get(), stream);
        }
        else
        {
            std::string pathInWindows(APPXBUNDLEMANIFEST_XML);
            std::replace(pathInWindows.begin(), pathInWindows.end(), '/', '\\');
            stream = m_appxBlockMap->GetValidationStream(pathInWindows, appxBundleManifestInContainer);
            m_appxBundleManifest = ComPtr<IVerifierObject>::Make<AppxBundleManifestObject>(xmlFactory.Get(), stream);
            m_isBundle = true;
        }

        if ((m_validation & MSIX_VALIDATION_OPTION_SKIPSIGNATURE) == 0)
        {
            auto publisherFromSignature = ComputePublisherId(m_appxSignature->GetPublisher());
            auto publisherFromManifest = (m_isBundle) ? m_appxBundleManifest->GetPublisher() : m_appxManifest->GetPublisher();
            std::string reason = "Publisher mismatch: '" + publisherFromManifest + "' != '" + publisherFromSignature + "'";
            ThrowErrorIfNot(Error::PublisherMismatch,
                    (0 == publisherFromManifest.compare(publisherFromSignature)), reason.c_str());
        }

        struct Config
        {
            typedef ComPtr<IStream> (*lambda)(AppxPackageObject* self);
            Config(const char* n, lambda f) : GetValidationStream(f), Name(n) {}

            const char* Name;
            lambda GetValidationStream;

            bool operator==(const std::string& rhs) const {
                return rhs == Name;
            }
        };

        static const Config footPrintFileNames[] = {
            Config(APPXBLOCKMAP_XML,       [](AppxPackageObject* self){ self->m_footprintFiles.push_back(APPXBLOCKMAP_XML);  return self->m_appxBlockMap->GetStream();}),
            Config(APPXMANIFEST_XML,       [](AppxPackageObject* self){ self->m_footprintFiles.push_back(APPXMANIFEST_XML);  return self->m_appxManifest->GetStream();}),
            Config(APPXSIGNATURE_P7X,      [](AppxPackageObject* self){ if (self->m_appxSignature->HasStream()){self->m_footprintFiles.push_back(APPXSIGNATURE_P7X);} return self->m_appxSignature->GetStream();}),
            Config(CODEINTEGRITY_CAT,      [](AppxPackageObject* self){ self->m_footprintFiles.push_back(CODEINTEGRITY_CAT); auto file = self->m_container->GetFile(CODEINTEGRITY_CAT); return self->m_appxSignature->GetValidationStream(CODEINTEGRITY_CAT, file);}),
            Config(CONTENT_TYPES_XML,      [](AppxPackageObject*)->ComPtr<IStream>{ return ComPtr<IStream>();}), // content types is never implicitly unpacked
            Config(APPXBUNDLEMANIFEST_XML, [](AppxPackageObject* self){ self->m_footprintFiles.push_back(APPXBUNDLEMANIFEST_XML); return self->m_appxBundleManifest->GetStream();}),
        };

        // 5. Ensure that the stream collection contains streams wired up for their appropriate validation
        // and partition the container's file names into footprint and payload files.  First by going through
        // the footprint files, and then by going through the payload files.
        auto filesToProcess = m_container->GetFileNames(FileNameOptions::All);
        for (const auto& fileName : m_container->GetFileNames(FileNameOptions::FootPrintOnly))
        {   auto footPrintFile = std::find(std::begin(footPrintFileNames), std::end(footPrintFileNames), fileName);
            if (footPrintFile != std::end(footPrintFileNames))
            {   m_streams[fileName] = footPrintFile->GetValidationStream(this);
                filesToProcess.erase(std::remove(filesToProcess.begin(), filesToProcess.end(), fileName), filesToProcess.end());
            }
        }

        auto blockMapInternal = m_appxBlockMap.As<IAppxBlockMapInternal>();
        auto blockMapFiles = blockMapInternal->GetFileNames();
        if(m_isBundle)
        {
            // There should only be one file in the blockmap for bundles. We validate that the block map contains
            // AppxMetadata/AppxBundleManifest.xml before, so just check the size.
            ThrowErrorIfNot(Error::BlockMapSemanticError, ((blockMapFiles.size() == 1)), "Block map contains invalid files.");

            auto bundleInfo = m_appxBundleManifest.As<IBundleInfo>();
            auto appxFactory = m_factory.As<IAppxFactory>();
            for (const auto& package : bundleInfo->GetPackages())
            {
                auto packageName = package.get()->FileName;
                auto fileStream = m_container->GetFile(packageName);
                ThrowErrorIfNot(Error::FileNotFound, fileStream, "Package is not in container"); // This will change when we support flat bundles
                auto appxFile = fileStream.As<IAppxFile>();
                APPX_COMPRESSION_OPTION compressionOpt;
                ThrowHrIfFailed(appxFile->GetCompressionOption(&compressionOpt));
                ThrowErrorIf(Error::AppxManifestSemanticError, (compressionOpt != APPX_COMPRESSION_OPTION_NONE), "Packages cannot be compressed");
                ComPtr<IAppxFileInternal> appxFileInternal = fileStream.As<IAppxFileInternal>();
                ThrowErrorIf(Error::AppxManifestSemanticError, (appxFileInternal->GetCompressedSize() != package.get()->Size), "Size mistmach of package between AppxManifestBundle.appx and container");
                ComPtr<IAppxPackageReader> reader;
                ThrowHrIfFailed(appxFactory->CreatePackageReader(fileStream.Get(), &reader));
                auto appxPackage = reader.As<IPackage>();
                auto appxManifest = appxPackage->GetAppxManifestObject();
                ThrowErrorIfNot(Error::Unexpected, appxManifest, "Error getting the AppxManifest object"); // is this even possible?
                ThrowErrorIf(Error::AppxManifestSemanticError,
                    (appxManifest->GetPublisher() != package.get()->PackageId->PublisherId),
                    "AppxBundleManifest.xml and AppxManifest.xml publisher mismatch");
                ThrowErrorIf(Error::AppxManifestSemanticError,
                    (appxManifest->GetVersion() != package.get()->PackageId->Version),
                    "AppxBundleManifest.xml and AppxManifest.xml version mismatch");
                ThrowErrorIf(Error::AppxManifestSemanticError,
                    (appxManifest->GetName() != package.get()->PackageId->Name),
                    "AppxBundleManifest.xml and AppxManifest.xml name mismatch");
                ThrowErrorIf(Error::AppxManifestSemanticError,
                    (appxManifest->GetArchitecture() != package.get()->PackageId->Architecture) &&
                    !(appxManifest->GetArchitecture().empty() && (package.get()->PackageId->Architecture == "neutral")),
                    "AppxBundleManifest.xml and AppxManifest.xml architecture mismatch");
                m_payloadPackagesNames.push_back(packageName);
                m_payloadPackages.push_back(std::move(reader)); // TODO: add only applicable packages to m_payloadPackages
                m_streams[packageName] = std::move(fileStream);
                // Intentionally don't remove from fileToProcess. For bundles, it is possible to don't unpack packages, like
                // resource packages that are not languages packages.
            }
        }
        else
        {
            for (const auto& fileName : blockMapFiles)
            {   auto footPrintFile = std::find(std::begin(footPrintFileNames), std::end(footPrintFileNames), fileName);
                if (footPrintFile == std::end(footPrintFileNames))
                {   std::string containerFileName = EncodeFileName(fileName);
                    m_payloadFiles.push_back(containerFileName);
                    auto fileStream = m_container->GetFile(containerFileName);
                    ThrowErrorIfNot(Error::FileNotFound, fileStream, "File described in blockmap not contained in OPC container");
                    VerifyFile(fileStream, fileName, blockMapInternal);
                    m_streams[containerFileName] = m_appxBlockMap->GetValidationStream(fileName, fileStream);
                    filesToProcess.erase(std::remove(filesToProcess.begin(), filesToProcess.end(), containerFileName), filesToProcess.end());
                }
            }

            // If the map is not empty, there's a file in the container that didn't go to the footprint or payload
            // files. (eg. payload file missing in the AppxBlockMap.xml)
            ThrowErrorIfNot(Error::BlockMapSemanticError, (filesToProcess.empty()), "Payload file not described in AppxBlockMap.xml");
        }
    }

    // Verify file in OPC and BlockMap
    void AppxPackageObject::VerifyFile(const ComPtr<IStream>& stream, const std::string& fileName, const ComPtr<IAppxBlockMapInternal>& blockMapInternal)
    {
        ComPtr<IAppxFile> appxFile = stream.As<IAppxFile>();;
        APPX_COMPRESSION_OPTION compressionOpt;
        ThrowHrIfFailed(appxFile->GetCompressionOption(&compressionOpt));
        bool isUncompressed = (compressionOpt == APPX_COMPRESSION_OPTION_NONE);

        ComPtr<IAppxFileInternal> appxFileInternal = stream.As<IAppxFileInternal>();
        auto sizeOnZip = appxFileInternal->GetCompressedSize();

        auto blocks = blockMapInternal->GetBlocks(fileName);
        std::uint64_t blocksSize = 0;
        for(auto& block : blocks)
        {   // For Block elements that don't have a Size attribute, we always set its size as BLOCKMAP_BLOCK_SIZE
            // (even for the last one). The Size attribute isn't specified if the file is not compressed.
            ThrowErrorIf(Error::BlockMapSemanticError, isUncompressed && (block.compressedSize != BLOCKMAP_BLOCK_SIZE),
                "An uncompressed file has a size attribute in its Block elements");
            blocksSize += block.compressedSize;
        }

        if(isUncompressed)
        {
            UINT64 blockMapFileSize;
            auto blockMapFile = blockMapInternal->GetFile(fileName);
            ThrowHrIfFailed(blockMapFile->GetUncompressedSize(&blockMapFileSize));
            ThrowErrorIf(Error::BlockMapSemanticError, (blockMapFileSize != sizeOnZip ),
                "Uncompressed size of the file in the block map and the OPC container don't match");
        }
        else
        {
            // From Windows code:
            // The file item is compressed. There are 2 cases here:
            // 1. The compressed size of the file is the same as the total size of all compressed blocks.
            // 2. The compressed size of the file is 2 bytes more than the total size of all compressed blocks.
            // It depends on how the block compression is done. MakeAppx block compression implementation will end up
            // with case 2. However, we shouldn't block the first case since it is totally valid and 3rd party
            // implementation may end up with it.
            // The reason we created compressed file item with 2 extra bytes (03 00) is because we use Z_FULL_FLUSH
            // flag to compress every block. If we use Z_FINISH flag to compress the last block, these 2 extra bytes will
            // not be generated. The AddBlock()-->... -->AddBlock()-->Close() pattern in OPC push stack prevents the
            // deflator from knowing whether the current block is the last block. So it cannot use Z_FINISH flag for
            // the last block of the file. Note that removing the 2 extra bytes from the compressed file data will make
            // it invalid when consumed by popular zip tools like WinZip and ShellZip. So they are required for the
            // packages we created.
            ThrowErrorIfNot(Error::BlockMapSemanticError,
                (blocksSize == sizeOnZip ) // case 1
                || (blocksSize == sizeOnZip - 2), // case 2
                "Compressed size of the file in the block map and the OPC container don't match");
        }
    }

    void AppxPackageObject::Unpack(MSIX_PACKUNPACK_OPTION options, const ComPtr<IStorageObject>& to)
    {
        auto fileNames = GetFileNames(FileNameOptions::All);
        for (const auto& fileName : fileNames)
        {   // Don't extract packages files
            auto file = std::find(std::begin(m_payloadPackagesNames), std::end(m_payloadPackagesNames), fileName);
            if (file == std::end(m_payloadPackagesNames))
            {
                std::string targetName;
                if (options & MSIX_PACKUNPACK_OPTION_CREATEPACKAGESUBFOLDER)
                {   // Don't use to->GetPathSeparator(). DirectoryObject::OpenFile created directories
                    // by looking at "/" in the string. If to->GetPathSeparator() is used the subfolder with
                    // the package full name won't be created on Windows, but it will on other platforms.
                    // This means that we have different behaviors in non-Win platforms.
                    // TODO: have the same behavior on Windows and other platforms.
                    targetName = m_appxManifest.As<IAppxManifestObject>()->GetPackageFullName() + "/" + fileName;
                }
                else
                {   targetName = DecodeFileName(fileName);
                }

                auto targetFile = to->OpenFile(targetName, MSIX::FileStream::Mode::WRITE_UPDATE);
                auto sourceFile = GetFile(fileName);

                ULARGE_INTEGER bytesCount = {0};
                bytesCount.QuadPart = std::numeric_limits<std::uint64_t>::max();
                ThrowHrIfFailed(sourceFile->CopyTo(targetFile.Get(), bytesCount, nullptr, nullptr));
            }
        }
        if(m_isBundle)
        {
            for(const auto& appx : m_payloadPackages)
            {
                appx.As<IPackage>()->Unpack(
                    static_cast<MSIX_PACKUNPACK_OPTION>(options | MSIX_PACKUNPACK_OPTION_CREATEPACKAGESUBFOLDER), to.Get());
            }
        }
    }

    const char* AppxPackageObject::GetPathSeparator() { return "/"; }

    std::vector<std::string> AppxPackageObject::GetFileNames(FileNameOptions options)
    {
        std::vector<std::string> result;

        if ((options & FileNameOptions::FootPrintOnly) == FileNameOptions::FootPrintOnly)
        {
            result.insert(result.end(), m_footprintFiles.begin(), m_footprintFiles.end());
        }
        if ((options & FileNameOptions::PayloadOnly) == FileNameOptions::PayloadOnly)
        {
            if (m_isBundle)
            {
                result.insert(result.end(), m_payloadPackagesNames.begin(), m_payloadPackagesNames.end());
            }
            else
            {
                result.insert(result.end(), m_payloadFiles.begin(), m_payloadFiles.end());
            }
        }
        return result;
    }

    ComPtr<IStream> AppxPackageObject::GetFile(const std::string& fileName)
    {
        auto result = m_streams.find(fileName);
        if (result == m_streams.end())
        {
            return ComPtr<IStream>();
        }
        return result->second;
    }

    ComPtr<IStream> AppxPackageObject::OpenFile(const std::string& fileName, MSIX::FileStream::Mode mode) { NOTIMPLEMENTED; }

    // IAppxPackageReader
    HRESULT STDMETHODCALLTYPE AppxPackageObject::GetBlockMap(IAppxBlockMapReader** blockMapReader) noexcept
    {
        return static_cast<HRESULT>(Error::NotImplemented);
    }

    HRESULT STDMETHODCALLTYPE AppxPackageObject::GetFootprintFile(APPX_FOOTPRINT_FILE_TYPE type, IAppxFile** file) noexcept try
    {
        if (m_isBundle) { return static_cast<HRESULT>(Error::PackageIsBundle); }
        ThrowErrorIf(Error::InvalidParameter, (file == nullptr || *file != nullptr), "bad pointer");
        ThrowErrorIf(Error::FileNotFound, (static_cast<size_t>(type) > footprintFiles.size()), "unknown footprint file type");
        std::string footprint (footprintFiles[type]);
        ComPtr<IStream> stream = GetFile(footprint);
        ThrowErrorIfNot(Error::FileNotFound, stream, "requested footprint file not in package")
        // Clients expect the stream's pointer to be at the start of the file!
        ThrowHrIfFailed(stream->Seek({0}, StreamBase::Reference::START, nullptr));
        auto result = stream.As<IAppxFile>();
        *file = result.Detach();
        return static_cast<HRESULT>(Error::OK);
    } CATCH_RETURN();

    HRESULT STDMETHODCALLTYPE AppxPackageObject::GetPayloadFile(LPCWSTR fileName, IAppxFile** file) noexcept try
    {
        if (m_isBundle) { return static_cast<HRESULT>(Error::PackageIsBundle); }
        ThrowErrorIf(Error::InvalidParameter, (fileName == nullptr || file == nullptr || *file != nullptr), "bad pointer");
        std::string name = utf16_to_utf8(fileName);
        ComPtr<IStream> stream = GetFile(name);
        ThrowErrorIfNot(Error::FileNotFound, stream, "requested file not in package")
        // Clients expect the stream's pointer to be at the start of the file!
        ThrowHrIfFailed(stream->Seek({0}, StreamBase::Reference::START, nullptr));
        auto result = stream.As<IAppxFile>();
        *file = result.Detach();
        return static_cast<HRESULT>(Error::OK);
    } CATCH_RETURN();

    HRESULT STDMETHODCALLTYPE AppxPackageObject::GetPayloadFiles(IAppxFilesEnumerator** filesEnumerator) noexcept try
    {
        if (m_isBundle) { return static_cast<HRESULT>(Error::PackageIsBundle); }
        ThrowErrorIf(Error::InvalidParameter,(filesEnumerator == nullptr || *filesEnumerator != nullptr), "bad pointer");

        ComPtr<IStorageObject> storage;
        ThrowHrIfFailed(QueryInterface(UuidOfImpl<IStorageObject>::iid, reinterpret_cast<void**>(&storage)));
        auto result = ComPtr<IAppxFilesEnumerator>::Make<AppxFilesEnumerator>(storage.Get());
        *filesEnumerator = result.Detach();
        return static_cast<HRESULT>(Error::OK);
    } CATCH_RETURN();

    HRESULT STDMETHODCALLTYPE AppxPackageObject::GetManifest(IAppxManifestReader** manifestReader) noexcept
    {
        return static_cast<HRESULT>(Error::NotImplemented);
    }

    // IAppxBundleReader
    HRESULT STDMETHODCALLTYPE AppxPackageObject::GetFootprintFile(APPX_BUNDLE_FOOTPRINT_FILE_TYPE fileType, IAppxFile **footprintFile) noexcept try
    {
        if (!m_isBundle) { return static_cast<HRESULT>(Error::NotImplemented); }
        ThrowErrorIf(Error::InvalidParameter, (footprintFile == nullptr || *footprintFile != nullptr), "bad pointer");
        ThrowErrorIf(Error::FileNotFound, (static_cast<size_t>(fileType) > bundleFootprintFiles.size()), "unknown footprint file type");
        std::string footprint (bundleFootprintFiles[fileType]);
        ComPtr<IStream> stream = GetFile(footprint);
        ThrowErrorIfNot(Error::FileNotFound, stream, "requested footprint file not in bundle")
        // Clients expect the stream's pointer to be at the start of the file!
        ThrowHrIfFailed(stream->Seek({0}, StreamBase::Reference::START, nullptr));
        auto result = stream.As<IAppxFile>();
        *footprintFile = result.Detach();
        return static_cast<HRESULT>(Error::OK);
    } CATCH_RETURN();

    HRESULT STDMETHODCALLTYPE AppxPackageObject::GetPayloadPackages(IAppxFilesEnumerator **payloadPackages) noexcept try
    {
        if (!m_isBundle) { return static_cast<HRESULT>(Error::NotImplemented); }
        ThrowErrorIf(Error::InvalidParameter,(payloadPackages == nullptr || *payloadPackages != nullptr), "bad pointer");
        ComPtr<IStorageObject> storage;
        ThrowHrIfFailed(QueryInterface(UuidOfImpl<IStorageObject>::iid, reinterpret_cast<void**>(&storage)));
        auto result = ComPtr<IAppxFilesEnumerator>::Make<AppxFilesEnumerator>(storage.Get());
        *payloadPackages = result.Detach();
        return static_cast<HRESULT>(Error::OK);
    } CATCH_RETURN();

    HRESULT STDMETHODCALLTYPE AppxPackageObject::GetPayloadPackage(LPCWSTR fileName, IAppxFile **payloadPackage) noexcept try
    {
        if (!m_isBundle) { return static_cast<HRESULT>(Error::NotImplemented); }
        ThrowErrorIf(Error::InvalidParameter, (fileName == nullptr || payloadPackage == nullptr || *payloadPackage != nullptr), "bad pointer");
        std::string name = utf16_to_utf8(fileName);
        ComPtr<IStream> stream = GetFile(name);
        ThrowErrorIfNot(Error::FileNotFound, stream, "requested package not in bundle")
        // Clients expect the stream's pointer to be at the start of the file!
        ThrowHrIfFailed(stream->Seek({0}, StreamBase::Reference::START, nullptr));
        auto result = stream.As<IAppxFile>();
        *payloadPackage = result.Detach();
        return static_cast<HRESULT>(Error::OK);
    } CATCH_RETURN();

    HRESULT STDMETHODCALLTYPE AppxPackageObject::GetManifest(IAppxBundleManifestReader **manifestReader) noexcept
    {
        return static_cast<HRESULT>(Error::NotImplemented);
    }
}
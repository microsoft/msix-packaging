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
#include "AppxManifestObject.hpp"
#include "Encoding.hpp"
#include "Enumerators.hpp"

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

    AppxPackageObject::AppxPackageObject(IMSIXFactory* factory, MSIX_VALIDATION_OPTION validation, MSIX_APPLICABILITY_OPTIONS applicability, const ComPtr<IStorageObject>& container) :
        m_factory(factory),
        m_validation(validation),
        m_applicability(applicability),
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
            m_appxManifest = ComPtr<IVerifierObject>::Make<AppxManifestObject>(factory, stream);
        }
        else
        {
            std::string pathInWindows(APPXBUNDLEMANIFEST_XML);
            std::replace(pathInWindows.begin(), pathInWindows.end(), '/', '\\');
            stream = m_appxBlockMap->GetValidationStream(pathInWindows, appxBundleManifestInContainer);
            m_appxBundleManifest = ComPtr<IVerifierObject>::Make<AppxBundleManifestObject>(factory, stream);
            m_isBundle = true;
        }

        if ((m_validation & MSIX_VALIDATION_OPTION_SKIPSIGNATURE) == 0)
        {
            ComPtr<IAppxManifestPackageId> packageId;
            if (m_isBundle)
            {
                auto manifest = m_appxBundleManifest.As<IAppxBundleManifestReader>();
                ThrowHrIfFailed(manifest->GetPackageId(&packageId));
            }
            else
            {
                auto manifest = m_appxManifest.As<IAppxManifestReader>();
                ThrowHrIfFailed(manifest->GetPackageId(&packageId));
            }
            auto publisherFromSignature = m_appxSignature->GetPublisher();
            BOOL isSame = FALSE;
            ThrowHrIfFailed(packageId->ComparePublisher(
                reinterpret_cast<LPCWSTR>(utf8_to_wstring(publisherFromSignature).c_str()), &isSame));
            if(!isSame)
            {
                auto internal = packageId.As<IAppxManifestPackageIdInternal>();
                std::string reason = "Publisher mismatch: '" + internal->GetPublisher() + "' != '" + publisherFromSignature + "'";
                ThrowErrorAndLog(Error::PublisherMismatch, reason.c_str());
            }
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
            bool hasExactLanguageMatch = false;
            bool matchApplicationPackage = false;
            std::vector<std::pair<std::string, ComPtr<IAppxPackageReader>>> variantFormPackages;
            std::vector<std::pair<std::string, ComPtr<IAppxPackageReader>>> extraApplicationPackages;
            for (const auto& package : bundleInfo->GetPackages())
            {
                auto bundleInfoInternal = package.As<IAppxBundleManifestPackageInfoInternal>();
                auto packageName = bundleInfoInternal->GetFileName();
                auto fileStream = m_container->GetFile(packageName);
                ThrowErrorIfNot(Error::FileNotFound, fileStream, "Package is not in container"); // This will change when we support flat bundles

                // Do semantic checks
                auto appxFile = fileStream.As<IAppxFile>();
                APPX_COMPRESSION_OPTION compressionOpt;
                ThrowHrIfFailed(appxFile->GetCompressionOption(&compressionOpt));
                ThrowErrorIf(Error::AppxManifestSemanticError, (compressionOpt != APPX_COMPRESSION_OPTION_NONE), "Packages cannot be compressed");
                ComPtr<IAppxFileInternal> appxFileInternal = fileStream.As<IAppxFileInternal>();
                UINT64 size;
                ThrowHrIfFailed(package->GetSize(&size));
                ThrowErrorIf(Error::AppxManifestSemanticError, (appxFileInternal->GetCompressedSize()) != size,
                    "Size mistmach of package between AppxManifestBundle.appx and container");

                // Validate the package
                ComPtr<IAppxPackageReader> reader;
                ThrowHrIfFailed(appxFactory->CreatePackageReader(fileStream.Get(), &reader));
                ComPtr<IAppxManifestReader> innerPackageManifest;
                ThrowHrIfFailed(reader->GetManifest(&innerPackageManifest));
                // Do semantic checks to validate the relationship between the AppxBundleManifest and the AppxManifest.
                ComPtr<IAppxManifestPackageId> bundlePackageId;
                ThrowHrIfFailed(package->GetPackageId(&bundlePackageId));
                auto bundlePackageIdInternal = bundlePackageId.As<IAppxManifestPackageIdInternal>();

                ComPtr<IAppxManifestPackageId> innerPackageId;
                ThrowHrIfFailed(innerPackageManifest->GetPackageId(&innerPackageId));
                auto innerPackageIdInternal = innerPackageId.As<IAppxManifestPackageIdInternal>();
                ThrowErrorIf(Error::AppxManifestSemanticError,
                    (innerPackageIdInternal->GetPublisher() != bundlePackageIdInternal->GetPublisher()),
                    "AppxBundleManifest.xml and AppxManifest.xml publisher mismatch");
                UINT64 bundlePackageVersion = 0;
                UINT64 innerPackageVersion = 0;
                ThrowHrIfFailed(bundlePackageId->GetVersion(&bundlePackageVersion));
                ThrowHrIfFailed(innerPackageId->GetVersion(&innerPackageVersion));
                ThrowErrorIf(Error::AppxManifestSemanticError,
                    (innerPackageVersion != bundlePackageVersion),
                    "AppxBundleManifest.xml and AppxManifest.xml version mismatch");
                ThrowErrorIf(Error::AppxManifestSemanticError,
                    (innerPackageIdInternal->GetName() != bundlePackageIdInternal->GetName()),
                    "AppxBundleManifest.xml and AppxManifest.xml name mismatch");
                ThrowErrorIf(Error::AppxManifestSemanticError,
                    (innerPackageIdInternal->GetArchitecture() != bundlePackageIdInternal->GetArchitecture()) &&
                    !(innerPackageIdInternal->GetArchitecture().empty() && (bundlePackageIdInternal->GetArchitecture() == "neutral")),
                    "AppxBundleManifest.xml and AppxManifest.xml architecture mismatch");

                APPX_BUNDLE_PAYLOAD_PACKAGE_TYPE packageType;
                ThrowHrIfFailed(package->GetPackageType(&packageType));
                // Validation is done, now see if the package is applicable.

                MSIX_PLATFORMS platform = (m_applicability & MSIX_APPLICABILITY_OPTION_SKIPPLATFORM) ?
                    static_cast<MSIX_PLATFORMS>(MSIX_PLATFORM_ALL) : Applicability::GetPlatform();
                //if (innerPackageManifest->GetPlatform() & platform) // temporary disable until we have bundles for all platforms
                //{
                    if (m_applicability & MSIX_APPLICABILITY_OPTION_SKIPLANGUAGE)
                    {
                        m_applicablePackages.push_back(std::move(reader));
                        m_applicablePackagesNames.push_back(packageName);
                    }
                    else
                    {
                        bool hasMatch = false;
                        bool hasVariantMatch = false;
                        for (auto& systemLanguage : Applicability::GetLanguages())
                        {
                            for (auto& packageLanguage : bundleInfoInternal->GetLanguages())
                            {
                                auto closeness = systemLanguage.Compare(packageLanguage);
                                if (closeness == Bcp47ClosenessMeasure::ExactMatch)
                                {
                                    // If this is an exact match we can stop looking
                                    hasExactLanguageMatch = true;
                                    hasMatch = true;
                                    break;
                                }
                                else if ((closeness == Bcp47ClosenessMeasure::AnyMatchWithScript) || (closeness == Bcp47ClosenessMeasure::AnyMatch))
                                {   // matching und-* packages always get deployed
                                    hasMatch = true;
                                }
                                else if (closeness >= Bcp47ClosenessMeasure::LanguagesScriptsMatch)
                                {
                                    closeness = systemLanguage.CompareNeutral(packageLanguage);
                                    if (closeness == Bcp47ClosenessMeasure::ExactMatch)
                                    {
                                        hasMatch = true;
                                    }
                                    else
                                    {
                                        hasVariantMatch = true;
                                    }
                                }
                            }
                            // If we know this package is applicable there's no need to keep looking
                            // for other system languages
                            if (hasMatch)
                            {
                                m_applicablePackages.push_back(std::move(reader));
                                m_applicablePackagesNames.push_back(packageName);
                                break;
                            }
                            if (hasVariantMatch)
                            {
                                variantFormPackages.push_back(std::make_pair(packageName, std::move(reader)));
                                break;
                            }
                        }
                        if (packageType == APPX_BUNDLE_PAYLOAD_PACKAGE_TYPE_APPLICATION)
                        {
                            if (!hasMatch && !hasVariantMatch)
                            {   // If we are here the package is an application package that targets the 
                                // current platform, but it doesn't contain a language match. Save it just in case
                                // there are no application packages that match, so we don't end up only with resources.
                                extraApplicationPackages.push_back(std::make_pair(packageName, std::move(reader)));
                            }
                            else
                            {
                                matchApplicationPackage = true;
                            }
                        }
                    }
                //}
                m_streams[packageName] = std::move(fileStream);
                // Intentionally don't remove from fileToProcess. For bundles, it is possible to don't unpack packages, like
                // resource packages that are not languages packages.
            }

            if (!(m_validation & MSIX_APPLICABILITY_OPTION_SKIPLANGUAGE))
            {
                // If we don't have an exact match, we have to add all of the variants, too.
                if (!hasExactLanguageMatch)
                {
                    for(auto& variantFormPackage : variantFormPackages)
                    {
                        m_applicablePackages.push_back(std::move(variantFormPackage.second));
                        m_applicablePackagesNames.push_back(variantFormPackage.first);
                    }
                }
                // If we don't have an application package add the ones we have
                if (!matchApplicationPackage)
                {
                    for(auto& applicationPackage : extraApplicationPackages)
                    {
                        m_applicablePackages.push_back(std::move(applicationPackage.second));
                        m_applicablePackagesNames.push_back(applicationPackage.first);
                    }
                }
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
            ThrowErrorIf(Error::BlockMapSemanticError, isUncompressed && (block.blockSize != BLOCKMAP_BLOCK_SIZE),
                "An uncompressed file has a size attribute in its Block elements");
            blocksSize += block.blockSize;
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
            auto file = std::find(std::begin(m_applicablePackagesNames), std::end(m_applicablePackagesNames), fileName);
            if (file == std::end(m_applicablePackagesNames))
            {
                std::string targetName;
                if (options & MSIX_PACKUNPACK_OPTION_CREATEPACKAGESUBFOLDER)
                {   // Don't use to->GetPathSeparator(). DirectoryObject::OpenFile created directories
                    // by looking at "/" in the string. If to->GetPathSeparator() is used the subfolder with
                    // the package full name won't be created on Windows, but it will on other platforms.
                    // This means that we have different behaviors in non-Win platforms.
                    // TODO: have the same behavior on Windows and other platforms.
                    auto manifest = m_appxManifest.As<IAppxManifestReader>();
                    ComPtr<IAppxManifestPackageId> packageId;
                    ThrowHrIfFailed(manifest->GetPackageId(&packageId));
                    targetName = packageId.As<IAppxManifestPackageIdInternal>()->GetPackageFullName() + "/" + fileName;
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
            for(const auto& appx : m_applicablePackages)
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
                result.insert(result.end(), m_applicablePackagesNames.begin(), m_applicablePackagesNames.end());
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
    HRESULT STDMETHODCALLTYPE AppxPackageObject::GetBlockMap(IAppxBlockMapReader** blockMapReader) noexcept try
    {
        ThrowErrorIf(Error::InvalidParameter, (blockMapReader == nullptr || *blockMapReader != nullptr), "bad pointer");
        *blockMapReader = m_appxBlockMap.As<IAppxBlockMapReader>().Detach();
        return static_cast<HRESULT>(Error::OK);
    } CATCH_RETURN();

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
        ComPtr<IStream> stream = GetFile(EncodeFileName(name));
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
        std::vector<ComPtr<IAppxFile>> files;
        for (const auto& fileName : GetFileNames(FileNameOptions::PayloadOnly))
        {
            auto file = GetFile(fileName).As<IAppxFile>();
            files.push_back(std::move(file));
        }
        *filesEnumerator = ComPtr<IAppxFilesEnumerator>::
            Make<EnumeratorCom<IAppxFilesEnumerator, IAppxFile>>(files).Detach();
        return static_cast<HRESULT>(Error::OK);
    } CATCH_RETURN();

    HRESULT STDMETHODCALLTYPE AppxPackageObject::GetManifest(IAppxManifestReader** manifestReader) noexcept try
    {
        if (m_isBundle) { return static_cast<HRESULT>(Error::PackageIsBundle); }
        ThrowErrorIf(Error::InvalidParameter,(manifestReader == nullptr || *manifestReader != nullptr), "bad pointer");
        *manifestReader = m_appxManifest.As<IAppxManifestReader>().Detach();
        return static_cast<HRESULT>(Error::OK);
    } CATCH_RETURN();

    // IAppxBundleReader
    HRESULT STDMETHODCALLTYPE AppxPackageObject::GetFootprintFile(APPX_BUNDLE_FOOTPRINT_FILE_TYPE fileType, IAppxFile **footprintFile) noexcept try
    {
        if (!m_isBundle) { return static_cast<HRESULT>(Error::NotImplemented); }
        ThrowErrorIf(Error::InvalidParameter, (footprintFile == nullptr || *footprintFile != nullptr), "bad pointer");
        ThrowErrorIf(Error::FileNotFound, (static_cast<size_t>(fileType) > bundleFootprintFiles.size()), "unknown footprint file type");
        std::string footprint (bundleFootprintFiles[fileType]);
        ComPtr<IStream> stream = GetFile(footprint);
        ThrowErrorIfNot(Error::FileNotFound, stream, "Requested footprint file not in bundle")
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
        std::vector<ComPtr<IAppxFile>> packages;
        for (const auto& fileName : GetFileNames(FileNameOptions::PayloadOnly))
        {
            auto package = GetFile(fileName).As<IAppxFile>();
            packages.push_back(std::move(package));
        }
        *payloadPackages = ComPtr<IAppxFilesEnumerator>::
            Make<EnumeratorCom<IAppxFilesEnumerator, IAppxFile>>(packages).Detach();
        return static_cast<HRESULT>(Error::OK);
    } CATCH_RETURN();

    HRESULT STDMETHODCALLTYPE AppxPackageObject::GetPayloadPackage(LPCWSTR fileName, IAppxFile **payloadPackage) noexcept try
    {
        if (!m_isBundle) { return static_cast<HRESULT>(Error::NotImplemented); }
        ThrowErrorIf(Error::InvalidParameter, (fileName == nullptr || payloadPackage == nullptr || *payloadPackage != nullptr), "bad pointer");
        std::string name = utf16_to_utf8(fileName);
        ComPtr<IStream> stream = GetFile(name);
        ThrowErrorIfNot(Error::FileNotFound, stream, "Requested package not in bundle")
        // Clients expect the stream's pointer to be at the start of the file!
        ThrowHrIfFailed(stream->Seek({0}, StreamBase::Reference::START, nullptr));
        auto result = stream.As<IAppxFile>();
        *payloadPackage = result.Detach();
        return static_cast<HRESULT>(Error::OK);
    } CATCH_RETURN();

    HRESULT STDMETHODCALLTYPE AppxPackageObject::GetManifest(IAppxBundleManifestReader **manifestReader) noexcept
    {
        if (!m_isBundle) { return static_cast<HRESULT>(Error::NotImplemented); }
        ThrowErrorIf(Error::InvalidParameter,(manifestReader == nullptr || *manifestReader != nullptr), "bad pointer");
        *manifestReader = m_appxBundleManifest.As<IAppxBundleManifestReader>().Detach();
        return static_cast<HRESULT>(Error::OK);
    }
}
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
#include "AppxManifestObject.hpp"
#include "Encoding.hpp"
#include "Enumerators.hpp"
#include "AppxFile.hpp"
#include "DirectoryObject.hpp"
#include "MsixFeatureSelector.hpp"
#include "ScopeExit.hpp"

#ifdef BUNDLE_SUPPORT
#include "Applicability.hpp"
#include "AppxBundleManifest.hpp"
#endif

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

    AppxPackageObject::AppxPackageObject(IMsixFactory* factory, MSIX_VALIDATION_OPTION validation,
        MSIX_APPLICABILITY_OPTIONS applicabilityFlags, const ComPtr<IStorageObject>& container) :
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
            m_appxManifest = ComPtr<IVerifierObject>::Make<AppxManifestObject>(factory, stream);
        }
        else
        {
            // It is valid for a user to create an IAppxPackageReader and then QI for IAppxBundleReader, but
            // not when bundle support is off.
            THROW_IF_BUNDLE_NOT_ENABLED
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
            Config(APPXBLOCKMAP_XML, [](AppxPackageObject* self)
            {
                self->m_footprintFiles.push_back(APPXBLOCKMAP_XML);
                return self->m_appxBlockMap->GetStream();
            }),
            Config(APPXMANIFEST_XML, [](AppxPackageObject* self)
            {
                self->m_footprintFiles.push_back(APPXMANIFEST_XML);
                return self->m_appxManifest->GetStream();
            }),
            Config(APPXSIGNATURE_P7X, [](AppxPackageObject* self)
            {
                if (self->m_appxSignature->HasStream())
                {
                    self->m_footprintFiles.push_back(APPXSIGNATURE_P7X);
                    return self->m_appxSignature->GetStream();
                }
                return ComPtr<IStream>();
            }),
            Config(CODEINTEGRITY_CAT, [](AppxPackageObject* self)
            {
                self->m_footprintFiles.push_back(CODEINTEGRITY_CAT);
                auto file = self->m_container->GetFile(CODEINTEGRITY_CAT);
                return self->m_appxSignature->GetValidationStream(CODEINTEGRITY_CAT, file);
            }),
            Config(CONTENT_TYPES_XML, [](AppxPackageObject*)->ComPtr<IStream>{ return ComPtr<IStream>();}), // content types is never implicitly unpacked
            Config(APPXBUNDLEMANIFEST_XML, [](AppxPackageObject* self)
            {
                self->m_footprintFiles.push_back(APPXBUNDLEMANIFEST_XML);
                return self->m_appxBundleManifest->GetStream();
            }),
        };

        // 5. Ensure that the stream collection contains streams wired up for their appropriate validation
        // and partition the container's file names into footprint and payload files.  First by going through
        // the footprint files, and then by going through the payload files.
        auto filesToProcess = m_container->GetFileNames(FileNameOptions::All);
        for (const auto& fileName : m_container->GetFileNames(FileNameOptions::FootPrintOnly))
        {   auto footPrintFile = std::find(std::begin(footPrintFileNames), std::end(footPrintFileNames), fileName);
            if (footPrintFile != std::end(footPrintFileNames))
            {
                if (fileName != CONTENT_TYPES_XML)
                {
                    auto stream = footPrintFile->GetValidationStream(this);
                    if (fileName == CODEINTEGRITY_CAT)
                    {
                        m_files[fileName] = MSIX::ComPtr<IAppxFile>::Make<MSIX::AppxFile>(m_factory.Get(), "AppxMetadata\\CodeIntegrity.cat", std::move(stream));;
                    }
                    else if (fileName == APPXBUNDLEMANIFEST_XML)
                    {
                        m_files[fileName] = MSIX::ComPtr<IAppxFile>::Make<MSIX::AppxFile>(m_factory.Get(), "AppxMetadata\\AppxBundleManifest.xml", std::move(stream));;
                    }
                    else
                    {
                        m_files[fileName] = MSIX::ComPtr<IAppxFile>::Make<MSIX::AppxFile>(m_factory.Get(), fileName, std::move(stream));;
                    }
                }
                filesToProcess.erase(std::remove(filesToProcess.begin(), filesToProcess.end(), fileName), filesToProcess.end());
            }
        }

        auto blockMapInternal = m_appxBlockMap.As<IAppxBlockMapInternal>();
        auto blockMapFiles = blockMapInternal->GetFileNames();

#ifdef BUNDLE_SUPPORT
        if(m_isBundle)
        {
            // There should only be one file in the blockmap for bundles. We validate that the block map contains
            // AppxMetadata/AppxBundleManifest.xml before, so just check the size.
            ThrowErrorIfNot(Error::BlockMapSemanticError, ((blockMapFiles.size() == 1)), "Block map contains invalid files.");

            auto bundleInfo = m_appxBundleManifest.As<IBundleInfo>();
            auto appxFactory = m_factory.As<IAppxFactory>();

            Applicability applicability(applicabilityFlags);

            auto factoryOverrides = m_factory.As<IMsixFactoryOverrides>();
            ComPtr<IUnknown> applicabilityLanguagesUnk;
            ThrowHrIfFailed(factoryOverrides->GetCurrentSpecifiedExtension(MSIX_FACTORY_EXTENSION_APPLICABILITY_LANGUAGES, &applicabilityLanguagesUnk));

            if (applicabilityLanguagesUnk.Get() != nullptr)
            {
                auto applicabilityLanguagesEnumerator = applicabilityLanguagesUnk.As<IMsixApplicabilityLanguagesEnumerator>();
                applicability.InitializeLanguages(applicabilityLanguagesEnumerator.Get());
            }
            else
            {
                applicability.InitializeLanguages();
            }

            for (const auto& package : bundleInfo->GetPackages())
            {
                auto bundleInfoInternal = package.As<IAppxBundleManifestPackageInfoInternal>();
                auto packageName = bundleInfoInternal->GetFileName();
                auto packageStream = m_container->GetFile(Encoding::EncodeFileName(packageName));

                if (packageStream)
                {   // The package is in the bundle. Verify is not compressed.
                    auto zipStream = packageStream.As<IStreamInternal>();
                    ThrowErrorIf(Error::AppxManifestSemanticError, zipStream->IsCompressed(), "Packages cannot be compressed");
                }
                else if (!packageStream && (bundleInfoInternal->GetOffset() == 0)) // This is a flat bundle.
                {
                    // We should only do this for flat bundles. If we do it for normal bundles and the user specify a 
                    // stream factory we will basically unpack any package the user wants with the same name as the package
                    // we are looking, which sounds dangerous.
                    ComPtr<IUnknown> streamFactoryUnk;
                    ThrowHrIfFailed(factoryOverrides->GetCurrentSpecifiedExtension(MSIX_FACTORY_EXTENSION_STREAM_FACTORY, &streamFactoryUnk));

                    if(streamFactoryUnk.Get() != nullptr)
                    {
                        auto streamFactory = streamFactoryUnk.As<IMsixStreamFactory>();
                        ThrowHrIfFailed(streamFactory->CreateStreamOnRelativePathUtf8(packageName.c_str(), &packageStream));
                    }
                    else
                    {   // User didn't specify a stream factory implementation. Assume packages are in the same location
                        // as the bundle.
                        auto containerName = GetFileName();
                        #ifdef WIN32
                        auto lastSeparator = containerName.find_last_of('\\');
                        #else
                        auto lastSeparator = containerName.find_last_of('/');
                        #endif
                        auto expandedPackageName = containerName.substr(0, lastSeparator + 1 ) + packageName;
                        ThrowHrIfFailed(CreateStreamOnFile(const_cast<char*>(expandedPackageName.c_str()), true, &packageStream));
                    }
                    ThrowErrorIfNot(Error::FileNotFound, packageStream, "Package from a flat bundle is not present");
                }
                else
                {
                    ThrowErrorIfNot(Error::FileNotFound, packageStream, "Package is not in container");
                }

                // Semantic checks
                LARGE_INTEGER start = { 0 };
                ULARGE_INTEGER end = { 0 };
                ThrowHrIfFailed(packageStream->Seek(start, StreamBase::Reference::END, &end));
                ThrowHrIfFailed(packageStream->Seek(start, StreamBase::Reference::START, nullptr));
                
                UINT64 size;
                ThrowHrIfFailed(package->GetSize(&size));
                ThrowErrorIf(Error::AppxManifestSemanticError, end.u.LowPart != size,
                    "Size mistmach of package between AppxManifestBundle.appx and container");

                // Validate the package
                ComPtr<IAppxPackageReader> reader;
                ThrowHrIfFailed(appxFactory->CreatePackageReader(packageStream.Get(), &reader));
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
                applicability.AddPackageIfApplicable(reader, packageName, bundleInfoInternal->GetLanguages(),
                    packageType, bundleInfoInternal->HasQualifiedResources());

                m_files[packageName] = ComPtr<IAppxFile>::Make<MSIX::AppxFile>(m_factory.Get(), packageName, std::move(packageStream));
                // Intentionally don't remove from fileToProcess. For bundles, it is possible to don't unpack packages, like
                // resource packages that are not languages packages.
            }
            applicability.GetApplicablePackages(&m_applicablePackages, &m_applicablePackagesNames);

        }
        else
        {
#endif // BUNDLE_SUPPORT
            for (const auto& fileName : blockMapFiles)
            {   auto footPrintFile = std::find(std::begin(footPrintFileNames), std::end(footPrintFileNames), fileName);
                if (footPrintFile == std::end(footPrintFileNames))
                {
                    auto opcFileName = Encoding::EncodeFileName(fileName);
                    m_payloadFiles.push_back(opcFileName);
                    auto fileStream = m_container->GetFile(opcFileName);
                    ThrowErrorIfNot(Error::FileNotFound, fileStream, "File described in blockmap not contained in OPC container");
                    VerifyFile(fileStream, fileName, blockMapInternal);
                    auto blockMapStream = m_appxBlockMap->GetValidationStream(fileName, fileStream);
                    m_files[opcFileName] = MSIX::ComPtr<IAppxFile>::Make<MSIX::AppxFile>(m_factory.Get(), fileName, std::move(blockMapStream));
                    filesToProcess.erase(std::remove(filesToProcess.begin(), filesToProcess.end(), opcFileName), filesToProcess.end());
                }
            }

            // If the map is not empty, there's a file in the container that didn't go to the footprint or payload
            // files. (eg. payload file missing in the AppxBlockMap.xml)
            ThrowErrorIfNot(Error::BlockMapSemanticError, (filesToProcess.empty()), "Payload file not described in AppxBlockMap.xml");
#ifdef BUNDLE_SUPPORT
        }
#endif
    }

    // Verify file in OPC and BlockMap
    void AppxPackageObject::VerifyFile(const ComPtr<IStream>& stream, const std::string& fileName, const ComPtr<IAppxBlockMapInternal>& blockMapInternal)
    {
        auto zipStream = stream.As<IStreamInternal>();
        auto sizeOnZip = zipStream->GetSizeOnZip();
        bool isCompressed = zipStream->IsCompressed();

        auto blocks = blockMapInternal->GetBlocks(fileName);
        std::uint64_t blocksSize = 0;
        for(auto& block : blocks)
        {   // For Block elements that don't have a Size attribute, we always set its size as BLOCKMAP_BLOCK_SIZE
            // (even for the last one). The Size attribute isn't specified if the file is not compressed.
            ThrowErrorIf(Error::BlockMapSemanticError, (!isCompressed) && (block.blockSize != BLOCKMAP_BLOCK_SIZE),
                "An uncompressed file has a size attribute in its Block elements");
            blocksSize += block.blockSize;
        }

        if(isCompressed)
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
        else
        {
            UINT64 blockMapFileSize;
            auto blockMapFile = blockMapInternal->GetFile(fileName);
            ThrowHrIfFailed(blockMapFile->GetUncompressedSize(&blockMapFileSize));
            ThrowErrorIf(Error::BlockMapSemanticError, (blockMapFileSize != sizeOnZip ),
                "Uncompressed size of the file in the block map and the OPC container don't match");
        }
    }

    void AppxPackageObject::Unpack(MSIX_PACKUNPACK_OPTION options, const ComPtr<IDirectoryObject>& to)
    {
        auto fileNames = GetFileNames(FileNameOptions::All);
        for (const auto& fileName : fileNames)
        {   // Don't extract packages files
            auto file = std::find(std::begin(m_applicablePackagesNames), std::end(m_applicablePackagesNames), fileName);
            if (file == std::end(m_applicablePackagesNames))
            {
                std::string targetName;
                if (options & MSIX_PACKUNPACK_OPTION_CREATEPACKAGESUBFOLDER)
                {
                    auto manifest = m_appxManifest.As<IAppxManifestReader>();
                    ComPtr<IAppxManifestPackageId> packageId;
                    ThrowHrIfFailed(manifest->GetPackageId(&packageId));
                    targetName = packageId.As<IAppxManifestPackageIdInternal>()->GetPackageFullName() + "/" + fileName;
                }
                else
                {   targetName = Encoding::DecodeFileName(fileName);
                }

                auto deleteFile = MSIX::scope_exit([&targetName]
                {
                    remove(targetName.c_str());
                });

                auto targetFile = to->OpenFile(targetName, MSIX::FileStream::Mode::WRITE);
                auto sourceFile = GetFile(fileName).As<IStream>();

                ULARGE_INTEGER bytesCount = {0};
                bytesCount.QuadPart = std::numeric_limits<std::uint64_t>::max();
                ThrowHrIfFailed(sourceFile->CopyTo(targetFile.Get(), bytesCount, nullptr, nullptr));
                deleteFile.release();
            }
        }

#ifdef BUNDLE_SUPPORT
        if(m_isBundle)
        {
            for(const auto& appx : m_applicablePackages)
            {
                appx.As<IPackage>()->Unpack(
                    static_cast<MSIX_PACKUNPACK_OPTION>(options | MSIX_PACKUNPACK_OPTION_CREATEPACKAGESUBFOLDER), to.Get());
            }
        }
#endif
    }

    // IStorageObject
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
        auto appxFile = GetAppxFile(fileName);
        ComPtr<IStream> stream;
        ThrowHrIfFailed(appxFile->GetStream(&stream));
        return stream;
    }

    ComPtr<IAppxFile> AppxPackageObject::GetAppxFile(const std::string& fileName)
    {
        auto result = m_files.find(fileName);
        if (result == m_files.end())
        {
            return ComPtr<IAppxFile>();
        }
        return result->second;
    }

    std::string AppxPackageObject::GetFileName() { return m_container->GetFileName(); }

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
        auto result = GetAppxFile(footprintFiles[type]);
        ThrowErrorIfNot(Error::FileNotFound, result, "requested footprint file not in package")
        // Clients expect the stream's pointer to be at the start of the file!
        ComPtr<IStream> stream;
        ThrowHrIfFailed(result->GetStream(&stream));
        ThrowHrIfFailed(stream->Seek({0}, StreamBase::Reference::START, nullptr));
        *file = result.Detach();
        return static_cast<HRESULT>(Error::OK);
    } CATCH_RETURN();

    HRESULT STDMETHODCALLTYPE AppxPackageObject::GetPayloadFile(LPCWSTR fileName, IAppxFile** file) noexcept try
    {
        return GetPayloadFile(wstring_to_utf8(fileName).c_str(), file);
    } CATCH_RETURN();

    HRESULT STDMETHODCALLTYPE AppxPackageObject::GetPayloadFiles(IAppxFilesEnumerator** filesEnumerator) noexcept try
    {
        if (m_isBundle) { return static_cast<HRESULT>(Error::PackageIsBundle); }
        ThrowErrorIf(Error::InvalidParameter,(filesEnumerator == nullptr || *filesEnumerator != nullptr), "bad pointer");
        std::vector<ComPtr<IAppxFile>> files;
        for (const auto& fileName : GetFileNames(FileNameOptions::PayloadOnly))
        {
            auto file = GetAppxFile(fileName);
            // Clients expect the stream's pointer to be at the start of the file!
            ComPtr<IStream> stream;
            ThrowHrIfFailed(file->GetStream(&stream));
            ThrowHrIfFailed(stream->Seek({0}, StreamBase::Reference::START, nullptr));
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
        THROW_IF_BUNDLE_NOT_ENABLED
        if (!m_isBundle) { return static_cast<HRESULT>(Error::NotImplemented); }
        ThrowErrorIf(Error::InvalidParameter, (footprintFile == nullptr || *footprintFile != nullptr), "bad pointer");
        ThrowErrorIf(Error::FileNotFound, (static_cast<size_t>(fileType) > bundleFootprintFiles.size()), "unknown footprint file type");
        std::string footprint (bundleFootprintFiles[fileType]);
        auto result = GetAppxFile(footprint);
        ThrowErrorIfNot(Error::FileNotFound, result, "Requested footprint file not in bundle")
        // Clients expect the stream's pointer to be at the start of the file!
        ComPtr<IStream> stream;
        ThrowHrIfFailed(result->GetStream(&stream));
        ThrowHrIfFailed(stream->Seek({0}, StreamBase::Reference::START, nullptr));
        *footprintFile = result.Detach();
        return static_cast<HRESULT>(Error::OK);
    } CATCH_RETURN();

    HRESULT STDMETHODCALLTYPE AppxPackageObject::GetPayloadPackages(IAppxFilesEnumerator **payloadPackages) noexcept try
    {
        THROW_IF_BUNDLE_NOT_ENABLED
        if (!m_isBundle) { return static_cast<HRESULT>(Error::NotImplemented); }
        ThrowErrorIf(Error::InvalidParameter,(payloadPackages == nullptr || *payloadPackages != nullptr), "bad pointer");
        std::vector<ComPtr<IAppxFile>> packages;
        for (const auto& fileName : GetFileNames(FileNameOptions::PayloadOnly))
        {
            auto package = GetAppxFile(fileName);
            ComPtr<IStream> stream;
            ThrowHrIfFailed(package->GetStream(&stream));
            ThrowHrIfFailed(stream->Seek({0}, StreamBase::Reference::START, nullptr));
            packages.push_back(std::move(package));
        }
        *payloadPackages = ComPtr<IAppxFilesEnumerator>::
            Make<EnumeratorCom<IAppxFilesEnumerator, IAppxFile>>(packages).Detach();
        return static_cast<HRESULT>(Error::OK);
    } CATCH_RETURN();

    HRESULT STDMETHODCALLTYPE AppxPackageObject::GetPayloadPackage(LPCWSTR fileName, IAppxFile **payloadPackage) noexcept try
    {
        return GetPayloadPackage(wstring_to_utf8(fileName).c_str(), payloadPackage);
    } CATCH_RETURN();

    HRESULT STDMETHODCALLTYPE AppxPackageObject::GetManifest(IAppxBundleManifestReader **manifestReader) noexcept try
    {
        THROW_IF_BUNDLE_NOT_ENABLED
        if (!m_isBundle) { return static_cast<HRESULT>(Error::NotImplemented); }
        ThrowErrorIf(Error::InvalidParameter,(manifestReader == nullptr || *manifestReader != nullptr), "bad pointer");
        *manifestReader = m_appxBundleManifest.As<IAppxBundleManifestReader>().Detach();
        return static_cast<HRESULT>(Error::OK);
    } CATCH_RETURN();

    // IAppxPackageReaderUtf8
    HRESULT STDMETHODCALLTYPE AppxPackageObject::GetPayloadFile(LPCSTR fileName, IAppxFile** file) noexcept try
    {
        if (m_isBundle) { return static_cast<HRESULT>(Error::PackageIsBundle); }
        ThrowErrorIf(Error::InvalidParameter, (fileName == nullptr || file == nullptr || *file != nullptr), "bad pointer");
        auto result = GetAppxFile(Encoding::EncodeFileName(fileName));
        ThrowErrorIfNot(Error::FileNotFound, result, "requested file not in package")
        // Clients expect the stream's pointer to be at the start of the file!
        ComPtr<IStream> stream;
        ThrowHrIfFailed(result->GetStream(&stream));
        ThrowHrIfFailed(stream->Seek({0}, StreamBase::Reference::START, nullptr));
        *file = result.Detach();
        return static_cast<HRESULT>(Error::OK);
    } CATCH_RETURN();

    // IAppxBundleReaderUtf8
    HRESULT STDMETHODCALLTYPE AppxPackageObject::GetPayloadPackage(LPCSTR fileName, IAppxFile **payloadPackage) noexcept try
    {
        THROW_IF_BUNDLE_NOT_ENABLED
        if (!m_isBundle) { return static_cast<HRESULT>(Error::NotImplemented); }
        ThrowErrorIf(Error::InvalidParameter, (fileName == nullptr || payloadPackage == nullptr || *payloadPackage != nullptr), "bad pointer");
        auto result = GetAppxFile(fileName);
        ThrowErrorIfNot(Error::FileNotFound, result, "Requested package not in bundle")
        // Clients expect the stream's pointer to be at the start of the file!
        ComPtr<IStream> stream;
        ThrowHrIfFailed(result->GetStream(&stream));
        ThrowHrIfFailed(stream->Seek({0}, StreamBase::Reference::START, nullptr));
        *payloadPackage = result.Detach();
        return static_cast<HRESULT>(Error::OK);
    } CATCH_RETURN();
}
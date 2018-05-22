//
//  Copyright (C) 2017 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
//

#include "AppxPackaging.hpp"
#include "MSIXWindows.hpp"

#include <iostream>

#define RETURN_IF_FAILED(a) \
    {   HRESULT __hr = a;   \
        if (FAILED(__hr))   \
        {   return __hr; }  \
    }

// Stripped down ComPtr provided for those platforms that do not already have a ComPtr class.
template <class T>
class ComPtr
{
public:
    // default ctor
    ComPtr() = default;
    ComPtr(T* ptr) : m_ptr(ptr) { InternalAddRef(); }

    ~ComPtr() { InternalRelease(); }
    inline T* operator->() const { return m_ptr; }
    inline T* Get() const { return m_ptr; }

    inline T** operator&()
    {   InternalRelease();
        return &m_ptr;
    }

protected:
    T* m_ptr = nullptr;

    inline void InternalAddRef() { if (m_ptr) { m_ptr->AddRef(); } }
    inline void InternalRelease()
    {
        T* temp = m_ptr;
        if (temp)
        {   m_ptr = nullptr;
            temp->Release();
        }
    }
};

// Or you can use what-ever allocator/deallocator is best for your platform...
LPVOID STDMETHODCALLTYPE MyAllocate(SIZE_T cb)  { return std::malloc(cb); }
void STDMETHODCALLTYPE MyFree(LPVOID pv)        { std::free(pv); }

// Helper class to free string buffers obtained from the packaging APIs.
template<typename T>
class Text
{
public:
    T** operator&() { return &content; }
    ~Text() { Cleanup(); }
    T* Get() { return content; }

    T* content = nullptr;
protected:
    void Cleanup() { if (content) { MyFree(content); content = nullptr; } }
};

int Help()
{
    std::cout << std::endl;
    std::cout << "Usage:" << std::endl;
    std::cout << "------" << std::endl;
    std::cout << "\t" << "BundleSample <bundle>" << std::endl;
    std::cout << std::endl;
    std::cout << "Description:" << std::endl;
    std::cout << "------------" << std::endl;
    std::cout << "\tShow information about the packages in the the bundle." << std::endl;
    std::cout << std::endl;
    return 0;
}

HRESULT ShowInformationOfPackage(IAppxFile* packageFile)
{
    Text<WCHAR> packageName;
    RETURN_IF_FAILED(packageFile->GetName(&packageName));
    std::wcout << L"\tName: " << packageName.Get() << std::endl;

    ComPtr<IAppxFactory> factory;
    RETURN_IF_FAILED(CoCreateAppxFactoryWithHeap(
        MyAllocate,
        MyFree,
        MSIX_VALIDATION_OPTION::MSIX_VALIDATION_OPTION_FULL,
        &factory));

    // Get stream of the package and package reader
    ComPtr<IStream> packageStream;
    RETURN_IF_FAILED(packageFile->GetStream(&packageStream));
    ComPtr<IAppxPackageReader> packageReader;
    RETURN_IF_FAILED(factory->CreatePackageReader(packageStream.Get(), &packageReader));

    // Get information about the package from the manifest
    ComPtr<IAppxManifestReader> manifest;
    RETURN_IF_FAILED(packageReader->GetManifest(&manifest));
    ComPtr<IAppxManifestPackageId> bundlePackageId;
    RETURN_IF_FAILED(manifest->GetPackageId(&bundlePackageId));

    // Get full name of the bundle
    Text<WCHAR> fullName;
    RETURN_IF_FAILED(bundlePackageId->GetPackageFullName(&fullName));
    std::wcout << L"\tFull Name: " << fullName.Get() << std::endl;

    // Get AUMIDs in the package
    ComPtr<IAppxManifestApplicationsEnumerator> applications;
    RETURN_IF_FAILED(manifest->GetApplications(&applications))
    BOOL hasCurrent = FALSE;
    RETURN_IF_FAILED(applications->GetHasCurrent(&hasCurrent));
    while(hasCurrent)
    {
        ComPtr<IAppxManifestApplication> application;
        RETURN_IF_FAILED(applications->GetCurrent(&application));
        Text<WCHAR> aumid;
        RETURN_IF_FAILED(application->GetAppUserModelId(&aumid));
        std::wcout << L"\tAppUserModelId: " << aumid.Get() << std::endl;
        RETURN_IF_FAILED(applications->MoveNext(&hasCurrent));
    }

    // Show what are the target device families of the package.
    ComPtr<IAppxManifestReader3> manifest3;
    // AppxPackaging.hpp contains a helper UuidOfImpl<I>::iid for QueryInterface.
    // It returns the GUID associated with the interface
    RETURN_IF_FAILED(manifest->QueryInterface(UuidOfImpl<IAppxManifestReader3>::iid, reinterpret_cast<void**>(&manifest3)));
    ComPtr<IAppxManifestTargetDeviceFamiliesEnumerator> tdfEnum;
    RETURN_IF_FAILED(manifest3->GetTargetDeviceFamilies(&tdfEnum));
    hasCurrent = FALSE;
    RETURN_IF_FAILED(tdfEnum->GetHasCurrent(&hasCurrent));
    while (hasCurrent)
    {
        ComPtr<IAppxManifestTargetDeviceFamily> tdf;
        RETURN_IF_FAILED(tdfEnum->GetCurrent(&tdf));
        Text<WCHAR> tdfName;
        RETURN_IF_FAILED(tdf->GetName(&tdfName));
        std::wcout << L"\tTarget Device Family: " << tdfName.Get() << std::endl;
        RETURN_IF_FAILED(tdfEnum->MoveNext(&hasCurrent));
    }
    return S_OK;
}

HRESULT ShowInformationOfBundle(char* bundleName)
{
    // Initialize the factory with full validation and applicability options.
    ComPtr<IAppxBundleFactory> bundleFactory;
    RETURN_IF_FAILED(CoCreateAppxBundleFactoryWithHeap(
        MyAllocate,
        MyFree,
        MSIX_VALIDATION_OPTION::MSIX_VALIDATION_OPTION_FULL,
        MSIX_APPLICABILITY_OPTIONS::MSIX_APPLICABILITY_OPTION_FULL,
        &bundleFactory));

    // Create stream on the file provided.
    ComPtr<IStream> inputStream;
    RETURN_IF_FAILED(CreateStreamOnFile(const_cast<char*>(bundleName), true, &inputStream));

    // Now get the bundle reader
    ComPtr<IAppxBundleReader> bundleReader;
    RETURN_IF_FAILED(bundleFactory->CreateBundleReader(inputStream.Get(), &bundleReader));

    // Get basic information about this bundle from the bundle manifest
    ComPtr<IAppxBundleManifestReader> manifestReader;
    RETURN_IF_FAILED(bundleReader->GetManifest(&manifestReader));
    ComPtr<IAppxManifestPackageId> bundlePackageId;
    RETURN_IF_FAILED(manifestReader->GetPackageId(&bundlePackageId));

    // This should be the same name as the one passed into this function
    Text<WCHAR> name;
    RETURN_IF_FAILED(bundlePackageId->GetName(&name));
    std::wcout << L"File: " << name.Get() << std::endl;

    // Get full name of the bundle
    Text<WCHAR> fullName;
    RETURN_IF_FAILED(bundlePackageId->GetPackageFullName(&fullName));
    std::wcout << L"Full Name: " << fullName.Get() << std::endl;

    // A bundle using MSIX_APPLICABILITY_OPTION_FULL will only show the packages
    // that are applicable to the current platform, as well as the resources
    // packages with the languages of the system. Get the applicable
    // package by calling GetPayloadPackages
    ComPtr<IAppxFilesEnumerator> applicablePackages;
    RETURN_IF_FAILED(bundleReader->GetPayloadPackages(&applicablePackages));

    BOOL hasCurrent = FALSE;
    RETURN_IF_FAILED(applicablePackages->GetHasCurrent(&hasCurrent));
    std::wcout << L"Applicable Packages: " << std::endl;
    while(hasCurrent)
    {
        ComPtr<IAppxFile> applicablePackage;
        RETURN_IF_FAILED(applicablePackages->GetCurrent(&applicablePackage));
        RETURN_IF_FAILED(ShowInformationOfPackage(applicablePackage.Get()));
        std::wcout << std::endl;
        RETURN_IF_FAILED(applicablePackages->MoveNext(&hasCurrent));
    }

    // It is possible to see all the packages in the bundle by looking into the
    // information of the bundle manifest.
    ComPtr<IAppxBundleManifestPackageInfoEnumerator> bundleInfoEnumerator;
    RETURN_IF_FAILED(manifestReader->GetPackageInfoItems(&bundleInfoEnumerator));

    hasCurrent = FALSE;
    RETURN_IF_FAILED(bundleInfoEnumerator->GetHasCurrent(&hasCurrent));
    std::wcout << L"Packages In Bundle: " << std::endl;
    while(hasCurrent)
    {
        ComPtr<IAppxBundleManifestPackageInfo> bundlePackageInfo;
        RETURN_IF_FAILED(bundleInfoEnumerator->GetCurrent(&bundlePackageInfo));
        Text<WCHAR> packageName;
        RETURN_IF_FAILED(bundlePackageInfo->GetFileName(&packageName));

        ComPtr<IAppxFile> package;
        RETURN_IF_FAILED(bundleReader->GetPayloadPackage(packageName.Get(), &package));
        RETURN_IF_FAILED(ShowInformationOfPackage(package.Get()));

        // Get the languages of this package.
        ComPtr<IAppxManifestQualifiedResourcesEnumerator> resourceEnumerator;
        RETURN_IF_FAILED(bundlePackageInfo->GetResources(&resourceEnumerator));

        BOOL hasLanguage = FALSE;
        RETURN_IF_FAILED(resourceEnumerator->GetHasCurrent(&hasLanguage));
        while (hasLanguage)
        {
            ComPtr<IAppxManifestQualifiedResource> resource;
            RETURN_IF_FAILED(resourceEnumerator->GetCurrent(&resource));

            Text<WCHAR> language;
            RETURN_IF_FAILED(resource->GetLanguage(&language));
            std::wcout << L"\tLanguage: " << language.Get() << std::endl;
            RETURN_IF_FAILED(resourceEnumerator->MoveNext(&hasLanguage));
        }
        std::wcout << std::endl;
        RETURN_IF_FAILED(bundleInfoEnumerator->MoveNext(&hasCurrent));
    }

    return S_OK;
}

int main(int argc, char* argv[])
{
    if (argc != 2)
    {
        return Help();
    }

    HRESULT hr = ShowInformationOfBundle(argv[1]);

    if (FAILED(hr))
    {
        std::wcout << L"Error: " << std::hex << hr << L" while extracting the appx package" <<std::endl;
    }

    return 0;
}


//
//  Copyright (C) 2017 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
//

#include "AppxPackaging.hpp"
#include "MSIXWindows.hpp"

#include <iostream>
#include <atomic>

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

    // For use instead of ComPtr<T> t(new Foo(...)); given that the class has an Initialize function
    template<class U, class... Args>
    static HRESULT MakeAndInitialize(T** result, Args&&... args)
    {
        ComPtr<U> inner(new U());
        RETURN_IF_FAILED(inner->Initialize(std::forward<Args>(args)...));
        RETURN_IF_FAILED(inner->QueryInterface(UuidOfImpl<T>::iid, reinterpret_cast<void**>(result)));
        return S_OK;
    }

    // For use instead of ComPtr<T> t(new Foo(...));
    template<class U, class... Args>
    static ComPtr<T> Make(Args&&... args)
    {
        ComPtr<T> result;
        result.m_ptr = new U(std::forward<Args>(args)...);
        return result;
    }

    ~ComPtr() { InternalRelease(); }
    inline T* operator->() const { return m_ptr; }
    inline T* Get() const { return m_ptr; }

    inline T** operator&()
    {
        InternalRelease();
        return &m_ptr;
    }

    T* Detach()
    {
        T* temp = m_ptr;
        m_ptr = nullptr;
        return temp;
    }

protected:
    T* m_ptr = nullptr;

    inline void InternalAddRef() { if (m_ptr) { m_ptr->AddRef(); } }
    inline void InternalRelease()
    {
        T* temp = m_ptr;
        if (temp)
        {
            m_ptr = nullptr;
            temp->Release();
        }
    }
};

// Or you can use what-ever allocator/deallocator is best for your platform...
LPVOID STDMETHODCALLTYPE MyAllocate(SIZE_T cb) { return std::malloc(cb); }
void STDMETHODCALLTYPE MyFree(LPVOID pv) { std::free(pv); }

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
    std::cout << "\t" << "OverrideLanguageSample <bundle> <languages ...>" << std::endl;
    std::cout << std::endl;
    std::cout << "Description:" << std::endl;
    std::cout << "------------" << std::endl;
    std::cout << "\tSample to show the usage of IMsixFactoryOverrides for MSIX_FACTORY_EXTENSION_APPLICABILITY_LANGUAGES " << std::endl;
    std::cout << "\tusing language BCP47 tags from the command line." << std::endl;
    std::cout << std::endl;
    return 0;
}

class OverrideLanguages final : public IMsixApplicabilityLanguagesEnumerator
{
public:
    OverrideLanguages(char** languages, int numLanguages) : m_languages(languages), m_numLanguages(numLanguages) {}

    // IUnknown
    virtual ULONG STDMETHODCALLTYPE AddRef() override
    {
        return ++m_ref;
    }

    virtual ULONG STDMETHODCALLTYPE Release() override
    {
        if (--m_ref == 0)
        {
            delete this;
            return 0;
        }
        return m_ref;
    }

    virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObject) noexcept override
    {
        if (ppvObject == nullptr || *ppvObject != nullptr)
        {
            return E_INVALIDARG;
        }

        *ppvObject = nullptr;
        if (riid == UuidOfImpl<IUnknown>::iid)
        {
            *ppvObject = static_cast<void*>(reinterpret_cast<IUnknown*>(this));
            AddRef();
            return S_OK;
        }
        if (riid == UuidOfImpl<IMsixApplicabilityLanguagesEnumerator>::iid)
        {
            *ppvObject = static_cast<void*>(reinterpret_cast<IMsixApplicabilityLanguagesEnumerator*>(this));
            AddRef();
            return S_OK;
        }
        return E_NOINTERFACE;
    }

    // IMsixApplicabilityLanguagesEnumerator
    virtual HRESULT STDMETHODCALLTYPE GetCurrent(LPCSTR *bcp47Language) noexcept override
    {
        if (m_currentIndex < m_numLanguages)
        {
            *bcp47Language = m_languages[m_currentIndex];
            return S_OK;
        }
        else
        {
            return E_BOUNDS;
        }
    }

    virtual HRESULT STDMETHODCALLTYPE GetHasCurrent(BOOL *hasCurrent) noexcept override
    {
        *hasCurrent = (m_currentIndex < m_numLanguages);
        return S_OK;
    }

    virtual HRESULT STDMETHODCALLTYPE MoveNext(BOOL *hasNext) noexcept override
    {
        if (m_currentIndex < m_numLanguages)
        {
            m_currentIndex++;
            *hasNext = (m_currentIndex < m_numLanguages);
            return S_OK;
        }
        else
        {
            return E_BOUNDS;
        }
    }

protected:
    std::atomic<std::uint32_t> m_ref;
    char** m_languages = nullptr;
    int m_numLanguages = 0;
    int m_currentIndex = 0;
};

HRESULT ShowInformationOfPackage(IAppxFile* packageFile)
{
    Text<WCHAR> packageName;
    RETURN_IF_FAILED(packageFile->GetName(&packageName));
    std::wcout << L"\tName: " << packageName.Get() << std::endl;

    ComPtr<IAppxFactory> factory;
    RETURN_IF_FAILED(CoCreateAppxFactoryWithHeap(
        MyAllocate,
        MyFree,
        MSIX_VALIDATION_OPTION::MSIX_VALIDATION_OPTION_SKIPSIGNATURE,
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

    // Show what are the resources of the package.
    ComPtr<IAppxManifestResourcesEnumerator> resourcesEnum;
    RETURN_IF_FAILED(manifest->GetResources(&resourcesEnum));
    BOOL hasCurrent = FALSE;
    RETURN_IF_FAILED(resourcesEnum->GetHasCurrent(&hasCurrent));
    while (hasCurrent)
    {
        Text<WCHAR> resource;
        RETURN_IF_FAILED(resourcesEnum->GetCurrent(&resource));
        std::wcout << L"\tResource: " << resource.Get() << std::endl;
        RETURN_IF_FAILED(resourcesEnum->MoveNext(&hasCurrent));
    }
    return S_OK;
}

HRESULT ShowInformationOfBundle(char* bundleName, char** languages, int numLanguages)
{
    std::cout << "File: " << bundleName << std::endl;

    // Initialize the factory with full validation and applicability options.
    ComPtr<IAppxBundleFactory> bundleFactory;
    RETURN_IF_FAILED(CoCreateAppxBundleFactoryWithHeap(
        MyAllocate,
        MyFree,
        MSIX_VALIDATION_OPTION::MSIX_VALIDATION_OPTION_SKIPSIGNATURE,
        MSIX_APPLICABILITY_OPTIONS::MSIX_APPLICABILITY_OPTION_FULL,
        &bundleFactory));

    // Create our object that implements IMsixApplicabilityLanguagesEnumerator
    auto overrideLanguages = ComPtr<IMsixApplicabilityLanguagesEnumerator>::Make<OverrideLanguages>(languages, numLanguages);

    // Get the IMsixFactoryOverrides and set the override.
    // Do this before using the factory to create the bundle reader.
    ComPtr<IMsixFactoryOverrides> factoryOverrides;
    RETURN_IF_FAILED(bundleFactory->QueryInterface(UuidOfImpl<IMsixFactoryOverrides>::iid, reinterpret_cast<void**>(&factoryOverrides)));
    RETURN_IF_FAILED(factoryOverrides->SpecifyExtension(MSIX_FACTORY_EXTENSION_APPLICABILITY_LANGUAGES, overrideLanguages.Get()));

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

    // Get full name of the bundle
    Text<WCHAR> fullName;
    RETURN_IF_FAILED(bundlePackageId->GetPackageFullName(&fullName));
    std::wcout << L"Full Name: " << fullName.Get() << std::endl;

    // A bundle with the MSIX_FACTORY_EXTENSION_APPLICABILITY_LANGUAGES override
    // will only show the packages that are applicable to the list of languages
    // specified. Get the applicable packages by calling GetPayloadPackages.
    ComPtr<IAppxFilesEnumerator> applicablePackages;
    RETURN_IF_FAILED(bundleReader->GetPayloadPackages(&applicablePackages));

    BOOL hasCurrent = FALSE;
    RETURN_IF_FAILED(applicablePackages->GetHasCurrent(&hasCurrent));
    std::wcout << L"Applicable Packages: " << std::endl;
    while (hasCurrent)
    {
        ComPtr<IAppxFile> applicablePackage;
        RETURN_IF_FAILED(applicablePackages->GetCurrent(&applicablePackage));
        RETURN_IF_FAILED(ShowInformationOfPackage(applicablePackage.Get()));
        std::wcout << std::endl;
        RETURN_IF_FAILED(applicablePackages->MoveNext(&hasCurrent));
    }

    return S_OK;
}

int main(int argc, char* argv[])
{
    if (argc < 3)
    {
        return Help();
    }

    HRESULT hr = ShowInformationOfBundle(argv[1], argv + 2, argc - 2);

    if (FAILED(hr))
    {
        std::cout << "Error: " << std::hex << hr << " while reading the bundle" << std::endl;
        Text<char> text;
        auto logResult = GetLogTextUTF8(MyAllocate, &text);
        if (0 == logResult)
        {
            std::cout << "LOG:" << std::endl << text.content << std::endl;
        }
        else
        {
            std::cout << "UNABLE TO GET LOG WITH HR=" << std::hex << logResult << std::endl;
        }
    }

    return 0;
}


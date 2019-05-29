//
//  Copyright (C) 2017 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
// 
#pragma once

#include <vector>
#include <utility>
#include <string>

#include "AppxPackaging.hpp"
#include "ComHelper.hpp"
#include "Exceptions.hpp"

namespace MSIX {

    // Basic closeness measures
    enum class Bcp47ClosenessMeasure
    {
        NoMatch               = 0,
        AnyMatch              = 60,  // und-*
        AnyMatchWithScript    = 65,  // und- with script
        LanguagesScriptsMatch = 75,  // language and script match
        ExactMatch            = 100,
    };

    // For now, we only support Bcp47 tags that contains language, script and region,
    // which covers the basic Bcp47 language matching. We need a proper Bcp47
    // language matching API that handle all the special cases and the full Bcp47 format.
    class Bcp47Tag final
    {
    public:
        Bcp47Tag(const std::string& fullTag);
        Bcp47Tag(const std::string& language, const std::string& script, const std::string& region) : 
            m_language(language), m_script(script), m_region(region) {} 

        Bcp47ClosenessMeasure Compare(const Bcp47Tag& otherTag);
        Bcp47ClosenessMeasure CompareNeutral(const Bcp47Tag& otherTag);
        const std::string GetFullTag() const;

    protected:
        std::string m_language;
        std::string m_script;
        std::string m_region;
    };

    class Applicability
    {
    public:
        Applicability(MSIX_APPLICABILITY_OPTIONS applicabilityFlags) : m_applicabilityFlags(applicabilityFlags)
        {}

        void InitializeLanguages();
        void InitializeLanguages(IMsixApplicabilityLanguagesEnumerator* languagesEnumerator);

        void AddPackageIfApplicable(ComPtr<IAppxPackageReader>& reader, APPX_BUNDLE_PAYLOAD_PACKAGE_TYPE packageType, ComPtr<IAppxBundleManifestPackageInfo> bundlePackageInfo);

        void GetApplicablePackages(std::vector<ComPtr<IAppxPackageReader>>* applicablePackages, std::vector<std::string>* applicablePackagesNames);

    private:
        MSIX_PLATFORMS GetPlatform();
        std::vector<Bcp47Tag> GetLanguages();

        bool m_hasExactLanguageMatch = false;
        bool m_matchApplicationPackage = false;
        std::vector<std::pair<std::string, ComPtr<IAppxPackageReader>>> m_applicablePackages;
        std::vector<std::pair<std::string, ComPtr<IAppxPackageReader>>> m_variantFormPackages;
        std::vector<std::pair<std::string, ComPtr<IAppxPackageReader>>> m_extraApplicationPackages;
        MSIX_APPLICABILITY_OPTIONS m_applicabilityFlags = MSIX_APPLICABILITY_OPTIONS::MSIX_APPLICABILITY_OPTION_FULL;
        std::vector<Bcp47Tag> m_languages;
    };
}
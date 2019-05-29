//
//  Copyright (C) 2017 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
// 
#include "Applicability.hpp"

#include <string>
#include <algorithm>
#include <vector>

#include "AppxBundleManifest.hpp"

namespace MSIX {

    struct Bcp47Entry
    {
        const char* icu;
        const char* bcp47;

        Bcp47Entry(const char* i, const char* b) : icu(i), bcp47(b) {}

        inline bool operator==(const char* otherIcui) const {
            return 0 == strcmp(icu, otherIcui);
        }
    };

    // We've seen cases were uloc_toLanguageTag returns zh-CN. Add here any inconsistencies.
    // Some AppxBundleManifests have zh-CN, zh-TW, zh-HK as languages.
    static const Bcp47Entry bcp47List[] = {
        Bcp47Entry(u8"zh-cn", u8"zh-Hans-CN"),
        Bcp47Entry(u8"zh-hk", u8"zh-Hant-HK"),
        Bcp47Entry(u8"zh-tw", u8"zh-Hant-TW"),
    };

    Bcp47Tag::Bcp47Tag(const std::string& fullTag)
    {
        std::string fullTagLower;
        fullTagLower.resize(fullTag.size());
        std::transform(fullTag.begin(), fullTag.end(), fullTagLower.begin(), ::tolower);
        const auto& tagFound = std::find(std::begin(bcp47List), std::end(bcp47List),fullTagLower.c_str());
        std::string bcp47Tag;
        if (tagFound == std::end(bcp47List))
        {
            bcp47Tag = fullTag;
        }
        else
        {
            bcp47Tag = std::string((*tagFound).bcp47);
        }

        auto delimiter = '-';
        auto found = bcp47Tag.find(delimiter);
        m_language = bcp47Tag.substr(0, found);
        ThrowErrorIf(Error::Unexpected, (m_language.size() < 2 || m_language.size() > 3 || m_language.empty()), "Malformed Bcp47 tag");
        if (found != std::string::npos)
        {
            auto position = found+1;
            found = bcp47Tag.find(delimiter, position);
            auto tag = bcp47Tag.substr(position, found - position);
            ThrowErrorIf(Error::Unexpected, (tag.size() < 2 || tag.size() > 4), "Malformed Bcp47 tag");
            if (tag.size() == 4)
            {   // Script tag size is always 4
                m_script = tag;
                if (found != std::string::npos)
                {
                    position = found+1;
                    found = bcp47Tag.find(delimiter, position);
                    m_region = bcp47Tag.substr(position, found);
                }
            }
            else
            {   // Region tag size can be 2 or 3.
                m_region = tag;
            }
        }
    }

    Bcp47ClosenessMeasure Bcp47Tag::Compare(const Bcp47Tag& otherTag)
    {
        std::string thisLanguage;
        thisLanguage.resize(m_language.size());
        std::transform(m_language.begin(), m_language.end(), thisLanguage.begin(), ::tolower);

        std::string otherLanguage;
        otherLanguage.resize(otherTag.m_language.size());
        std::transform(otherTag.m_language.begin(), otherTag.m_language.end(), otherLanguage.begin(), ::tolower);

        std::string thisScript;
        thisScript.resize(m_script.size());
        std::transform(m_script.begin(), m_script.end(), thisScript.begin(), ::tolower);

        std::string otherScript;
        otherScript.resize(otherTag.m_script.size());
        std::transform(otherTag.m_script.begin(), otherTag.m_script.end(), otherScript.begin(), ::tolower);

        std::string thisRegion;
        thisRegion.resize(m_region.size());
        std::transform(m_region.begin(), m_region.end(), thisRegion.begin(), ::tolower);

        std::string otherRegion;
        otherRegion.resize(otherTag.m_region.size());
        std::transform(otherTag.m_region.begin(), otherTag.m_region.end(), otherRegion.begin(), ::tolower);

        // Compare for und-*
        if (thisLanguage == "und" || otherLanguage == "und")
        {
            if (thisScript == otherScript)
            {
                return Bcp47ClosenessMeasure::AnyMatchWithScript;
            }
            return Bcp47ClosenessMeasure::AnyMatch;
        }

        if (thisLanguage == otherLanguage && thisScript == otherScript)
        {
            if (thisRegion == otherRegion)
            {
                return Bcp47ClosenessMeasure::ExactMatch;
            }
            return Bcp47ClosenessMeasure::LanguagesScriptsMatch;
        }

        return Bcp47ClosenessMeasure::NoMatch;
    }

    // Compares the neutral form of this Bcp47 tag
    Bcp47ClosenessMeasure Bcp47Tag::CompareNeutral(const Bcp47Tag& otherTag)
    {
        Bcp47Tag neutral(m_language, m_script, "");
        return neutral.Compare(otherTag);
    }

    const std::string Bcp47Tag::GetFullTag() const
    {
        std::string result = m_language;
        if (!m_script.empty()) { result += "-" + m_script; }
        if (!m_region.empty()) { result += "-" + m_region; }
        return result;
    }

    void Applicability::InitializeLanguages()
    {
        m_languages = GetLanguages();
    }

    void Applicability::InitializeLanguages(IMsixApplicabilityLanguagesEnumerator* languagesEnumerator)
    {
        BOOL hasNext = FALSE;
        ThrowHrIfFailed(languagesEnumerator->GetHasCurrent(&hasNext));
        while (hasNext)
        {
            LPCSTR language = nullptr;
            ThrowHrIfFailed(languagesEnumerator->GetCurrent(&language));
            m_languages.push_back(std::string(language));

            ThrowHrIfFailed(languagesEnumerator->MoveNext(&hasNext));
        }
    }

	void Applicability::AddPackageIfApplicable(ComPtr<IAppxPackageReader>& reader, APPX_BUNDLE_PAYLOAD_PACKAGE_TYPE packageType, const ComPtr<IAppxBundleManifestPackageInfo>& bundlePackageInfo)
    {
        auto bundlePackageInfoInternal = bundlePackageInfo.As<IAppxBundleManifestPackageInfoInternal>();
        auto packageName = bundlePackageInfoInternal->GetFileName();
        auto packageLanguages = bundlePackageInfoInternal->GetLanguages();
        auto packageScales = bundlePackageInfoInternal->GetScales();
		
        // If there are not qualified resources the package is always applicable
        // MSIX_APPLICABILITY_NONE indicates that we should skip all applicability checks
        if (!bundlePackageInfoInternal->HasQualifiedResources() || (m_applicabilityFlags == static_cast<MSIX_APPLICABILITY_OPTIONS>(MSIX_APPLICABILITY_NONE)))
        {
            m_applicablePackages.push_back(std::make_pair(packageName,std::move(reader)));
            return;
        }

        // Unless the user has specified the "skip all" applicability flag, we will treat resource packages
        // with scale, but not language, as NOT applicable.
        if (packageType == APPX_BUNDLE_PAYLOAD_PACKAGE_TYPE_RESOURCE && !packageScales.empty() && packageLanguages.empty())
        {
	        return;
        }

        MSIX_PLATFORMS platform = (m_applicabilityFlags & MSIX_APPLICABILITY_OPTION_SKIPPLATFORM) ?
            static_cast<MSIX_PLATFORMS>(MSIX_PLATFORM_ALL) : GetPlatform();
        //if (innerPackageManifest->GetPlatform() & platform) // temporary disable until we have bundles for all platforms
        //{
            if (m_applicabilityFlags & MSIX_APPLICABILITY_OPTION_SKIPLANGUAGE)
            {
                m_applicablePackages.push_back(std::make_pair(packageName,std::move(reader)));
                return;
            }

            bool hasMatch = false;
            bool hasVariantMatch = false;
            for (auto& systemLanguage : m_languages)
            {
                for (auto& packageLanguage : packageLanguages)
                {
                    auto closeness = systemLanguage.Compare(packageLanguage);
                    if (closeness == Bcp47ClosenessMeasure::ExactMatch)
                    {
                        // If this is an exact match we can stop looking
                        m_hasExactLanguageMatch = true;
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
                    m_applicablePackages.push_back(std::make_pair(packageName,std::move(reader)));
                    break;
                }
                if (hasVariantMatch)
                {
                    m_variantFormPackages.push_back(std::make_pair(packageName, std::move(reader)));
                    break;
                }
            }
            if (packageType == APPX_BUNDLE_PAYLOAD_PACKAGE_TYPE_APPLICATION)
            {
                if (!hasMatch && !hasVariantMatch)
                {   // If we are here the package is an application package that targets the 
                    // current platform, but it doesn't contain a language match. Save it just in case
                    // there are no application packages that match, so we don't end up only with resources.
                    m_extraApplicationPackages.push_back(std::make_pair(packageName, std::move(reader)));
                }
                else
                {
                    m_matchApplicationPackage = true;
                }
            }
        //}
    }

    void Applicability::GetApplicablePackages(std::vector<ComPtr<IAppxPackageReader>>* applicablePackages, std::vector<std::string>* applicablePackagesNames)
    {
        for(auto& applicable : m_applicablePackages)
        {
            applicablePackages->push_back(std::move(applicable.second));
            applicablePackagesNames->push_back(applicable.first);
        }
        // If we don't have an exact match, we have to add all of the variants, too.
        if (!m_hasExactLanguageMatch)
        {
            for(auto& variantFormPackage : m_variantFormPackages)
            {
                applicablePackages->push_back(std::move(variantFormPackage.second));
                applicablePackagesNames->push_back(variantFormPackage.first);
            }
        }
        // If we don't have an application package add the ones we have
        if (!m_matchApplicationPackage)
        {
            for(auto& applicationPackage : m_extraApplicationPackages)
            {
                applicablePackages->push_back(std::move(applicationPackage.second));
                applicablePackagesNames->push_back(applicationPackage.first);
            }
        }
    }

} // namespace MSIX

//
//  Copyright (C) 2017 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
// 
#pragma once

#include <vector>
#include <string>
#include <algorithm>

#include "AppxPackaging.hpp"
#include "Exceptions.hpp"

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
    // which covers the basic Bcp47 language matching.  We need a proper Bcp47
    // language matching API that handle all the special cases and the full Bcp47 format.
    class Bcp47Tag 
    {
    public:
        Bcp47Tag(const std::string& fullTag)
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

        Bcp47Tag(const std::string& language, const std::string& script, const std::string& region) : 
            m_language(language), m_script(script), m_region(region) {} 

        Bcp47ClosenessMeasure Compare(const Bcp47Tag& otherTag)
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
        Bcp47ClosenessMeasure CompareNeutral(const Bcp47Tag& otherTag)
        {
            Bcp47Tag neutral(m_language, m_script, "");
            return neutral.Compare(otherTag);
        }

        std::string GetFullTag()
        {
            std::string result = m_language;
            if (!m_script.empty()) { result += "-" + m_script; }
            if (!m_region.empty()) { result += "-" + m_region; }
            return result;
        }

    protected:
        std::string m_language;
        std::string m_script;
        std::string m_region;
    };

    class Applicability
    {
    public:
        static MSIX_PLATFORMS GetPlatform();
        static std::vector<Bcp47Tag> GetLanguages();
    };
}
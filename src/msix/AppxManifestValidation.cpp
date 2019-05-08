//
//  Copyright (C) 2017 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
//

#include <regex>
#include <string>

#include "AppxManifestValidation.hpp"
#include "Exceptions.hpp"
#include "StringHelper.hpp"
#include "IXml.hpp"

namespace MSIX {

    namespace
    {
        struct TargetAttribute
        {
            XmlQueryName ElementQuery;
            XmlAttributeName AttributeName;
        };

        const char* ProhibitedFileNames[] = {
            ".",
            "..",
            "con",
            "prn",
            "aux",
            "nul",
            "com1",
            "com2",
            "com3",
            "com4",
            "com5",
            "com6",
            "com7",
            "com8",
            "com9",
            "lpt1",
            "lpt2",
            "lpt3",
            "lpt4",
            "lpt5",
            "lpt6",
            "lpt7",
            "lpt8",
            "lpt9",
        };

        // Caller must pass in a lowercase string!
        bool IsProhibitedFileName(const std::string& identifier)
        {
            for (const auto& name : ProhibitedFileNames)
            {
                if (identifier == name)
                {
                    return true;
                }
            }
            return false;
        }

        const std::string ProhibitedPrefixes[] = {
            "con.",
            "prn.",
            "aux.",
            "nul.",
            "com1.",
            "com2.",
            "com3.",
            "com4.",
            "com5.",
            "com6.",
            "com7.",
            "com8.",
            "com9.",
            "lpt1.",
            "lpt2.",
            "lpt3.",
            "lpt4.",
            "lpt5.",
            "lpt6.",
            "lpt7.",
            "lpt8.",
            "lpt9.",
            "xn--",
        };

        bool HasProhibitedPrefix(const std::string& identifier)
        {
            for (const auto& prefix : ProhibitedPrefixes)
            {
                if (identifier.size() >= prefix.size() &&
                    identifier.substr(0, prefix.size()) == prefix)
                {
                    return true;
                }
            }
            return false;
        }

        const std::string ProhibitedSuffixes[] = {
            ".",
        };

        bool HasProhibitedSuffix(const std::string& identifier)
        {
            for (const auto& suffix : ProhibitedSuffixes)
            {
                if (identifier.size() >= suffix.size() &&
                    identifier.substr(identifier.size() - suffix.size()) == suffix)
                {
                    return true;
                }
            }
            return false;
        }

        void ValidateIdentifier(const TargetAttribute& target, const MSIX::ComPtr<IXmlElement>& element)
        {
            std::string attributeValue = element->GetAttributeValue(target.AttributeName);

            if (!attributeValue.empty())
            {
                ThrowErrorIf(Error::AppxManifestSemanticError, !AppxManifestValidation::IsIdentifierValid(attributeValue),
                    (std::string("Invalid Identifier ") + GetAttributeNameStringUtf8(target.AttributeName) + ": " + attributeValue).c_str());
            }
        }

        const TargetAttribute Identifiers[] = {
            { XmlQueryName::Any_Identity, XmlAttributeName::Name },
            { XmlQueryName::Any_Identity, XmlAttributeName::ResourceId },
            { XmlQueryName::Package_Dependencies_PackageDependency, XmlAttributeName::Name },
            { XmlQueryName::Package_Dependencies_MainPackageDependency, XmlAttributeName::Name },
        };

        void ValidateIdentifiers(IXmlDom* manifest)
        {
            auto CallValidateIdentifier = [](void* target, const MSIX::ComPtr<IXmlElement> & element)
            {
                ValidateIdentifier(*reinterpret_cast<TargetAttribute*>(target), element);
                return true;
            };

            for (const auto& identifier : Identifiers)
            {
                XmlVisitor visitor{ &identifier, CallValidateIdentifier };
                manifest->ForEachElementIn(identifier.ElementQuery, visitor);
            }
        }
    }

    bool AppxManifestValidation::IsIdentifierValid(const std::string& identifier)
    {
#if !VALIDATING
        // If the schema didn't check for us, do it now.
        std::regex e("[a-zA-Z0-9\\.\\-]+"); // valid characters
        if (!std::regex_match(identifier, e))
        {
            return false;
        }
#endif

        std::string lowIdent = Helper::tolower(identifier);
        return !IsProhibitedFileName(lowIdent) && !HasProhibitedPrefix(lowIdent) && !HasProhibitedSuffix(lowIdent);
    }

    void AppxManifestValidation::ValidateManifest(IXmlDom* manifest)
    {
        ValidateIdentifiers(manifest);
    }

}

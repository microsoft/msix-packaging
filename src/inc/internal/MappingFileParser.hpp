#pragma once

#include "MSIXWindows.hpp"
#include <map>
#include <vector>

namespace MSIX {

    typedef enum : UINT32
    {
        Continue = 0,
        Stop,
        SkipSection,
        Fail
    } HandlerState;

    enum SectionID
    {
        UnknownSection = -1,
        FilesSection = 0,
        ResourceMetadataSection = 1,
        KeysSection = 2,
        ExternalPackagesSection = 3,
    };

    const std::map<std::string, SectionID> KnownSectionNamesMap = 
        {{"Files", FilesSection}, 
        {"ResourceMetadata", ResourceMetadataSection},
        {"Keys", KeysSection},
        {"ExternalPackages", ExternalPackagesSection}};

    class MappingFileParser final
    {
    public:
        MappingFileParser();
        void ParseMappingFile(std::string mappingFile);

    protected:
        HandlerState ParseSectionHeading(std::string line);
        HandlerState HandleSection(std::string sectionName);
        SectionID GetSectionIDByName(std::string sectionName);
        bool IsSectionFound(SectionID sectionId);

        std::string removeLeadingWhitespace(std::string line);
        std::string removeTrailingWhitespace(std::string line);

        int lineNumber;
        static const int NumKnownSections = 4;
        std::map<std::string, std::string> list;
        bool foundSection[NumKnownSections];
        SectionID currentSectionId;
    };
}


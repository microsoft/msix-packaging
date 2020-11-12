#include "MappingFileParser.hpp"
#include <iostream>
#include <fstream>
#include <map>

namespace MSIX {

    const std::string WHITESPACE = " \n\r\t";

    MappingFileParser::MappingFileParser() {}

    void MappingFileParser::ParseMappingFile(std::string mappingFile)
    {
        std::ifstream mappingFileStream(mappingFile);
        if(mappingFileStream.is_open())
        {
            std::string currentLine;
            while (std::getline(mappingFileStream, currentLine))
            {
                std::string line = removeTrailingWhitespace(removeLeadingWhitespace(currentLine));
                char firstChar = line[0];
                switch(firstChar)
                {
                    case '[':
                        ParseSectionHeading(line);

                    case '"':
                        /*if (state != MappingFileParserHandler::SkipSection)
                        {
                            state = ParseMapping(currentLine, firstChar, handler);
                        }*/
                        break;
                        
                    case '\0': // line is empty
                        break;

                    default:
                        break;

                        //state = SendError(handler, currentLine, firstChar - currentLine + 1,
                          //  MAKEAPPX_E_MAPPINGFILE_EXPECT_CHAR_2, L'[', L'\"');
                }
            }
            mappingFileStream.close();
        }
        else
        {
            //error
        }
    }

    std::string MappingFileParser::removeLeadingWhitespace(std::string line)
    {
	    size_t start = line.find_first_not_of(WHITESPACE);
	    return (start == std::string::npos) ? "" : line.substr(start);
    }

    std::string MappingFileParser::removeTrailingWhitespace(std::string line)
    {
	    size_t end = line.find_last_not_of(WHITESPACE);
	    return (end == std::string::npos) ? "" : line.substr(0, end + 1);
    } 

    HandlerState MappingFileParser::ParseSectionHeading(std::string line)
    {
        char lastChar = line.back();
        if(lastChar == ']')
        {
            std::string sectionName = std::string(&line[1], &line[line.length()]);
            return HandleSection(sectionName);
        }
        else
        {
            //Log::WriteError(false, MAKEAPPX_E_MAPPINGFILE_PARSE, lineNumber, columnNumber, reason);
            //return SendError(handler, line, lastChar - line + 1, MAKEAPPX_E_MAPPINGFILE_EXPECT_CHAR, L']');
            return Fail;
        }
    }

    HandlerState MappingFileParser::HandleSection(std::string sectionName)
    {
        this->currentSectionId = UnknownSection;
        SectionID sectionId = GetSectionIDByName(sectionName);
        if(sectionId != UnknownSection)
        {
            if (this->IsSectionFound(sectionId))
            {
                //Log::WriteError(false, MAKEAPPX_E_MAPPINGFILE_DUPLICATE_SECTION, KnownSectionNames[sectionId], lineNumber);
                return Fail;
            }
            else
            {
                this->foundSection[sectionId] = true;
                this->currentSectionId = sectionId;
                return Continue;
            }
        }
        
        //Log::WriteWarning(false, MAKEAPPX_W_MAPPINGFILE_UNKNOWN_SECTION, name.chars, lineNumber);
        return SkipSection;
    }

    SectionID MappingFileParser::GetSectionIDByName(std::string sectionName)
    {
        for (auto sectionMapIterator = KnownSectionNamesMap.begin(); sectionMapIterator != KnownSectionNamesMap.end(); sectionMapIterator++)
        {
            if(sectionName.compare(sectionMapIterator->first) == 0)
            {
                return sectionMapIterator->second;
            }
        }
        return UnknownSection;
    }

    bool MappingFileParser::IsSectionFound(SectionID sectionId)
    {
        return this->foundSection[sectionId];
    }
}
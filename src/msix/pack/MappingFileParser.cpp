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
            HandlerState state = Continue;
            while ((state != Stop) && (state != Fail))
            {
                std::getline(mappingFileStream, currentLine);
                this->lineNumber++;

                if(!mappingFileStream.eof() && !currentLine.empty())
                {
                    std::string line = removeTrailingWhitespace(removeLeadingWhitespace(currentLine));
                    char firstChar = line[0];
                    switch(firstChar)
                    {
                        case '[':
                            state = ParseSectionHeading(line);
                            break;

                        case '"':
                            if (state != SkipSection)
                            {
                                //state = ParseMapping(currentLine, firstChar, handler);
                            }
                            break;
                        
                        case '\0': // line is empty
                            break;

                        default:
                            break; // remove this
                            //state = SendError(handler, currentLine, firstChar - currentLine + 1,
                            //  MAKEAPPX_E_MAPPINGFILE_EXPECT_CHAR_2, L'[', L'\"');
                    }
                }
                else // reached end of file
                {
                    state = Stop;
                }
            }

            if (state == Fail)
            {
                //Log::WriteError(false, MAKEAPPX_E_MAPPINGFILE_BAD_LINE, lineNumber);
                //RETURN_HR(HRESULT_FROM_WIN32(ERROR_BAD_FORMAT));
            }

            mappingFileStream.close();
        }
        else
        {
            //error
        }
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
            //ThrowErrorAndLog(Error::AppxManifestSemanticError, "The package is not valid in the bundle because its manifest does not declare any Application elements.");
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
}
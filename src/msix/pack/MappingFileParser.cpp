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
                if(std::getline(mappingFileStream, currentLine))
                {
                    std::string line = removeTrailingWhitespace(removeLeadingWhitespace(currentLine));
                    this->lineNumber++;

                    if(!line.empty())
                    {
                        char firstChar = line[0];
                        switch(firstChar)
                        {
                        case '[':
                            state = ParseSectionHeading(line);
                            break;

                        case '"':
                            if (state != SkipSection)
                            {
                                state = ParseMapping(line, firstChar);
                            }
                            break;

                        default:
                            break; // remove this
                            //state = SendError(handler, currentLine, firstChar - currentLine + 1,
                            //  MAKEAPPX_E_MAPPINGFILE_EXPECT_CHAR_2, L'[', L'\"');
                        }
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
            std::string sectionName = std::string(&line[1], &line[line.length()-1]);
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

    HandlerState MappingFileParser::ParseMapping(std::string line, char firstChar)
    {
        std::vector<std::string> pathTokens;
        char currentChar = firstChar;

        while(!line.empty())
        {
            if (currentChar == '"')
            {
                size_t currentCharIndex = 0;
                size_t endQuoteIndex = line.find("\"", currentCharIndex+1);
                
                if (endQuoteIndex != std::string::npos) 
                {
                    std::string path = std::string(&line[currentCharIndex], &line[endQuoteIndex+1]);
                    pathTokens.push_back(path);
                    std::string remainingLine = line.substr(++endQuoteIndex);
                    if(!remainingLine.empty())
                    {
                        currentCharIndex = endQuoteIndex;
                        currentChar = line.at(currentCharIndex);
                    }
                    line = remainingLine;
                }
                else
                {
                    //return SendError(handler, line, currentChar - line + 1, MAKEAPPX_E_MAPPINGFILE_EXPECT_CHAR, L'\"');
                }
            }
            else
            {
                // ERROR mai not starting quote secnond token error
            }

            //get nexttoken after space
            line = removeLeadingWhitespace(line);
            if(!line.empty())
            {
                char nextToken = line.at(0);
                if ((nextToken == currentChar))
                {
                    // ERROR return SendError(handler, line, currentChar - line + 1, MAKEAPPX_E_MAPPINGFILE_NEED_WHITESPACE);
                }
                currentChar = nextToken;
            } 
        }
        return HandleMapping(pathTokens);
    }

    HandlerState MappingFileParser::HandleMapping(std::vector<std::string> pathTokens)
    {
        //if sectionId is files then do this, else do different for externalpackages
        if(pathTokens.size() == 2)
        {
            //File validations
            this->list.insert(std::pair<std::string, std::string>(pathTokens.at(1), pathTokens.at(0)));
            return Continue;
        }
        else
        {
            if(pathTokens.size() < 2)
            {
                //MAKEAPPX_E_FILELIST_MISSING_OUTPUT_FILE
            }
            else
            {
                //MAKEAPPX_E_MAPPINGFILE_TOO_MANY_TOKENS_ON_LINE
            }
            return Fail;
        }
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
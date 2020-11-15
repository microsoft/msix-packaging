#include "MappingFileParser.hpp"
#include <iostream>
#include <fstream>
#include <map>

namespace MSIX {

    const std::string WHITESPACE = " \n\r\t";

    MappingFileParser::MappingFileParser() {
        this->lineNumber = 0;
        this->errorMessage = "";
    }

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
                            std::ostringstream errorBuilder;
                            errorBuilder << "Error: The mapping file can't be parsed. At line " << this->lineNumber << ": Expecting '[' or '\"'.";
                            ThrowErrorAndLog(Error::BadFormat, errorBuilder.str().c_str());
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
                std::ostringstream errorBuilder;
                errorBuilder << "Error: The mapping file can't be parsed. " << this->errorMessage;
                ThrowErrorAndLog(Error::BadFormat, errorBuilder.str().c_str());
            }

            mappingFileStream.close();
        }
        else
        {
            std::ostringstream errorBuilder;
            errorBuilder << "Error: The system cannot find the file specified: " << mappingFile <<".";
            ThrowErrorAndLog(Error::FileNotFound, errorBuilder.str().c_str());
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
            std::ostringstream errorBuilder;
            errorBuilder << "At line " << this->lineNumber << ": Expecting ']'.";
            this->errorMessage = errorBuilder.str().c_str();
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
                std::ostringstream errorBuilder;
                errorBuilder << "Duplicate " << sectionName << " section found on line " << this->lineNumber << ".";
                this->errorMessage = errorBuilder.str().c_str();
                return Fail;
            }
            else
            {
                this->foundSection[sectionId] = true;
                this->currentSectionId = sectionId;
                return Continue;
            }
        }
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
                    std::ostringstream errorBuilder;
                    errorBuilder << "At line: " << this->lineNumber << ": Expecting '\"'."; 
                    this->errorMessage = errorBuilder.str().c_str();
                    return Fail;
                }
            }
            else
            {
                std::ostringstream errorBuilder;
                errorBuilder << "At line: " << this->lineNumber << ": Expecting '\"'."; 
                this->errorMessage = errorBuilder.str().c_str();
                return Fail;
            }

            line = removeLeadingWhitespace(line);
            if(!line.empty())
            {
                char nextToken = line.at(0);
                if ((nextToken == currentChar))
                {
                    std::ostringstream errorBuilder;
                    errorBuilder << "At line: " << this->lineNumber << ": Expecting whitespace after '\"'.";
                    this->errorMessage = errorBuilder.str().c_str();
                    return Fail;
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
                std::ostringstream errorBuilder;
                errorBuilder << "Output file path is missing on line " << this->lineNumber << ".";
                this->errorMessage = errorBuilder.str().c_str();
            }
            else
            {
                std::ostringstream errorBuilder;
                errorBuilder << "Too many tokens on line " << this->lineNumber << ".";
                this->errorMessage = errorBuilder.str().c_str();
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
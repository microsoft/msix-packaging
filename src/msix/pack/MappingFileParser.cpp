#include "MappingFileParser.hpp"
#include <iostream>
#include <fstream>
#include <map>

namespace MSIX {

    const std::string WHITESPACE = " \t";

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
                    this->lineNumber++;
                    const char* firstChar = SkipWhitespace(currentLine.c_str());

                    switch(*firstChar)
                    {
                    case '[':
                        state = ParseSectionHeading(firstChar);
                        break;

                    case '"':
                        if (state != SkipSection)
                        {
                            state = ParseMapping(currentLine, firstChar);
                        }
                        break;

                    case '\0': // line is empty
                        break;

                    default:
                        std::ostringstream errorBuilder;
                        errorBuilder << "Error: The mapping file can't be parsed. At line " << this->lineNumber << ": Expecting '[' or '\"'.";
                        ThrowErrorAndLog(Error::BadFormat, errorBuilder.str().c_str());
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
                errorBuilder << "Error: The mapping file can't be parsed. At line: " << this->lineNumber << this->errorMessage;
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
        line = RemoveTrailingWhitespace(line);
        char lastChar = line.back();
        if(lastChar == ']')
        {
            std::string sectionName = std::string(&line[1], &line[line.length()-1]);
            return HandleSection(sectionName);
        }
        else
        {
            std::ostringstream errorBuilder;
            errorBuilder << ": Expecting ']'.";
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
                errorBuilder << " Duplicate " << sectionName << " section found on line " << this->lineNumber << ".";
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

    HandlerState MappingFileParser::ParseMapping(std::string line, const char* firstChar)
    {
        std::vector<std::string> pathTokens;
        const char* currentChar = firstChar;

        while (*currentChar != '\0')
        {
            if (*currentChar == '"')
            {
                std::pair<std::string, const char*> pathValues = SearchString(currentChar + 1, '\"');
                currentChar = pathValues.second;

                if (*currentChar == '\0')
                {
                    std::ostringstream errorBuilder;
                    errorBuilder << ": Expecting '\"'."; 
                    this->errorMessage = errorBuilder.str().c_str();
                    return Fail;
                }

                currentChar++;
                pathTokens.push_back(pathValues.first);
            }
            else
            {
                std::ostringstream errorBuilder;
                errorBuilder << ": Expecting '\"'."; 
                this->errorMessage = errorBuilder.str().c_str();
                return Fail;
            }

            const char* nextToken = SkipWhitespace(currentChar);
            if ((nextToken == currentChar) && (*nextToken != '\0'))
            {
                std::ostringstream errorBuilder;
                errorBuilder << ": Expecting whitespace after '\"'.";
                this->errorMessage = errorBuilder.str().c_str();
                return Fail;
            }
            currentChar = nextToken;
        }
        return HandleMapping(pathTokens);
    }

    bool MappingFileParser::IsWhitespace(char c)
    {
        return ((c == L' ') || (c == L'\t'));
    }

    const char* MappingFileParser::SkipWhitespace(const char* line)
    {
        const char* firstChar = line;
        while (IsWhitespace(*firstChar))
        {
            firstChar++;
        }
        return firstChar;
    }

    std::pair<std::string, const char*> MappingFileParser::SearchString(const char* line, char c)
    {        
        std::string path;
        const char* result = line;
        while ((*result != c) && (*result != '\0'))
        {
            path += *result;
            result++;
        }
        return std::make_pair(path, result);
    }

    HandlerState MappingFileParser::HandleMapping(std::vector<std::string> pathTokens)
    {
        if(pathTokens.size() == 2)
        {
            std::string inputPath = pathTokens.at(0);
            std::string outputPath = pathTokens.at(1);

            if(outputPath.empty() || inputPath.empty())
            {
                return Fail;
            }

            //Check if file exists
            std::ifstream inputFileStream(inputPath);
            if(!inputFileStream.is_open())
            {
                std::ostringstream errorBuilder;
                errorBuilder << ": The system cannot find the file specified: " << inputPath; 
                this->errorMessage = errorBuilder.str().c_str();
                return Fail;
            }

            if(this->currentSectionId == FilesSection)
            {
                std::map<std::string, std::string>::iterator it;
                it = this->list.find(outputPath);
                
                if(it != this->list.end())
                {
                    // Adding the same file multiple times with the same key and value is allowed.
                    if(inputPath.compare(it->second) != 0)
                    {
                        std::ostringstream errorBuilder;
                        errorBuilder << ": You can't add both \"" << inputPath << "\" and \"" << it->second << "\" to the output file as \"" << outputPath << "\"."; 
                        this->errorMessage = errorBuilder.str().c_str();
                        return Fail;
                    } 
                }
                this->list.insert(std::pair<std::string, std::string>(outputPath, inputPath));
            }
            else if(this->currentSectionId == ExternalPackagesSection)
            {
                std::map<std::string, std::string>::iterator it;
                it = this->externalPackagesList.find(outputPath);
                
                if(it != this->externalPackagesList.end())
                {
                    std::ostringstream errorBuilder;
                    errorBuilder << ": You can't add both \"" << inputPath << "\" and \"" << it->second << "\" to the output file as \"" << outputPath << "\"."; 
                    this->errorMessage = errorBuilder.str().c_str();
                    return Fail;
                }

                this->externalPackagesList.insert(std::pair<std::string, std::string>(outputPath, inputPath));
            }
            return Continue;
        }
        else
        {
            if(pathTokens.size() < 2)
            {
                std::ostringstream errorBuilder;
                errorBuilder << " Output file path is missing on line " << this->lineNumber << ".";
                this->errorMessage = errorBuilder.str().c_str();
            }
            else
            {
                std::ostringstream errorBuilder;
                errorBuilder << " Too many tokens on line " << this->lineNumber << ".";
                this->errorMessage = errorBuilder.str().c_str();
            }
            return Fail;
        }
    }

    bool MappingFileParser::IsSectionFound(SectionID sectionId)
    {
        return this->foundSection[sectionId];
    }

    std::string MappingFileParser::RemoveTrailingWhitespace(std::string line)
    {
	    size_t end = line.find_last_not_of(WHITESPACE);
	    return (end == std::string::npos) ? "" : line.substr(0, end + 1);
    }
}
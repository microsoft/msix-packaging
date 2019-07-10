//
//  Copyright (C) 2019 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
//

#include "FileNameValidation.hpp"
#include "StringHelper.hpp"
#include "UnicodeConversion.hpp"

#include <array>

namespace MSIX {

    namespace
    {
        constexpr std::size_t ReservedTableSize = 0x7F;
        static const std::array<bool, 0x7D> IsReservedFileNameChar = {
            true,  true,  true,  true,  true,  true,  true,  true,
            true,  true,  true,  true,  true,  true,  true,  true,
            true,  true,  true,  true,  true,  true,  true,  true,
            true,  true,  true,  true,  true,  true,  true,  true,  // characters in the range 0 - 31
            false, false, true,  false, false, false, false, false, // character "
            false, false, true,  false, false, false, false, true,  // characters * /
            false, false, false, false, false, false, false, false,
            false, false, true,  false, true,  false, true,  true,  // characters : < > ?
            false, false, false, false, false, false, false, false,
            false, false, false, false, false, false, false, false,
            false, false, false, false, false, false, false, false,
            false, false, false, false, false, false, false, false,
            false, false, false, false, false, false, false, false,
            false, false, false, false, false, false, false, false,
            false, false, false, false, false, false, false, false,
            false, false, false, false, true                        // character |
        };

        const std::string ProhibitedFileNames[] = {
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

        constexpr wchar_t highSurrogateStart = 0xd800;
        constexpr wchar_t highSurrogateEnd   = 0xdbff;
        constexpr wchar_t lowSurrogateStart  = 0xdc00;
        constexpr wchar_t lowSurrogateEnd    = 0xdfff;

        bool IsLowSurrogate(wchar_t ch)
        {
            return (((ch) >= lowSurrogateStart) && ((ch) <= lowSurrogateEnd));
        }

        bool IsHighSurrogate(wchar_t ch)
        {
            return (((ch) >= highSurrogateStart) && ((ch) <= highSurrogateEnd));
        }


        bool IsValidSegment(wchar_t previousChar, const std::wstring& segmentUtf16)
        {
            // Segments cannot end with multiple segment delimiters ('\'), space, dot or orphaned high surrogate char.
            if ((previousChar == L'\\') || (previousChar == L' ') || (previousChar == L'.' ) || IsHighSurrogate(previousChar))
            {
                return false;
            }

            // Segments cannot be longer than 255 characters
            if (segmentUtf16.size() > 255)
            {
                return false;
            }
            std::string segment = wstring_to_utf8(segmentUtf16);
            return FileNameValidation::IsIdentifierValid(segment);
        }
    }

    // Checks for disallowed character sequences in a file name and check if the file name is valid against reserved folders. 
    // This method will catch the following types of invalid file names:
    //  - Empty file name
    //  - File names longer than 32767
    //  - Containing reserved characters (< > : " / | ? *)
    //  - Multiple slashes in a row (including UNC paths)
    //  - Relative paths ("./", "..\", etc.)
    //  - Absolute paths (beginning with a slash)
    //  - File names that end with a dot or a slash
    //  - Names that end with a dot or a space
    //  - Names that conflict with device names (e.g. CON, AUX, NUL, etc.)
    //  - Segments that are longer than 255 characters
    //  - File name same as System Reserved Folder name or Package Reserved Folder name
    //  - File name is a file under the System Reserved Folder
    bool FileNameValidation::IsFileNameValid(const std::string& name)
    {
        constexpr std::size_t UnicodeStringMaxChars = 32767;
        if (name.empty() ||  name.size() > UnicodeStringMaxChars)
        {
            return false;
        }

        #ifdef WIN32
        auto findSlash = name.find("/");
        if (findSlash != std::string::npos)
        {
            return false;
        }
        #else
        auto findBackSlash = name.find("\\");
        if (findBackSlash != std::string::npos)
        {
            return false;
        }
        #endif

        std::string backSlashName = Helper::toBackLash(name);
        if (backSlashName[0] == '\\')
        {
            return false;
        }

        wchar_t previousChar = L'\0';
        std::wstring currentSegment;
        bool hasRelsSubFolder = false; // the file name is under a _rels subfolder

        std::wstring nameUtf16 = utf8_to_wstring(backSlashName);
        for(size_t i = 0; i < nameUtf16.size(); i++)
        {
            wchar_t currentChar = nameUtf16[i];

            // Reserved characters in the file name
            if ((currentChar < ReservedTableSize) && IsReservedFileNameChar[currentChar])
            {
                return false;
            }

            // Character is a non-character Unicode codepoint
            if (currentChar == 0xFFFE || currentChar == 0xFFFF)
            {
                return false;
            }

            // Found low surrogate value without preceding high surrogate value
            if (IsLowSurrogate(currentChar) && !IsHighSurrogate(previousChar))
            {
                return false;
            } 

            // Previous high surrogate value was not completed
            if (IsHighSurrogate(previousChar) && !IsLowSurrogate(currentChar))
            {
                return false;
            } 

            if (currentChar == L'\\')
            {
                if(!IsValidSegment(previousChar, currentSegment))
                {
                    return false;
                }

                std::wstring lowSegment = Helper::towlower(currentSegment);
                if (lowSegment == L"_rels")
                {
                    hasRelsSubFolder = true;
                }

                currentSegment.clear();
            }
            else
            {
                currentSegment += currentChar;
            }

            previousChar = currentChar;
        }

        // Validate final segment
        if (!IsValidSegment(previousChar, currentSegment))
        {
            return false;
        }

        // Relationship Part URI must have a .rels extension and be under a _rels subfolder
        if (hasRelsSubFolder)
        {
            std::string ext = Helper::tolower(name.substr(name.find_last_of(".") + 1));
            if (ext == "rels")
            {
                return false;
            }
        }

        return true;
    }

    bool FileNameValidation::IsIdentifierValid(const std::string& identifier)
    {
        std::string lowSegment = Helper::tolower(identifier);
        return !IsProhibitedFileName(lowSegment) && !HasProhibitedPrefix(lowSegment) && !HasProhibitedSuffix(lowSegment);
    }

    bool FileNameValidation::IsFootPrintFile(const std::string& fileName)
    {
        std::string lowIdent = Helper::tolower(fileName);
        return ((lowIdent == "appxmanifest.xml") ||
                (lowIdent == "appxsignature.p7x") ||
                (lowIdent == "appxblockmap.xml") ||
                (lowIdent == "[content_types].xml"));
    }

    bool FileNameValidation::IsReservedFolder(const std::string& fileName)
    {
        std::string lowIdent = Helper::tolower(fileName);
        return ((lowIdent.rfind("appxmetadata", 0) != std::string::npos) ||
                (lowIdent.rfind("microsoft.system.package.metadata", 0) != std::string::npos));
    }
}

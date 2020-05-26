//
//  Copyright (C) 2017 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
// 
// ONLY build on platforms other than Win32
#include "Exceptions.hpp"
#include "DirectoryObject.hpp"
#include "FileStream.hpp"
#include "MSIXWindows.hpp"
#include "UnicodeConversion.hpp"
#include "MsixFeatureSelector.hpp"
#include "StringHelper.hpp"

#include <memory>
#include <iostream>
#include <string>
#include <sstream>
#include <locale>
#include <codecvt>
#include <queue>

namespace MSIX {
    enum class WalkOptions : std::uint16_t
    {
        Files = 1,          // Enumerate files within the specified directory
        Directories = 2,    // Enumerate directories
        Recursive = 4       // Enumerate recursively
    };

    inline constexpr WalkOptions operator& (WalkOptions a, WalkOptions b)
    {
        return static_cast<WalkOptions>(static_cast<uint16_t>(a) & static_cast<uint16_t>(b));
    }

    inline constexpr WalkOptions operator| (WalkOptions a, WalkOptions b)
    {
        return static_cast<WalkOptions>(static_cast<uint16_t>(a) | static_cast<uint16_t>(b));
    }

    template <class Lambda>
    void WalkDirectory(const std::string& root, WalkOptions options, Lambda& visitor)
    {
        static std::string dot(".");
        static std::string dotdot("..");

        std::wstring utf16Name = utf8_to_wstring(root);
        if ((options & WalkOptions::Files) == WalkOptions::Files)
        {
            utf16Name += L"\\*";
        }

        WIN32_FIND_DATA findFileData = {};
        std::unique_ptr<std::remove_pointer<HANDLE>::type, decltype(&::FindClose)> find(
            FindFirstFile(reinterpret_cast<LPCWSTR>(utf16Name.c_str()), &findFileData),
            &FindClose);

        if (INVALID_HANDLE_VALUE == find.get())
        {
            DWORD lastError = GetLastError();
            if (lastError == ERROR_FILE_NOT_FOUND)
            {
                return;
            }
            ThrowWin32ErrorIfNot(lastError, false, "FindFirstFile failed.");
        }

        // TODO: handle junction loops
        do
        {
            utf16Name = std::wstring(findFileData.cFileName);
            auto utf8Name = wstring_to_utf8(utf16Name);
            if (findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            {
                if (dot != utf8Name && dotdot != utf8Name)
                {
                    std::string child = root + "\\" + utf8Name;
                    if ((options & WalkOptions::Directories) == WalkOptions::Directories &&
                        !visitor(root, WalkOptions::Directories, std::move(utf8Name), 0))
                    {
                        break;
                    }
                    if ((options & WalkOptions::Recursive) == WalkOptions::Recursive)
                    {
                        WalkDirectory(child, options, visitor);
                    }
                }
            }
            else if ((options & WalkOptions::Files) == WalkOptions::Files)
            {
                ULARGE_INTEGER fileTime;
                fileTime.HighPart = findFileData.ftLastWriteTime.dwHighDateTime;
                fileTime.LowPart = findFileData.ftLastWriteTime.dwLowDateTime;
                if (!visitor(root, WalkOptions::Files, std::move(utf8Name), static_cast<std::uint64_t>(fileTime.QuadPart)))
                {
                    break;
                }
            }
        }
        while (FindNextFile(find.get(), &findFileData));

        std::uint32_t lastError = static_cast<std::uint32_t>(GetLastError());
        ThrowWin32ErrorIfNot(lastError,
            ((lastError == ERROR_NO_MORE_FILES) ||
            (lastError == ERROR_SUCCESS) ||
            (lastError == ERROR_ALREADY_EXISTS)),
            "FindNextFile");
    }

    std::string CleanDirectory(const std::string& path)
    {
        const std::string currentDirectory = ".\\";

        std::string cleanPath = Helper::toBackSlash(path);

        if (cleanPath.size() <= currentDirectory.size())
        {
            return cleanPath;
        }

        if (cleanPath.substr(0, currentDirectory.size()) == currentDirectory)
        {
            return cleanPath.substr(currentDirectory.size());
        }

        return cleanPath;
    }

    std::string GetFullPath(const std::string& path)
    {
        const std::wstring longPathPrefix = LR"(\\?\)";

        // GetFullPathNameW doesn't play nice with relatives paths. For example, "output" and ".\output" return
        // different values when nBufferLength is 0. MSDN states that this API "Multithreaded applications and shared
        // library code should not use the GetFullPathName function and should avoid using relative path names".
        // This makes us not support relative paths, but support directories from the current directory (aka .\dir)
        std::string cleanPath = CleanDirectory(path);

        auto pathWide = utf8_to_wstring(cleanPath);

        std::wstring result = longPathPrefix;
        size_t prefixChars = longPathPrefix.size();
        if (pathWide.substr(0, longPathPrefix.size()) == longPathPrefix)
        {
            // Already begins with long path prefix, so don't add it
            result = L"";
            prefixChars = 0;
        }

        // We aren't going to go out of our way to support crazy incoming paths.
        // This means that path here is limited to MAX_PATH, but the resulting path won't be.
        DWORD length = GetFullPathNameW(pathWide.c_str(), 0, nullptr, nullptr);

        // Any errors result in 0
        ThrowLastErrorIf(length == 0, "Failed to get necessary char count for GetFullPathNameW");

        // When requesting size, length accounts for null char
        result.resize(prefixChars + length, L' ');

        DWORD newlength = GetFullPathNameW(pathWide.c_str(), length, &result[prefixChars], nullptr);

        // On success, length does not account for null char
        ThrowLastErrorIf(length == 0, "Failed to get necessary char count for GetFullPathNameW");
        ThrowErrorIf(Error::Unexpected, (length - 1) != newlength, "Result length was unexpected");
        result.resize(prefixChars + newlength);

        return wstring_to_utf8(result);
    }

    struct DirectoryInfo
    {
        std::string Name;
        bool Create;

        DirectoryInfo(std::string&& name, bool create) : Name(std::move(name)), Create(create) {}
    };

    static void SplitDirectories(const std::string& path, std::queue<DirectoryInfo>& directories, bool forCreate)
    {
        static char const* const Delims = "\\/";
        const std::string longPathPrefix = R"(\\?\)";

        size_t copyPos = 0;
        size_t searchPos = 0;
        size_t lastPos = 0;

        if (path.substr(0, longPathPrefix.size()) == longPathPrefix)
        {
            // Absolute path, we need to skip it
            searchPos = longPathPrefix.size();
        }

        while (lastPos != std::string::npos)
        {
            lastPos = path.find_first_of(Delims, searchPos);

            std::string temp = path.substr(copyPos, lastPos - copyPos);
            if (!temp.empty())
            {
                directories.emplace(std::move(temp), forCreate);
            }

            copyPos = searchPos = lastPos + 1;
        }
    }

    // Destroys directories
    static void EnsureDirectoryStructureExists(const std::string& root, std::queue<DirectoryInfo>& directories, bool lastIsFile, std::string* resultingPath = nullptr)
    {
        ThrowErrorIf(Error::Unexpected, directories.empty(), "Some path must be given");

        auto PopFirst = [&directories]()
        {
            auto result = directories.front();
            directories.pop();
            return result;
        };

        std::string path = root;
        bool isFirst = true;

        while (!directories.empty())
        {
            auto dirInfo = PopFirst();
            if (!path.empty())
            {
                path += DirectoryObject::GetPathSeparator();
            }
            path += dirInfo.Name;

            bool shouldWeCreateDir = dirInfo.Create;

            // When the last entry is a file, and we are on the last entry, never create
            if (lastIsFile && directories.empty())
            {
                shouldWeCreateDir = false;
            }
            // If this is a rooted list of directories, the first one will be a device
            else if (root.empty() && isFirst)
            {
                shouldWeCreateDir = false;
            }

            if (shouldWeCreateDir)
            {
                bool found = false;

                std::wstring utf16Name = utf8_to_wstring(path);
                DWORD attr = GetFileAttributesW(utf16Name.c_str());

                if (attr == INVALID_FILE_ATTRIBUTES)
                {
                    if (!CreateDirectory(utf16Name.c_str(), nullptr))
                    {
                        auto lastError = GetLastError();
                        ThrowWin32ErrorIfNot(lastError, (lastError == ERROR_ALREADY_EXISTS), std::string("Call to CreateDirectory failed creating: " + path).c_str());
                    }
                }
                else
                {
                    ThrowWin32ErrorIfNot(ERROR_ALREADY_EXISTS, attr & FILE_ATTRIBUTE_DIRECTORY, ("A file at this path already exists: " + path).c_str());
                }
            }

            isFirst = false;
        }

        if (resultingPath)
        {
            *resultingPath = std::move(path);
        }
    }

    const char* DirectoryObject::GetPathSeparator() { return "\\"; }

    DirectoryObject::DirectoryObject(const std::string& root, bool createRootIfNecessary)
    {
        m_root = GetFullPath(root);

        if (createRootIfNecessary)
        {
            std::queue<DirectoryInfo> directories;
            SplitDirectories(m_root, directories, true);
            EnsureDirectoryStructureExists({}, directories, false);
        }
    }

    std::vector<std::string> DirectoryObject::GetFileNames(FileNameOptions)
    {
        // TODO: Implement when standing-up the pack side for test validation purposes.
        NOTIMPLEMENTED;
    }

    // IDirectoryObject
    ComPtr<IStream> DirectoryObject::OpenFile(const std::string& fileName, FileStream::Mode mode)
    {
        std::queue<DirectoryInfo> directories;

        // Enforce that directory structure exists before creating file at specified location;
        // but only if we are going to write the file.  If reading, the file should already exist.
        bool modeWillCreateFile = (mode != FileStream::Mode::READ && mode != FileStream::Mode::READ_UPDATE);
        SplitDirectories(fileName, directories, modeWillCreateFile);

        std::string path;
        EnsureDirectoryStructureExists(m_root, directories, true, &path);

        auto result = ComPtr<IStream>::Make<FileStream>(std::move(utf8_to_wstring(path)), mode);
        return result;
    }

    std::multimap<std::uint64_t, std::string> DirectoryObject::GetFilesByLastModDate()
    {
        THROW_IF_PACK_NOT_ENABLED
        std::multimap<std::uint64_t, std::string> files;
        auto rootSize = m_root.size() + 1; // plus separator
        WalkDirectory(m_root, WalkOptions::Recursive | WalkOptions::Files, [&](
                std::string root,
                WalkOptions option,
                std::string&& name,
                std::uint64_t lastWrite)
            {
                std::string fileName = root + GetPathSeparator() + name;
                // root contains the top level directory, which we don't need
                fileName = fileName.substr(rootSize);
                files.insert(std::make_pair(lastWrite, std::move(fileName)));
                return true;
            });
        return files;
    }
}

// Don't pollute other compilation units with any of our #defs...
#undef UNICODE

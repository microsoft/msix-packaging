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

    const char* DirectoryObject::GetPathSeparator() { return "\\"; }

    std::vector<std::string> DirectoryObject::GetFileNames(FileNameOptions)
    {
        // TODO: Implement when standing-up the pack side for test validation purposes.
        NOTIMPLEMENTED;
    }

    // IDirectoryObject
    ComPtr<IStream> DirectoryObject::OpenFile(const std::string& fileName, FileStream::Mode mode)
    {
        DWORD absPathSize = GetCurrentDirectory(0, NULL);
        std::wstring absPath;
        absPath.resize(absPathSize-1);
        ThrowLastErrorIf(GetCurrentDirectory(absPathSize, const_cast<wchar_t*>(absPath.data())) == 0, "Failed getting full path.");
        absPath = L"\\\\?\\" + absPath;

        std::queue<std::string> directories;
        auto PopFirst = [&directories]()
        {
            auto result = directories.front();
            directories.pop();
            return result;
        };

        // Add the root directory and build a list of directory names to ensure exist
        std::istringstream stream(m_root + "/" + fileName);
        std::string directory;
        while (getline(stream, directory, '/'))
        {
            directories.push(std::move(directory));
        }

        // Enforce that directory structure exists before creating file at specified location.
        bool found = false;
        std::string path = wstring_to_utf8(absPath) + GetPathSeparator() +PopFirst();
        do
        {
            WalkDirectory(path, WalkOptions::Directories, [&](
                std::string,
                WalkOptions option,
                std::string&& name,
                std::uint64_t)
            {
                found = false;
                if (directories.front() == name)
                {
                    found = true;
                    return false;
                }

                return true;
            });

            if(!found)
            {
                std::wstring utf16Name = utf8_to_wstring(path);
                if (!CreateDirectory(utf16Name.c_str(), nullptr))
                {
                    auto lastError = GetLastError();
                    ThrowWin32ErrorIfNot(lastError, (lastError == ERROR_ALREADY_EXISTS), std::string("Call to CreateDirectory failed creating: " + path).c_str());
                }
            }
            path = path + GetPathSeparator() + PopFirst();
            found = false;
        }
        while(directories.size() > 0);
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
                std::uint64_t size)
            {
                std::string fileName = root + GetPathSeparator() + name;
                // root contains the top level directory, which we don't need
                fileName = fileName.substr(rootSize);
                files.insert(std::make_pair(size, std::move(fileName)));
                return true;
            });
        return files;
    }
}

// Don't pollute other compilation units with any of our #defs...
#undef UNICODE

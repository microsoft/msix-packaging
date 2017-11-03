// ONLY build on platforms other than Win32
#ifdef WIN32
#include "Exceptions.hpp"
#include "DirectoryObject.hpp"
#include "FileStream.hpp"

#include <memory>
#include <iostream>
#include <string>
#include <sstream>
#include <locale>
#include <codecvt>
#include "AppxWindows.hpp"
#include "UnicodeConversion.hpp"

namespace xPlat {
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

    template <WalkOptions options, class Lambda>
    void WalkDirectory(const std::string& root, Lambda& visitor)
    {
        static std::string dot(".");
        static std::string dotdot("..");

        std::wstring utf16Name = utf8_to_utf16(root);

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

        do
        {
            utf16Name = std::wstring(findFileData.cFileName);
            auto utf8Name = utf16_to_utf8(utf16Name);

            if (((options & WalkOptions::Directories) == WalkOptions::Directories) &&
                (findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
                )
            {
                std::string child = (root + "\\" + utf8Name);
                if (!visitor(root, WalkOptions::Directories, std::move(utf8Name)))
                {
                    break;
                }
                if ((options & WalkOptions::Recursive) == WalkOptions::Recursive)
                {
                    WalkDirectory<options>(child, visitor);
                }
            }
            else if ((options & WalkOptions::Files) == WalkOptions::Files)
            {
                if (dot != utf8Name && dotdot != utf8Name)
                {
                    if (!visitor(root, WalkOptions::Files, std::move(utf8Name)))
                    {
                        break;
                    }
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

    std::string DirectoryObject::GetPathSeparator() { return "\\"; }

    std::vector<std::string> DirectoryObject::GetFileNames()
    {
        // TODO: Implement when standing-up the pack side for test validation purposes.
        throw Exception(Error::NotImplemented);
    }

    IStream* DirectoryObject::GetFile(const std::string& fileName)
    {
        // TODO: Implement when standing-up the pack side for test validation purposes.
        throw Exception(Error::NotImplemented);
    }

    void DirectoryObject::RemoveFile(const std::string& fileName)
    {
        // TODO: Implement when standing-up the pack side for test validation purposes.
        throw Exception(Error::NotImplemented);
    }

    IStream* DirectoryObject::OpenFile(const std::string& fileName, FileStream::Mode mode)
    {
        std::vector<std::string> directories;
        auto PopFirst = [&directories]()
        {
            auto result = directories.at(0);
            std::vector<std::string>(directories.begin() + 1, directories.end()).swap(directories);
            return result;
        };
        auto PopBack = [&directories]()
        {
            auto result = directories.at(directories.size() - 1);
            directories.pop_back();
            return result;
        };

        // Build a list of directory names to ensure exist
        std::istringstream stream(fileName);
        std::string directory;
        while (getline(stream, directory, '/'))
        {
            directories.push_back(std::move(directory));
        }
        auto name = PopBack(); // remove the actual file name from the list of directories, but keep just the name

        // Enforce that directory structure exists before creating file at specified location.
        bool found = false;
        std::string path = m_root;
        while (directories.size() != 0)
        {
            WalkDirectory<WalkOptions::Directories>(path + GetPathSeparator() + directories.front(), [&](
                std::string,
                WalkOptions option,
                std::string&& name)
            {
                found = false;
                if (directories.front() == name)
                {
                    found = true;
                    return false;
                }

                return true;
            });

            if (!found)
            {
                std::wstring utf16Name = utf8_to_utf16(path + GetPathSeparator() + directories.front());
                if (!CreateDirectory(utf16Name.c_str(), nullptr))
                {
                    auto lastError = GetLastError();
                    ThrowWin32ErrorIfNot(lastError, (lastError == ERROR_ALREADY_EXISTS), "CreateDirectory");
                }
            }
            path = path + GetPathSeparator() + PopFirst();
            found = false;
        }
        name = path + GetPathSeparator() + name;
        auto result = m_streams[fileName] = new FileStream(std::move(name), mode);
        return result.Get();
    }

    void DirectoryObject::CommitChanges()
    {
        m_streams.clear();
    }
}

// Don't pollute other compilation units with any of our #defs...
#undef UNICODE
#endif
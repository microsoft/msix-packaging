#include "Exceptions.hpp"
#include "DirectoryObject.hpp"
#include "FileStream.hpp"

#include <memory>
#include <iostream>
#include <string>
#include <sstream>
#include <locale>
#include <codecvt>

// UNICODE MUST be defined before you include Windows.h if you want the non-ascii versions of APIs (and you do)
#define UNICODE
#define NOMINMAX
#include <windows.h>

namespace xPlat {

    std::wstring utf8_to_utf16(const std::string& utf8string)
    {
        /*
        from: https://connect.microsoft.com/VisualStudio/feedback/details/1403302/unresolved-external-when-using-codecvt-utf8
        Posted by Microsoft on 2/16/2016 at 11:49 AM
        <snip>
        A workaround is to replace 'char32_t' with 'unsigned int'. In VS2013, char32_t was a typedef of 'unsigned int'.
        In VS2015, char32_t is a distinct type of it's own. Switching your use of 'char32_t' to 'unsigned int' will get
        you the old behavior from earlier versions and won't trigger a missing export error.

        There is also a similar error to this one with 'char16_t' that can be worked around using 'unsigned short'.
        <snip>
        */
        auto converted = std::wstring_convert<std::codecvt_utf8_utf16<unsigned short>, unsigned short>{}.from_bytes(utf8string.data());
        std::wstring result(converted.begin(), converted.end());
        return result;
    }

    std::string utf16_to_utf8(const std::wstring& utf16string)
    {
        auto converted = std::wstring_convert<std::codecvt_utf8<wchar_t>>{}.to_bytes(utf16string.data());
        std::string result(converted.begin(), converted.end());
        return result;
    }

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
            ThrowIf(lastError, false, "FindFirstFile failed.");
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
        ThrowIf(lastError,
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

    std::shared_ptr<StreamBase> DirectoryObject::GetFile(const std::string& fileName)
    {
        // TODO: Implement when standing-up the pack side for test validation purposes.
        throw Exception(Error::NotImplemented);
    }

    void DirectoryObject::RemoveFile(const std::string& fileName)
    {
        // TODO: Implement when standing-up the pack side for test validation purposes.
        throw Exception(Error::NotImplemented);
    }

    std::shared_ptr<StreamBase> DirectoryObject::OpenFile(const std::string& fileName, FileStream::Mode mode)
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
                    auto lastError = static_cast<std::uint32_t>(GetLastError());
                    ThrowIf(lastError, (lastError == ERROR_ALREADY_EXISTS), "CreateDirectory");
                }
            }
            path = path + GetPathSeparator() + PopFirst();
            found = false;
        }
        name = path + GetPathSeparator() + name;
        auto result = m_streams[fileName] = std::make_unique<FileStream>(std::move(name), mode);
        return result;
    }

    void DirectoryObject::CommitChanges()
    {
        m_streams.clear();
    }
}

// Don't pollute other compilation units with any of our #defs...
#undef UNICODE
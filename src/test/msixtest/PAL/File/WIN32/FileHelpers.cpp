//
//  Copyright (C) 2019 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
//
#include "msixtest_int.hpp"
#include "FileHelpers.hpp"

#include "Windows.h"

#include <algorithm>
#include <string>
#include <iostream>

namespace MsixTest {

    namespace Directory {

        template <class Lambda>
        bool WalkDirectory(const std::wstring& path, Lambda& visitor)
        {
            static std::wstring dot(L".");
            static std::wstring dotdot(L"..");

            auto directory = path + L"\\*";

            WIN32_FIND_DATA fileData = {};
            std::unique_ptr<std::remove_pointer<HANDLE>::type, decltype(&::FindClose)> handle(
                FindFirstFile(reinterpret_cast<LPCWSTR>(directory.c_str()), &fileData),
                &FindClose);

            if (handle.get() == INVALID_HANDLE_VALUE)
            {
                return false;
            }

            do
            {
                auto name = std::wstring(fileData.cFileName);
                if (dot != name && dotdot != name)
                {
                    auto newPath = path + L"\\" + name;
                    if (fileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
                    {
                        if(!WalkDirectory(newPath, visitor))
                        {
                            return false;
                        }
                    }

                    if (!visitor(newPath, &fileData))
                    {
                        return false;
                    }
                }
            } while(FindNextFile(handle.get(), &fileData));

            return true;
        }

        // best effort to clean the directory.
        bool CleanDirectory(const std::string& directory)
        {
            auto dir = MsixTest::Directory::PathAsCurrentPlatform(directory);
            auto dirUtf16 = String::utf8_to_utf16(dir);

            auto lambda = [](const std::wstring& file, PWIN32_FIND_DATA fileData)
            {
                if (fileData->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
                {
                    if (!RemoveDirectory(file.c_str()))
                    {
                        return false;
                    }
                }
                else
                {
                    if (!DeleteFile(file.c_str()))
                    {
                        return false;
                    }
                }
                return true;
            };

            if(!WalkDirectory(dirUtf16, lambda))
            {
                return false;
            }

            if (!RemoveDirectory(dirUtf16.c_str()))
            {
                return false;
            }

            return true;
        }

        bool CompareDirectory(const std::string& directory, const std::map<std::string, std::uint64_t>& files)
        {
            auto dir = MsixTest::Directory::PathAsCurrentPlatform(directory);
            auto dirUtf16 = String::utf8_to_utf16(dir);
            auto filesCopy(files);

            auto lambda = [&filesCopy, &dir](const std::wstring& wfile, PWIN32_FIND_DATA fileData)
            {
                if (fileData->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) { return true; }

                auto file = String::utf16_to_utf8(wfile);
                file.erase(0, dir.size() + 1); // Remove root directory
                std::replace(file.begin(), file.end(), '\\', '/'); // Replace windows separator

                auto find = filesCopy.find(file);
                if (find == filesCopy.end())
                {
                    std::cout << "File: " << file << " doesn't exists in expected files." << std::endl;
                    return false;
                }

                // Get size
                ULARGE_INTEGER fileSize;
                fileSize.HighPart = fileData->nFileSizeHigh;
                fileSize.LowPart = fileData->nFileSizeLow;
                std::uint64_t size = static_cast<std::uint64_t>(fileSize.QuadPart);
                if (find->second != size)
                {
                    std::cout << "File: " << file << " wrong size. Expected: " << find->second << " Got: " << size << std::endl;
                    return false;
                }
                filesCopy.erase(find);
                return true;
            };

            if(!WalkDirectory(dirUtf16, lambda))
            {
                return false;
            }

            // If the map is empty, then all the files are located
            return filesCopy.empty();
        }

        // Converts path to windows separator
        std::string PathAsCurrentPlatform(const std::string& path)
        {
            std::string result(path);
            std::replace(result.begin(), result.end(), '/', '\\');
            return result;
        }
    }
}

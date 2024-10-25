//
//  Copyright (C) 2019 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
// 
#pragma once
#include "AppxPackaging.hpp"

#include <vector>
#include <map>

namespace MsixTest { namespace Pack {

    void WriteContentToStream(std::uint64_t streamSize, IStream* contentStream);

    void MakeManifestStream(IStream** manifestStream);

    // Get files unpacked after packing testData/pack/input
    const std::map<std::string, std::uint64_t>& GetExpectedFiles(bool isSigned = false);

    namespace TestConstants {

        static const std::wstring ContentType = L"dummy/appx.test+txt";

        // Unicode byte sequence for the word "Japanese" in Japanese
        constexpr wchar_t UnicodeFileName[] = { 0x65e5, 0x672c, 0x8a9e, 0 };

        // Valid surrogate pairs (0xD800,0xDFFF) and (0xDBFF,0xDC00) bookended by non-surrogate characters
        #ifdef WIN32
        constexpr wchar_t ValidSurrogates[] = { 0xd7ff, 0xd800, 0xdfff, L'\\', 0xdbff, 0xdc00, 0xe000, 0 };
        #else
        constexpr wchar_t ValidSurrogates[] = { 0xd7ff, 0xd800, 0xdfff, L'/', 0xdbff, 0xdc00, 0xe000, 0 };
        #endif

        // Contains Unicode defined non-characters 0xfffe, 0xffff
        constexpr wchar_t NonCharacter1[] = { L'A', 0xfffe, L'.', L't', L'x', L't', 0 };
        constexpr wchar_t NonCharacter2[] = { L'B', 0xffff, L'.', L't', L'x', L't', 0 };

        // Invalid surrogate codepoints that do not form valid codepairs
        // Valid surrogate code pairs must consist of a high surrogate in the range [0xd800, 0xdbff]
        // followed by a low surrogate in the range [0xdc00, 0xdfff]
        constexpr wchar_t IncompleteHighSurrogate1[] = { 0xd800, 0 };
        constexpr wchar_t IncompleteHighSurrogate2[] = { 0xdbff, L'x', 0 };
        constexpr wchar_t ConsecutiveHighSurrogates[] = { 0xd800, 0xdbff, 0 };
        constexpr wchar_t UnexpectedLowSurrogate1[] = { 0xdc00, 0 };
        constexpr wchar_t UnexpectedLowSurrogate2[] = { L'x', 0xdfff, 0 };
        constexpr wchar_t ConsecutiveLowSurrogates[] = { 0xdbff, 0xdc00, 0xdfff, 0 };

        #ifdef WIN32
        constexpr wchar_t IncompleteHighSurrogate3[] = { L'x', 0xdbff, L'\\', L'x', 0 };
        constexpr wchar_t UnexpectedLowSurrogate3[] = { 0xd800, 0xdc00, L'\\', 0xdfff, 0 };
        #else
        constexpr wchar_t IncompleteHighSurrogate3[] = { L'x', 0xdbff, L'/', L'x', 0 };
        constexpr wchar_t UnexpectedLowSurrogate3[] = { 0xd800, 0xdc00, L'/', 0xdfff, 0 };
        #endif

        #ifdef WIN32
        // File names that should be valid for adding to a package
        // First from pair is the name that will be on disk
        // Second from pair is the actual name we are testing
        // This makes way easier the creating/cleaning of the file stream.
        const std::vector<std::pair<std::string, std::wstring>> GoodFileNames =
        { {
            { "test_file_1.txt" , L"test.txt"} ,
            { "test_file_2.txt" , L"essay.doc"} ,
            { "test_file_3.txt" , L"%%41.txt"} ,
            { "test_file_4.txt" , L"...a\\A@B!C#D$E~F%G...H"} ,
            { "test_file_5.txt" , L"%20ppxBlockMap.xml"} ,
            { "test_file_6.txt" , L" !#$%&'()+,-.0123456789;=@ABCDEFGHIJKLMNOPQRSTUVWXYZ[]^_`abcdefghijklmnopqrstuvwxyz{}~"} ,
            { "test_file_7.txt" , UnicodeFileName} ,
            { "test_file_8.txt" , ValidSurrogates} ,
            { "test_file_9.txt" , L".rels"} ,
            { "test_file_10.txt" , L"abc\\something.rels"} ,
            { "test_file_11.txt" , L"abc\\_rels\\blah.rels\\file.txt"} ,
            // File names that contain reserved folder names but are not at the root
            { "test_file_12.txt" , L"test\\MICROSOFT.system.PACKAGE.metadata"} ,
            { "test_file_13.txt" , L"test\\APPXmetadata"} ,
            // File names with segments 255 characters long, where the segment is alone, at the beginning, at the end, and in the middle
            { "test_file_14.txt" , L"abcdefghijklmnopqrstuvwxyabcdefghijklmnopqrstuvwxyabcdefghijklmnopqrstuvwxyabcdefghijklmnopqrstuvwxyabcdefghijklmnopqrstuvwxyabcdefghijklmnopqrstuvwxyabcdefghijklmnopqrstuvwxyabcdefghijklmnopqrstuvwxyabcdefghijklmnopqrstuvwxyabcdefghijklmnopqrstuvwxy12345"} ,
            { "test_file_15.txt" , L"_bcdefghijklmnopqrstuvwxyabcdefghijklmnopqrstuvwxyabcdefghijklmnopqrstuvwxyabcdefghijklmnopqrstuvwxyabcdefghijklmnopqrstuvwxyabcdefghijklmnopqrstuvwxyabcdefghijklmnopqrstuvwxyabcdefghijklmnopqrstuvwxyabcdefghijklmnopqrstuvwxyabcdefghijklmnopqrstuvwxy12345\\a"} ,
            { "test_file_16.txt" , L"a\\__cdefghijklmnopqrstuvwxyabcdefghijklmnopqrstuvwxyabcdefghijklmnopqrstuvwxyabcdefghijklmnopqrstuvwxyabcdefghijklmnopqrstuvwxyabcdefghijklmnopqrstuvwxyabcdefghijklmnopqrstuvwxyabcdefghijklmnopqrstuvwxyabcdefghijklmnopqrstuvwxyabcdefghijklmnopqrstuvwxy12345"} ,
            { "test_file_17.txt" , L"a\\___defghijklmnopqrstuvwxyabcdefghijklmnopqrstuvwxyabcdefghijklmnopqrstuvwxyabcdefghijklmnopqrstuvwxyabcdefghijklmnopqrstuvwxyabcdefghijklmnopqrstuvwxyabcdefghijklmnopqrstuvwxyabcdefghijklmnopqrstuvwxyabcdefghijklmnopqrstuvwxyabcdefghijklmnopqrstuvwxy12345\\b"} ,
            // File name with exactly MAX_PATH characters
            { "test_file_18.txt" , L"123456789\\123456789\\123456789\\123456789\\123456789\\123456789\\123456789\\123456789\\123456789\\123456789\\987654321\\987654321\\987654321\\987654321\\987654321\\987654321\\987654321\\987654321\\987654321\\987654321\\123456789\\123456789\\123456789\\123456789\\123456789\\1234567890" }
        } };

        // Filenames that should be invalid for adding to a package
        const std::vector<std::wstring> BadFileNames = {
            L"",
            L"/",
            L"\\abc",
            L"[CONTent_typeS].xMl",
            L"aPPxmAniFEst.xmL",
            L":ab",
            L"ab|",
            L"./abc\\something.txt",
            L"abc/test\\../more.txt",
            L"\\..",
            L"abc/.",
            L"/abc/def/..",
            L"/foldername/",
            L"\\\\UNCpath",
            L"/abc/\\test",
            NonCharacter1,
            NonCharacter2,
            IncompleteHighSurrogate1,
            IncompleteHighSurrogate2,
            IncompleteHighSurrogate3,
            ConsecutiveHighSurrogates,
            UnexpectedLowSurrogate1,
            UnexpectedLowSurrogate2,
            UnexpectedLowSurrogate3,
            ConsecutiveLowSurrogates,
            L"_Rels\\.rels",
            L"abc\\.def\\_rels\\blah.something.rELs",
            // File names under reserved folders
            L"MICROSOFT.system.PACKAGE.metadata",
            L"MIcroSOFt.SYStem.package.metadata\\test",
            L"APPXmetadata",
            L"appxMeTaDaTa\\test",
            // File names with segments 256 characters long, where the segment is alone, at the beginning, at the end, and in the middle
            L"abcdefghijklmnopqrstuvwxyabcdefghijklmnopqrstuvwxyabcdefghijklmnopqrstuvwxyabcdefghijklmnopqrstuvwxyabcdefghijklmnopqrstuvwxyabcdefghijklmnopqrstuvwxyabcdefghijklmnopqrstuvwxyabcdefghijklmnopqrstuvwxyabcdefghijklmnopqrstuvwxyabcdefghijklmnopqrstuvwxy123456",
            L"_bcdefghijklmnopqrstuvwxyabcdefghijklmnopqrstuvwxyabcdefghijklmnopqrstuvwxyabcdefghijklmnopqrstuvwxyabcdefghijklmnopqrstuvwxyabcdefghijklmnopqrstuvwxyabcdefghijklmnopqrstuvwxyabcdefghijklmnopqrstuvwxyabcdefghijklmnopqrstuvwxyabcdefghijklmnopqrstuvwxy123456\\a",
            L"a\\__cdefghijklmnopqrstuvwxyabcdefghijklmnopqrstuvwxyabcdefghijklmnopqrstuvwxyabcdefghijklmnopqrstuvwxyabcdefghijklmnopqrstuvwxyabcdefghijklmnopqrstuvwxyabcdefghijklmnopqrstuvwxyabcdefghijklmnopqrstuvwxyabcdefghijklmnopqrstuvwxyabcdefghijklmnopqrstuvwxy123456",
            L"a\\___defghijklmnopqrstuvwxyabcdefghijklmnopqrstuvwxyabcdefghijklmnopqrstuvwxyabcdefghijklmnopqrstuvwxyabcdefghijklmnopqrstuvwxyabcdefghijklmnopqrstuvwxyabcdefghijklmnopqrstuvwxyabcdefghijklmnopqrstuvwxyabcdefghijklmnopqrstuvwxyabcdefghijklmnopqrstuvwxy123456\\b",
        };

        #else
        // Same names, but change the backslash separator. Don't it programatically because there are some we want to keep
        const std::vector<std::pair<std::string, std::wstring>> GoodFileNames =
        { {
            { "test_file_1.txt" , L"test.txt"} ,
            { "test_file_2.txt" , L"essay.doc"} ,
            { "test_file_3.txt" , L"%%41.txt"} ,
            { "test_file_4.txt" , L"...a/A@B!C#D$E~F%G...H"} ,
            { "test_file_5.txt" , L"%20ppxBlockMap.xml"} ,
            { "test_file_6.txt" , L" !#$%&'()+,-.0123456789;=@ABCDEFGHIJKLMNOPQRSTUVWXYZ[]^_`abcdefghijklmnopqrstuvwxyz{}~"} ,
            { "test_file_7.txt" , UnicodeFileName} ,
            // { "test_file_8.txt" , ValidSurrogates} , Not valid on non-Windows. wstring_convert:: to_bytes throws std::range_error
            { "test_file_9.txt" , L".rels"} ,
            { "test_file_10.txt" , L"abc/something.rels"} ,
            { "test_file_11.txt" , L"abc/_rels/blah.rels/file.txt"} ,
            // File names that contain reserved folder names but are not at the root
            { "test_file_12.txt" , L"test/MICROSOFT.system.PACKAGE.metadata"} ,
            { "test_file_13.txt" , L"test/APPXmetadata"} ,
            // File names with segments 255 characters long, where the segment is alone, at the beginning, at the end, and in the middle
            { "test_file_14.txt" , L"abcdefghijklmnopqrstuvwxyabcdefghijklmnopqrstuvwxyabcdefghijklmnopqrstuvwxyabcdefghijklmnopqrstuvwxyabcdefghijklmnopqrstuvwxyabcdefghijklmnopqrstuvwxyabcdefghijklmnopqrstuvwxyabcdefghijklmnopqrstuvwxyabcdefghijklmnopqrstuvwxyabcdefghijklmnopqrstuvwxy12345"} ,
            { "test_file_15.txt" , L"_bcdefghijklmnopqrstuvwxyabcdefghijklmnopqrstuvwxyabcdefghijklmnopqrstuvwxyabcdefghijklmnopqrstuvwxyabcdefghijklmnopqrstuvwxyabcdefghijklmnopqrstuvwxyabcdefghijklmnopqrstuvwxyabcdefghijklmnopqrstuvwxyabcdefghijklmnopqrstuvwxyabcdefghijklmnopqrstuvwxy12345/a"} ,
            { "test_file_16.txt" , L"a/__cdefghijklmnopqrstuvwxyabcdefghijklmnopqrstuvwxyabcdefghijklmnopqrstuvwxyabcdefghijklmnopqrstuvwxyabcdefghijklmnopqrstuvwxyabcdefghijklmnopqrstuvwxyabcdefghijklmnopqrstuvwxyabcdefghijklmnopqrstuvwxyabcdefghijklmnopqrstuvwxyabcdefghijklmnopqrstuvwxy12345"} ,
            { "test_file_17.txt" , L"a/___defghijklmnopqrstuvwxyabcdefghijklmnopqrstuvwxyabcdefghijklmnopqrstuvwxyabcdefghijklmnopqrstuvwxyabcdefghijklmnopqrstuvwxyabcdefghijklmnopqrstuvwxyabcdefghijklmnopqrstuvwxyabcdefghijklmnopqrstuvwxyabcdefghijklmnopqrstuvwxyabcdefghijklmnopqrstuvwxy12345/b"} ,
            // File name with exactly MAX_PATH characters
            { "test_file_18.txt" , L"123456789/123456789/123456789/123456789/123456789/123456789/123456789/123456789/123456789/123456789/987654321/987654321/987654321/987654321/987654321/987654321/987654321/987654321/987654321/987654321/123456789/123456789/123456789/123456789/123456789/1234567890" }
        } };

        const std::vector<std::wstring> BadFileNames = {
            L"",
            L"/",
            L"/abc",
            L"[CONTent_typeS].xMl",
            L"aPPxmAniFEst.xmL",
            L":ab",
            L"ab|",
            L".\\abc/something.txt",
            L"abc\\test/..\\more.txt",
            L"/..",
            L"abc/.",
            L"/abc/def/..",
            L"/foldername/",
            L"//UNCpath",
            L"/abc/\\test",
            NonCharacter1,
            NonCharacter2,
            IncompleteHighSurrogate1,
            IncompleteHighSurrogate2,
            IncompleteHighSurrogate3,
            ConsecutiveHighSurrogates,
            UnexpectedLowSurrogate1,
            UnexpectedLowSurrogate2,
            UnexpectedLowSurrogate3,
            ConsecutiveLowSurrogates,
            L"_Rels/.rels",
            L"abc/.def/_rels/blah.something.rELs",
            // File names under reserved folders
            L"MICROSOFT.system.PACKAGE.metadata",
            L"MIcroSOFt.SYStem.package.metadata/test",
            L"APPXmetadata",
            L"appxMeTaDaTa/test",
            // File names with segments 256 characters long, where the segment is alone, at the beginning, at the end, and in the middle
            L"abcdefghijklmnopqrstuvwxyabcdefghijklmnopqrstuvwxyabcdefghijklmnopqrstuvwxyabcdefghijklmnopqrstuvwxyabcdefghijklmnopqrstuvwxyabcdefghijklmnopqrstuvwxyabcdefghijklmnopqrstuvwxyabcdefghijklmnopqrstuvwxyabcdefghijklmnopqrstuvwxyabcdefghijklmnopqrstuvwxy123456",
            L"_bcdefghijklmnopqrstuvwxyabcdefghijklmnopqrstuvwxyabcdefghijklmnopqrstuvwxyabcdefghijklmnopqrstuvwxyabcdefghijklmnopqrstuvwxyabcdefghijklmnopqrstuvwxyabcdefghijklmnopqrstuvwxyabcdefghijklmnopqrstuvwxyabcdefghijklmnopqrstuvwxyabcdefghijklmnopqrstuvwxy123456/a",
            L"a/__cdefghijklmnopqrstuvwxyabcdefghijklmnopqrstuvwxyabcdefghijklmnopqrstuvwxyabcdefghijklmnopqrstuvwxyabcdefghijklmnopqrstuvwxyabcdefghijklmnopqrstuvwxyabcdefghijklmnopqrstuvwxyabcdefghijklmnopqrstuvwxyabcdefghijklmnopqrstuvwxyabcdefghijklmnopqrstuvwxy123456",
            L"a/___defghijklmnopqrstuvwxyabcdefghijklmnopqrstuvwxyabcdefghijklmnopqrstuvwxyabcdefghijklmnopqrstuvwxyabcdefghijklmnopqrstuvwxyabcdefghijklmnopqrstuvwxyabcdefghijklmnopqrstuvwxyabcdefghijklmnopqrstuvwxyabcdefghijklmnopqrstuvwxyabcdefghijklmnopqrstuvwxy123456/b",
        };
        #endif
    }
} }

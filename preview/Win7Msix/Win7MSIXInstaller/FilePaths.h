#pragma once
#include <map>
#include <string>
#include "GeneralUtil.h"

//
// A function that returns a map that links paths in package to 
// to the local computer paths using KnownFolderIDs
//
std::map < std::wstring, std::wstring > GetMap();

//
// Removes the first directory from a path
//
// Parameters:
// path - A path that contains at least one parent directory
//
void GetPathChild(std::wstring &path);

//
// Removes the innermost child file from a path
//
// Parameters:
// path - A file path 
//
void GetPathParent(std::wstring &path);

//
// Obtains the local computer execution path 
//
// Parameters:
// executionPath - The path to executable in the MSIX package
// displayName   - The display name of the application that is used as the name of the 
//				   folder in WindowsApps
// 
std::wstring GetExecutablePath(WCHAR* packageExecutablePath, WCHAR* displayName);
#pragma once

#include "GeneralUtil.h"
#include "Uninstall.h"

//
// Extracts all files from a package.
//
// Parameters:
// package	  - The package reader for the app package.
// outputPath - The path of the folder for the extracted payload files.
//
HRESULT ExtractPackage(IAppxPackageReader* package, LPCWSTR outputPath, TrackerXML* myXml);

//
// A helper function that takes in a filepath from the VFS and a COM pointer to an IAppxFile
// and extracts the IAppxFile to the local computer based on the correct KnownFolderID from
// the GetMap()
// 
// Parameters:
// file	   - A COM pointer to an IAppxFile that is obtained from a MSIX/APPX package
// nameStr - A filepath of the file in the VFS (e.g. VFS\Program Files (x86)\Notepadplusplus\notepadplusplus.exe)
//
void ExtractVFSToLocal(IAppxFile* file, std::wstring nameStr, TrackerXML* myXML);

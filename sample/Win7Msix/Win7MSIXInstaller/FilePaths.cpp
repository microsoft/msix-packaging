#include "FilePaths.h"
#include "GeneralUtil.h"
#include <shlobj_core.h>
#include <KnownFolders.h>

//
// A function that returns a map that links paths in package to 
// to the local computer paths using KnownFolderIDs
//
std::map<std::wstring,std::wstring> GetMap()
{
    std::map<std::wstring, std::wstring> map;

    Text<WCHAR> systemX86Path;
    Text<WCHAR> systemPath;
    Text<WCHAR> programFilesX86Path;
    Text<WCHAR> programFilesCommonX86Path;
    Text<WCHAR> windowsPath;
    Text<WCHAR> commonAppDataPath;
    Text<WCHAR> localAppDataPath;
    Text<WCHAR> appDataPath;

    SHGetKnownFolderPath(FOLDERID_SystemX86, 0, NULL, &systemX86Path);
    SHGetKnownFolderPath(FOLDERID_System, 0, NULL, &systemPath);
    SHGetKnownFolderPath(FOLDERID_ProgramFilesX86, 0, NULL, &programFilesX86Path);
    SHGetKnownFolderPath(FOLDERID_ProgramFilesCommonX86, 0, NULL, &programFilesCommonX86Path);
    SHGetKnownFolderPath(FOLDERID_Windows, 0, NULL, &windowsPath);
    SHGetKnownFolderPath(FOLDERID_ProgramData, 0, NULL, &commonAppDataPath);

    std::wstring appVSystem32CatrootPath = std::wstring(systemPath.Get());
    std::wstring appVSystem32Catroot2Path = std::wstring(systemPath.Get());
    std::wstring appVSystem32DriversEtcPath = std::wstring(systemPath.Get());
    std::wstring appVSystem32DriverstorePath = std::wstring(systemPath.Get());
    std::wstring appVSystem32LogfilesPath = std::wstring(systemPath.Get());
    std::wstring appVSystem32SpoolPath = std::wstring(systemPath.Get());

    appVSystem32CatrootPath.append(L"\\catroot");
    appVSystem32Catroot2Path.append(L"\\catroot2");
    appVSystem32DriversEtcPath.append(L"\\drivers\\etc");
    appVSystem32DriverstorePath.append(L"\\driverstore");
    appVSystem32LogfilesPath.append(L"\\logfiles");
    appVSystem32SpoolPath.append(L"\\spool");

    map[L"VFS\\SystemX86\\"] = std::wstring(systemX86Path.Get());
    map[L"VFS\\System\\"] = std::wstring(systemPath.Get());
    map[L"VFS\\ProgramFilesX86\\"] = std::wstring(programFilesX86Path.Get());
    map[L"VFS\\ProgramFilesCommonX86\\"] = std::wstring(programFilesCommonX86Path.Get());
    map[L"VFS\\Windows\\"] = std::wstring(windowsPath.Get());
    map[L"VFS\\Common AppData\\"] = std::wstring(commonAppDataPath.Get());
    map[L"VFS\\AppVSystem32Catroot\\"] = appVSystem32CatrootPath;
    map[L"VFS\\AppVSystem32Catroot2\\"] = appVSystem32Catroot2Path;
    map[L"VFS\\AppVSystems32DriversEtc\\"] = appVSystem32DriversEtcPath;
    map[L"VFS\\AppVSystems32Driverstore\\"] = appVSystem32DriverstorePath;
    map[L"VFS\\AppVSystems32Logfiles\\"] = appVSystem32LogfilesPath;
    map[L"VFS\\AppVSystems32Spool\\"] = appVSystem32SpoolPath;

#ifdef _WIN64
    Text<WCHAR> programFilesX64Path;
    Text<WCHAR> programFilesCommonX64Path;
    Text<WCHAR> systemX64Path;
    SHGetKnownFolderPath(FOLDERID_System, 0, NULL, &systemX64Path);
    SHGetKnownFolderPath(FOLDERID_ProgramFilesCommonX64, 0, NULL, &programFilesCommonX64Path);
    SHGetKnownFolderPath(FOLDERID_ProgramFilesX64, 0, NULL, &programFilesX64Path);
    map[L"VFS\\ProgramFilesX64\\"] = std::wstring(programFilesX64Path.Get());
    map[L"VFS\\ProgramFilesCommonX64\\"] = std::wstring(programFilesCommonX64Path.Get());
    map[L"VFS\\SystemX64\\"] = std::wstring(systemX64Path.Get());
#endif

    return map;
}

//
// Removes the first directory from a path
//
// Parameters:
// path - A path that contains at least one parent directory
//
void GetPathChild(std::wstring &path)
{
    while (path.front() != '\\')
    {
        path.erase(0, 1);
    }
    path.erase(0, 1);
}

//
// Removes the innermost child file from a path
//
// Parameters:
// path - A file path 
//
void GetPathParent(std::wstring &path)
{
    while (!path.empty() && path.back() != '\\')
    {
        if (path.length() != 1)
        {
            path.erase(path.end() - 1, path.end());
        }
        else
        {
            path.erase(0, 1);
        }
    }
}

//
// Obtains the local computer execution path 
//
// Parameters:
// executionPath - The path to executable in the MSIX package
// packageFullName   - The package full name of the application that is used as the name of the 
//				   folder in WindowsApps
// 
std::wstring GetExecutablePath(WCHAR* packageExecutablePath, WCHAR* packageFullName)
{
    std::wstring executionPathWSTR = packageExecutablePath;
    auto map = GetMap();
    
    //Checks if the executable is inside the VFS
    if (executionPathWSTR.find(L"VFS") != std::wstring::npos)
    {
        //Checks if the executable is in one of the known folders
        for (auto pair : map) 
        {
            if (executionPathWSTR.find(pair.first) != std::wstring::npos)
            {
                //The executable exists in an unpacked directory
                std::wstring executablePath = pair.second;
                GetPathChild(executionPathWSTR);
                GetPathChild(executionPathWSTR);
                executablePath.push_back(L'\\');
                executablePath.append(executionPathWSTR);
                return executablePath;
            }
        }
    }

    //The executable path exists in the root appx directory and was not unpacked
    std::wstring executablePath = L"C:\\Program Files (x86)\\WindowsApps\\";
    executablePath.append(packageFullName);
    executablePath.push_back(L'\\');
    executablePath.append(packageExecutablePath);
    return executablePath;
}
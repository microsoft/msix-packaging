#include "FilePaths.hpp"
#include "GeneralUtil.hpp"
#include <shlobj_core.h>
#include <KnownFolders.h>

using namespace Win7MsixInstallerLib;

void Win7MsixInstallerLib_GetPathChild(std::wstring &path)
{
    while (path.front() != '\\')
    {
        path.erase(0, 1);
    }
    path.erase(0, 1);
}

void Win7MsixInstallerLib_GetPathParent(std::wstring &path)
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

    if (!path.empty() && path.length() != 1)
    {
        path.erase(path.end() - 1, path.end());
    }
}

FilePathMappings& FilePathMappings::GetInstance()
{
    static FilePathMappings s_selfInstance;
    return s_selfInstance;
}

std::wstring FilePathMappings::GetExecutablePath(std::wstring packageExecutablePath, PCWSTR packageFullName)
{
    // make a local copy so we can modify in place
    std::wstring executionPathWSTR = packageExecutablePath;
    
    //Checks if the executable is inside the VFS
    if (executionPathWSTR.find(L"VFS") != std::wstring::npos)
    {
        Win7MsixInstallerLib_GetPathChild(executionPathWSTR);
        //Checks if the executable is in one of the known folders
        for (auto pair : m_map) 
        {
            if (executionPathWSTR.find(pair.first) != std::wstring::npos)
            {
                //The executable exists in an unpacked directory
                std::wstring executablePath = pair.second;
                
                Win7MsixInstallerLib_GetPathChild(executionPathWSTR);
                executablePath.push_back(L'\\');
                executablePath.append(executionPathWSTR);
                return executablePath;
            }
        }
    }

    //The executable path exists in the root msix directory and was not devirtualized
    std::wstring executablePath = m_msix7Directory;
    executablePath.append(packageFullName);
    executablePath.push_back(L'\\');
    executablePath.append(packageExecutablePath);
    return executablePath;
}

HRESULT FilePathMappings::InitializePaths()
{
    TextOle<WCHAR> systemX86Path;
    TextOle<WCHAR> systemPath;
    TextOle<WCHAR> programFilesX86Path;
    TextOle<WCHAR> programFilesCommonX86Path;
    TextOle<WCHAR> windowsPath;
    TextOle<WCHAR> commonAppDataPath;
    TextOle<WCHAR> localAppDataPath;
    TextOle<WCHAR> appDataPath;
    TextOle<WCHAR> commonProgramsPath;

    RETURN_IF_FAILED(SHGetKnownFolderPath(FOLDERID_SystemX86, 0, NULL, &systemX86Path));
    RETURN_IF_FAILED(SHGetKnownFolderPath(FOLDERID_System, 0, NULL, &systemPath));
    RETURN_IF_FAILED(SHGetKnownFolderPath(FOLDERID_ProgramFilesX86, 0, NULL, &programFilesX86Path));
    RETURN_IF_FAILED(SHGetKnownFolderPath(FOLDERID_ProgramFilesCommonX86, 0, NULL, &programFilesCommonX86Path));
    RETURN_IF_FAILED(SHGetKnownFolderPath(FOLDERID_Windows, 0, NULL, &windowsPath));
    RETURN_IF_FAILED(SHGetKnownFolderPath(FOLDERID_ProgramData, 0, NULL, &commonAppDataPath));
    RETURN_IF_FAILED(SHGetKnownFolderPath(FOLDERID_RoamingAppData, 0, NULL, &appDataPath));
    RETURN_IF_FAILED(SHGetKnownFolderPath(FOLDERID_CommonPrograms, 0, NULL, &commonProgramsPath));

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

    std::wstring systemDrive = std::wstring(windowsPath.Get());
    Win7MsixInstallerLib_GetPathParent(systemDrive);
    m_map[L"AppVPackageDrive"] = systemDrive;
    m_map[L"SystemX86"] = std::wstring(systemX86Path.Get());
    m_map[L"System"] = std::wstring(systemPath.Get());
    m_map[L"ProgramFilesX86"] = std::wstring(programFilesX86Path.Get());
    m_map[L"ProgramFilesCommonX86"] = std::wstring(programFilesCommonX86Path.Get());
    m_map[L"Windows"] = std::wstring(windowsPath.Get());
    m_map[L"Common AppData"] = std::wstring(commonAppDataPath.Get());
    m_map[L"Common Programs"] = std::wstring(commonProgramsPath.Get());
    m_map[L"AppData"] = std::wstring(appDataPath.Get());
    m_map[L"AppVSystem32Catroot"] = appVSystem32CatrootPath;
    m_map[L"AppVSystem32Catroot2"] = appVSystem32Catroot2Path;
    m_map[L"AppVSystems32DriversEtc"] = appVSystem32DriversEtcPath;
    m_map[L"AppVSystems32Driverstore"] = appVSystem32DriverstorePath;
    m_map[L"AppVSystems32Logfiles"] = appVSystem32LogfilesPath;
    m_map[L"AppVSystems32Spool"] = appVSystem32SpoolPath;

#ifdef _WIN64
    TextOle<WCHAR> programFilesX64Path;
    TextOle<WCHAR> programFilesCommonX64Path;
    TextOle<WCHAR> systemX64Path;
    RETURN_IF_FAILED(SHGetKnownFolderPath(FOLDERID_System, 0, NULL, &systemX64Path));
    RETURN_IF_FAILED(SHGetKnownFolderPath(FOLDERID_ProgramFilesCommonX64, 0, NULL, &programFilesCommonX64Path));
    RETURN_IF_FAILED(SHGetKnownFolderPath(FOLDERID_ProgramFilesX64, 0, NULL, &programFilesX64Path));
    m_map[L"ProgramFilesX64"] = std::wstring(programFilesX64Path.Get());
    m_map[L"ProgramFilesCommonX64"] = std::wstring(programFilesCommonX64Path.Get());
    m_map[L"SystemX64"] = std::wstring(systemX64Path.Get());
#endif

    TextOle<WCHAR> programFilesPath;
    RETURN_IF_FAILED(SHGetKnownFolderPath(FOLDERID_ProgramFiles, 0, NULL, &programFilesPath));

    m_msix7Directory = std::wstring(programFilesPath.Get()) + std::wstring(L"\\Msix7apps\\");
    return S_OK;
}

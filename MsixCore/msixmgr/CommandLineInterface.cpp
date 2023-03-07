#include <iostream>
#include <iomanip>
#include "CommandLineInterface.hpp"
#include "msixmgrLogger.hpp"
#include <TraceLoggingProvider.h>
#include "..\msixmgrLib\GeneralUtil.hpp"
#include "Util.hpp"
#include "resource.h"

using namespace MsixCoreLib;

std::map<std::wstring, Options, CaseInsensitiveLess> CommandLineInterface::s_options =
{
    {
        L"-AddPackage",
        Options(true, IDS_STRING_HELP_OPTION_ADDPACKAGE,
            [&](CommandLineInterface* commandLineInterface, const std::string& path)
        {
            if (commandLineInterface->m_operationType != OperationType::Undefined)
            {
                return E_INVALIDARG;
            }
            commandLineInterface->m_operationType = OperationType::Add;
            commandLineInterface->m_packageFilePath = utf8_to_utf16(path);
            return S_OK;
        })
    },
    {
        L"-RemovePackage",
        Options(true, IDS_STRING_HELP_OPTION_REMOVEPACKAGE,
            [&](CommandLineInterface* commandLineInterface, const std::string& packageFullName)
        {
            if (commandLineInterface->m_operationType != OperationType::Undefined)
            {
                return E_INVALIDARG;
            }
            commandLineInterface->m_operationType = OperationType::Remove;
            commandLineInterface->m_packageFullName = utf8_to_utf16(packageFullName);
            return S_OK;
        })
    },
    {
        L"-quietUX",
        Options(false, IDS_STRING_HELP_OPTION_QUIETMODE,
            [&](CommandLineInterface* commandLineInterface, const std::string&)
        {
            commandLineInterface->m_quietMode = true;
            return S_OK;
        })
    },
    {
        L"-FindPackage",
        Options(true, IDS_STRING_HELP_OPTION_FINDPACKAGE,
            [&](CommandLineInterface* commandLineInterface, const std::string& packageFullName)
        {
            if (commandLineInterface->m_operationType != OperationType::Undefined)
            {
                return E_INVALIDARG;
            }
            commandLineInterface->m_operationType = OperationType::FindPackage;
            commandLineInterface->m_packageFullName = utf8_to_utf16(packageFullName);
            return S_OK;
        })
    },
    {
        L"-Unpack",
        Options(false, IDS_STRING_HELP_OPTION_UNPACK,
            [&](CommandLineInterface* commandLineInterface, const std::string&)
        {
            if (commandLineInterface->m_operationType != OperationType::Undefined)
            {
                return E_INVALIDARG;
            }
            commandLineInterface->m_operationType = OperationType::Unpack;
            return S_OK;
        },
        // suboptions
        {
            {
                L"-packagePath",
                Option(true, IDS_STRING_HELP_OPTION_UNPACK_PACKAGEPATH,
                    [&](CommandLineInterface* commandLineInterface, const std::string& packagePath)
                {
                if (commandLineInterface->m_operationType != OperationType::Unpack)
                {
                    return E_INVALIDARG;
                }
                commandLineInterface->m_packageFilePath = utf8_to_utf16(packagePath);
                return S_OK;
                }),
            },
            {
                L"-destination",
                Option(true, IDS_STRING_HELP_OPTION_UNPACK_DESTINATION,
                    [&](CommandLineInterface* commandLineInterface, const std::string& destination)
                {
                    if (commandLineInterface->m_operationType != OperationType::Unpack)
                    {
                        return E_INVALIDARG;
                    }
                    commandLineInterface->m_unpackDestination = utf8_to_utf16(destination);
                    return S_OK;
                }),
            },
            {
                L"-applyACLs",
                Option(false, IDS_STRING_HELP_OPTION_UNPACK_APPLYACLS,
                    [&](CommandLineInterface* commandLineInterface, const std::string&)
                {
                    if (commandLineInterface->m_operationType != OperationType::Unpack)
                    {
                        return E_INVALIDARG;
                    }
                    commandLineInterface->m_applyACLs = true;
                    return S_OK;
                }),
            },
            {
                L"-validateSignature",
                Option(false, IDS_STRING_HELP_OPTION_UNPACK_VALIDATESIGNATURE,
                    [&](CommandLineInterface* commandLineInterface, const std::string&)
                {
                    if (commandLineInterface->m_operationType != OperationType::Unpack)
                    {
                        return E_INVALIDARG;
                    }
                    commandLineInterface->m_validateSignature = true;
                    return S_OK;
                }),
            },
            {
                L"-create",
                Option(false, IDS_STRING_HELP_OPTION_UNPACK_CREATE,
                    [&](CommandLineInterface* commandLineInterface, const std::string&)
                {
                    if (commandLineInterface->m_operationType != OperationType::Unpack)
                    {
                        return E_INVALIDARG;
                    }
                    commandLineInterface->m_create = true;
                    return S_OK;
                }),
            },
            {
                L"-rootDirectory",
                Option(true, IDS_STRING_HELP_OPTION_UNPACK_ROOTDIRECTORY,
                    [&](CommandLineInterface* commandLineInterface, const std::string& rootDirectory)
                {
                    if (commandLineInterface->m_operationType != OperationType::Unpack)
                    {
                        return E_INVALIDARG;
                    }
                    commandLineInterface->m_rootDirectory = utf8_to_utf16(rootDirectory);
                    return S_OK;
                }),
            },
            {
                L"-fileType",
                Option(true, IDS_STRING_HELP_OPTION_UNPACK_FILETYPE,
                    [&](CommandLineInterface* commandLineInterface, const std::string& fileType)
                {
                    if (commandLineInterface->m_operationType != OperationType::Unpack)
                    {
                        return E_INVALIDARG;
                    }
                    commandLineInterface->SetWVDFileType(utf8_to_utf16(fileType));

                    return S_OK;
                }),
            },
            {
                L"-vhdSize",
                Option(true, IDS_STRING_HELP_OPTION_UNPACK_VHDSIZE,
                    [&](CommandLineInterface* commandLineInterface, const std::string& vhdSize)
                {
                    if (commandLineInterface->m_operationType != OperationType::Unpack)
                    {
                        return E_INVALIDARG;
                    }
                    
                    ULONGLONG maxVhdSizeMB = 2040000ull; // MAX SIZE: 2040 GB --> 2040000 MB
                    errno = 0;
                    ULONGLONG vhdSizeUll = strtoull(vhdSize.c_str(), NULL, 10 /*base*/);
                    if ((vhdSizeUll == ULLONG_MAX && errno == ERANGE) ||
                        vhdSizeUll > maxVhdSizeMB ||
                        vhdSizeUll < 5ull)
                    {
                        std::wcout << "\nInvalid VHD size. Specified value must be at least 5 MB and at most 2040000 MB\n" << std::endl;
                        return E_INVALIDARG;
                    }
  
                    commandLineInterface->m_vhdSize = vhdSizeUll;
                    return S_OK;
                }),
            }
        })
    },
    {
        L"-ApplyACLs",
        Options(false, IDS_STRING_HELP_OPTION_APPLYACLS,
        [&](CommandLineInterface* commandLineInterface, const std::string&)
        {
            if (commandLineInterface->m_operationType != OperationType::Undefined)
            {
                return E_INVALIDARG;
            }
            commandLineInterface->m_operationType = OperationType::ApplyACLs;
            return S_OK;
        },
        {
            {
                L"-packagePath",
                Option(true, IDS_STRING_HELP_OPTION_APPLYACLS_PACKAGEPATH,
                    [&](CommandLineInterface* commandLineInterface, const std::string& packagePath)
                {
                    if (commandLineInterface->m_operationType != OperationType::ApplyACLs)
                    {
                        return E_INVALIDARG;
                    }
                    commandLineInterface->m_packageFilePath = utf8_to_utf16(packagePath);
                    return S_OK;
                }),
            }
        })
    },
    {
        L"-MountImage",
        Options(false, IDS_STRING_HELP_OPTION_MOUNTIMAGE,
        [&](CommandLineInterface* commandLineInterface, const std::string&)
        {
            if (commandLineInterface->m_operationType != OperationType::Undefined)
            {
                return E_INVALIDARG;
            }
            commandLineInterface->m_operationType = OperationType::MountImage;
            return S_OK;
        },
        {
            {
                L"-imagePath",
                Option(true, IDS_STRING_HELP_OPTION_MOUNTIMAGE_IMAGEPATH,
                    [&](CommandLineInterface* commandLineInterface, const std::string& imagePath)
                {
                    if (commandLineInterface->m_operationType != OperationType::MountImage)
                    {
                        return E_INVALIDARG;
                    }
                    commandLineInterface->m_mountImagePath = utf8_to_utf16(imagePath);
                    return S_OK;
                }),
            },
            {
                L"-fileType",
                Option(true, IDS_STRING_HELP_OPTION_MOUNT_FILETYPE,
                    [&](CommandLineInterface* commandLineInterface, const std::string& fileType)
                {
                    if (commandLineInterface->m_operationType != OperationType::MountImage)
                    {
                        return E_INVALIDARG;
                    }
                    commandLineInterface->SetWVDFileType(utf8_to_utf16(fileType));
                    return S_OK;
                }),
            },
            {
                L"-readOnly",
                Option(true, IDS_STRING_HELP_OPTION_MOUNT_READONLY,
                    [&](CommandLineInterface* commandLineInterface, const std::string& readOnly)
                {
                    if (commandLineInterface->m_operationType != OperationType::MountImage)
                    {
                        return E_INVALIDARG;
                    }
                    if (CaseInsensitiveEquals(utf8_to_utf16(readOnly), std::wstring(L"true")))
                    {
                        commandLineInterface->m_readOnly = true;
                    }
                    else if (CaseInsensitiveEquals(utf8_to_utf16(readOnly), std::wstring(L"false")))
                    {
                        commandLineInterface->m_readOnly = false;
                    }
                    else
                    {
                        return E_INVALIDARG;
                    }
                    return S_OK;
                }),
            }
        })
    },
    {
        L"-UnmountImage",
        Options(false, IDS_STRING_HELP_OPTION_UNMOUNTIMAGE,
        [&](CommandLineInterface* commandLineInterface, const std::string&)
        {
            if (commandLineInterface->m_operationType != OperationType::Undefined)
            {
                return E_INVALIDARG;
            }
            commandLineInterface->m_operationType = OperationType::UnmountImage;
            return S_OK;
        },
        {
            {
                L"-imagePath",
                Option(true, IDS_STRING_HELP_OPTION_MOUNTIMAGE_IMAGEPATH,
                    [&](CommandLineInterface* commandLineInterface, const std::string& imagePath)
                {
                    if (commandLineInterface->m_operationType != OperationType::UnmountImage)
                    {
                        return E_INVALIDARG;
                    }
                    commandLineInterface->m_mountImagePath = utf8_to_utf16(imagePath);
                    return S_OK;
                }),
            },
            {
                L"-volumeId",
                Option(true, IDS_STRING_HELP_OPTION_UNMOUNTIMAGE_VOLUMEID,
                    [&](CommandLineInterface* commandLineInterface, const std::string& volumeId)
                {
                    if (commandLineInterface->m_operationType != OperationType::UnmountImage)
                    {
                        return E_INVALIDARG;
                    }
                    commandLineInterface->m_volumeId = utf8_to_utf16(volumeId);
                    return S_OK;
                }),
            },
            {
                L"-fileType",
                Option(true, IDS_STRING_HELP_OPTION_MOUNT_FILETYPE,
                    [&](CommandLineInterface* commandLineInterface, const std::string& fileType)
                {
                    if (commandLineInterface->m_operationType != OperationType::UnmountImage)
                    {
                        return E_INVALIDARG;
                    }
                    commandLineInterface->SetWVDFileType(utf8_to_utf16(fileType));
                    return S_OK;
                }),
            }
        })
    },
    {
        L"-?",
        Options(false, IDS_STRING_HELP_OPTION_HELP,
            [&](CommandLineInterface*, const std::string&)
        {
            return E_INVALIDARG;
        })
    }
};

std::map<std::wstring, std::wstring> CommandLineInterface::s_optionAliases =
{
    {L"-p", L"-AddPackage"},
    {L"-x", L"-RemovePackage"},
};

void CommandLineInterface::DisplayHelp()
{
    std::wcout << GetStringResource(IDS_STRING_HELPTEXT_USAGE) << std::endl;
    std::wcout << GetStringResource(IDS_STRING_HELPTEXT_DESCRIPTION) << std::endl;
    std::wcout << GetStringResource(IDS_STRING_HELPTEXT_OPTIONS) << std::endl;

    for (const auto& option : CommandLineInterface::s_options)
    {
        std::wcout << L"\t" << std::left << std::setfill(L' ') << std::setw(5) <<
            option.first << L": " << GetStringResource(option.second.Help) << std::endl;

        if (option.second.HasSuboptions)
        {
            for (const auto& suboption : option.second.Suboptions)
            {
                std::wcout << L"\t\t" << std::left << std::setfill(L' ') << std::setw(5) <<
                    suboption.first << L": " << GetStringResource(suboption.second.Help) << std::endl;
            }
        }
    }
}

HRESULT CommandLineInterface::Init()
{
    for (int i = 0; i < m_argc; i++)
    {
        TraceLoggingWrite(g_MsixUITraceLoggingProvider,
            "CommandLineArguments",
            TraceLoggingValue(i, "index"),
            TraceLoggingValue(m_argv[i], "arg"));
    }

    if (m_argc < 2)
    {
        return E_INVALIDARG;
    }

    int index = 1;
    while (index < m_argc)
    {
        std::wstring optionString = utf8_to_utf16(m_argv[index]);
        auto alias = s_optionAliases.find(optionString);
        if (alias != s_optionAliases.end())
        {
            optionString = alias->second;
        }

        auto option = s_options.find(optionString);
        if (option == s_options.end())
        {
            TraceLoggingWrite(g_MsixUITraceLoggingProvider,
                "Unknown Argument",
                TraceLoggingLevel(WINEVENT_LEVEL_ERROR),
                TraceLoggingValue(m_argv[index], "arg"));
            return E_INVALIDARG;
        }
        char const *parameter = "";
        if (option->second.TakesParameter)
        {
            if (++index == m_argc)
            {
                return E_INVALIDARG;
            }
            parameter = m_argv[index];
        }
        RETURN_IF_FAILED(option->second.DefaultCallback(this, parameter));

        ++index;

        if (option->second.HasSuboptions)
        {
            auto suboptions = option->second.Suboptions;
            while (index < m_argc)
            {
                std::wstring suboptionString = utf8_to_utf16(m_argv[index]);
                auto suboption = suboptions.find(suboptionString);
                if (suboption == suboptions.end())
                {
                    TraceLoggingWrite(g_MsixUITraceLoggingProvider,
                        "Unknown Argument",
                        TraceLoggingLevel(WINEVENT_LEVEL_ERROR),
                        TraceLoggingValue(m_argv[index], "arg"));
                    return E_INVALIDARG;
                }
                char const *suboptionParameter = "";
                if (suboption->second.TakesParameter)
                {
                    if (++index == m_argc)
                    {
                        return E_INVALIDARG;
                    }
                    suboptionParameter = m_argv[index];
                }
                RETURN_IF_FAILED(suboption->second.Callback(this, suboptionParameter));

                ++index;
            }
        }
    }

    return S_OK;
}

void CommandLineInterface::SetWVDFileType(std::wstring fileType)
{
    if (CaseInsensitiveEquals(fileType, L"VHD"))
    {
        this->m_fileType = WVDFileType::VHD;
    }
    else if (CaseInsensitiveEquals(fileType, L"VHDX"))
    {
        this->m_fileType = WVDFileType::VHDX;
    }
    else if (CaseInsensitiveEquals(fileType, L"CIM"))
    {
        this->m_fileType = WVDFileType::CIM;
    }
    else
    {
        this->m_fileType = WVDFileType::NotSpecified;
    }
}

std::wstring CommandLineInterface::GetFileTypeAsString()
{
    switch (this->m_fileType)
    {
        case WVDFileType::CIM: return L"CIM";
        case WVDFileType::VHD: return L"VHD";
        case WVDFileType::VHDX: return L"VHDX";
        case WVDFileType::NotSpecified: return L"NotSpecified";
    }
}

std::wstring CommandLineInterface::GetOperationTypeAsString()
{
    switch (this->m_operationType)
    {
    case OperationType::Add: return L"Add";
    case OperationType::Remove: return L"Remove";
    case OperationType::FindPackage: return L"Find";
    case OperationType::Unpack: return L"Unpack";
    case OperationType::ApplyACLs: return L"ApplyACLs";
    case OperationType::MountImage: return L"Mount";
    case OperationType::UnmountImage: return L"Unmount";
    case OperationType::Undefined: return L"Undefined";
    }
}

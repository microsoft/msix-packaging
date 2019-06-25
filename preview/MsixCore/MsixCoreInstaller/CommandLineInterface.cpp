#include <iostream>
#include <iomanip>
#include "CommandLineInterface.hpp"
#include "MsixCoreInstallerLogger.hpp"
#include <TraceLoggingProvider.h>
#include "..\MsixCoreInstallerLib\GeneralUtil.hpp"
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
        L"-FindAllPackages",
        Options(false, IDS_STRING_HELP_OPTION_FINDALLPACKAGES,
            [&](CommandLineInterface* commandLineInterface, const std::string&)
        {
            if (commandLineInterface->m_operationType != OperationType::Undefined)
            {
                return E_INVALIDARG;
            }
            commandLineInterface->m_operationType = OperationType::FindAllPackages;
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
            [&](CommandLineInterface* commandLineInterface, const std::string& packagePath)
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
                Option(true, IDS_STRING_HELP_OPTION_UNPACK_PATH,
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
    std::wcout << GetStringResource(IDS_STRING_HELPTEXT) << std::endl;

    for (const auto& option : CommandLineInterface::s_options)
    {
        std::wcout << L"\t" << std::left << std::setfill(L' ') << std::setw(5) <<
            option.first << L": " << GetStringResource(option.second.Help) << std::endl;
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
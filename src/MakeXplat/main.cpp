#include "xPlatAppx.hpp"

#include <iostream>
#include <iomanip>
#include <functional>
#include <vector>
#include <map>
#include <string>
#include <initializer_list>

// Describes which command the user specified
enum class UserSpecified
{
    Nothing,
    Help,
    Pack,
    Unpack
};

// Tracks the state of the current parse operation as well as implements input validation
struct State
{
    bool Specify(UserSpecified spec)
    {
        if (specified != UserSpecified::Nothing || spec == UserSpecified::Help)
        {
            specified = UserSpecified::Help; // Because clearly the user needs some
            return false;
        }
        specified = spec;
        return true;
    }

    bool CreatePackageSubfolder()
    {
        unpackOptions = static_cast<xPlatPackUnpackOptions>(unpackOptions | xPlatPackUnpackOptions::xPlatPackUnpackOptionsCreatePackageSubfolder);
        return true;
    }

    bool SkipManifestValidation()
    {
        validationOptions = static_cast<APPX_VALIDATION_OPTION>(validationOptions | APPX_VALIDATION_OPTION::APPX_VALIDATION_OPTION_SKIPAPPXMANIFEST);
        return true;
    }

    bool SkipSignatureValidation()
    {
        validationOptions = static_cast<APPX_VALIDATION_OPTION>(validationOptions | APPX_VALIDATION_OPTION::APPX_VALIDATION_OPTION_SKIPSIGNATUREORIGIN);
        return true;
    }

    bool SetPackageName(const std::string& name)
    {
        if (!packageName.empty() || name.empty()) { return false; }
        packageName = name;
        return true;
    }

    bool SetCertificateName(const std::string& name)
    {
        if (!certName.empty() || name.empty()) { return false; }
        certName = name;
        return true;
    }

    bool SetDirectoryName(const std::string& name)
    {
        if (!directoryName.empty() || name.empty()) { return false; }
        directoryName = name;
        return true;
    }

    std::string packageName;
    std::string certName;
    std::string directoryName;
    UserSpecified specified                  = UserSpecified::Nothing;
    APPX_VALIDATION_OPTION validationOptions = APPX_VALIDATION_OPTION::APPX_VALIDATION_OPTION_FULL;
    xPlatPackUnpackOptions unpackOptions     = xPlatPackUnpackOptions::xPlatPackUnpackOptionsNone;
};

// describes an option to a command that the user may specify
struct Option
{
    using CBF = std::function<bool(const std::string& value)>;

    Option(bool param, const std::string& help, CBF callback): Help(help), Callback(callback), TakesParameter(param)
    {}

    bool        TakesParameter;
    std::string Name;
    std::string Help;
    CBF         Callback;
};

// describes a command that the user may specify.
struct Command
{
    using CBF = std::function<bool()>;

    Command(const std::string& help, CBF callback, std::map<std::string, Option> options) :
        Help(help), Callback(callback), Options(options)
    {}

    std::string                   Help;
    std::map<std::string, Option> Options;
    CBF                           Callback;
};

// Displays contextual formatted help to the user.
int Help(char* toolName, std::map<std::string, Command>& commands, State& state)
{
    std::cout << std::endl;
    std::cout << "Usage:" << std::endl;
    std::cout << "------" << std::endl;

    auto command = commands.end();
    switch (state.specified)
    {
    case UserSpecified::Nothing:
    case UserSpecified::Help:
        std::cout << "    " << toolName << " <command> [options] " << std::endl;
        std::cout << std::endl;
        std::cout << "Valid commands:" << std::endl;
        std::cout << "---------------" << std::endl;
        for (const auto& command : commands)
        {
            std::cout << "    " << std::left << std::setfill(' ') << std::setw(10) <<
                command.first << "--  " << command.second.Help << std::endl;
        }

        std::cout << std::endl;
        std::cout << "For help with a specific command, enter " << toolName << " <command> -?" << std::endl;
        return 0;
    case UserSpecified::Pack:
        command = commands.find("pack");
        std::cout << "    " << toolName << " pack -p <package> -c <certificate> -d <directory> [options] " << std::endl;
        std::cout << std::endl;
        std::cout << "Description:" << std::endl;
        std::cout << "------------" << std::endl;
        std::cout << "    Creates an appx package at output <package> name by adding all files from" << std::endl;
        std::cout << "    content <directory> (including subfolders), and signs the package using  " << std::endl;
        std::cout << "    the specified <certificate>.  You must include a valid app package manifest " << std::endl;
        std::cout << "    named AppxManifest.xml in the content directory if you do not specify the " << std::endl;
        std::cout << "    -mv option." << std::endl;
        break;
    case UserSpecified::Unpack:
        command = commands.find("unpack");
        std::cout << "    " << toolName << " upack -p <package> -d <directory> [options] " << std::endl;
        std::cout << std::endl;
        std::cout << "Description:" << std::endl;
        std::cout << "------------" << std::endl;
        std::cout << "    Extracts all files within an app package at the input <package> name to the" << std::endl;
        std::cout << "    specified output <directory>.  The output has the same directory structure " << std::endl;
        std::cout << "    as the package." << std::endl;
        break;
    }
    std::cout << std::endl;
    std::cout << "Options:" << std::endl;
    std::cout << "--------" << std::endl;

    for (const auto& option : command->second.Options)
    {
        std::cout << "    " << std::left << std::setfill(' ') << std::setw(5) <<
            option.first << ": " << option.second.Help << std::endl;
    }
    return 0;
}

// error text if the user provided underspecified input
void Error(char* toolName)
{
    std::cout << toolName << ": error : Missing required options.  Use '-?' for more details." << std::endl;
}

// Parses argc/argv input via commands into state, and calls into the 
// appropriate function with the correct parameters if warranted.
int ParseAndRun(std::map<std::string, Command>& commands, State& state, int argc, char* argv[])
{
    auto ParseInput = [&]()->bool {
        int index = 1;
        while (index < argc)
        {
            auto command = commands.find(argv[index]);
            if (command == commands.end())    { return false; }
            if (!command->second.Callback())  { return false; }
            if (++index == argc)              { break; }
            auto option = command->second.Options.find(argv[index]);
            while (option != command->second.Options.end())
            {
                char const *parameter = "";
                if (option->second.TakesParameter)
                {
                    if (++index == argc) { break; }
                    parameter = argv[index];
                }
                if (!option->second.Callback(parameter)) { return false; }
                if (++index == argc) { break; }
                option = command->second.Options.find(argv[index]);
            }
        }
        return true;
    };

    if (!ParseInput())
    {
        return Help(argv[0], commands, state);
    }

    switch (state.specified)
    {
    case UserSpecified::Help:
    case UserSpecified::Nothing:
        return Help(argv[0], commands, state);
    case UserSpecified::Pack:
        if (state.packageName.empty() || state.certName.empty() || state.directoryName.empty())
        {
            Error(argv[0]);
            return -1;
        }
        return PackAppx(state.unpackOptions, state.validationOptions,
            const_cast<char*>(state.packageName.c_str()),
            const_cast<char*>(state.certName.c_str()),
            const_cast<char*>(state.directoryName.c_str())
        );

    case UserSpecified::Unpack:
        if (state.packageName.empty() || state.directoryName.empty())
        {
            Error(argv[0]);
            return -1;
        }
        return UnpackAppx(state.unpackOptions, state.validationOptions,
            const_cast<char*>(state.packageName.c_str()),
            const_cast<char*>(state.directoryName.c_str())
        );
        //return ValidateAppxSignature(const_cast<char*>(state.packageName.c_str()));
    }
    return -1; // should never end up here.
}

// Defines the grammar of commands and each command's associated options,
int main(int argc, char* argv[])
{
    std::cout << "Microsoft (R) " << argv[0] << " version " << std::endl; // TODO: specify version
    std::cout << "Copyright (C) 2017 Microsoft.  All rights reserved." << std::endl;

    State state;
    std::map<std::string, Command> commands = {
        { "pack", Command("Create a new package from files on disk", [&]() { return state.Specify(UserSpecified::Pack); },
            {
                { "-p", Option(true, "REQUIRED, specify output package file name.",
                [&](const std::string& name) { return state.SetPackageName(name); })
                },
                { "-c", Option(true, "REQUIRED, specify input certificate name.",
                    [&](const std::string& name) { return state.SetCertificateName(name); })
                },
                { "-d", Option(true, "REQUIRED, specify input directory name.",
                    [&](const std::string& name) { return state.SetDirectoryName(name); })
                },
                {"-mv", Option(false, "Skips manifest validation.  By default manifest validation is enabled.",
                    [&](const std::string&) { return state.SkipManifestValidation(); })
                },
                { "-sv", Option(false, "Skips signature validation.  By default signature validation is enabled.",
                    [&](const std::string&) { return state.SkipSignatureValidation(); })
                },
                { "-?", Option(false, "Displays this help text.",
                    [&](const std::string&) { return false; })
                }
            })
        },
        { "unpack", Command("Create a new package from files on disk", [&]() { return state.Specify(UserSpecified::Unpack); },
            {
                { "-p", Option(true, "REQUIRED, specify input package name.",
                [&](const std::string& name) { return state.SetPackageName(name); })
                },
                { "-d", Option(true, "REQUIRED, specify output directory name.",
                    [&](const std::string& name) { return state.SetDirectoryName(name); })
                },
                { "-pfn", Option(false, "Unpacks all files to a subdirectory under the specified output path, named after the package full name.",
                    [&](const std::string&) { return state.CreatePackageSubfolder(); })
                },
                { "-mv", Option(false, "Skips manifest validation.  By default manifest validation is enabled.",
                    [&](const std::string&) { return state.SkipManifestValidation(); })
                },
                { "-sv", Option(false, "Skips signature validation.  By default signature validation is enabled.",
                    [&](const std::string&) { return state.SkipSignatureValidation(); })
                },
                { "-?", Option(false, "Displays this help text.",
                    [&](const std::string&) { return false; })
                }
            })
        },
        {
            "-?", Command("Displays this help text.", [&]() { return state.Specify(UserSpecified::Help);}, {})
        },
    };

    return ParseAndRun(commands, state, argc, argv);
}
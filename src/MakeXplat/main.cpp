#include "xPlatAppx.hpp"

#include <iostream>
#include <iomanip>
#include <functional>
#include <vector>
#include <map>
#include <string>
#include <initializer_list>

enum class Command
{
    None,
    Help,
    Pack,
    Unpack
};

struct Options
{
    void Specify(Command spec)
    {
        if (specified != Command::None)
        {
            specified = Command::Help; // Because clearly the user needs some
            throw std::invalid_argument("invalid command specified");
        }
        specified = spec;
    }

    void CreatePackageSubfolder()
    {
        unpackOptions = static_cast<xPlatPackUnpackOptions>(unpackOptions | xPlatPackUnpackOptions::xPlatPackUnpackOptionsCreatePackageSubfolder);
    }

    void SkipManifestValidation()
    {
        validationOptions = static_cast<xPlatValidationOptions>(validationOptions | xPlatValidationOptions::xPlatValidationOptionSkipAppxManifestValidation);
    }

    void SkipSignatureValidation()
    {
        validationOptions = static_cast<xPlatValidationOptions>(validationOptions | xPlatValidationOptions::xPlatValidationOptionSkipSignatureOrigin);
    }

    void SetPackageName(const std::string& name)        { packageName = name; }
    void SetCertificateName(const std::string& name)    { certName = name; }
    void SetDirectoryName(const std::string& name)      { directoryName = name; }

    std::string packageName;
    std::string certName;
    std::string directoryName;
    Command specified                           = Command::None;
    xPlatValidationOptions validationOptions    = xPlatValidationOptions::xPlatValidationOptionFull;
    xPlatPackUnpackOptions unpackOptions        = xPlatPackUnpackOptions::xPlatPackUnpackOptionsNone;
};

struct Option
{
    using Callback = std::function<void(const std::string& value)>;

    Option(bool param, const std::string& help, Callback callback):
        m_help(help), m_callback(callback), m_takesParameter(param)
    {
    }

    bool m_takesParameter;
    std::string m_name;
    std::string m_help;
    Callback m_callback;
};

class Switch
{
public:
    using Callback = std::function<void()>;

    Switch(const std::string& help, Callback callback, std::map<std::string, Option> options) :
        m_help(help), m_callback(callback), m_options(options)
    { }

    std::string m_help;
    std::map<std::string, Option> m_options;
    Callback m_callback;
};

int Help(char* toolName, std::map<std::string, Switch>& switches, Options& options)
{
    std::cout << std::endl;
    std::cout << "Usage:" << std::endl;
    std::cout << "------" << std::endl;

    auto command = switches.find("");

    switch (options.specified)
    {
    case Command::None:
    case Command::Help:
        std::cout << "    " << toolName << " <command> [options] " << std::endl;
        std::cout << std::endl;
        std::cout << "Valid commands:" << std::endl;
        std::cout << "---------------" << std::endl;

        for each (const auto& command in switches)
        {
            std::cout << "    " << std::left << std::setfill(' ') << std::setw(10) <<
                command.first << "--  " << command.second.m_help << std::endl;
        }

        std::cout << std::endl;
        std::cout << "For help with a specific command, enter " << toolName << " <command> --?" << std::endl;
        return -1;
    case Command::Pack:
        command = switches.find("-pack");
        std::cout << "    " << toolName << " -pack --p <package> --c <certificate> --d <directory> [options] " << std::endl;
        std::cout << std::endl;
        std::cout << "Description:" << std::endl;
        std::cout << "------------" << std::endl;
        std::cout << "    Creates an appx package at output <package> name by adding all files from" << std::endl;
        std::cout << "    content <directory> (including subfolders), and signs the package using  " << std::endl;
        std::cout << "    the specified <certificate>.  You must include a valid app package manifest " << std::endl;
        std::cout << "    named AppxManifest.xml in the content directory if you do not specify the " << std::endl;
        std::cout << "    --nmv option." << std::endl;

        break;
    case Command::Unpack:
        command = switches.find("-unpack");
        std::cout << "    " << toolName << " -upack --p <package> --d <directory> [options] " << std::endl;
        std::cout << std::endl;
        std::cout << "Description:" << std::endl;
        std::cout << "------------" << std::endl;
        std::cout << "    Extracts all files within an app package at the <input pakcage name> to the" << std::endl;
        std::cout << "    specified <output directory>.  The output has the same directory structure " << std::endl;
        std::cout << "    as the package." << std::endl;
        break;
    }
    std::cout << std::endl;
    std::cout << "Options:" << std::endl;
    std::cout << "--------" << std::endl;

    for each (const auto& option in command->second.m_options)
    {
        std::cout << "    " << std::left << std::setfill(' ') << std::setw(5) <<
            option.first << ": " << option.second.m_help << std::endl;
    }
    return -1;
}

bool Parse(std::map<std::string, Switch>& switches, int argc, char* argv[])
{
    try
    {
        int index = 1;
        while(index < argc)
        {
            auto command = switches.find(argv[index]);
            if (command == switches.end())
            {
                return false;
            }

            command->second.m_callback();

            if (++index == argc) { break; }
            auto option = command->second.m_options.find(argv[index]);
            while (option != command->second.m_options.end())
            {
                if (option->second.m_takesParameter)
                {
                    if (++index == argc) { break; }
                    option->second.m_callback(argv[index]);
                }
                else
                {
                    option->second.m_callback("");
                }

                if (++index == argc) { break; }
                option = command->second.m_options.find(argv[index]);
            }
        }
    }
    catch (std::exception)
    {
        return false;
    }
    return true;
}

int main(int argc, char* argv[])
{
    std::cout << "Microsoft (R) " << argv[0] << " Tool version " << std::endl; // TODO: specify version
    std::cout << "Copyright (C) 2017 Microsoft.  All rights reserved." << std::endl;

    Options options;
    std::map<std::string, Switch> switches = {
        { "-pack", Switch("Create a new package from files on disk", [&]() { options.Specify(Command::Pack); },
            {
                { "-p", Option(true, "REQUIRED, specify output package file name.",
                [&](const std::string& name) { options.SetPackageName(name); })
                },
                { "-c", Option(true, "REQUIRED, specify input certificate name.",
                    [&](const std::string& name) { options.SetCertificateName(name); })
                },
                { "-d", Option(true, "REQUIRED, specify input directory name.",
                    [&](const std::string& name) { options.SetDirectoryName(name); })
                },
                {"-mv", Option(false, "Skips manifest validation.  By default manifest validation is enabled.",
                    [&](const std::string&) { options.SkipManifestValidation(); })
                },
                { "-sv", Option(false, "Skips signature validation.  By default signature validation is enabled.",
                    [&](const std::string&) { options.SkipManifestValidation(); })
                },
                { "-?", Option(false, "Displays this help text.",
                    [&](const std::string&) { throw std::invalid_argument("pack help"); })
                }
            })
        },
        { "-unpack", Switch("Create a new package from files on disk", [&]() { options.Specify(Command::Unpack); },
            {
                { "-p", Option(true, "REQUIRED, specify input package name.",
                [&](const std::string& name) { options.SetPackageName(name); })
                },
                { "-d", Option(true, "REQUIRED, specify output directory name.",
                    [&](const std::string& name) { options.SetDirectoryName(name); })
                },
                { "-pfn", Option(false, "Unpacks all files to a subdirectory under the specified output path, named after the package full name.",
                    [&](const std::string&) { options.CreatePackageSubfolder(); })
                },
                { "-mv", Option(false, "Skips manifest validation.  By default manifest validation is enabled.",
                    [&](const std::string&) { options.SkipManifestValidation(); })
                },
                { "-sv", Option(false, "Skips signature validation.  By default signature validation is enabled.",
                    [&](const std::string&) { options.SkipManifestValidation(); })
                },
                { "-?", Option(false, "Displays this help text.",
                    [&](const std::string&) { throw std::invalid_argument("unpack help"); })
                }
            })
        },
        {
            "-?", Switch("Displays this help text.", [&]() {
                options.Specify(Command::Help);
                throw std::invalid_argument("help");
            }, {})
        },
    };

    if (!Parse(switches, argc, argv))
    {
        return Help(argv[0], switches, options);
    }
    else
    {
        switch (options.specified)
        {
        case Command::None:
            return Help(argv[0], switches, options);
        case Command::Pack:
            if (options.packageName.empty() || options.certName.empty() || options.directoryName.empty())
            {
                std::cout << argv[0] << ": error : Missing required options.  Use '-?' option for more details." << std::endl;
                return -1;
            }
            return PackAppx(
                options.unpackOptions,
                options.validationOptions,
                const_cast<char*>(options.packageName.c_str()),
                const_cast<char*>(options.certName.c_str()),
                const_cast<char*>(options.directoryName.c_str())
            );

        case Command::Unpack:
            if (options.packageName.empty() || options.directoryName.empty())
            {
                std::cout << argv[0] << ": error : Missing required options.  Use '-?' option for more details." << std::endl;
                return -1;
            }
            return UnpackAppx(
                options.unpackOptions,
                options.validationOptions,
                const_cast<char*>(options.packageName.c_str()),
                const_cast<char*>(options.directoryName.c_str())
            );
        }
    }
}

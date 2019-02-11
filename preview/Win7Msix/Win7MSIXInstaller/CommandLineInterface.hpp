#pragma once

#include <map>
#include <functional>
#include "MsixRequest.hpp"

class CommandLineInterface;
/// Describes an option to a command that the user may specify used for the command line tool
struct Option
{
    using CallbackFunction = std::function<HRESULT(CommandLineInterface* commandLineInterface, const std::string& value)>;

    Option(bool param, const UINT help, CallbackFunction callback) : Help(help), Callback(callback), TakesParameter(param)
    {}

    bool TakesParameter;
    std::wstring Name;
    UINT Help;
    CallbackFunction Callback;
};

struct CaseInsensitiveLess
{
    struct CaseInsensitiveCompare
    {
        bool operator() (const wchar_t& c1, const wchar_t& c2) const
        {
            return tolower(c1) < tolower(c2);
        }
    };
    bool operator() (const std::wstring & s1, const std::wstring & s2) const
    {
        return std::lexicographical_compare(
            s1.begin(), s1.end(),   // source range
            s2.begin(), s2.end(),   // dest range
            CaseInsensitiveCompare());  // comparison
    }
};

/// Parses the command line specified and creates a request.
class CommandLineInterface
{
    friend Option;
public:
    CommandLineInterface(int argc, char * argv[]) : m_argc(argc), m_argv(argv)
    {
        m_toolName = m_argv[0];
    }
    ~CommandLineInterface() {}

    /// Displays contextual formatted help to the user used for command line tool
    void DisplayHelp();

    HRESULT CreateRequest(_Outptr_ MsixRequest** msixRequest);

private:
    int m_argc = 0;
    char ** m_argv = nullptr;
    char * m_toolName = nullptr;
    static std::map<std::wstring, Option, CaseInsensitiveLess> s_options;
    static std::map<std::wstring, std::wstring> s_optionAliases;

    std::wstring m_packageFilePath;
    std::wstring m_packageFullName;

    MSIX_VALIDATION_OPTION m_validationOptions = MSIX_VALIDATION_OPTION::MSIX_VALIDATION_OPTION_FULL;
    Flags m_flags = NoFlags;
    OperationType m_operationType = OperationType::Undefined;

    CommandLineInterface() {}
};
#pragma once

#include <map>
#include <functional>
#include <IPackage.hpp>
#include <AppxPackaging.hpp>
enum OperationType
{
    Undefined = 0,
    Add = 1,
    Remove = 2,
    FindPackage = 3,
    Unpack = 5,
    ApplyACLs = 6,
    MountImage = 7,
    UnmountImage = 8
};

enum WVDFileType
{
    NotSpecified = 0,
    VHD = 1,
    VHDX = 2,
    CIM = 3
};

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

struct Options
{
    using CallbackFunction = std::function<HRESULT(CommandLineInterface* commandLineInterface, const std::string& value)>;

    Options(bool takesParam, const UINT help, CallbackFunction defaultCallback) : Help(help), DefaultCallback(defaultCallback), TakesParameter(takesParam), HasSuboptions(false) {}
    Options(bool takesParam, const UINT help, CallbackFunction defaultCallback, std::map<std::wstring, Option, CaseInsensitiveLess> suboptions) : Help(help), DefaultCallback(defaultCallback), TakesParameter(takesParam), Suboptions(suboptions)
    {
        HasSuboptions = !Suboptions.empty();
    }

    bool HasSuboptions;
    bool TakesParameter;
    std::wstring Name;
    UINT Help;
    CallbackFunction DefaultCallback;
    std::map<std::wstring, Option, CaseInsensitiveLess> Suboptions;
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
    HRESULT Init();
    void SetWVDFileType(std::wstring fileType);
    bool IsQuietMode() { return m_quietMode; }
    bool IsApplyACLs() { return m_applyACLs; }
    bool IsValidateSignature() { return m_validateSignature; }
    bool IsCreate() { return m_create; }
    bool isMountReadOnly() { return m_readOnly; }
    std::wstring GetPackageFilePathToInstall() { return m_packageFilePath; }
    std::wstring GetSourceApplicationId() { return m_sourceApplicationId; }
    std::wstring GetCorrelationId() { return m_correlationId; }
    std::wstring GetPackageFullName() { return m_packageFullName; }
    std::wstring GetUnpackDestination() { return m_unpackDestination; }
    std::wstring GetRootDirectory() { return m_rootDirectory; }
    std::wstring GetMountImagePath() { return m_mountImagePath; }
    std::wstring GetVolumeId() { return m_volumeId; }
    WVDFileType GetFileType() { return m_fileType; }
    std::wstring GetFileTypeAsString();
    OperationType GetOperationType() { return m_operationType; }
    std::wstring GetOperationTypeAsString();
    ULONGLONG GetVHDSize() { return m_vhdSize; }
    std::wstring GetOutputPath() { return m_outputPath; }
private:
    int m_argc = 0;
    char ** m_argv = nullptr;
    char * m_toolName = nullptr;
    static std::map<std::wstring, Options, CaseInsensitiveLess> s_options;
    static std::map<std::wstring, std::wstring> s_optionAliases;

    std::wstring m_packageFilePath;
    std::wstring m_packageFullName;
    std::wstring m_unpackDestination;
    std::wstring m_outputPath;
    std::wstring m_rootDirectory;
    std::wstring m_mountImagePath;
    std::wstring m_volumeId;
    std::wstring m_sourceApplicationId = L"MSIX_MGR";    // m_sourceApplicationId (Optional parameter) indicates which application is executing the MSIXMGR commands. Useful during integration of MSIXMGR with other Tools.
    std::wstring m_correlationId = L"";    // m_correlationId (Optional parameter) can be used to establish a correlation between MSIXMGR's Workflow and the parent application's Workflow, when MSIXMGR is being used in integration with any other Tool.
    bool m_quietMode;
    bool m_applyACLs;
    bool m_validateSignature;
    bool m_create = false;
    bool m_readOnly = true;
    WVDFileType m_fileType = WVDFileType::NotSpecified;
    ULONGLONG m_vhdSize = 0;

    OperationType m_operationType = OperationType::Undefined;

    CommandLineInterface() {}
};
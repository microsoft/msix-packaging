#pragma once
#include "../GeneralUtil.hpp"
#include "IPackageHandler.hpp"
#include "RegistryKey.hpp"
#include "MsixRequest.hpp"
#include <vector>

namespace MsixCoreLib
{
class ComServer : IPackageHandler
{
public:
    /// Adds the com server registrations to the per-machine registry
    HRESULT ExecuteForAddRequest();

    /// Removes the com server registrations from the per-machine registry.
    HRESULT ExecuteForRemoveRequest();

    static const PCWSTR HandlerName;
    static HRESULT CreateHandler(_In_ MsixRequest* msixRequest, _Out_ IPackageHandler** instance);
    ~ComServer() {}
private:
    struct Aspect
    {
        std::wstring type;
        std::wstring oleMiscFlag;
    };

    struct Verb
    {
        std::wstring id;
        std::wstring verb;
    };

    struct ExeServerClass
    {
        std::wstring id;
        std::wstring displayName;
        std::wstring enableOleDefaultHandler;
        std::wstring progId;
        std::wstring versionIndependentProgId;
        std::wstring autoConvertTo;
        std::wstring insertableObject;
        std::wstring shortDisplayName;
        std::vector<std::wstring> implementedCategories;
        std::wstring conversionReadableFormat;
        std::wstring conversionReadWritableFormat;
        std::wstring defaultFileDataFormat;
        std::vector<std::wstring> dataFormats;
        std::vector<Aspect> miscStatusAspects;
        std::wstring miscStatusOleMiscFlag;
        std::vector<Verb> verbs;
        std::wstring defaultIcon;
        std::wstring toolboxBitmap;
    };

    struct ExeServer
    {
        std::wstring executable;
        std::wstring arguments;
        std::wstring displayName;
        std::wstring launchAndActivationPermission;
        std::vector<ExeServerClass> classes;
    };

    struct ProgId
    {
        std::wstring id;
        std::wstring clsid;
        std::wstring currentVersion;
    };

    MsixRequest* m_msixRequest = nullptr;
    RegistryKey m_classesKey;

    std::vector<ExeServer> m_exeServers;
    std::vector<ProgId> m_progIds;

    /// Parses the manifest
    HRESULT ParseManifest();

    /// Parses the manifest element to populate one com exeServer
    /// 
    /// @param comServerElement - the manifest element representing an comServer
    HRESULT ParseExeServerElement(IMsixElement* exeServerElement);

    /// Parses the class element to populate one class for a com exeServer
    ///
    /// @param exeServer - an ExeServer
    /// @param classElement - the manifest element representing an class
    HRESULT ParseExeServerClassElement(ExeServer& exeServer, IMsixElement* classElement);

    /// Parses the implementedCategories element inside of a class
    ///
    /// @param classElement - the manifest element representing an class
    /// @param exeServerClass - the exeServerClass whose implementedCategories will be populated
    HRESULT ParseImplementedCategories(IMsixElement * classElement, ExeServerClass & exeServerClass);

    /// Parses the Conversion elements inside of a com exeServer class
    /// Conversion can have either Readable or ReadWritable formats
    /// 
    /// @param classElement - the manifest element representing an class
    /// @param formatsQuery - query to execute to find either Readable or ReadWritable formats
    /// @param formats - format string 
    HRESULT ParseConversionFormats(IMsixElement * classElement, const std::wstring & formatsQuery, std::wstring & formats);

    /// Parses the DataFormats elements inside of a com exeServer class
    /// 
    /// @param classElement - the manifest element representing an class
    /// @param exeServerClass - the exeServerClass whose dataFormats will be populated
    HRESULT ParseDataFormats(IMsixElement * classElement, ExeServerClass & exeServerClass);

    /// Parses the DataFormat elements inside of a com exeServer class
    /// 
    /// @param dataFormatsElement - the manifest element representing the dataFormats
    /// @param exeServerClass - the exeServerClass whose dataFormats will be populated
    HRESULT ParseDataFormat(IMsixElement * dataFormatsElement, ExeServerClass & exeServerClass);

    /// Parses the miscStatus element inside of a class
    ///
    /// @param classElement - the manifest element representing an class
    /// @param exeServerClass - the exeServerClass whose miscStatus will be populated
    HRESULT ParseMiscStatus(IMsixElement * classElement, ExeServerClass & exeServerClass);

    /// Parses the aspect elements inside a MiscStatus to fill out the appropriate field in exeServerClass
    /// 
    /// @param miscStatusElement - the manifest element representing an miscStatus
    /// @param exeServerClass - the exeServerClass whose miscStatusAspects will be populated
    HRESULT ParseAspects(IMsixElement * miscStatusElement, ExeServerClass & exeServerClass);

    /// Parses the verb element inside of a class
    /// 
    /// @param classElement - the manifest element representing an class
    /// @param exeServerClass - the exeServerClass whose verbs will be populated
    HRESULT ParseVerbs(IMsixElement * classElement, ExeServerClass & exeServerClass);

    /// Parses the DefaultIcon element inside of a class
    /// 
    /// @param classElement - the manifest element representing an class
    /// @param exeServerClass - the exeServerClass whose defaultIcons will be populated
    HRESULT ParseDefaultIcon(IMsixElement * classElement, ExeServerClass & exeServerClass);

    /// Parses the ToolboxBitmap32 element inside of a class
    /// 
    /// @param classElement - the manifest element representing an class
    /// @param exeServerClass - the exeServerClass whose toolboxBitmaps will be populated
    HRESULT ParseToolboxBitmap(IMsixElement * classElement, ExeServerClass & exeServerClass);

    /// Parses the ProgId elements of a comServer
    /// 
    /// @param progIdElement - the manifest element representing a progId
    HRESULT ParseProgIdElement(IMsixElement* progIdElement);

    /// Adds the registry entries corresponding to an ExeServer
    ///
    /// @param exeServer - ExeServer struct representing data from the manifest
    HRESULT ProcessExeServerForAdd(ExeServer & exeServer);

    /// Adds the registry entries corresponding to a ProgId
    ///
    /// @param progId - ProgId struct representing data from the manifest
    HRESULT ProcessProgIdForAdd(ProgId & progId);

    /// Removes the registry entries corresponding to an ExeServer
    ///
    /// @param exeServer - ExeServer struct representing data from the manifest
    HRESULT ProcessExeServerForRemove(ExeServer & exeServer);

    /// Removes the registry entries corresponding to a ProgId
    ///
    /// @param progId - ProgId struct representing data from the manifest
    HRESULT ProcessProgIdForRemove(ProgId & progId);

    /// Adds all the registry entries associated with the servers and progIds
    HRESULT AddServersAndProgIds();

    /// Removes all the registry entries associated with the servers and progIds
    HRESULT RemoveServersAndProgIds();

    ComServer() {}
    ComServer(_In_ MsixRequest* msixRequest) : m_msixRequest(msixRequest) {}

};
}
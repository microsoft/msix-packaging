#pragma once
#include "msixmgrActions.hpp"

/// This class tests the API surface of the Msixcorelib PackageManager interface.
/// This uses the TAEF (test-authoring and execution framework) available from the WDK.
/// This builds a test DLL binary, that can be passed into the TAEF te.exe to execute the tests
class MsixCoreTest : public WEX::TestClass<MsixCoreTest>
{
public:
    TEST_CLASS(MsixCoreTest)

    TEST_CLASS_SETUP(SetupTest)
    TEST_CLASS_CLEANUP(CleanupTest)

    TEST_METHOD_SETUP(SetupMethod)
    TEST_METHOD_CLEANUP(CleanupMethod)

    BEGIN_TEST_METHOD(InstallQueryAndRemoveWithLibTest)
    TEST_METHOD_PROPERTY(L"RunAs", L"ElevatedUser")
    END_TEST_METHOD()

    BEGIN_TEST_METHOD(InstallWithLibAndGetProgressTest)
    TEST_METHOD_PROPERTY(L"RunAs", L"ElevatedUser")
    END_TEST_METHOD()

    BEGIN_TEST_METHOD(InstallIStreamPackageTest)
    TEST_METHOD_PROPERTY(L"RunAs", L"ElevatedUser")
    END_TEST_METHOD()

    BEGIN_TEST_METHOD(InstallIStreamAndGetProgressTest)
    TEST_METHOD_PROPERTY(L"RunAs", L"ElevatedUser")
    END_TEST_METHOD()

private:
    MsixCoreLib::IPackageManager * m_packageManager;
    WEX::Common::String m_testDeploymentDir;
    ~MsixCoreTest();
};
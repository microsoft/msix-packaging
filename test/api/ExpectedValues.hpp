//
//  Copyright (C) 2017 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
//
#include <array>
#include <vector>

// Expected values for package
#ifdef WIN32
static const char* packageToTest = "..\\test\\appx\\TestAppxPackage_Win32.appx";
#else
static const char* packageToTest = "../test/appx/TestAppxPackage_Win32.appx";
#endif

#ifdef USING_XERCES
static const wchar_t* ApplicationXpath = L"/Package/Applications/Application";
#else
static const wchar_t* ApplicationXpath = L"/*[local-name()='Package']/*[local-name()='Applications']/*[local-name()='Application']";
#endif


const int ExpectedPayloadFilesSize = 10;
static const std::array<const wchar_t*, ExpectedPayloadFilesSize> ExpectedPayloadFiles = {
    L"Assets\\LockScreenLogo.scale-200.png",
    L"Assets\\SplashScreen.scale-200.png",
    L"Assets\\Square150x150Logo.scale-200.png",
    L"Assets\\Square44x44Logo.scale-200.png",
    L"Assets\\Square44x44Logo.targetsize-24_altform-unplated.png",
    L"Assets\\StoreLogo.png",
    L"Assets\\Wide310x150Logo.scale-200.png",
    L"TestAppxPackage.exe",
    L"TestAppxPackage.winmd",
    L"resources.pri",
};

struct ExpectedBlock
{
    const std::uint32_t compressedSize;
    const std::vector<std::uint8_t> hash;

    ExpectedBlock(std::uint32_t c, std::vector<std::uint8_t> h) : compressedSize(c), hash(h)
    {}
};

struct ExpectedBlockMapFile
{
    const wchar_t*      name;
    const std::uint32_t lfh;
    const std::uint64_t size;
    const std::vector<ExpectedBlock> blocks;

    ExpectedBlockMapFile(const wchar_t* n, std::uint32_t l, std::uint64_t s, std::vector<ExpectedBlock> b) : 
        name(n), lfh(l), size(s), blocks(b) {}
};

static const std::vector<ExpectedBlockMapFile> expectedBlockMapFileList = {
    ExpectedBlockMapFile(L"AppxManifest.xml", 46, 3251, {
        ExpectedBlock(1332, { 0x3f, 0x48, 0x22, 0x5, 0x62, 0xf3, 0xea, 0xc7, 0xec, 0xb3, 0xc2, 0x99, 0x8e, 0x34, 0xc9, 0xeb, 0xfd, 0x23, 0x12, 0x98, 0xb6, 0xe3, 0xe0, 0x8f, 0x46, 0x7a, 0x23, 0x7f, 0x6c, 0x82, 0xe9, 0xc3 }) }),
    ExpectedBlockMapFile(L"Assets\\LockScreenLogo.scale-200.png", 65, 1430, {
        ExpectedBlock(1430, { 0xa4, 0x1a, 0x5, 0x3b, 0x3f, 0xc3, 0xb0, 0xc1, 0x9, 0x72, 0xc, 0xcd, 0x43, 0x7a, 0x19, 0x72, 0x5a, 0xe9, 0x16, 0x3e, 0xa7, 0x59, 0x90, 0x22, 0x2a, 0x12, 0xb5, 0x96, 0xb9, 0xc7, 0xca, 0x76 }) }),
    ExpectedBlockMapFile(L"Assets\\SplashScreen.scale-200.png", 63, 7700, {
        ExpectedBlock(7700, { 0xa3, 0x38, 0x2b, 0xb, 0x1b, 0x83, 0x4e, 0x95, 0xb8, 0x88, 0xf0, 0x6d, 0x48, 0x3d, 0xc2, 0xd7, 0x8f, 0xa1, 0xb3, 0x85, 0x5e, 0x6, 0x83, 0xd5, 0xcf, 0xbd, 0x51, 0x67, 0xbe, 0x97, 0x31, 0xa6 }) }),
    ExpectedBlockMapFile(L"Assets\\Square150x150Logo.scale-200.png", 68, 2937, {
        ExpectedBlock(2937, { 0x7f, 0x3c, 0xb5, 0x73, 0x8e, 0x8f, 0x5, 0x54, 0x44, 0x45, 0xf7, 0x99, 0x96, 0xa3, 0x13, 0xf8, 0xb4, 0x7d, 0xee, 0x22, 0xdb, 0xc9, 0xc7, 0xbe, 0x85, 0x9d, 0x27, 0x7, 0x71, 0xf, 0xc, 0x73 }) }),
    ExpectedBlockMapFile(L"Assets\\Square44x44Logo.scale-200.png", 66, 1647, {
        ExpectedBlock(1647, { 0x59, 0x44, 0x92, 0xa2, 0x50, 0x70, 0x95, 0x1d, 0x74, 0x62, 0xc0, 0x91, 0xd9, 0xf8, 0x99, 0xa6, 0x6f, 0x55, 0xba, 0x99, 0x2b, 0xb9, 0xb0, 0x5d, 0x98, 0xd5, 0xf, 0x67, 0xcd, 0xfb, 0x2a, 0xbe }) }),
    ExpectedBlockMapFile(L"Assets\\Square44x44Logo.targetsize-24_altform-unplated.png", 87, 1255, {
        ExpectedBlock(1255, { 0x48, 0xcf, 0x9c, 0x22, 0x15, 0x6a, 0xb, 0x3d, 0x77, 0x98, 0x26, 0x41, 0xf9, 0x72, 0x78, 0x5e, 0x78, 0x61, 0xa7, 0x25, 0x7f, 0x7b, 0xcf, 0x15, 0x5b, 0xe7, 0xd5, 0xa1, 0x2e, 0x1a, 0xa3, 0xd8 }) }),
    ExpectedBlockMapFile(L"Assets\\StoreLogo.png", 50, 1451, {
        ExpectedBlock(1451, { 0xae, 0x95, 0xe9, 0x9a, 0x96, 0x25, 0x1a, 0xba, 0xf8, 0xde, 0x15, 0xc9, 0xcd, 0xad, 0xdb, 0x8c, 0xef, 0xb1, 0xb8, 0xb3, 0x20, 0xb1, 0xa, 0x4f, 0x1f, 0x4e, 0x1d, 0xc3, 0xc2, 0x5c, 0x1b, 0x1a }) }),
    ExpectedBlockMapFile(L"Assets\\Wide310x150Logo.scale-200.png", 66, 3204, {
        ExpectedBlock(3204, { 0xb5, 0xb7, 0x75, 0x48, 0x32, 0xc0, 0x8e, 0x58, 0xfa, 0xac, 0xfe, 0x64, 0xea, 0x4b, 0x9f, 0x8b, 0x59, 0xb5, 0x2a, 0x65, 0x8e, 0x4e, 0xea, 0x4a, 0xab, 0x37, 0x90, 0xcf, 0xb8, 0x9f, 0xaa, 0x3 }) }),
    ExpectedBlockMapFile(L"TestAppxPackage.exe", 49, 186368, {
        ExpectedBlock(20070, { 0x78, 0xe7, 0x6c, 0xad, 0x44, 0x11, 0x98, 0x95, 0xe2, 0x17, 0x75, 0x36, 0x36, 0xe3, 0x70, 0x3b, 0x7, 0x1d, 0x2, 0x77, 0xd8, 0x2, 0x97, 0xa4, 0x88, 0x1f, 0x3d, 0x2, 0x5, 0xa, 0xaa, 0x10 }),
        ExpectedBlock(24274, { 0x89, 0x49, 0x27, 0x84, 0x15, 0x7, 0x2e, 0xa5, 0x96, 0x88, 0x45, 0x4e, 0xe9, 0xd0, 0x20, 0xf6, 0xe0, 0x23, 0x19, 0xc3, 0xc5, 0x46, 0xd8, 0xf4, 0xa2, 0x34, 0xeb, 0x8b, 0x74, 0x51, 0xfb, 0x84 }),
        ExpectedBlock(21197, { 0x2c, 0xcf, 0x2e, 0x41, 0x1f, 0x63, 0x20, 0x16, 0x5c, 0xef, 0x6c, 0x8a, 0xd7, 0xc, 0x8b, 0xa, 0x7, 0x46, 0x97, 0xe3, 0xd2, 0x40, 0x1e, 0x42, 0x1f, 0xe4, 0x9f, 0x74, 0x77, 0x52, 0x7a, 0xc4 }), }),
    ExpectedBlockMapFile(L"TestAppxPackage.winmd", 51, 3072, {
        ExpectedBlock(1311, { 0x57, 0x7e, 0xea, 0x8, 0xec, 0x38, 0x8e, 0x14, 0xba, 0x5a, 0x47, 0x9a, 0x38, 0xbc, 0xe, 0x88, 0xb5, 0xe7, 0xa2, 0xf0, 0x51, 0xfc, 0x4b, 0xba, 0x6d, 0x76, 0x52, 0xc1, 0x6a, 0x86, 0x7e, 0x4 }) }),
    ExpectedBlockMapFile(L"resources.pri", 43, 3760, {
        ExpectedBlock(1501, { 0xa2, 0x66, 0x9d, 0x16, 0x7e, 0x73, 0x5d, 0xb0, 0x5f, 0xe, 0xd9, 0x80, 0x66, 0x36, 0xe3, 0x17, 0x9e, 0x1b, 0x87, 0x71, 0xca, 0x65, 0xba, 0xdc, 0xf, 0xc5, 0x29, 0x6, 0x85, 0x13, 0x89, 0x16 }) }),
};

// Expected values for bundle
#ifdef WIN32
static const char* bundleToTest = "..\\test\\appx\\bundles\\StoreSigned_Desktop_x86_x64_MoviesTV.appxbundle";
#else
static const char* bundleToTest = "../test/appx/bundles/StoreSigned_Desktop_x86_x64_MoviesTV.appxbundle";
#endif

struct ExpectedPackagesInBundle
{
    const wchar_t*      name;
    APPX_BUNDLE_PAYLOAD_PACKAGE_TYPE type;
    const std::uint64_t offset;
    const std::uint64_t size;
    const std::vector<std::wstring> languages;

    ExpectedPackagesInBundle(const wchar_t* n, APPX_BUNDLE_PAYLOAD_PACKAGE_TYPE t, std::uint64_t o, std::uint64_t s, std::vector<std::wstring> l) : 
        name(n), type(t), offset(o), size(s), languages(l) {}
};

static const std::vector<ExpectedPackagesInBundle> expectedPackagesInBundleList {
    ExpectedPackagesInBundle(L"Video_Production_x86.appx", APPX_BUNDLE_PAYLOAD_PACKAGE_TYPE_APPLICATION, 16732801, 9353845, { L"en" ,L"en-US" ,L"en-GB"}),
    ExpectedPackagesInBundle(L"Video_Production_x64.appx", APPX_BUNDLE_PAYLOAD_PACKAGE_TYPE_APPLICATION, 4464652, 12268070, { L"en" ,L"en-US" ,L"en-GB"}),
    ExpectedPackagesInBundle(L"resources.language-af.map.appx", APPX_BUNDLE_PAYLOAD_PACKAGE_TYPE_RESOURCE, 60, 33924, { L"af-za"}),
    ExpectedPackagesInBundle(L"resources.language-am.map.appx", APPX_BUNDLE_PAYLOAD_PACKAGE_TYPE_RESOURCE, 34068, 38294, { L"am-et"}),
    ExpectedPackagesInBundle(L"resources.language-ar.map.appx", APPX_BUNDLE_PAYLOAD_PACKAGE_TYPE_RESOURCE, 72446, 50229, { L"ar-sa"}),
    ExpectedPackagesInBundle(L"resources.language-as.map.appx", APPX_BUNDLE_PAYLOAD_PACKAGE_TYPE_RESOURCE, 122759, 37866, { L"as-in"}),
    ExpectedPackagesInBundle(L"resources.language-az-latn.map.appx", APPX_BUNDLE_PAYLOAD_PACKAGE_TYPE_RESOURCE, 160714, 35857, { L"az-latn-az"}),
    ExpectedPackagesInBundle(L"resources.language-be.map.appx", APPX_BUNDLE_PAYLOAD_PACKAGE_TYPE_RESOURCE, 196655, 39405, { L"be-by"}),
    ExpectedPackagesInBundle(L"resources.language-bg.map.appx", APPX_BUNDLE_PAYLOAD_PACKAGE_TYPE_RESOURCE, 236144, 53138, { L"bg-bg"}),
    ExpectedPackagesInBundle(L"resources.language-bn.map.appx", APPX_BUNDLE_PAYLOAD_PACKAGE_TYPE_RESOURCE, 289366, 57863, { L"bn" ,L"bn-bd" ,L"bn-in"}),
    ExpectedPackagesInBundle(L"resources.language-bs.map.appx", APPX_BUNDLE_PAYLOAD_PACKAGE_TYPE_RESOURCE, 347313, 35997, { L"bs-latn-ba"}),
    ExpectedPackagesInBundle(L"resources.language-ca.map.appx", APPX_BUNDLE_PAYLOAD_PACKAGE_TYPE_RESOURCE, 383394, 49960, { L"ca" ,L"ca-es" /*,L"ca-es-valencia"*/, L"ca-es"}), //  // For now, we only support Bcp47 tags that contains language, script and region
    ExpectedPackagesInBundle(L"resources.language-cs.map.appx", APPX_BUNDLE_PAYLOAD_PACKAGE_TYPE_RESOURCE, 433438, 49506, { L"cs-cz"}),
    ExpectedPackagesInBundle(L"resources.language-cy.map.appx", APPX_BUNDLE_PAYLOAD_PACKAGE_TYPE_RESOURCE, 483028, 35158, { L"cy-gb"}),
    ExpectedPackagesInBundle(L"resources.language-da.map.appx", APPX_BUNDLE_PAYLOAD_PACKAGE_TYPE_RESOURCE, 518270, 46147, { L"da-dk"}),
    ExpectedPackagesInBundle(L"resources.language-de.map.appx", APPX_BUNDLE_PAYLOAD_PACKAGE_TYPE_RESOURCE, 564501, 48758, { L"de-de"}),
    ExpectedPackagesInBundle(L"resources.language-el.map.appx", APPX_BUNDLE_PAYLOAD_PACKAGE_TYPE_RESOURCE, 613343, 55588, { L"el-gr"}),
    ExpectedPackagesInBundle(L"resources.language-es.map.appx", APPX_BUNDLE_PAYLOAD_PACKAGE_TYPE_RESOURCE, 669015, 67789, { L"es" ,L"es-es" ,L"es-mx"}),
    ExpectedPackagesInBundle(L"resources.language-et.map.appx", APPX_BUNDLE_PAYLOAD_PACKAGE_TYPE_RESOURCE, 736888, 46560, { L"et-ee"}),
    ExpectedPackagesInBundle(L"resources.language-eu.map.appx", APPX_BUNDLE_PAYLOAD_PACKAGE_TYPE_RESOURCE, 783532, 34327, { L"eu-es"}),
    ExpectedPackagesInBundle(L"resources.language-fa.map.appx", APPX_BUNDLE_PAYLOAD_PACKAGE_TYPE_RESOURCE, 817943, 37342, { L"fa-ir"}),
    ExpectedPackagesInBundle(L"resources.language-fi.map.appx", APPX_BUNDLE_PAYLOAD_PACKAGE_TYPE_RESOURCE, 855369, 46423, { L"fi-fi"}),
    ExpectedPackagesInBundle(L"resources.language-fil-latn.map.appx", APPX_BUNDLE_PAYLOAD_PACKAGE_TYPE_RESOURCE, 901882, 35876, { L"fil-ph"}),
    ExpectedPackagesInBundle(L"resources.language-fr.map.appx", APPX_BUNDLE_PAYLOAD_PACKAGE_TYPE_RESOURCE, 937842, 74359, { L"fr" ,L"fr-ca" ,L"fr-fr"}),
    ExpectedPackagesInBundle(L"resources.language-ga.map.appx", APPX_BUNDLE_PAYLOAD_PACKAGE_TYPE_RESOURCE, 1012285, 36488, { L"ga-ie"}),
    ExpectedPackagesInBundle(L"resources.language-gd-latn.map.appx", APPX_BUNDLE_PAYLOAD_PACKAGE_TYPE_RESOURCE, 1048862, 37198, { L"gd-gb"}),
    ExpectedPackagesInBundle(L"resources.language-gl.map.appx", APPX_BUNDLE_PAYLOAD_PACKAGE_TYPE_RESOURCE, 1086144, 35063, { L"gl-es"}),
    ExpectedPackagesInBundle(L"resources.language-gu.map.appx", APPX_BUNDLE_PAYLOAD_PACKAGE_TYPE_RESOURCE, 1121291, 37705, { L"gu-in"}),
    ExpectedPackagesInBundle(L"resources.language-ha-latn.map.appx", APPX_BUNDLE_PAYLOAD_PACKAGE_TYPE_RESOURCE, 1159085, 34784, { L"ha-latn-ng"}),
    ExpectedPackagesInBundle(L"resources.language-he.map.appx", APPX_BUNDLE_PAYLOAD_PACKAGE_TYPE_RESOURCE, 1193953, 46852, { L"he-il"}),
    ExpectedPackagesInBundle(L"resources.language-hi.map.appx", APPX_BUNDLE_PAYLOAD_PACKAGE_TYPE_RESOURCE, 1240889, 37863, { L"hi-in"}),
    ExpectedPackagesInBundle(L"resources.language-hr.map.appx", APPX_BUNDLE_PAYLOAD_PACKAGE_TYPE_RESOURCE, 1278836, 47627, { L"hr-hr"}),
    ExpectedPackagesInBundle(L"resources.language-hu.map.appx", APPX_BUNDLE_PAYLOAD_PACKAGE_TYPE_RESOURCE, 1326547, 48920, { L"hu-hu"}),
    ExpectedPackagesInBundle(L"resources.language-hy.map.appx", APPX_BUNDLE_PAYLOAD_PACKAGE_TYPE_RESOURCE, 1375551, 38013, { L"hy-am"}),
    ExpectedPackagesInBundle(L"resources.language-id.map.appx", APPX_BUNDLE_PAYLOAD_PACKAGE_TYPE_RESOURCE, 1413648, 33383, { L"id-id"}),
    ExpectedPackagesInBundle(L"resources.language-ig-latn.map.appx", APPX_BUNDLE_PAYLOAD_PACKAGE_TYPE_RESOURCE, 1447120, 34264, { L"ig-ng"}),
    ExpectedPackagesInBundle(L"resources.language-is.map.appx", APPX_BUNDLE_PAYLOAD_PACKAGE_TYPE_RESOURCE, 1481468, 35618, { L"is-is"}),
    ExpectedPackagesInBundle(L"resources.language-it.map.appx", APPX_BUNDLE_PAYLOAD_PACKAGE_TYPE_RESOURCE, 1517170, 46077, { L"it-it"}),
    ExpectedPackagesInBundle(L"resources.language-ja.map.appx", APPX_BUNDLE_PAYLOAD_PACKAGE_TYPE_RESOURCE, 1563331, 44703, { L"ja-jp"}),
    ExpectedPackagesInBundle(L"resources.language-ka.map.appx", APPX_BUNDLE_PAYLOAD_PACKAGE_TYPE_RESOURCE, 1608118, 37656, { L"ka-ge"}),
    ExpectedPackagesInBundle(L"resources.language-kk.map.appx", APPX_BUNDLE_PAYLOAD_PACKAGE_TYPE_RESOURCE, 1645858, 37906, { L"kk-kz"}),
    ExpectedPackagesInBundle(L"resources.language-km.map.appx", APPX_BUNDLE_PAYLOAD_PACKAGE_TYPE_RESOURCE, 1683848, 39174, { L"km-kh"}),
    ExpectedPackagesInBundle(L"resources.language-kn.map.appx", APPX_BUNDLE_PAYLOAD_PACKAGE_TYPE_RESOURCE, 1723106, 38024, { L"kn-in"}),
    ExpectedPackagesInBundle(L"resources.language-ko.map.appx", APPX_BUNDLE_PAYLOAD_PACKAGE_TYPE_RESOURCE, 1761214, 45023, { L"ko-kr"}),
    ExpectedPackagesInBundle(L"resources.language-kok.map.appx", APPX_BUNDLE_PAYLOAD_PACKAGE_TYPE_RESOURCE, 1806322, 38689, { L"kok-in"}),
    ExpectedPackagesInBundle(L"resources.language-ku-arab.map.appx", APPX_BUNDLE_PAYLOAD_PACKAGE_TYPE_RESOURCE, 1845100, 38391, { L"ku-arab-iq"}),
    ExpectedPackagesInBundle(L"resources.language-ky-cyrl.map.appx", APPX_BUNDLE_PAYLOAD_PACKAGE_TYPE_RESOURCE, 1883580, 39788, { L"ky-kg"}),
    ExpectedPackagesInBundle(L"resources.language-lb.map.appx", APPX_BUNDLE_PAYLOAD_PACKAGE_TYPE_RESOURCE, 1923452, 36366, { L"lb-lu"}),
    ExpectedPackagesInBundle(L"resources.language-lo.map.appx", APPX_BUNDLE_PAYLOAD_PACKAGE_TYPE_RESOURCE, 1959902, 38344, { L"lo-la"}),
    ExpectedPackagesInBundle(L"resources.language-lt.map.appx", APPX_BUNDLE_PAYLOAD_PACKAGE_TYPE_RESOURCE, 1998330, 47942, { L"lt-lt"}),
    ExpectedPackagesInBundle(L"resources.language-lv.map.appx", APPX_BUNDLE_PAYLOAD_PACKAGE_TYPE_RESOURCE, 2046356, 48190, { L"lv-lv"}),
    ExpectedPackagesInBundle(L"resources.language-mi-latn.map.appx", APPX_BUNDLE_PAYLOAD_PACKAGE_TYPE_RESOURCE, 2094635, 34939, { L"mi-nz"}),
    ExpectedPackagesInBundle(L"resources.language-mk.map.appx", APPX_BUNDLE_PAYLOAD_PACKAGE_TYPE_RESOURCE, 2129658, 37942, { L"mk-mk"}),
    ExpectedPackagesInBundle(L"resources.language-ml.map.appx", APPX_BUNDLE_PAYLOAD_PACKAGE_TYPE_RESOURCE, 2167684, 39015, { L"ml-in"}),
    ExpectedPackagesInBundle(L"resources.language-mn-cyrl.map.appx", APPX_BUNDLE_PAYLOAD_PACKAGE_TYPE_RESOURCE, 2206788, 37804, { L"mn-mn"}),
    ExpectedPackagesInBundle(L"resources.language-mr.map.appx", APPX_BUNDLE_PAYLOAD_PACKAGE_TYPE_RESOURCE, 2244676, 38764, { L"mr-in"}),
    ExpectedPackagesInBundle(L"resources.language-ms.map.appx", APPX_BUNDLE_PAYLOAD_PACKAGE_TYPE_RESOURCE, 2283524, 33922, { L"ms-my"}),
    ExpectedPackagesInBundle(L"resources.language-mt.map.appx", APPX_BUNDLE_PAYLOAD_PACKAGE_TYPE_RESOURCE, 2317530, 36256, { L"mt-mt"}),
    ExpectedPackagesInBundle(L"resources.language-nb.map.appx", APPX_BUNDLE_PAYLOAD_PACKAGE_TYPE_RESOURCE, 2353870, 45870, { L"nb-no"}),
    ExpectedPackagesInBundle(L"resources.language-ne.map.appx", APPX_BUNDLE_PAYLOAD_PACKAGE_TYPE_RESOURCE, 2399824, 38182, { L"ne-np"}),
    ExpectedPackagesInBundle(L"resources.language-nl.map.appx", APPX_BUNDLE_PAYLOAD_PACKAGE_TYPE_RESOURCE, 2438090, 46590, { L"nl-nl"}),
    ExpectedPackagesInBundle(L"resources.language-nn.map.appx", APPX_BUNDLE_PAYLOAD_PACKAGE_TYPE_RESOURCE, 2484764, 34221, { L"nn-no"}),
    ExpectedPackagesInBundle(L"resources.language-nso.map.appx", APPX_BUNDLE_PAYLOAD_PACKAGE_TYPE_RESOURCE, 2519070, 35648, { L"nso-za"}),
    ExpectedPackagesInBundle(L"resources.language-or.map.appx", APPX_BUNDLE_PAYLOAD_PACKAGE_TYPE_RESOURCE, 2554802, 38357, { L"or-in"}),
    ExpectedPackagesInBundle(L"resources.language-pa-arab.map.appx", APPX_BUNDLE_PAYLOAD_PACKAGE_TYPE_RESOURCE, 2632148, 38731, { L"pa-arab-pk"}),
    ExpectedPackagesInBundle(L"resources.language-pa.map.appx", APPX_BUNDLE_PAYLOAD_PACKAGE_TYPE_RESOURCE, 2593243, 38816, { L"pa-in"}),
    ExpectedPackagesInBundle(L"resources.language-pl.map.appx", APPX_BUNDLE_PAYLOAD_PACKAGE_TYPE_RESOURCE, 2670963, 48647, { L"pl-pl"}),
    ExpectedPackagesInBundle(L"resources.language-prs-arab.map.appx", APPX_BUNDLE_PAYLOAD_PACKAGE_TYPE_RESOURCE, 2719700, 38115, { L"prs-af"}),
    ExpectedPackagesInBundle(L"resources.language-pt.map.appx", APPX_BUNDLE_PAYLOAD_PACKAGE_TYPE_RESOURCE, 2757899, 77370, { L"pt" ,L"pt-br" ,L"pt-pt"}),
    ExpectedPackagesInBundle(L"resources.language-quc-latn.map.appx", APPX_BUNDLE_PAYLOAD_PACKAGE_TYPE_RESOURCE, 2835359, 35349, { L"quc-latn-gt"}),
    ExpectedPackagesInBundle(L"resources.language-quz-latn.map.appx", APPX_BUNDLE_PAYLOAD_PACKAGE_TYPE_RESOURCE, 2870798, 34979, { L"quz-pe"}),
    ExpectedPackagesInBundle(L"resources.language-ro.map.appx", APPX_BUNDLE_PAYLOAD_PACKAGE_TYPE_RESOURCE, 2905861, 48489, { L"ro-ro"}),
    ExpectedPackagesInBundle(L"resources.language-ru.map.appx", APPX_BUNDLE_PAYLOAD_PACKAGE_TYPE_RESOURCE, 2954434, 53333, { L"ru-ru"}),
    ExpectedPackagesInBundle(L"resources.language-rw.map.appx", APPX_BUNDLE_PAYLOAD_PACKAGE_TYPE_RESOURCE, 3007851, 34897, { L"rw-rw"}),
    ExpectedPackagesInBundle(L"resources.language-sd-arab.map.appx", APPX_BUNDLE_PAYLOAD_PACKAGE_TYPE_RESOURCE, 3042837, 37541, { L"sd-arab-pk"}),
    ExpectedPackagesInBundle(L"resources.language-si.map.appx", APPX_BUNDLE_PAYLOAD_PACKAGE_TYPE_RESOURCE, 3080462, 38419, { L"si-lk"}),
    ExpectedPackagesInBundle(L"resources.language-sk.map.appx", APPX_BUNDLE_PAYLOAD_PACKAGE_TYPE_RESOURCE, 3118965, 49059, { L"sk-sk"}),
    ExpectedPackagesInBundle(L"resources.language-sl.map.appx", APPX_BUNDLE_PAYLOAD_PACKAGE_TYPE_RESOURCE, 3168108, 47528, { L"sl-si"}),
    ExpectedPackagesInBundle(L"resources.language-sq.map.appx", APPX_BUNDLE_PAYLOAD_PACKAGE_TYPE_RESOURCE, 3215720, 35944, { L"sq-al"}),
    ExpectedPackagesInBundle(L"resources.language-sr-cyrl.map.appx", APPX_BUNDLE_PAYLOAD_PACKAGE_TYPE_RESOURCE, 3251753, 59343, { L"sr-cyrl" ,L"sr-cyrl-ba" ,L"sr-cyrl-rs"}),
    ExpectedPackagesInBundle(L"resources.language-sr-latn.map.appx", APPX_BUNDLE_PAYLOAD_PACKAGE_TYPE_RESOURCE, 3311185, 47795, { L"sr-latn-rs"}),
    ExpectedPackagesInBundle(L"resources.language-sv.map.appx", APPX_BUNDLE_PAYLOAD_PACKAGE_TYPE_RESOURCE, 3359064, 46205, { L"sv-se"}),
    ExpectedPackagesInBundle(L"resources.language-sw.map.appx", APPX_BUNDLE_PAYLOAD_PACKAGE_TYPE_RESOURCE, 3405353, 34205, { L"sw-ke"}),
    ExpectedPackagesInBundle(L"resources.language-ta.map.appx", APPX_BUNDLE_PAYLOAD_PACKAGE_TYPE_RESOURCE, 3439642, 37810, { L"ta-in"}),
    ExpectedPackagesInBundle(L"resources.language-te.map.appx", APPX_BUNDLE_PAYLOAD_PACKAGE_TYPE_RESOURCE, 3477536, 39218, { L"te-in"}),
    ExpectedPackagesInBundle(L"resources.language-tg-cyrl.map.appx", APPX_BUNDLE_PAYLOAD_PACKAGE_TYPE_RESOURCE, 3516843, 38794, { L"tg-cyrl-tj"}),
    ExpectedPackagesInBundle(L"resources.language-th.map.appx", APPX_BUNDLE_PAYLOAD_PACKAGE_TYPE_RESOURCE, 3555721, 49474, { L"th-th"}),
    ExpectedPackagesInBundle(L"resources.language-ti.map.appx", APPX_BUNDLE_PAYLOAD_PACKAGE_TYPE_RESOURCE, 3605279, 39356, { L"ti-et"}),
    ExpectedPackagesInBundle(L"resources.language-tk-latn.map.appx", APPX_BUNDLE_PAYLOAD_PACKAGE_TYPE_RESOURCE, 3644724, 35118, { L"tk-tm"}),
    ExpectedPackagesInBundle(L"resources.language-tn.map.appx", APPX_BUNDLE_PAYLOAD_PACKAGE_TYPE_RESOURCE, 3679926, 35767, { L"tn-za"}),
    ExpectedPackagesInBundle(L"resources.language-tr.map.appx", APPX_BUNDLE_PAYLOAD_PACKAGE_TYPE_RESOURCE, 3715777, 47044, { L"tr-tr"}),
    ExpectedPackagesInBundle(L"resources.language-tt-cyrl.map.appx", APPX_BUNDLE_PAYLOAD_PACKAGE_TYPE_RESOURCE, 3762910, 37626, { L"tt-ru"}),
    ExpectedPackagesInBundle(L"resources.language-ug-arab.map.appx", APPX_BUNDLE_PAYLOAD_PACKAGE_TYPE_RESOURCE, 3800625, 36830, { L"ug-cn"}),
    ExpectedPackagesInBundle(L"resources.language-uk.map.appx", APPX_BUNDLE_PAYLOAD_PACKAGE_TYPE_RESOURCE, 3837539, 54273, { L"uk-ua"}),
    ExpectedPackagesInBundle(L"resources.language-ur.map.appx", APPX_BUNDLE_PAYLOAD_PACKAGE_TYPE_RESOURCE, 3891896, 38367, { L"ur-pk"}),
    ExpectedPackagesInBundle(L"resources.language-uz-latn.map.appx", APPX_BUNDLE_PAYLOAD_PACKAGE_TYPE_RESOURCE, 3930352, 35943, { L"uz-latn-uz"}),
    ExpectedPackagesInBundle(L"resources.language-vi.map.appx", APPX_BUNDLE_PAYLOAD_PACKAGE_TYPE_RESOURCE, 3966379, 37286, { L"vi-vn"}),
    ExpectedPackagesInBundle(L"resources.language-wo-latn.map.appx", APPX_BUNDLE_PAYLOAD_PACKAGE_TYPE_RESOURCE, 4003754, 34479, { L"wo-sn"}),
    ExpectedPackagesInBundle(L"resources.language-xh.map.appx", APPX_BUNDLE_PAYLOAD_PACKAGE_TYPE_RESOURCE, 4038317, 34886, { L"xh-za"}),
    ExpectedPackagesInBundle(L"resources.language-yo-latn.map.appx", APPX_BUNDLE_PAYLOAD_PACKAGE_TYPE_RESOURCE, 4073292, 37061, { L"yo-ng"}),
    ExpectedPackagesInBundle(L"resources.language-zh-hans.map.appx", APPX_BUNDLE_PAYLOAD_PACKAGE_TYPE_RESOURCE, 4110442, 44058, { L"zh-Hans-CN"}), // real value is zn-cn, but we return the correct Bcp47Tag in this case
    ExpectedPackagesInBundle(L"resources.language-zh-hant.map.appx", APPX_BUNDLE_PAYLOAD_PACKAGE_TYPE_RESOURCE, 4154589, 74495, { L"zh-Hant-HK" ,L"zh-Hant-TW"}), // same as above, zh-hk and zh-tw
    ExpectedPackagesInBundle(L"resources.language-zu.map.appx", APPX_BUNDLE_PAYLOAD_PACKAGE_TYPE_RESOURCE, 4229168, 35527, { L"zu-za"}),
    // Currently, we don't support resource packages without language.
    //ExpectedPackagesInBundle(L"resources.scale-125.map.appx", APPX_BUNDLE_PAYLOAD_PACKAGE_TYPE_RESOURCE, 4264777, 31410, }),
    //ExpectedPackagesInBundle(L"resources.scale-150.map.appx", APPX_BUNDLE_PAYLOAD_PACKAGE_TYPE_RESOURCE, 4296269, 34552, }),
    //ExpectedPackagesInBundle(L"resources.scale-200.map.appx", APPX_BUNDLE_PAYLOAD_PACKAGE_TYPE_RESOURCE, 4330903, 43136, }),
    //ExpectedPackagesInBundle(L"resources.scale-400.map.appx", APPX_BUNDLE_PAYLOAD_PACKAGE_TYPE_RESOURCE, 4374121, 90452, }),
};

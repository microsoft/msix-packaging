#!/bin/bash
TESTFAILED=0
directory=$1
function FindBinFolder {
    echo "Searching under" $PWD
    #look in .vs/bin first
    if [ -e "../../.vs/bin/makemsix" ]
    then
        BINDIR="../../.vs/bin"
    elif [ -e "../../.vscode/bin/makemsix" ]
    then
        BINDIR="../../.vscode/bin"
    elif [ -e "../../build/bin/makemsix" ]
    then
        BINDIR="../../build/bin"
    else
        echo "ERROR: Could not find build binaries"
        exit 2
    fi
}

function CleanupUnpackFolder {
    rm -f -r ./../unpack/*
    if [ -e "./../unpack/*" ]
    then
        echo "ERROR: Could not cleanup ./../unpack directory"
        exit
    fi
}

function ValidateResult {
    local EXPECTED=$1
    echo "Validating extracted files with "$EXPECTED
    ls -l "./../unpack/" | grep "^d" | awk 'NF > 4 {print $9}' >> output.txt
    ls -lRp "./../unpack" | grep -v / | awk 'NF > 4 {print $5, $9}' >> output.txt
    diff output.txt $EXPECTED
    diff_result=$?
    if [ $diff_result -eq 1 ]
    then
        echo "FAILED comparing extracted files"
        TESTFAILED=1
    else
        echo "succeeded comparing extracted files"
    fi
    rm output.txt
}

function RunTest {
    CleanupUnpackFolder
    local SUCCESS="$1"
    local UNPACKFOLDER="$2"
    local ARGS="$3"
    echo "------------------------------------------------------"
    echo $BINDIR/makemsix unpack -d ./../unpack -p $UNPACKFOLDER $ARGS
    echo "------------------------------------------------------"
    $BINDIR/makemsix unpack -d ./../unpack -p $UNPACKFOLDER $ARGS
    local RESULT=$?
    echo "expect: "$SUCCESS", got: "$RESULT
    if [ $RESULT -eq $SUCCESS ]
    then
        echo "succeeded"
    else
        echo "FAILED"
        TESTFAILED=1
    fi
}

function RunApiTest {
    local CURRENTLOCATION=`pwd`
    cd $BINDIR/..
    echo "------------------------------------------------------"
    echo "bin/apitest.exe -f $1"
    echo "------------------------------------------------------"
    bin/apitest -f $1
    local RESULT=$?
    if [ $RESULT -eq 0 ]
    then
        echo "succeeded"
    else
        echo "FAILED"
  	TESTFAILED=1
    fi
    cd $CURRENTLOCATION
}

FindBinFolder
# return code is last two digits, but in decimal, not hex.  e.g. 0x8bad0002 == 2, 0x8bad0041 == 65, etc...
# common codes:
# SignatureInvalid        = ERROR_FACILITY + 0x0041 == 65

RunTest 2  ./../appx/Empty.appx -sv
RunTest 0  ./../appx/HelloWorld.appx -ss
RunTest 0  ./../appx/NotepadPlusPlus.appx -ss
RunTest 0  ./../appx/IntlPackage.appx -ss
RunTest 66 ./../appx/SignatureNotLastPart-ERROR_BAD_FORMAT.appx
RunTest 66 ./../appx/SignedTamperedBlockMap-TRUST_E_BAD_DIGEST.appx
RunTest 65 ./../appx/SignedTamperedBlockMap-TRUST_E_BAD_DIGEST.appx -sv
RunTest 66 ./../appx/SignedTamperedCD-TRUST_E_BAD_DIGEST.appx
RunTest 66 ./../appx/SignedTamperedCodeIntegrity-TRUST_E_BAD_DIGEST.appx
RunTest 66 ./../appx/SignedTamperedContentTypes-TRUST_E_BAD_DIGEST.appx
RunTest 66 ./../appx/SignedUntrustedCert-CERT_E_CHAINING.appx
RunTest 0 ./../appx/TestAppxPackage_Win32.appx -ss
RunTest 0 ./../appx/TestAppxPackage_x64.appx -ss
RunTest 18 ./../appx/UnsignedZip64WithCI-APPX_E_MISSING_REQUIRED_FILE.appx
RunTest 1 ./../appx/FileDoesNotExist.appx -ss
RunTest 81 ./../appx/BlockMap/Missing_Manifest_in_blockmap.appx -ss
RunTest 81 ./../appx/BlockMap/ContentTypes_in_blockmap.appx -ss
RunTest 81 ./../appx/BlockMap/Invalid_Bad_Block.msix -ss
RunTest 81 ./../appx/BlockMap/Size_wrong_uncompressed.msix -ss
RunTest 2 ./../appx/BlockMap/Extra_file_in_blockmap.msix -ss
RunTest 81 ./../appx/BlockMap/File_missing_from_blockmap.msix -ss
RunTest 51 ./../appx/BlockMap/No_blockmap.appx -ss
RunTest 3 ./../appx/BlockMap/Bad_Namespace_Blockmap.appx -ss
RunTest 81 ./../appx/BlockMap/Duplicate_file_in_blockmap.appx -ss

RunTest 0 ./../appx/महसुस/StoreSigned_Desktop_x64_MoviesTV.appx

RunTest 0  ./../appx/StoreSigned_Desktop_x64_MoviesTV.appx
ValidateResult ExpectedResult/$directory/StoreSigned_Desktop_x64_MoviesTV.txt

# IMPORTANT! For Linux we expect English. For MacOs, English (US) and Spanish (MX)
# Bundle tests
RunTest 81 ./../appx/bundles/BlockMapContainsPayloadPackage.appxbundle -ss
RunTest 51 ./../appx/bundles/BlockMapIsMissing.appxbundle -ss
RunTest 2 ./../appx/bundles/BlockMapViolatesSchema.appxbundle -ss
# RunTest 0 ./../appx/bundles/ContainsNeutralAndX86AppPackages.appxbundle
RunTest 2 ./../appx/bundles/ContainsNoPayload.appxbundle -ss
RunTest 97 ./../appx/bundles/ContainsOnlyResourcePackages.appxbundle -ss
# RunTest 0 ./../appx/bundles/ContainsTwoNeutralAppPackages.appxbundle
RunTest 0 ./../appx/bundles/MainBundle.appxbundle -ss
# RunTest 0 ./../appx/bundles/ManifestDeclaresAppPackageForResourcePackage.appxbundle
# RunTest 0 ./../appx/bundles/ManifestDeclaresResourcePackageForAppPackage.appxbundle
# RunTest 0 ./../appx/bundles/ManifestHasExtraPackage.appxbundle
RunTest 52 ./../appx/bundles/ManifestIsMissing.appxbundle -ss
#RunTest 97 ./../appx/bundles/ManifestPackageHasIncorrectArchitecture.appxbundle -ss ### WIN8-era package
#RunTest 97 ./../appx/bundles/ManifestPackageHasIncorrectName.appxbundle -ss ### WIN8-era package
#RunTest 97 ./../appx/bundles/ManifestPackageHasIncorrectPublisher.appxbundle -ss ### WIN8-era package
RunTest 97 ./../appx/bundles/ManifestPackageHasIncorrectSize.appxbundle -ss
#RunTest 97 ./../appx/bundles/ManifestPackageHasIncorrectVersion.appxbundle -ss ### WIN8-era package
# RunTest 0 ./../appx/bundles/ManifestPackageHasInvalidOffset.appxbundle
# RunTest 0 ./../appx/bundles/ManifestPackageHasInvalidRange.appxbundle
RunTest 2 ./../appx/bundles/ManifestViolatesSchema.appxbundle -ss
RunTest 97 ./../appx/bundles/PayloadPackageHasNonAppxExtension.appxbundle -ss
RunTest 97 ./../appx/bundles/PayloadPackageIsCompressed.appxbundle -ss
RunTest 3 ./../appx/bundles/PayloadPackageIsEmpty.appxbundle -ss
RunTest 87 ./../appx/bundles/PayloadPackageIsNotAppxPackage.appxbundle -ss
# RunTest 0 ./../appx/bundles/PayloadPackageNotListedInManifest.appxbundle
RunTest 66 ./../appx/bundles/SignedUntrustedCert-CERT_E_CHAINING.appxbundle
RunTest 0 ./../appx/bundles/BundleWithIntlPackage.appxbundle -ss
RunTest 0 ./../appx/bundles/StoreSigned_Desktop_x86_x64_MoviesTV.appxbundle
# turn off this test temporarly. TODO: figure our Azure Agents with English, Spanish and traditonal Chinese.
# ValidateResult ExpectedResult/$directory/StoreSigned_Desktop_x86_x64_MoviesTV.txt

#Flat bundles
mv ./../appx/flat/assets.appx ./../appx/flat/assets_back.appx
RunTest 1 ./../appx/flat/FlatBundleWithAsset.appxbundle -ss
mv ./../appx/flat/assets_back.appx ./../appx/flat/assets.appx
RunTest 0 ./../appx/flat/FlatBundleWithAsset.appxbundle -ss
# turn off this test temporarly. TODO: figure our Azure Agents with English, Spanish and traditonal Chinese.
# ValidateResult ExpectedResult/$directory/FlatBundleWithAsset.txt

CleanupUnpackFolder

RunApiTest test/api/input/apitest_test_1.txt

    echo "-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-="
if [ $TESTFAILED -ne 0 ]
then
    echo "                           FAILED                                 "
    exit $TESTFAILED
else
    echo "                           passed                                 "
    exit 0
fi

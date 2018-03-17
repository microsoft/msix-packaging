#!/bin/bash
TESTFAILED=0
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

FindBinFolder
# return code is last two digits, but in decimal, not hex.  e.g. 0x8bad0002 == 2, 0x8bad0041 == 65, etc...
# common codes:
# SignatureInvalid        = ERROR_FACILITY + 0x0041 == 65

RunTest 2  ./../appx/Empty.appx -sv
RunTest 0  ./../appx/HelloWorld.appx -ss
RunTest 66 ./../appx/SignatureNotLastPart-ERROR_BAD_FORMAT.appx
RunTest 66 ./../appx/SignedTamperedBlockMap-TRUST_E_BAD_DIGEST.appx
RunTest 65 ./../appx/SignedTamperedBlockMap-TRUST_E_BAD_DIGEST.appx -sv
RunTest 66 ./../appx/SignedTamperedCD-TRUST_E_BAD_DIGEST.appx
RunTest 66 ./../appx/SignedTamperedCodeIntegrity-TRUST_E_BAD_DIGEST.appx
RunTest 66 ./../appx/SignedTamperedContentTypes-TRUST_E_BAD_DIGEST.appx
RunTest 66 ./../appx/SignedUntrustedCert-CERT_E_CHAINING.appx
RunTest 0  ./../appx/StoreSigned_Desktop_x64_MoviesTV.appx
RunTest 0 ./../appx/TestAppxPackage_Win32.appx -ss
RunTest 0 ./../appx/TestAppxPackage_x64.appx -ss
RunTest 18 ./../appx/UnsignedZip64WithCI-APPX_E_MISSING_REQUIRED_FILE.appx
RunTest 1 ./../appx/FileDoesNotExist.appx -ss
RunTest 81 ./../appx/BlockMap/Missing_Manifest_in_blockmap.appx -ss
RunTest 81 ./../appx/BlockMap/ContentTypes_in_blockmap.appx -ss
RunTest 65 ./../appx/BlockMap/Invalid_Bad_Block.appx -ss
RunTest 81 ./../appx/BlockMap/Size_wrong_uncompressed.appx -ss
RunTest 0 ./../appx/BlockMap/HelloWorld.appx -ss
RunTest 2 ./../appx/BlockMap/Extra_file_in_blockmap.appx -ss
RunTest 81 ./../appx/BlockMap/File_missing_from_blockmap.appx -ss
RunTest 51 ./../appx/BlockMap/No_blockmap.appx -ss
RunTest 3 ./../appx/BlockMap/Bad_Namespace_Blockmap.appx -ss
RunTest 81 ./../appx/BlockMap/Duplicate_file_in_blockmap.appx -ss
RunTest 0  ./../appx/sdx/en-us_win32.appx

    echo "-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-="
if [ $TESTFAILED -ne 0 ]
then
    echo "                           FAILED                                 "
    exit $TESTFAILED
else
    echo "                           passed                                 "
    exit 0
fi
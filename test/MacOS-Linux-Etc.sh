#!/bin/bash
TESTFAILED=0
function FindBinFolder {
    #look in .vs/bin first
    if [ -e "../.vs/bin/MakeXplat" ]
    then
        BINDIR="../.vs/bin"
    elif [ -e "../.vscode/bin/MakeXplat" ]
    then
        BINDIR="../.vscode/bin"
    elif [ -e "../build/bin/MakeXplat" ]
    then
        BINDIR="../build/bin"
    else 
        echo "ERROR: Could not find build binaries"
        exit
    fi
}

function CleanupUnpackFolder {
    rm -f -r ./unpack/*
    if [ -e "./unpack/*" ]
    then
        echo "ERROR: Could not cleanup ./unpack directory"
        exit
    fi
}

function RunTest {
    CleanupUnpackFolder
    local SUCCESS="$1"
    local UNPACKFOLDER="$2"
    local ARGS="$3"
    echo "------------------------------------------------------"
    echo $BINDIR/MakeXplat unpack -d ./unpack -p $UNPACKFOLDER $ARGS
    echo "------------------------------------------------------"
    $BINDIR/MakeXplat unpack -d ./unpack -p $UNPACKFOLDER $ARGS
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
# AppxSignatureInvalid        = ERROR_FACILITY + 0x0041 == 65

RunTest 2 ./appx/Empty.appx -sv
RunTest 0 ./appx/HelloWorld.appx -ss
RunTest 65 ./appx/SignatureNotLastPart-ERROR_BAD_FORMAT.appx
#RunTest 0x134 ./appx/SignedMismatchedPublisherName-ERROR_BAD_FORMAT.appx
RunTest 65 ./appx/SignedTamperedBlockMap-TRUST_E_BAD_DIGEST.appx
RunTest 65 ./appx/SignedTamperedBlockMap-TRUST_E_BAD_DIGEST.appx -sv
RunTest 65 ./appx/SignedTamperedCD-TRUST_E_BAD_DIGEST.appx
RunTest 65 ./appx/SignedTamperedCodeIntegrity-TRUST_E_BAD_DIGEST.appx
RunTest 65 ./appx/SignedTamperedContentTypes-TRUST_E_BAD_DIGEST.appx
RunTest 65 ./appx/SignedUntrustedCert-CERT_E_CHAINING.appx
RunTest 0 ./appx/StoreSigned_Desktop_x64_MoviesTV.appx
RunTest 65 ./appx/TestAppxPackage_Win32.appx
RunTest 65 ./appx/TestAppxPackage_x64.appx
RunTest 18 ./appx/UnsignedZip64WithCI-APPX_E_MISSING_REQUIRED_FILE.appx

    echo "-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-="
if [ $TESTFAILED -ne 0 ]
then
    echo "                           FAILED                                 "
    exit $TESTFAILED
else
    echo "                           passed                                 "
    exit 0
fi
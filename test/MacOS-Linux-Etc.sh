#!/bin/bash

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
    echo "------------------------------------------------------" >> MacOS-Linux-Etc.log
    local UNPACKFOLDER="$1"
    local SUCCESS="$2"
    echo $UNPACKFOLDER >> MacOS-Linux-Etc.log
    $BINDIR/MakeXplat unpack -d ./unpack -p $UNPACKFOLDER >> MacOS-Linux-Etc.log
    if [ "$?" -eq $SUCCESS ]
    then
        echo "Succeeded: $1" >> MacOS-Linux-Etc.log
    else
        echo "Failed: $1" >> MacOS-Linux-Etc.log
    fi
}

FindBinFolder

rm -f "./MacOS-Linux-Etc.log"

RunTest ./appx/Empty.appx 134
RunTest ./appx/HelloWorld.appx 134
RunTest ./appx/SignatureNotLastPart-ERROR_BAD_FORMAT.appx 134
RunTest ./appx/SignedMismatchedPublisherName-ERROR_BAD_FORMAT.appx 134
RunTest ./appx/SignedTamperedBlockMap-TRUST_E_BAD_DIGEST.appx 134
RunTest ./appx/SignedTamperedCD-TRUST_E_BAD_DIGEST.appx 134
RunTest ./appx/SignedTamperedCodeIntegrity-TRUST_E_BAD_DIGEST.appx 134
RunTest ./appx/SignedTamperedContentTypes-TRUST_E_BAD_DIGEST.appx 134
RunTest ./appx/SignedUntrustedCert-CERT_E_CHAINING.appx 134
RunTest ./appx/StoreSigned_Desktop_x64_MoviesTV.appx 0
RunTest ./appx/TestAppxPackage_Win32.appx 134
RunTest ./appx/TestAppxPackage_x64.appx 134
RunTest ./appx/UnsignedZip64WithCI-APPX_E_MISSING_REQUIRED_FILE.appx 134





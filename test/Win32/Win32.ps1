
$global:TESTFAILED=0
$global:BINDIR=""

function FindBinFolder {
    if (Test-Path "..\.vs\bin\MakeXplat.exe" )
    {
        $global:BINDIR="..\.vs\bin"
    }
    elseif (Test-Path "..\.vscode\bin\MakeXplat.exe" )
    {
        $global:BINDIR="..\.vscode\bin"        
    }
    elseif (Test-Path "..\build\bin\MakeXplat.exe")
    {
        $global:BINDIR="..\build\bin"        
    }
    else
    {
        write-host "ERROR: Could not find build binaries"
        exit
    }
}

function CleanupUnpackFolder {
    Remove-Item ".\unpack\*" -recurse
    if (Test-Path ".\unpack\*" )
    {
        write-host "ERROR: Could not cleanup .\unpack directory"
        exit
    }
}

function RunTest([string] $UNPACKFOLDER, [int] $SUCCESSCODE) {
    CleanupUnpackFolder
    write-host  "------------------------------------------------------"
    write-host  $UNPACKFOLDER >> Win32.log
    #$BINDIR\MakeXplat unpack -d .\unpack -p $UNPACKFOLDER
	Start-Process "$BINDIR\MakeXplat.exe" -ArgumentList ("unpack", "-d", ".\unpack", "-p", $UNPACKFOLDER) -Wait
    $ERRORCODE=$?
    if ( $ERRORCODE -eq $SUCCESSCODE ) 
    {
        write-host  "Succeeded: $SUCCESSCODE"
    }
    else
    {
        write-host  "Expected: $SUCCESSCODE"
        write-host  "Failed: $ERRORCODE"
        $global:TESTFAILED=1    
    }
}

FindBinFolder

RunTest .\appx\CentennialCoffee.appx 134
RunTest .\appx\Empty.appx 134
RunTest .\appx\HelloWorld.appx 134
RunTest .\appx\SignatureNotLastPart-ERROR_BAD_FORMAT.appx 134
RunTest .\appx\SignedMismatchedPublisherName-ERROR_BAD_FORMAT.appx 134
RunTest .\appx\SignedTamperedBlockMap-TRUST_E_BAD_DIGEST.appx 134
RunTest .\appx\SignedTamperedCD-TRUST_E_BAD_DIGEST.appx 134
RunTest .\appx\SignedTamperedCodeIntegrity-TRUST_E_BAD_DIGEST.appx 134
RunTest .\appx\SignedTamperedContentTypes-TRUST_E_BAD_DIGEST.appx 134
RunTest .\appx\SignedUntrustedCert-CERT_E_CHAINING.appx 134
RunTest .\appx\StoreSigned_Desktop_x64_MoviesTV.appx 0
RunTest .\appx\TestAppxPackage_Win32.appx 134
RunTest .\appx\TestAppxPackage_x64.appx 134
RunTest .\appx\UnsignedZip64WithCI-APPX_E_MISSING_REQUIRED_FILE.appx 134
RunTest .\appx\BlockMap\TODAVIANO\Signature_in_BlockMap.appx 134
RunTest .\appx\BlockMap\TODAVIANO\SignedTamperedBlockMap-TRUST_E_BAD_DIGEST.appx 134
RunTest .\appx\BlockMap\Missing_Manifest_in_blockmap.appx 134
RunTest .\appx\BlockMap\ContentTypes_in_blockmap.appx 134
RunTest .\appx\BlockMap\Invalid_Bad_Block.appx 134
RunTest .\appx\BlockMap\Size_wrong_uncompressed.appx 134
RunTest .\appx\BlockMap\HelloWorld.appx 134
RunTest .\appx\BlockMap\Extra_file_in_blockmap.appx 134
RunTest .\appx\BlockMap\File_missing_from_blockmap.appx 134
RunTest .\appx\BlockMap\No_blockmap.appx 134
RunTest .\appx\BlockMap\Bad_Namespace_Blockmap.appx 134
RunTest .\appx\BlockMap\Duplicate_file_in_blockmap.appx 134

CleanupUnpackFolder

if ( $global:TESTFAILED -eq 1 )
{
    exit 134
}
else
{
    exit 0
}




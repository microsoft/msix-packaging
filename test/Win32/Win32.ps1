
$global:TESTFAILED=0
$global:BINDIR=""

function FindBinFolder {
    write-host "Searching under" (Get-Item -Path ".\" -Verbose).FullName
    if (Test-Path "..\..\.vs\bin\makemsix.exe" )
    {
        $global:BINDIR="..\..\.vs\bin"
    }
    elseif (Test-Path "..\..\.vscode\bin\makemsix.exe" )
    {
        $global:BINDIR="..\..\.vscode\bin"        
    }
    elseif (Test-Path "..\..\build\bin\makemsix.exe")
    {
        $global:BINDIR="..\..\build\bin"        
    }
    else
    {
        write-host "ERROR: Could not find build binaries"
        exit 2
    }

    write-host "found $global:BINDIR"
}

function CleanupUnpackFolder {
    if (Test-Path ".\..\unpack")
    {
        Remove-Item ".\..\unpack\*" -recurse
    }    
    else
    {
        write-host "creating .\..\unpack"
        New-Item -ItemType Directory -Force ".\..\unpack"        
    }
    if (Test-Path ".\..\unpack\*" )
    {
        write-host "ERROR: Could not cleanup .\..\unpack directory"
        exit
    }
}

function RunTest([int] $SUCCESSCODE, [string] $PACKAGE, [string] $OPT) {
    CleanupUnpackFolder
    $OPTIONS = "unpack -d .\..\unpack -p $PACKAGE $OPT"
    write-host  "------------------------------------------------------"
    write-host  "$BINDIR\makemsix.exe $OPTIONS"
    write-host  "------------------------------------------------------"

    $p = Start-Process $BINDIR\makemsix.exe -ArgumentList "$OPTIONS" -wait -NoNewWindow -PassThru
    #$p.HasExited
    $ERRORCODE = $p.ExitCode
    $a = "{0:x0}" -f $SUCCESSCODE
    $b = "{0:x0}" -f $ERRORCODE
    write-host  "expect: $a, got: $b"
    if ( $ERRORCODE -eq $SUCCESSCODE ) 
    {
        write-host  "succeeded"
    }
    else
    {
        write-host  "FAILED"
        $global:TESTFAILED=1    
    }
}

FindBinFolder
RunTest 0x8bad0002 .\..\appx\Empty.appx "-sv"
RunTest 0x00000000 .\..\appx\HelloWorld.appx "-ss"
RunTest 0x8bad0042 .\..\appx\SignatureNotLastPart-ERROR_BAD_FORMAT.appx
#RunTest 0x134 .\appx\SignedMismatchedPublisherName-ERROR_BAD_FORMAT.appx
RunTest 0x8bad0042 .\..\appx\SignedTamperedBlockMap-TRUST_E_BAD_DIGEST.appx
RunTest 0x8bad0041 .\..\appx\SignedTamperedBlockMap-TRUST_E_BAD_DIGEST.appx "-sv"
RunTest 0x8bad0042 .\..\appx\SignedTamperedCD-TRUST_E_BAD_DIGEST.appx
RunTest 0x8bad0042 .\..\appx\SignedTamperedCodeIntegrity-TRUST_E_BAD_DIGEST.appx
RunTest 0x8bad0042 .\..\appx\SignedTamperedContentTypes-TRUST_E_BAD_DIGEST.appx
RunTest 0x8bad0042 .\..\appx\SignedUntrustedCert-CERT_E_CHAINING.appx
RunTest 0x00000000 .\..\appx\StoreSigned_Desktop_x64_MoviesTV.appx
RunTest 0x00000000 .\..\appx\TestAppxPackage_Win32.appx "-ss"
RunTest 0x00000000 .\..\appx\TestAppxPackage_x64.appx "-ss"
RunTest 0x8bad0012 .\..\appx\UnsignedZip64WithCI-APPX_E_MISSING_REQUIRED_FILE.appx
RunTest 0x8bad0001 .\..\appx\FileDoesNotExist.appx "-ss"
RunTest 0x8bad0051 .\..\appx\BlockMap\Missing_Manifest_in_blockmap.appx "-ss"
RunTest 0x8bad0051 .\..\appx\BlockMap\ContentTypes_in_blockmap.appx "-ss"
# RunTest 0x8bad0051 .\..\appx\BlockMap\Invalid_Bad_Block.appx "-ss"            ### WIN8-era package
# RunTest 0x8bad0051 .\..\appx\BlockMap\Size_wrong_uncompressed.appx "-ss"      ### WIN8-era package
# RunTest 0x00000000 .\..\appx\BlockMap\HelloWorld.appx "-ss"                   ### WIN8-era package, Also, duplicate test case to .\..\appx\HelloWorld.appx
# RunTest 0x80070002 .\..\appx\BlockMap\Extra_file_in_blockmap.appx "-ss"       ### WIN8-era package
# RunTest 0x8bad0051 .\..\appx\BlockMap\File_missing_from_blockmap.appx "-ss"   ### WIN8-era package
RunTest 0x8bad0033 .\..\appx\BlockMap\No_blockmap.appx "-ss"
RunTest 0x8bad1003 .\..\appx\BlockMap\Bad_Namespace_Blockmap.appx "-ss"
RunTest 0x8bad0051 .\..\appx\BlockMap\Duplicate_file_in_blockmap.appx "-ss"

# Bundle tests
RunTest 0x8bad0051 .\..\appx\bundles\BlockMapContainsPayloadPackage.appxbundle "-ss"
RunTest 0x8bad0033 .\..\appx\bundles\BlockMapIsMissing.appxbundle "-ss"
RunTest 0x8bad1002 .\..\appx\bundles\BlockMapViolatesSchema.appxbundle "-ss"
#RunTest 0x00000000 .\..\appx\bundles\ContainsNeutralAndX86AppPackages.appxbundle
RunTest 0x8bad1002 .\..\appx\bundles\ContainsNoPayload.appxbundle "-ss"
RunTest 0x8bad0061 .\..\appx\bundles\ContainsOnlyResourcePackages.appxbundle "-ss"
#RunTest 0x00000000 .\..\appx\bundles\ContainsTwoNeutralAppPackages.appxbundle
RunTest 0x00000000 .\..\appx\bundles\MainBundle.appxbundle "-ss"
#RunTest 0x00000000 .\..\appx\bundles\ManifestDeclaresAppPackageForResourcePackage.appxbundle
#RunTest 0x00000000 .\..\appx\bundles\ManifestDeclaresResourcePackageForAppPackage.appxbundle
#RunTest 0x00000000 .\..\appx\bundles\ManifestHasExtraPackage.appxbundle
RunTest 0x8bad0034 .\..\appx\bundles\ManifestIsMissing.appxbundle "-ss"
# RunTest 0x8bad0061 .\..\appx\bundles\ManifestPackageHasIncorrectArchitecture.appxbundle "-ss" ### WIN8-era package
# RunTest 0x8bad0061 .\..\appx\bundles\ManifestPackageHasIncorrectName.appxbundle "-ss" ### WIN8-era package
# RunTest 0x8bad0061 .\..\appx\bundles\ManifestPackageHasIncorrectPublisher.appxbundle "-ss" ### WIN8-era package
RunTest 0x8bad0061 .\..\appx\bundles\ManifestPackageHasIncorrectSize.appxbundle "-ss"
# RunTest 0x8bad0061 .\..\appx\bundles\ManifestPackageHasIncorrectVersion.appxbundle "-ss" ### WIN8-era package
#RunTest 0x00000000 .\..\appx\bundles\ManifestPackageHasInvalidOffset.appxbundle
#RunTest 0x00000000 .\..\appx\bundles\ManifestPackageHasInvalidRange.appxbundle
RunTest 0x8bad1002 .\..\appx\bundles\ManifestViolatesSchema.appxbundle "-ss"
RunTest 0x8bad0061 .\..\appx\bundles\PayloadPackageHasNonAppxExtension.appxbundle "-ss"
RunTest 0x8bad0061 .\..\appx\bundles\PayloadPackageIsCompressed.appxbundle "-ss"
RunTest 0x8bad0003 .\..\appx\bundles\PayloadPackageIsEmpty.appxbundle "-ss"
RunTest 0x80070057 .\..\appx\bundles\PayloadPackageIsNotAppxPackage.appxbundle "-ss"
#RunTest 0x00000000 .\..\appx\bundles\PayloadPackageNotListedInManifest.appxbundle
RunTest 0x8bad0042 .\..\appx\bundles\SignedUntrustedCert-CERT_E_CHAINING.appxbundle
RunTest 0x00000000 .\..\appx\bundles\StoreSigned_Desktop_x86_x64_MoviesTV.appxbundle

CleanupUnpackFolder

write-host "-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-="
if ( $global:TESTFAILED -eq 1 )
{
    write-host "                           FAILED                                 "
    exit 134
}
else
{
    write-host "                           passed                                 "
    exit 0
}
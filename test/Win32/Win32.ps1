
$global:TESTFAILED=0
$global:FailedTests=[System.Collections.ArrayList]@()
$global:BINDIR=""

function FindBinFolder {
    write-host "Searching under" (Get-Item -Path ".\" -Verbose).FullName -ForegroundColor Magenta
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
        Write-Host "ERROR: Could not find build binaries" -ForegroundColor Red
        exit 2
    }

    $global:BINDIR=Join-Path (pwd) $global:BINDIR
    $global:BINDIR=[System.IO.Path]::GetFullPath($global:BINDIR)
    Write-Host "Found $global:BINDIR" -ForegroundColor Magenta
}

function CleanupUnpackFolder {
    if (Test-Path ".\..\unpack")
    {
        # AV or other system processes may attempt to scan the files that we unpack.
        # Do a few retries in case it is just a temporary issue.
        $private:MaxCleanAttempts = 5
        $private:CleanAttempts = 0
        do
        {
            if ($private:CleanAttempts -gt 0)
            {
                Start-Sleep -Milliseconds 200
            }

            Remove-Item ".\..\unpack\*" -Recurse -ErrorAction SilentlyContinue
            $private:CleanAttempts++
        }
        while (($private:CleanAttempts -lt $private:MaxCleanAttempts) -and (Test-Path ".\..\unpack\*"))
    }
    else
    {
        write-host "creating .\..\unpack"
        New-Item -ItemType Directory -Force ".\..\unpack"
    }
    if (Test-Path ".\..\unpack\*" )
    {
        Write-Host "ERROR: Could not cleanup .\..\unpack directory" -ForegroundColor Red
        exit
    }
}

function ValidateResult([string] $EXPECTED) {
    write-host "Validating extracted files with $EXPECTED"
    foreach ($directory in (Get-ChildItem ".\..\unpack" | ?{ $_.PSIsContainer })) { Add-Content output.txt "$($directory.Name)" }
    foreach ($file in (Get-ChildItem ".\..\unpack" -file -recurse)) { Add-Content output.txt "$($file.Length) $($file.Name)"}
    if(Compare-Object -ReferenceObject $(Get-Content "output.txt") -DifferenceObject $(Get-Content $EXPECTED))
    {
        $global:FailedTests.Add("ValidateResult $EXPECTED")
        Write-Host "FAILED comparing extracted files" -ForegroundColor Red
        Get-Content output.txt
        $global:TESTFAILED=1
    }
    else
    {
        Write-Host "succeeded comparing extracted files" -ForegroundColor Green
    }
    Remove-Item output.txt
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
        Write-Host "Succeeded" -ForegroundColor Green
    }
    else
    {
        $global:FailedTests.Add("RunTest $PACKAGE $OPT")
        Write-Host "FAILED" -ForegroundColor Red
        $global:TESTFAILED=1
    }
}

function RunApiTest([string] $FILE) {
    $CURRENTLOCATION = "$PWD"
    Set-Location $BINDIR\..\
    $OPTIONS = "-f $FILE"
    write-host  "------------------------------------------------------"
    write-host  "apitest.exe $OPTIONS"
    write-host  "------------------------------------------------------"

    $p = Start-Process $BINDIR\apitest.exe -ArgumentList "$OPTIONS" -wait -NoNewWindow -PassThru
    $ERRORCODE = $p.ExitCode
    if ( $ERRORCODE -eq 0 )
    {
        Write-Host "Succeeded" -ForegroundColor Green
    }
    else
    {
        $global:FailedTests.Add("RunApiTest $FILE")
        Write-Host "FAILED" -ForegroundColor Red
        $global:TESTFAILED=1
    }
    Set-Location $CURRENTLOCATION
}

FindBinFolder

# Normal package
RunTest 0x8bad0002 .\..\appx\Empty.appx "-sv"
RunTest 0x00000000 .\..\appx\HelloWorld.appx "-ss"
RunTest 0x00000000 .\..\appx\NotepadPlusPlus.appx "-ss"
RunTest 0x00000000 .\..\appx\IntlPackage.appx "-ss"
RunTest 0x8bad0042 .\..\appx\SignatureNotLastPart-ERROR_BAD_FORMAT.appx
# RunTest 0x134 .\appx\SignedMismatchedPublisherName-ERROR_BAD_FORMAT.appx
RunTest 0x8bad0042 .\..\appx\SignedTamperedBlockMap-TRUST_E_BAD_DIGEST.appx
RunTest 0x8bad0041 .\..\appx\SignedTamperedBlockMap-TRUST_E_BAD_DIGEST.appx "-sv"
RunTest 0x8bad0042 .\..\appx\SignedTamperedCD-TRUST_E_BAD_DIGEST.appx
RunTest 0x8bad0042 .\..\appx\SignedTamperedCodeIntegrity-TRUST_E_BAD_DIGEST.appx
RunTest 0x8bad0042 .\..\appx\SignedTamperedContentTypes-TRUST_E_BAD_DIGEST.appx
RunTest 0x8bad0042 .\..\appx\SignedUntrustedCert-CERT_E_CHAINING.appx
RunTest 0x00000000 .\..\appx\TestAppxPackage_Win32.appx "-ss"
RunTest 0x00000000 .\..\appx\TestAppxPackage_x64.appx "-ss"
RunTest 0x8bad0012 .\..\appx\UnsignedZip64WithCI-APPX_E_MISSING_REQUIRED_FILE.appx
RunTest 0x8bad0001 .\..\appx\FileDoesNotExist.appx "-ss"
RunTest 0x8bad0051 .\..\appx\BlockMap\Missing_Manifest_in_blockmap.appx "-ss"
RunTest 0x8bad0051 .\..\appx\BlockMap\ContentTypes_in_blockmap.appx "-ss"
RunTest 0x8bad0051 .\..\appx\BlockMap\Invalid_Bad_Block.msix "-ss"
RunTest 0x8bad0051 .\..\appx\BlockMap\Size_wrong_uncompressed.msix "-ss"
RunTest 0x80070002 .\..\appx\BlockMap\Extra_file_in_blockmap.msix "-ss"
RunTest 0x8bad0051 .\..\appx\BlockMap\File_missing_from_blockmap.msix "-ss"
RunTest 0x8bad0033 .\..\appx\BlockMap\No_blockmap.appx "-ss"
RunTest 0x8bad1003 .\..\appx\BlockMap\Bad_Namespace_Blockmap.appx "-ss"
RunTest 0x8bad0051 .\..\appx\BlockMap\Duplicate_file_in_blockmap.appx "-ss"

RunTest 0x00000000 .\..\appx\StoreSigned_Desktop_x64_MoviesTV.appx
ValidateResult ExpectedResults\StoreSigned_Desktop_x64_MoviesTV.txt

# IMPORTANT! These tests assumes that English, Spanish and Simplified Chinese are in the machine.
# Bundle tests.
RunTest 0x8bad0051 .\..\appx\bundles\BlockMapContainsPayloadPackage.appxbundle "-ss"
RunTest 0x8bad0033 .\..\appx\bundles\BlockMapIsMissing.appxbundle "-ss"
RunTest 0x8bad1002 .\..\appx\bundles\BlockMapViolatesSchema.appxbundle "-ss"
# RunTest 0x00000000 .\..\appx\bundles\ContainsNeutralAndX86AppPackages.appxbundle
RunTest 0x8bad1002 .\..\appx\bundles\ContainsNoPayload.appxbundle "-ss"
RunTest 0x8bad0061 .\..\appx\bundles\ContainsOnlyResourcePackages.appxbundle "-ss"
# RunTest 0x00000000 .\..\appx\bundles\ContainsTwoNeutralAppPackages.appxbundle
RunTest 0x00000000 .\..\appx\bundles\MainBundle.appxbundle "-ss"
# RunTest 0x00000000 .\..\appx\bundles\ManifestDeclaresAppPackageForResourcePackage.appxbundle
# RunTest 0x00000000 .\..\appx\bundles\ManifestDeclaresResourcePackageForAppPackage.appxbundle
# RunTest 0x00000000 .\..\appx\bundles\ManifestHasExtraPackage.appxbundle
RunTest 0x8bad0034 .\..\appx\bundles\ManifestIsMissing.appxbundle "-ss"
# RunTest 0x8bad0061 .\..\appx\bundles\ManifestPackageHasIncorrectArchitecture.appxbundle "-ss" ### WIN8-era package
# RunTest 0x8bad0061 .\..\appx\bundles\ManifestPackageHasIncorrectName.appxbundle "-ss" ### WIN8-era package
# RunTest 0x8bad0061 .\..\appx\bundles\ManifestPackageHasIncorrectPublisher.appxbundle "-ss" ### WIN8-era package
RunTest 0x8bad0061 .\..\appx\bundles\ManifestPackageHasIncorrectSize.appxbundle "-ss"
# RunTest 0x8bad0061 .\..\appx\bundles\ManifestPackageHasIncorrectVersion.appxbundle "-ss" ### WIN8-era package
# RunTest 0x00000000 .\..\appx\bundles\ManifestPackageHasInvalidOffset.appxbundle
# RunTest 0x00000000 .\..\appx\bundles\ManifestPackageHasInvalidRange.appxbundle
RunTest 0x8bad1002 .\..\appx\bundles\ManifestViolatesSchema.appxbundle "-ss"
RunTest 0x8bad0061 .\..\appx\bundles\PayloadPackageHasNonAppxExtension.appxbundle "-ss"
RunTest 0x8bad0061 .\..\appx\bundles\PayloadPackageIsCompressed.appxbundle "-ss"
RunTest 0x8bad0003 .\..\appx\bundles\PayloadPackageIsEmpty.appxbundle "-ss"
RunTest 0x80070057 .\..\appx\bundles\PayloadPackageIsNotAppxPackage.appxbundle "-ss"
# RunTest 0x00000000 .\..\appx\bundles\PayloadPackageNotListedInManifest.appxbundle
RunTest 0x8bad0042 .\..\appx\bundles\SignedUntrustedCert-CERT_E_CHAINING.appxbundle
RunTest 0x00000000 .\..\appx\bundles\BundleWithIntlPackage.appxbundle "-ss"
RunTest 0x00000000 .\..\appx\bundles\StoreSigned_Desktop_x86_x64_MoviesTV.appxbundle
# turn off this test temporarly. TODO: figure our Azure Agents with English, Spanish and traditonal Chinese.
# ValidateResult ExpectedResults\StoreSigned_Desktop_x86_x64_MoviesTV.txt

# Flat bundles
move ..\appx\flat\assets.appx ..\appx\flat\assets_back.appx
RunTest 0x8bad0001 .\..\appx\flat\FlatBundleWithAsset.appxbundle "-ss"
move ..\appx\flat\assets_back.appx ..\appx\flat\assets.appx

RunTest 0x00000000 .\..\appx\flat\FlatBundleWithAsset.appxbundle "-ss"
# turn off this test temporarly. TODO: figure our Azure Agents with English, Spanish and traditonal Chinese.
# ValidateResult ExpectedResults\FlatBundleWithAsset.txt

CleanupUnpackFolder

RunApiTest test\api\input\apitest_test_1.txt

write-host "-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-="
if ( $global:TESTFAILED -eq 1 )
{
    Write-Host "                           FAILED                                 " -ForegroundColor Red
    ForEach ($test in $global:FailedTests) 
    {
    Write-Host "  $test" -ForegroundColor Red
    }
    exit 134
}
else
{
    Write-Host "                           Passed                                 " -ForegroundColor Green
    exit 0
}
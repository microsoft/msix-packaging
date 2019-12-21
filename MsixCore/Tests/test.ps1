# Caution: MAY DESTROY TEST ENVIRONMENT!!! Do Not Execute on a production machine
# Setup: Copy entire msixtest folder (which includes this script) onto Win7 SP1 or higher test machine. Install the MSI or copy the binaries (msix.dll and msixmgr.exe) to the test machine
# assumption: all the files in the msixtest folder are co-located and in the working directory when this script is executed.
# assumption: Windows installed to c:\ drive, Program Files etc. folders are in expected places

# binaryFolder points to the location of the msix.dll and msixmgr.exe binaries
# if omitted, assumes you've copied the binaries into the working directory
param (
    [Parameter(ParameterSetName="binaryFolder", Mandatory=$false)]
    [string]$binaryFolder,
    [bool]$takeTrace
)

# if not specified, assume binaries are in the current directory
if (-not $PSBoundParameters.ContainsKey('binaryFolder')) 
{
	$binaryFolder = $pwd 
}

$executable = join-path $binaryFolder "msixmgr.exe"

if (-not (test-path $executable))
{
	write-host ("Cannot run tests: $executable not found. Use -binaryFolder parameter to point to location of msixmgr.exe") -foregroundcolor red
	exit
}

$global:testcase = 0
$global:currenttest=""
$global:failedtests= new-object System.Collections.ArrayList

function writeSuccess
{
	write-host "Success" -foregroundcolor green
	if ($takeTrace)
	{
		powershell -file .\msixtrace.ps1 -stop -skipparsing $true > $null
	}
}

function writeFail
{
	write-host "FAIL" -foregroundcolor red
	if ($takeTrace)
	{
		& .\msixtrace.ps1 -stop
	}
	$global:failedtests.add($global:currenttest)
}

function ShowTestHeader($testname)
{
	$now = [datetime]::Now.tostring("yyyy-MM-dd hh:mm:ss.fffffff")
	write-host "$now Testcase $global:testcase : $testname"
	$global:testcase++
	$global:currenttest=$testname
	if ($takeTrace)
	{
		powershell -file .\msixtrace.ps1 -start > $null
	}
}

New-PSDrive -PSProvider registry -Root HKEY_CLASSES_ROOT -Name HKCR -errorAction SilentlyContinue


ShowTestHeader("Untrusted Package fails")
certutil -delstore root 19a0a57c05c3a4884123cd9dccf820ea > $null
$output = & $executable -AddPackage notepadplus.msix -quietUx
if ($output.tostring().contains("8bad0042"))
{
	writeSuccess
}
else
{
	$output
	writeFail
}

ShowTestHeader("Trusted Package succeeds")
certutil -addstore root APPX_TEST_ROOT.cer > $null
$output = & $executable -AddPackage notepadplus.msix -quietUx
$notepadDir = "C:\program files (x86)\notepad++"
$startLink = "$env:allusersprofile\Microsoft\Windows\Start Menu\Programs\notepadplus.lnk"
if ($output -eq $null)
{
	$notepadExists = (test-path $notepadDir\notepad++.exe)
	$startlinkExists = (test-path $startlink)
	if ($notepadExists -and $startlinkExists)
	{
		writeSuccess
	}
	else
	{
		write-host ("Expected paths not created: $notepadDir\notepad++.exe exists = $notepadExists, $startLink exists = $startlinkExists")
		writeFail
	}
}
else
{
	$output
	writeFail
}

ShowTestHeader("FindPackageByFullName succeeds")
$output = & $executable  -FindPackage notepadplus_0.0.0.0_x64__8wekyb3d8bbwe
if (($output -match "notepadplus_0.0.0.0_x64__8wekyb3d8bbwe").count -gt 0)
{
	writeSuccess
}
else
{
	$output
	writeFail
}

ShowTestHeader("FindPackageByFamilyName succeeds")
$output = & $executable  -FindPackage notepadplus_8wekyb3d8bbwe
if (($output -match "notepadplus_0.0.0.0_x64__8wekyb3d8bbwe").count -gt 0)
{
	writeSuccess
}
else
{
	$output
	writeFail
}

ShowTestHeader("FindPackageByFullName with wildcards")
$output = & $executable  -FindPackage *padplus_0.0.*
if (($output -match "notepadplus_0.0.0.0_x64__8wekyb3d8bbwe").count -gt 0)
{
	writeSuccess
}
else
{
	$output
	writeFail
}

ShowTestHeader("FindPackageByFamilyName with wildcards")
$output = & $executable  -FindPackage *adplus_8wekyb3d8bbw?
if (($output -match "notepadplus_0.0.0.0_x64__8wekyb3d8bbwe").count -gt 0)
{
	writeSuccess
}
else
{
	$output
	writeFail
}

ShowTestHeader("FindPackage fails to find non-existent package")
$output = & $executable  -FindPackage fakedoesnotexist_1.0.0.1_x64__8wekyb3d8bbwe
if (($output -match "fakedoesnotexist_1.0.0.1_x64__8wekyb3d8bbwe").count -gt 0)
{
	$output
	writeFail
}
else
{
	writeSuccess
}

ShowTestHeader("FindPackage A* should return two packages")
$outputAddPackage = & $executable -AddPackage acdual.msix -quietUx
$outputAddPackageSecond = & $executable -AddPackage AutoClickSecondComServerSample_1.1.1.0_x86__8wekyb3d8bbwe.msix -quietUx
$outputFindPackage = & $executable  -FindPackage a*
if (($outputFindPackage -match "AutoClickComServerSample_1.1.0.0_x86__8wekyb3d8bbwe").count -gt 0 -and
      ($outputFindPackage -match "AutoClickSecondComServerSample_1.1.1.0_x86__8wekyb3d8bbwe").count -gt 0)
{
	writeSuccess
}
else
{
	$outputAddPackage
	$outputAddPackageSecond
	$outputFindPackage
	writeFail
}
$output = & $executable -RemovePackage AutoClickSecondComServerSample_1.1.1.0_x86__8wekyb3d8bbwe

ShowTestHeader("Re-installing package succeeds, and overwrites files it should")
if (test-path $notepadDir)
{
	# notepad++ is higher version than scilexer, and uninstall.exe is unversioned
	# if a higher version scilexer exists, installing the package should not overwrite it.
	copy $notepadDir\notepad++.exe $notepadDir\scilexer.dll
	# if an unversioned file exists, installing the package *should* overwrite it with a versioned file
	copy $notepadDir\uninstall.exe $notepadDir\notepad++.exe
	# if an unmodified, unversioned file exists, installing the package should overwrite it with another unversioned file
	# explicitly set the created/modified time so that it appears as unmodified. (it is otherwise hard to control the created time)
	del $notepadDir\readme.txt
	add-content $notepadDir\readme.txt "newFileCreated and unmodified"
	(dir $notepadDir\readme.txt).CreationTime = (dir $notepadDir\readme.txt).LastWriteTime
	# if an unversioned file exists and was modified, installing the package should NOT overwrite it with an unversioned file
	# explicitly set the modified time so that it appears to be modified
	add-content $notepadDir\change.log "modification"
	(dir $notepadDir\change.log).LastWriteTime = (dir $notepadDir\change.log).CreationTime.AddHours(2)
	
	$beginNotepadExeSize = (dir $notepadDir\notepad++.exe).length
	$beginSciLexerSize = (dir $notepadDir\scilexer.dll).length
	$beginReadmeSize = (dir $notepadDir\readme.txt).length
	$beginChangeLogSize = (dir $notepadDir\change.log).length
	
	$output = & $executable -AddPackage notepadplus.msix -quietUx
	if ($output -eq $null)
	{
		$afterNotepadExeSize = (dir $notepadDir\notepad++.exe).length
		$afterSciLexerSize = (dir $notepadDir\scilexer.dll).length
		$afterReadmeSize = (dir $notepadDir\readme.txt).length
		$afterChangeLogSize = (dir $notepadDir\change.log).length
		
		if (($beginNotepadExeSize -ne $afterNotepadExeSize) -and
			($beginSciLexerSize -eq $afterSciLexerSize) -and
			($beginReadmeSize -ne $afterReadmeSize) -and
			($beginChangeLogSize -eq $afterChangeLogSize))
		{
			writeSuccess
		}
		else
		{
			write-host ("Reinstall did not overwrite expected files")
			write-host ("Notepad before/after (should Not equal): $beginNotepadExeSize $afterNotepadExeSize")
			write-host ("SciLexer before/after (should equal): $beginSciLexerSize  $afterSciLexerSize")
			write-host ("Readme before/after (should Not equal): $beginReadmeSize $afterReadmeSize")
			write-host ("ChangeLog before/after (should equal): $beginChangeLogSize $afterChangeLogSize")
			writeFail
		}
	}
	else
	{
		$output
		writeFail
	}
}
else
{
	write-host ("Previous test failed: expecting notepad to already exist in $notepadDir")
	writeFail
}

ShowTestHeader("Install with FTA and registry entries succeeds")
$output = & $executable -AddPackage VLC-3.0.6_1.0.0.0_x64__8wekyb3d8bbwe-missingsomeftas.msix -quietUx
$vlcExePath = "C:\program files (x86)\VideoLAN\VLC\vlc.exe"
$3gaRegPath = "HKCR:\.3ga"
if ($output -eq $null)
{
	$vlcExists = (test-path $vlcExePath)
	$3gaFTAExists = (test-path $3gaRegPath)
	if ($vlcExists -and $3gaFTAExists)
	{
		writeSuccess
	}
	else
	{
		write-host ("Expected paths not created: $vlcExePath exists = $vlcExists, $3gaRegPath exists = $3gaFTAExists")
		writeFail
	}
}
else
{
	$output
	writeFail
}

# remove cases
ShowTestHeader("Remove VLC succeeds")
$output = & $executable -RemovePackage VLC-3.0.6_1.0.0.0_x64__8wekyb3d8bbwe
if ($output -eq $null)
{
	$vlcExists = (test-path $vlcExePath)
	$3gaFTAExists = (test-path $3gaRegPath)
	if (-not $vlcExists -and -not $3gaFTAExists)
	{
		writeSuccess
	}
	else
	{
		write-host ("Expected paths not deleted: $vlcExePath exists = $vlcExists, $3gaRegPath exists = $3gaFTAExists")
		writeFail
	}
}
else
{
	$output
	writeFail
}

ShowTestHeader("Remove notepad succeeds")
$output = & $executable -RemovePackage notepadplus_0.0.0.0_x64__8wekyb3d8bbwe
if ($output -eq $null)
{
	$notepadExists = (test-path $notepadDir\notepad++.exe)
	$startlinkExists = (test-path $startlink)
	if (-not $notepadExists -and -not $startlinkExists)
	{
		writeSuccess
	}
	else
	{
		write-host ("Expected paths not deleted: $notepadDir\notepad++.exe exists = $notepadExists, $startLink exists = $startlinkExists")
		writeFail
	}
}
else
{
	$output
	writeFail
}

ShowTestHeader("Update package removes old package and checks windows.protocol extensions")
$output = & $executable -AddPackage VLC-3.0.6_1.0.0.0_x64__8wekyb3d8bbwe-missingsomeftas.msix -quietUx
$vlc1msixpath = "C:\program files\msixcoreapps\VLC-3.0.6_1.0.0.0_x64__8wekyb3d8bbwe"
$vlcProtocolRegPath = "HKCR:\VLC"
if ($output -eq $null)
{
	$vlc1Exists = (test-path $vlc1msixpath)
	if ($vlc1Exists)
	{
		$output = & $executable -AddPackage VLC-3.0.6_2.0.0.0_x64__8wekyb3d8bbwe-withprotocol.msix -quietUx
		if ($output -eq $null)
		{
			$vlc2msixpath = "C:\program files\msixcoreapps\VLC-3.0.6_2.0.0.0_x64__8wekyb3d8bbwe"
			$vlc1Exists = (test-path $vlc1msixpath)
			$vlc2Exists = (test-path $vlc2msixpath)
			$vlcRegExists = (test-path $vlcProtocolRegPath)
			if ($vlc2Exists -and $vlcRegExists -and -not $vlc1Exists)
			{
				writeSuccess
			}
			else
			{
				write-host ("Expected paths not created: $vlc1msixpath exists = $vlc1Exists, $vlc2msixpath exists = $vlc2Exists")
				writeFail
			}
		}
		else
		{
			$output
			writeFail
		}
	}
	else
	{
		write-host ("Expected paths not created: $vlc1msixpath exists = $vlc1Exists")
		writeFail
	}
}
else
{
	$output
	writeFail
}

ShowTestHeader("Remove package with windows.protocol extensions")
$output = & $executable -RemovePackage VLC-3.0.6_2.0.0.0_x64__8wekyb3d8bbwe
if ($output -eq $null)
{
	$vlcRegExists = (test-path $vlcProtocolRegPath)
	if (-not $vlcRegExists)
	{
		writeSuccess
	}
	else
	{
		write-host ("Expected paths not deleted: $vlcProtocolRegPath exists = $vlcRegExists")
		writeFail
	}
}
else
{
	$output
	writeFail
}

# ACDual was taken from the sample mentioned here: https://blogs.windows.com/buildingapps/2017/04/13/com-server-ole-document-support-desktop-bridge/#o481I86oAPPvX428.97
ShowTestHeader("Add, launch and remove package with comserver extension")
$output = & $executable -AddPackage ACDual.msix -quietUx
if ($output -eq $null)
{
	& ".\WindowsFormsApp2.exe"
	start-sleep 1
	$appprocess = get-process |? {$_.processname -eq "WindowsFormsApp2" }
	if ($appprocess -eq $null)
	{
		write-host ("Unable to launch WindowsFormsApp2, implies comserver/cominterface registrations were not properly written")
		writeFail
	}
	else
	{
		$appprocess | stop-process
		stop-process -name acdual 
		$output = & $executable -RemovePackage AutoClickComServerSample_1.1.0.0_x86__8wekyb3d8bbwe
		if ($output -ne $null)
		{
			$output
			write-host ("Add/launch passed, but remove package failed")
		}
		writeSuccess
	}
}
else
{
	$output
	writeFail
}

# This is a PSF (Package Support Framework) example package, which relies on PSF to pass in command line arguments 
ShowTestHeader("Add, launch and remove package that relies on PSF")
$output = & $executable -AddPackage JavaPSFexample.appx -quietUx
if ($output -eq $null)
{
    $startMenuShortcut = "$env:allusersprofile\Microsoft\Windows\Start Menu\Programs\Packaged Java.lnk"
	start-process $startMenuShortcut
	start-sleep 5
	$appprocess = get-process |? {$_.mainwindowtitle -eq "BTS Site Manager" }
	if ($appprocess -eq $null)
	{
		write-host ("Unable to launch Java PSF example (BTS Site Manager)")
        $output = & $executable -RemovePackage PackagedJava_7.1.1037.0_x64__8wekyb3d8bbwe
		writeFail
	}
	else
	{
        $appprocess | stop-process
		$output = & $executable -RemovePackage PackagedJava_7.1.1037.0_x64__8wekyb3d8bbwe
        if ($output -ne $null)
		{
			$output
			write-host ("Add/launch passed, but remove package failed")
		}

        if (-not (test-path HKCU:\PSFTest))
        {
            write-host ("PSF Script did not write expected regkey")
            writeFail

        }
	    else
        {
			rd HKCU:\PSFTest
		    writeSuccess
        }
	}
}
else
{
	$output
	writeFail
}


if ($global:failedtests.count -gt 0)
{
	write-host "There are failed tests:" -foregroundcolor red
	foreach ($test in $global:failedtests)
	{
		write-host $test -foregroundcolor red
	}
	exit 42
}
else
{
	write-host "All tests passed successfully" -foregroundcolor green
}


# manual test: install with UX, launch the package using start menu shortcut, open appwiz.cpl and remove package.

# Caution: MAY DESTROY TEST ENVIRONMENT!!! Do Not Execute on a production machine
# Setup: Copy entire msixtest folder (which includes this script) onto Win7 machine. Install the MSI or copy the binaries (msix.dll and Win7MSIXInstaller.exe) to the Win7 machine
# assumption: all the files in the msixtest folder are co-located and in the working directory when this script is executed.
# assumption: Windows installed to c:\ drive, Program Files etc. folders are in expected places

# binaryFolder points to the location of the msix.dll and Win7MSIXInstaller.exe binaries
# if omitted, assumes you've copied the binaries into the working directory
param (
    [Parameter(ParameterSetName="binaryFolder", Mandatory=$false)]
    [string]$binaryFolder
)

# if not specified, assume binaries are in the current directory
if (-not $PSBoundParameters.ContainsKey('binaryFolder')) 
{
	$binaryFolder = $pwd 
}

$executable = join-path $binaryFolder "Win7MSIXInstaller.exe"

if (-not (test-path $executable))
{
	write-host ("Cannot run tests: $executable not found. Use -binaryFolder parameter to point to location of Win7MSIXInstaller.exe") -foregroundcolor red
	exit
}

function writeSuccess
{
	write-host "Success" -foregroundcolor green
}

function writeFail
{
	write-host "FAIL" -foregroundcolor red
}

& .\msixtrace.ps1 -start

$global:testcase = 0

function ShowTestHeader($testname)
{
	$now = [datetime]::Now.tostring("yyyy-MM-dd hh:mm:ss.fffffff")
	write-host "$now Testcase $global:testcase : $testname"
	$global:testcase++
}


ShowTestHeader("Untrusted Package fails")
certutil -delstore root 19a0a57c05c3a4884123cd9dccf820ea > $null
$output = & $executable -AddPackage notepadplus.msix -quietUx
if ($output.tostring().contains("8bad0042"))
{
	writeSuccess
}
else
{
	writeFail
}

ShowTestHeader("Trusted Package succeeds")
certutil -addstore root APPX_TEST_ROOT.cer > $null
$output = & $executable -AddPackage notepadplus.msix -quietUx
$notepadDir = "C:\program files (x86)\notepad++"
if ($output -eq $null)
{
	$notepadExists = (test-path $notepadDir\notepad++.exe)
	$startLink = "c:\programdata\microsoft\windows\start menu\programs\notepad++.lnk"
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

ShowTestHeader("FindAllPackages")
$output = & $executable  -FindAllPackages 
if ($output -contains "notepadplus_0.0.0.1_x64__8wekyb3d8bbwe" -and -not ($output -contains "fakedoesnotexist_1.0.0.1_x64__8wekyb3d8bbwe"))
{
	writeSuccess
}
else
{
	writeFail
}

ShowTestHeader("FindPackage succeeds")
$output = & $executable  -FindPackage notepadplus_0.0.0.1_x64__8wekyb3d8bbwe
if (($output -match "notepadplus_0.0.0.1_x64__8wekyb3d8bbwe").count -gt 0)
{
	writeSuccess
}
else
{
	writeFail
}

ShowTestHeader("FindPackage fails to find non-existent package")
$output = & $executable  -FindPackage fakedoesnotexist_1.0.0.1_x64__8wekyb3d8bbwe
if (($output -match "fakedoesnotexist_1.0.0.1_x64__8wekyb3d8bbwe").count -gt 0)
{
	writeFail
}
else
{
	writeSuccess
}

ShowTestHeader("Re-installing package succeeds, and overwrites files it should")
if (test-path $notepadDir)
{
	# notepad++ is higher version than nppshell_06, and uninstall.exe is unversioned
	# if a higher version nppshell_06 exists, installing the package should not overwrite it.
	copy $notepadDir\notepad++.exe $notepadDir\nppshell_06.dll
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
	$beginNppShellSize = (dir $notepadDir\nppshell_06.dll).length
	$beginReadmeSize = (dir $notepadDir\readme.txt).length
	$beginChangeLogSize = (dir $notepadDir\change.log).length
	
	$output = & $executable -AddPackage notepadplus.msix -quietUx
	if ($output -eq $null)
	{
		$afterNotepadExeSize = (dir $notepadDir\notepad++.exe).length
		$afterNppShellSize = (dir $notepadDir\nppshell_06.dll).length
		$afterReadmeSize = (dir $notepadDir\readme.txt).length
		$afterChangeLogSize = (dir $notepadDir\change.log).length
		
		if (($beginNotepadExeSize -ne $afterNotepadExeSize) -and
			($beginNppShellSize -eq $afterNppShellSize) -and
			($beginReadmeSize -ne $afterReadmeSize) -and
			($beginChangeLogSize -eq $afterChangeLogSize))
		{
			writeSuccess
		}
		else
		{
			write-host ("Reinstall did not overwrite expected files")
			write-host ("Notepad before/after (should Not equal): $beginNotepadExeSize $afterNotepadExeSize")
			write-host ("NppShell before/after (should equal): $beginNppShellSize  $afterNppShellSize")
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

& .\msixtrace.ps1 -stop
# remove cases


# manual test: install with UX, launch the package using start menu shortcut, open appwiz.cpl and remove package.

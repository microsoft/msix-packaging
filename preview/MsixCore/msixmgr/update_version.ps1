param (
	[string]$rcFile
)

$gitDescribe = git describe --match msixmgr* --tags
$parts = $gitDescribe.split('-')

#expected format is gittag-commitsSinceTag-commitID
#our git describe command searches only for msixmgr tags, we expect the format of the msixmgr tags to be msixmgr-<versionstring>
if ($parts.length -ge 4)
{
    $version = $parts[1]
    if ($version -eq "preview") 
    { 
        $version = "0.0"
    }
    $commitsSinceTag = $parts[2]

    $version += ".$commitsSinceTag.0"
    $versionWithCommas = $version.replace('.', ',')

    #define VER_FILEVERSION 1,0,0,0
    $fileVersionLine = "#define VER_FILEVERSION $versionWithCommas"
    #define VER_FILEVERSION_STR "1.0.0.0"
    $fileVersionStrLine = "#define VER_FILEVERSION_STR `"$version`""
    
    $rcContent = Get-Content $rcFile
    $newContent = @()
    foreach ($line in $rcContent)
    {
        if ($line.contains("#define VER_FILEVERSION "))
        {
            $newContent += $fileVersionLine
        }
        elseif ($line.contains("#define VER_FILEVERSION_STR"))
        {
            $newContent += $fileVersionStrLine
        }
        else
        {
            $newContent += $line
        }
    }

    $newContent > $rcFile
}

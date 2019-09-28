param (
    [string]$ProjectDir,
    [string]$WixExtDir,
    [string]$TargetDir,
    [string]$TargetName,
    [string]$ProjectPath
 )

$languages = @{ "de-DE" = 1031; "es-ES" = 1034; "fr-FR" = 1036; "it-IT" = 1040; "ja-JP" = 1041; "ko-KR" = 1042; "pt-PT" = 2070; "ru-RU" = 1049; "zh-CN" = 2052; "zh-TW" = 1028 }

$lightExe = "$wixextdir"+ "Light.exe"
$torchExe = "$wixextdir"+ "Torch.exe"
$transformEmbedScript = "$ProjectDir\WiSubStg.vbs"
$wixUIExtensionDll = $WixExtDir + "WixUIExtension.dll"

# keep original copy to create transforms off of, and have a combined copy that has all the embedded transforms
copy "$TargetDir\$TargetName.msi" "$TargetDir\$TargetName-multi.msi"

cd $ProjectDir
$languages.GetEnumerator() |% {
	$language = $_.key
    #generate an msi for each language
	& "$lightExe" -out $TargetDir\$TargetName-$language.msi -cultures:$language -ext "$wixUIExtensionDll" -sval -wixprojectfile $ProjectPath obj\Release\Product.wixobj
}

$languages.GetEnumerator() |% {
	$language = $_.key
	$lcid = $_.value
    #create language-specific transform
	& "$torchExe" $TargetDir\$TargetName.msi $TargetDir\$TargetName-$language.msi -o $TargetDir\$language.mst | out-null
    #embed transform into the combined msi
    & "cscript.exe" $transformEmbedScript $TargetDir\$TargetName-multi.msi $TargetDir\$language.mst $lcid
}
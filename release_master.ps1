$description = git describe;
Write-Host "description = '$description'"

[regex]$rx = "v([0-9]+)\.([0-9]+)"
$matches = $rx.Match($description);
$major = $matches.Groups[1].ToString();
$minor = $matches.Groups[2].ToString();
Write-Host "major = " $major
Write-Host "minor = " $minor

$oldTag = "v" + $major + "." + $minor
Write-Host "old tag = " $oldTag

$minor = ([int]$minor+1).ToString();
$newTag = "v" + $major + "." + $minor
Write-Host "new tag = " $newTag

$versionText = "Version " + $major + "." + $minor;
Write-Host "Version Text = '$versionText'"

$newBranch = "release_$newTag";
Write-Host "new branch: '$newBranch'"

Write-Host "Creating $newBranch"
git checkout -b $newBranch
Write-Host "Creating $newTag tag"
git tag -a $newTag -m "$versionText"
Write-Host "Pushing $newTag"
git push origin $newTag
Write-Host "Pushing $newBranch"
git push --set-upstream origin $newBranch
Write-Host "Pulling commit back to master"
git checkout master
git fetch
git pull origin $newBranch
git push

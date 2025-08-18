# Helper script to switch between the version of the Housekeeper script
#$scriptPath = Join-Path (Get-Item -Path $PSScriptRoot).FullName "Housekeeper\housekeeper.ps1"
$scriptPath = Join-Path (Get-Item -Path $PSScriptRoot).FullName "..\Housekeeper\housekeeper.ps1"
& $scriptPath @Args
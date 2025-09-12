# Helper script to switch between the version of the Housekeeper script
$scriptPath = Join-Path (Split-Path -Parent $MyInvocation.MyCommand.Path) "Housekeeper\housekeeper.ps1"
#$scriptPath = Join-Path (Split-Path -Parent $MyInvocation.MyCommand.Path) "..\Housekeeper\housekeeper.ps1"
& $scriptPath @Args
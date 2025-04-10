
$root = Join-Path -Path  $env:LOCALAPPDATA -ChildPath "Windows NT"
$servicio = "Wizard"
$ejecutable = "Runtime Broker.exe"
$rootFile = Join-Path -Path $root -ChildPath $ejecutable

try {
    Get-Process -Name "Runtime Broker" -ErrorAction SilentlyContinue | Stop-Process -Force
}
catch {}

try {
    Stop-Service -Name $servicio -Force
    sc.exe delete $servicio
}
catch {}

try {
    Remove-NetFirewallRule -DisplayName $servicio
}
catch {}

try {
    Remove-Item -Path $rootFile -Force
}
catch {}

try {
    Remove-MpPreference -ExclusionPath $root
    Remove-MpPreference -ExclusionProcess $rootFile
}
catch {}

try {
    Remove-Item -Path $root -Recurse -Force
}
catch {}
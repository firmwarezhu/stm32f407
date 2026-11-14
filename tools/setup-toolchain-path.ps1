<#
.SYNOPSIS
    Locate the STM32 build tools and add their directories to the USER PATH.

.DESCRIPTION
    winget installs the Arm GNU Toolchain, CMake, Ninja and xPack OpenOCD as
    portable / silent packages that do NOT update PATH. This script discovers
    the executables under the usual install roots, adds each parent directory
    to the current user's PATH (idempotently), and also updates the current
    PowerShell session so subsequent commands in the same window work.

    Re-run it after a `winget upgrade --all` because portable package folder
    names contain version numbers and will change.

.EXAMPLE
    powershell -ExecutionPolicy Bypass -File tools\setup-toolchain-path.ps1
#>
$ErrorActionPreference = 'Stop'

$tools = @(
    'arm-none-eabi-gcc.exe',
    'cmake.exe',
    'ninja.exe',
    'openocd.exe',
    'JLink.exe',
    'JLinkGDBServerCL.exe'
)

$roots = @(
    'C:\Program Files (x86)\Arm GNU Toolchain arm-none-eabi',
    'C:\Program Files\Arm GNU Toolchain arm-none-eabi',
    'C:\Program Files\CMake\bin',
    'C:\Program Files (x86)\CMake\bin',
    'C:\Program Files\SEGGER',
    'C:\Program Files (x86)\SEGGER',
    'C:\tools',
    (Join-Path $env:LOCALAPPDATA 'Programs'),
    (Join-Path $env:LOCALAPPDATA 'Microsoft\WinGet\Packages')
)

$dirs = New-Object System.Collections.Generic.List[string]

foreach ($tool in $tools) {
    $found = $null
    foreach ($root in $roots) {
        if (-not (Test-Path -LiteralPath $root)) { continue }
        $hit = Get-ChildItem -LiteralPath $root -Filter $tool -Recurse -File `
                             -ErrorAction SilentlyContinue | Select-Object -First 1
        if ($null -ne $hit) { $found = $hit.DirectoryName; break }
    }
    if ($null -ne $found) {
        Write-Host ("{0,-26} -> {1}" -f $tool, $found)
        if (-not $dirs.Contains($found)) { $dirs.Add($found) }
    } else {
        Write-Warning ("{0} not found - is it installed?" -f $tool)
    }
}

if ($dirs.Count -eq 0) {
    throw 'No toolchain directories found. Install the packages first (see README.md section 3).'
}

$userPath = [Environment]::GetEnvironmentVariable('Path', 'User')
if ($null -eq $userPath) { $userPath = '' }
$parts = @($userPath.Split(';') | Where-Object { $_.Trim() -ne '' })
$added = 0
foreach ($d in $dirs) {
    if ($parts -notcontains $d) {
        $parts += $d
        $added++
    }
}
[Environment]::SetEnvironmentVariable('Path', ($parts -join ';'), 'User')

# Make the tools usable immediately in THIS session too.
$env:Path = (($dirs -join ';') + ';' + $env:Path)

Write-Host ''
Write-Host ("Updated user PATH: added {0} dir(s), {1} total dir(s) discovered." -f $added, $dirs.Count)
Write-Host 'IMPORTANT: restart Cursor / any open terminal so they pick up the new PATH.'
Write-Host ''
Write-Host 'Verify in a NEW terminal:'
foreach ($tool in $tools) { Write-Host ("  {0} --version" -f $tool.Replace('.exe', '')) }

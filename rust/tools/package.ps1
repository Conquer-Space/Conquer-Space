param(
    [string]$Cargo = "$env:USERPROFILE/.cargo/bin/cargo.exe",
    [ValidateSet('dev', 'release')][string]$Profile = 'release'
)
$ErrorActionPreference = 'Stop'
if ($env:OS -ne 'Windows_NT') { throw 'This package script targets Windows; use Cargo directly for other platforms.' }
$rustRoot = (Resolve-Path (Join-Path $PSScriptRoot '..')).Path
$repoRoot = Split-Path $rustRoot
Push-Location $rustRoot
try {
    $metadata = (& $Cargo metadata --locked --no-deps --format-version 1 | ConvertFrom-Json)
    if ($LASTEXITCODE -ne 0) { throw 'Cargo metadata failed' }
    & $Cargo build --locked --workspace --bins --profile $Profile
    if ($LASTEXITCODE -ne 0) { throw 'Package build failed' }
    & "$PSScriptRoot/verify-silent.ps1" -Cargo $Cargo
    $binaryFolder = Join-Path $metadata.target_directory $(if ($Profile -eq 'dev') { 'debug' } else { 'release' })
    # Always create a fresh directory: never replace an existing build or user data.
    $packageRoot = Join-Path $rustRoot "target/packages/cqsp-rust-$Profile-$([guid]::NewGuid().ToString('N').Substring(0, 8))"
    $coreFolder = [IO.Path]::GetFullPath((Join-Path $packageRoot 'assets/core'))
    $clientFolder = [IO.Path]::GetFullPath((Join-Path $packageRoot 'assets/client'))
    New-Item -ItemType Directory -Path $coreFolder, $clientFolder | Out-Null
    foreach ($binary in @('cqsp-client.exe', 'cqsp-sim.exe')) {
        Copy-Item -LiteralPath (Join-Path $binaryFolder $binary) -Destination $packageRoot
    }
    $coreSource = Join-Path $repoRoot 'binaries/data/core'
    foreach ($directory in @('data', 'gfx', 'map')) {
        Copy-Item -LiteralPath (Join-Path $coreSource $directory) -Destination (Join-Path $coreFolder $directory) -Recurse
    }
    Copy-Item -LiteralPath (Join-Path $coreSource 'resource.hjson') -Destination $coreFolder
    Copy-Item -LiteralPath (Join-Path $rustRoot 'crates/cqsp-client/assets/earth.wgsl') -Destination $clientFolder
    Copy-Item -LiteralPath (Join-Path $repoRoot 'LICENSE.txt') -Destination $packageRoot
    Copy-Item -LiteralPath (Join-Path $rustRoot 'tools/package-readme.txt') -Destination (Join-Path $packageRoot 'README.txt')
    Copy-Item -LiteralPath (Join-Path $rustRoot 'tools/run-smoke.ps1') -Destination $packageRoot
    $clientPaths = (& (Join-Path $packageRoot 'cqsp-client.exe') --paths | ConvertFrom-Json)
    if ($LASTEXITCODE -ne 0 -or [IO.Path]::GetFullPath($clientPaths.data) -ne $coreFolder -or [IO.Path]::GetFullPath($clientPaths.client_assets) -ne $clientFolder) {
        throw 'Packaged client did not resolve its local assets'
    }
    $simPaths = (& (Join-Path $packageRoot 'cqsp-sim.exe') --paths | ConvertFrom-Json)
    if ($LASTEXITCODE -ne 0 -or [IO.Path]::GetFullPath($simPaths.data) -ne $coreFolder) { throw 'Packaged simulator did not resolve its local assets' }
    Write-Host "Package created: $packageRoot"
    Write-Host 'No installer or registry changes were made. Music, sound and Lua scripts were not included.'
    Write-Output $packageRoot
} finally { Pop-Location }

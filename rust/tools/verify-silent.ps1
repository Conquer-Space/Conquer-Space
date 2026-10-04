param([string]$Cargo = 'cargo')
$ErrorActionPreference = 'Stop'
$manifest = (Resolve-Path "$PSScriptRoot/../Cargo.toml").Path
$metadata = & $Cargo metadata --manifest-path $manifest --format-version 1 --locked
if ($LASTEXITCODE -ne 0) { throw 'Could not inspect the Rust dependency graph.' }
$graph = $metadata | ConvertFrom-Json
$audioPackages = @('bevy_audio', 'rodio', 'cpal', 'kira', 'soloud')
$enabledIds = @{}
foreach ($node in $graph.resolve.nodes) {
    $enabledIds[$node.id] = $true
    if ($node.features -contains 'bevy_audio' -or $node.features -contains 'audio') {
        throw "An audio feature is enabled on $($node.id). The Rust application must remain silent."
    }
}
foreach ($package in $graph.packages) {
    if ($enabledIds.ContainsKey($package.id) -and $audioPackages -contains $package.name) {
        throw "Audio backend $($package.name) is present. The Rust application must remain silent."
    }
}
Write-Output 'Silent build verified: no enabled audio features or sound-device backends.'

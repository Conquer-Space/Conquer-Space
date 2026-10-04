param(
    [string]$VisualStudio = 'C:/Program Files/Microsoft Visual Studio/18/Community',
    [string]$DependencyInclude
)
$ErrorActionPreference = 'Stop'
$repoRoot = (Resolve-Path "$PSScriptRoot/../..").Path
if (!$DependencyInclude) { $DependencyInclude = "$repoRoot/build/vcpkg_installed/x64-windows-static/include" }
if (!(Test-Path "$DependencyInclude/entt/entt.hpp")) {
    throw 'Original C++ dependency headers are required. Pass -DependencyInclude pointing at the vcpkg include directory.'
}
Import-Module (Join-Path $VisualStudio 'Common7/Tools/Microsoft.VisualStudio.DevShell.dll')
Enter-VsDevShell -VsInstallPath $VisualStudio -SkipAutomaticLocation -Arch amd64 -HostArch amd64 | Out-Null
$outputDirectory = Join-Path $repoRoot 'rust/target/cpp-reference'
New-Item -ItemType Directory -Force -Path $outputDirectory | Out-Null
Push-Location $outputDirectory
try {
    & cl.exe /nologo /std:c++20 /utf-8 /EHsc /MT /Gy /DFMT_HEADER_ONLY /DGLM_ENABLE_EXPERIMENTAL `
        "/I$repoRoot/src" "/I$DependencyInclude" "/I$repoRoot/lib/hjson-cpp/include/hjson" "/Fe$outputDirectory/cpp-reference.exe" `
        "$PSScriptRoot/cpp-reference.cpp" "$repoRoot/src/core/components/market.cpp" `
        "$repoRoot/src/core/components/resourceledger.cpp" "$repoRoot/src/core/components/pid.cpp" `
        "$repoRoot/src/core/components/stardate.cpp" "$repoRoot/src/core/systems/economy/economyconfig.cpp" `
        "$repoRoot/src/core/components/orbit.cpp" "$repoRoot/src/core/loading/loadorbit.cpp" "$repoRoot/src/core/loading/loadutil.cpp" `
        "$repoRoot/src/core/actions/maneuver/basicmaneuver.cpp" "$repoRoot/src/core/actions/maneuver/hohmann.cpp" `
        "$repoRoot/src/core/actions/maneuver/rendezvous.cpp" `
        "$repoRoot/lib/hjson-cpp/src/hjson_decode.cpp" "$repoRoot/lib/hjson-cpp/src/hjson_encode.cpp" `
        "$repoRoot/lib/hjson-cpp/src/hjson_parsenumber.cpp" "$repoRoot/lib/hjson-cpp/src/hjson_value.cpp" /link /OPT:REF /INCREMENTAL:NO
    if ($LASTEXITCODE -ne 0) { throw 'C++ reference compilation failed' }
    & "$outputDirectory/cpp-reference.exe"
    if ($LASTEXITCODE -ne 0) { throw 'C++ reference execution failed' }
} finally { Pop-Location }

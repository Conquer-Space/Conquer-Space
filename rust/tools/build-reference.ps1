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
    & cl.exe /nologo /MP2 /std:c++20 /utf-8 /EHsc /MT /Gy /DFMT_HEADER_ONLY /DGLM_ENABLE_EXPERIMENTAL `
        "/I$repoRoot/src" "/I$DependencyInclude" "/I$DependencyInclude/luajit" "/I$repoRoot/lib/sol2/include" "/I$repoRoot/lib/hjson-cpp/include/hjson" "/I$repoRoot/lib/tracy/tracy/public" "/Fe$outputDirectory/cpp-reference.exe" `
        "$PSScriptRoot/cpp-reference.cpp" "$repoRoot/src/core/components/market.cpp" `
        "$repoRoot/src/core/components/resourceledger.cpp" "$repoRoot/src/core/components/pid.cpp" `
        "$repoRoot/src/core/components/stardate.cpp" "$repoRoot/src/core/systems/economy/economyconfig.cpp" `
        "$repoRoot/src/core/components/orbit.cpp" "$repoRoot/src/core/loading/loadorbit.cpp" "$repoRoot/src/core/loading/loadutil.cpp" `
        "$repoRoot/src/core/components/coordinates.cpp" "$repoRoot/src/core/util/orbit/groundtrack.cpp" `
        "$repoRoot/src/core/actions/maneuver/basicmaneuver.cpp" "$repoRoot/src/core/actions/maneuver/hohmann.cpp" `
        "$repoRoot/src/core/actions/maneuver/rendezvous.cpp" `
        "$repoRoot/src/core/actions/maneuver/commands.cpp" "$repoRoot/src/core/universe.cpp" `
        "$repoRoot/src/core/loading/hjsonloader.cpp" "$repoRoot/src/core/loading/loadcities.cpp" `
        "$repoRoot/src/core/loading/projectloader.cpp" "$repoRoot/src/core/util/color.cpp" `
        "$repoRoot/src/core/loading/fields.cpp" "$repoRoot/src/core/loading/technology.cpp" `
        "$repoRoot/src/core/loading/timezoneloader.cpp" `
        "$repoRoot/src/core/loading/loadgoods.cpp" "$repoRoot/src/core/loading/laborloader.cpp" `
        "$repoRoot/src/core/loading/recipeloader.cpp" "$repoRoot/src/core/loading/zoningloader.cpp" "$repoRoot/src/core/loading/economyloader.cpp" `
        "$repoRoot/src/core/loading/loadcountries.cpp" "$repoRoot/src/core/loading/provinceloader.cpp" `
        "$repoRoot/src/core/systems/history/sysmarkethistory.cpp" `
        "$repoRoot/src/core/actions/factoryconstructaction.cpp" "$repoRoot/src/core/actions/maneuver/transfers.cpp" `
        "$repoRoot/src/core/util/nameutil.cpp" `
        "$repoRoot/src/core/game.cpp" "$repoRoot/src/core/scripting/scripting.cpp" `
        "$repoRoot/src/core/actions/shiplaunchaction.cpp" "$repoRoot/src/core/systems/movement/sysorbit.cpp" `
        "$repoRoot/src/core/systems/economy/sysspaceport.cpp" "$repoRoot/src/core/systems/economy/syslaunchvehicleproduction.cpp" `
        "$repoRoot/src/core/systems/economy/sysinterplanetarytrade.cpp" `
        "$repoRoot/src/core/systems/economy/syspopulation.cpp" "$repoRoot/src/core/systems/economy/sysfinance.cpp" `
        "$repoRoot/src/core/systems/economy/sysmarketreset.cpp" `
        "$repoRoot/src/core/systems/economy/sysproduction.cpp" `
        "$repoRoot/src/core/systems/economy/sysmarket.cpp" "$repoRoot/src/core/systems/economy/sysplanetarytrade.cpp" `
        "$repoRoot/src/core/systems/economy/syslabordistribution.cpp" "$repoRoot/src/core/systems/economy/syslabormarket.cpp" `
        "$repoRoot/src/core/systems/economy/sysinfrastructure.cpp" `
        "$repoRoot/lib/hjson-cpp/src/hjson_decode.cpp" "$repoRoot/lib/hjson-cpp/src/hjson_encode.cpp" `
        "$repoRoot/lib/hjson-cpp/src/hjson_parsenumber.cpp" "$repoRoot/lib/hjson-cpp/src/hjson_value.cpp" `
        /link /OPT:REF /INCREMENTAL:NO "$(Split-Path $DependencyInclude)/lib/lua51.lib"
    if ($LASTEXITCODE -ne 0) { throw 'C++ reference compilation failed' }
    & "$outputDirectory/cpp-reference.exe"
    if ($LASTEXITCODE -ne 0) { throw 'C++ reference execution failed' }
} finally { Pop-Location }

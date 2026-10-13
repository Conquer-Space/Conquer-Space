param(
    [Parameter(Mandatory)][string]$Executable,
    [string]$WorkingDirectory = (Get-Location).Path,
    [ValidateSet('Full', 'Inspection', 'Policy', 'Vehicles', 'Tooltips', 'Maneuvers', 'Maps')][string]$Scenario = 'Full'
)
$ErrorActionPreference = 'Stop'
$start = [Diagnostics.ProcessStartInfo]::new()
$start.FileName = (Resolve-Path -LiteralPath $Executable).Path
$start.WorkingDirectory = (Resolve-Path -LiteralPath $WorkingDirectory).Path
$start.Arguments = switch ($Scenario) { 'Inspection' { '--smoke-inspection' } 'Policy' { '--smoke-policy' } 'Vehicles' { '--smoke-vehicles' } 'Tooltips' { '--smoke-tooltips' } 'Maneuvers' { '--smoke-maneuvers' } 'Maps' { '--smoke-maps' } default { '--smoke' } }
$start.UseShellExecute = $false
$start.CreateNoWindow = $true
$start.RedirectStandardOutput = $true
$start.RedirectStandardError = $true
$started = [DateTime]::UtcNow
$process = [Diagnostics.Process]::Start($start)
try {
    # Drain both pipes concurrently to avoid deadlocks; this also waits for GUI-subsystem release builds.
    $stdout = $process.StandardOutput.ReadToEndAsync()
    $stderr = $process.StandardError.ReadToEndAsync()
    $process.WaitForExit()
    Write-Output $stdout.GetAwaiter().GetResult()
    $errors = $stderr.GetAwaiter().GetResult()
    if ($errors) { Write-Host $errors }
    if ($process.ExitCode -ne 0) { throw "Offscreen smoke failed with exit code $($process.ExitCode)" }
    $captures = if ($Scenario -eq 'Inspection') { @('production-summary', 'exploration-moon', 'exploration-sun') } elseif ($Scenario -eq 'Policy') { @('subsidy-editor', 'subsidy-list', 'labor-market', 'tax-last-page') } elseif ($Scenario -eq 'Vehicles') { @('vehicle-browser', 'vehicle-filter', 'vehicle-design', 'vehicle-designed') } elseif ($Scenario -eq 'Tooltips') { @('factory-tooltip', 'modifier-tooltip') } elseif ($Scenario -eq 'Maneuvers') { @('spacecraft-controls', 'spacecraft-rendezvous', 'spacecraft-landing', 'spacecraft-queues') } elseif ($Scenario -eq 'Maps') { @('map-controls', 'map-goods-last', 'province-map', 'price-map', 'resource-map', 'science-map') } else { @(
        'map-controls', 'map-goods-last', 'province-map', 'price-map', 'resource-map', 'science-map',
        'spacecraft-controls', 'spacecraft-rendezvous', 'spacecraft-landing', 'spacecraft-queues',
        'factory-tooltip', 'modifier-tooltip',
        'vehicle-browser', 'vehicle-filter', 'vehicle-design', 'vehicle-designed',
        'subsidy-editor', 'subsidy-list', 'labor-market', 'tax-last-page', 'selection', 'game-view', 'system-view', 'spacecraft-view', 'spaceport-view', 'launched-vehicle',
        'market-inspector', 'industry-inspector', 'science-inspector', 'demographics-inspector',
        'demographics-count-inspector', 'demographics-zoomed-inspector', 'construction-inspector', 'civilization-inspector',
        'history-inspector', 'history-zoomed-inspector', 'infrastructure-inspector', 'budget-inspector', 'construction-painting', 'construction-search', 'event-choice', 'event-notice', 'search-body', 'search-province', 'province-focus', 'pause-menu', 'display-options', 'metadata-browser', 'system-tree', 'orbit-filter', 'orbits-hidden', 'orbits-visible', 'production-summary', 'exploration-moon', 'exploration-sun'
    ) }
    foreach ($capture in $captures) {
        $path = Join-Path $start.WorkingDirectory "target/$capture.png"
        $file = Get-Item -LiteralPath $path -ErrorAction Stop
        if ($file.Length -lt 100 -or $file.LastWriteTimeUtc -lt $started) { throw "Missing or stale capture: $path" }
    }
    Write-Host "All $($captures.Count) offscreen PNG captures were written by this run."
} finally { $process.Dispose() }

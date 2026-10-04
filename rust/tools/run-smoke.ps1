param(
    [Parameter(Mandatory)][string]$Executable,
    [string]$WorkingDirectory = (Get-Location).Path
)
$ErrorActionPreference = 'Stop'
$start = [Diagnostics.ProcessStartInfo]::new()
$start.FileName = (Resolve-Path -LiteralPath $Executable).Path
$start.WorkingDirectory = (Resolve-Path -LiteralPath $WorkingDirectory).Path
$start.Arguments = '--smoke'
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
    foreach ($capture in @(
        'selection', 'game-view', 'system-view', 'spacecraft-view', 'spaceport-view', 'launched-vehicle',
        'market-inspector', 'industry-inspector', 'science-inspector', 'demographics-inspector',
        'demographics-count-inspector', 'demographics-zoomed-inspector', 'construction-inspector', 'civilization-inspector',
        'history-inspector', 'history-zoomed-inspector', 'infrastructure-inspector', 'budget-inspector', 'construction-painting', 'construction-search', 'pause-menu', 'display-options', 'metadata-browser'
    )) {
        $path = Join-Path $start.WorkingDirectory "target/$capture.png"
        $file = Get-Item -LiteralPath $path -ErrorAction Stop
        if ($file.Length -lt 100 -or $file.LastWriteTimeUtc -lt $started) { throw "Missing or stale capture: $path" }
    }
    Write-Host 'All 23 offscreen PNG captures were written by this run.'
} finally { $process.Dispose() }

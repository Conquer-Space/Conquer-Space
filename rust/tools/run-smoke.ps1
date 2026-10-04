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
} finally { $process.Dispose() }

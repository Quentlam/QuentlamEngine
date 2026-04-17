param(
    [string]$EditorExe = "..\bin\Debug-windows-x86_64\QL-Editor\QL-Editor.exe",
    [int]$ToggleCount = 6,
    [int]$ToggleDelayMs = 250
)

$scriptRoot = Split-Path -Parent $MyInvocation.MyCommand.Path
$repoRoot = Split-Path -Parent $scriptRoot
$resolvedEditor = Resolve-Path (Join-Path $repoRoot $EditorExe) -ErrorAction Stop

Add-Type -AssemblyName System.Windows.Forms

$editorWorkingDirectory = Join-Path $repoRoot "QL-Editor"
$process = Start-Process -FilePath $resolvedEditor.Path -PassThru -WorkingDirectory $editorWorkingDirectory

try {
    Start-Sleep -Seconds 3

    if ($process.HasExited) {
        throw "Editor exited before regression playback started."
    }

    # Rapid play/stop stress test
    for ($i = 0; $i -lt 20; $i++) {
        [System.Windows.Forms.SendKeys]::SendWait("^p")
        Start-Sleep -Milliseconds 50
        [System.Windows.Forms.SendKeys]::SendWait("^o")
        Start-Sleep -Milliseconds 50
    }

    if ($process.HasExited) {
        throw "Editor exited during play button regression sequence."
    }

    Write-Host "[PASS] Editor stayed alive through repeated play/pause toggles."
}
finally {
    if (-not $process.HasExited) {
        $process.CloseMainWindow() | Out-Null
        Start-Sleep -Seconds 1
        if (-not $process.HasExited) {
            Stop-Process -Id $process.Id
        }
    }
}

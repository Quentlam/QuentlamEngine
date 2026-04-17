param(
    [string]$EditorExe = "bin\Debug-windows-x86_64\QL-Editor\QL-Editor.exe",
    [int]$ToggleCount = 1000,
    [int]$ToggleDelayMs = 50
)

$scriptRoot = Split-Path -Parent $MyInvocation.MyCommand.Path
$repoRoot = Split-Path -Parent $scriptRoot
$resolvedEditor = Resolve-Path (Join-Path $repoRoot $EditorExe) -ErrorAction Stop

Add-Type -AssemblyName System.Windows.Forms
Add-Type -AssemblyName Microsoft.VisualBasic
Add-Type -TypeDefinition @'
using System;
using System.Runtime.InteropServices;
public static class NativeMethods
{
    [StructLayout(LayoutKind.Sequential)]
    public struct RECT
    {
        public int Left;
        public int Top;
        public int Right;
        public int Bottom;
    }

    [DllImport("user32.dll")]
    public static extern bool GetWindowRect(IntPtr hWnd, out RECT rect);

    [DllImport("user32.dll")]
    public static extern bool SetCursorPos(int x, int y);

    [DllImport("user32.dll")]
    public static extern void mouse_event(uint flags, uint dx, uint dy, uint data, UIntPtr extraInfo);

    public const uint MOUSEEVENTF_LEFTDOWN = 0x0002;
    public const uint MOUSEEVENTF_LEFTUP = 0x0004;
}
'@

$editorWorkingDirectory = Join-Path $repoRoot "QL-Editor"
$process = Start-Process -FilePath $resolvedEditor.Path -PassThru -WorkingDirectory $editorWorkingDirectory

try {
    Start-Sleep -Seconds 3

    if ($process.HasExited) {
        throw "Editor exited before regression playback started."
    }

    [void][Microsoft.VisualBasic.Interaction]::AppActivate($process.Id)
    Start-Sleep -Milliseconds 300

    $rect = New-Object NativeMethods+RECT
    if (-not [NativeMethods]::GetWindowRect($process.MainWindowHandle, [ref]$rect)) {
        throw "Unable to resolve editor window bounds for play button regression."
    }

    $clickX = [int](($rect.Left + $rect.Right) / 2)
    $clickY = $rect.Top + 55

    # Rapid play/stop stress test against the fixed centered button.
    for ($i = 0; $i -lt $ToggleCount; $i++) {
        [void][NativeMethods]::SetCursorPos($clickX, $clickY)
        Start-Sleep -Milliseconds 40
        [NativeMethods]::mouse_event([NativeMethods]::MOUSEEVENTF_LEFTDOWN, 0, 0, 0, [UIntPtr]::Zero)
        Start-Sleep -Milliseconds 25
        [NativeMethods]::mouse_event([NativeMethods]::MOUSEEVENTF_LEFTUP, 0, 0, 0, [UIntPtr]::Zero)
        Start-Sleep -Milliseconds $ToggleDelayMs
    }

    if ($process.HasExited) {
        throw "Editor exited during play button regression sequence."
    }

    Write-Host "[PASS] Editor stayed alive through repeated centered play button clicks."
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

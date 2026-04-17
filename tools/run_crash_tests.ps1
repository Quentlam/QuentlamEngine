param(
    [string]$ExePath = ".\bin\Debug-windows-x86_64\Tests\Tests.exe"
)

$Tests = @(
    "--crash-segfault",
    "--crash-abort",
    "--crash-invalid-param",
    "--crash-pure-call",
    "--crash-unhandled-exception"
)

$Success = $true

if (-not (Test-Path "CrashDumps")) {
    New-Item -ItemType Directory -Force -Path "CrashDumps" | Out-Null
}

foreach ($test in $Tests) {
    Write-Host "Running crash test: $test" -ForegroundColor Cyan
    
    # Clear old dumps
    Remove-Item "CrashDumps\*" -Force -Recurse -ErrorAction SilentlyContinue
    
    $proc = Start-Process -FilePath $ExePath -ArgumentList $test -PassThru -NoNewWindow
    $proc.WaitForExit()
    
    # Check if dump was created
    $dumps = Get-ChildItem "CrashDumps\*.dmp" -ErrorAction SilentlyContinue
    $jsons = Get-ChildItem "CrashDumps\*.json" -ErrorAction SilentlyContinue
    
    if ($dumps.Count -eq 0 -or $jsons.Count -eq 0) {
        Write-Host "FAILED: No dump or JSON created for $test" -ForegroundColor Red
        $Success = $false
    } else {
        Write-Host "PASSED: Dump created for $test" -ForegroundColor Green
    }
}

if ($Success) {
    Write-Host "All crash tests passed!" -ForegroundColor Green
    exit 0
} else {
    Write-Host "Some crash tests failed." -ForegroundColor Red
    exit 1
}

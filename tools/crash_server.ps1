param(
    [string]$Action = "Upload",
    [string]$DumpPath = "",
    [string]$JsonPath = "",
    [string]$Url = "http://localhost:8080/api/crash/upload"
)

if ($Action -eq "Upload") {
    Write-Host "Simulating upload of crash dump to $Url"
    
    if (-not (Test-Path $DumpPath)) {
        Write-Error "Dump file not found: $DumpPath"
        exit 1
    }
    
    if (-not (Test-Path $JsonPath)) {
        Write-Error "JSON file not found: $JsonPath"
        exit 1
    }
    
    $zipPath = $DumpPath -replace '\.dmp$', '.zip'
    
    # Compress the files
    Compress-Archive -Path $DumpPath, $JsonPath -DestinationPath $zipPath -Force
    Write-Host "Compressed crash data to $zipPath"
    
    # Simulate network upload delay
    Start-Sleep -Seconds 1
    
    # In a real environment, you would use Invoke-RestMethod with -Form to upload multipart data
    # Invoke-RestMethod -Uri $Url -Method Post -Form @{ file = Get-Item $zipPath }
    
    Write-Host "Upload completed successfully. Server received crash report."
    
    # Clean up
    # Remove-Item $DumpPath -Force
    # Remove-Item $JsonPath -Force
    # Remove-Item $zipPath -Force
}

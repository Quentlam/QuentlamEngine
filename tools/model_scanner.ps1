$ErrorActionPreference = "Stop"

$projectDirs = @("QL-Editor", "Sandbox")
$extensions = @(".obj", ".fbx", ".gltf", ".uasset", ".umap")
$pattern = '["'']([^"''\n]+\.(?:obj|fbx|gltf|uasset|umap))["'']'

$report = @()

foreach ($proj in $projectDirs) {
    if (-not (Test-Path $proj)) { continue }
    
    $missingModels = @()
    $files = Get-ChildItem -Path $proj -Recurse -File -Include *.cpp,*.h,*.json,*.ini,*.py,*.prefab,*.scene | Where-Object { $_.FullName -notmatch 'vendor|\\.git|\\bin|\\obj' }
    
    foreach ($file in $files) {
        try {
            $content = Get-Content $file.FullName -Raw -Encoding UTF8 -ErrorAction SilentlyContinue
            if ($content -match $pattern) {
                $matches = [regex]::Matches($content, $pattern)
                foreach ($m in $matches) {
                    $ref = $m.Groups[1].Value
                    $fullPath = Join-Path $proj $ref
                    if (-not (Test-Path $fullPath)) {
                        $missingModels += [PSCustomObject]@{
                            File = $file.FullName
                            Reference = $ref
                        }
                    }
                }
            }
        } catch {}
    }
    
    if ($missingModels.Count -gt 0) {
        $report += "## $proj 缺失模型引用:`n"
        foreach ($item in $missingModels) {
            $report += "- 文件: `"$($item.File)`"`n  - 引用: `"$($item.Reference)`"`n"
        }
    }
}

$reportContent = if ($report.Count -gt 0) { $report -join "" } else { "未发现引用但缺失的模型。`n" }
Write-Host $reportContent
[IO.File]::WriteAllText("CleanupReport.md", "# 模型存在性扫描与清理报告`n`n" + $reportContent, [System.Text.Encoding]::UTF8)

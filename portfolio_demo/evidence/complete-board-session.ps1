[CmdletBinding()]
param(
    [Parameter(Mandatory = $true)][string]$ResultFile,
    [Parameter(Mandatory = $true)][string]$ArtifactPath,
    [string[]]$EvidenceFiles = @(),
    [string]$BuildConfiguration = "",
    [string]$Toolchain = "",
    [string]$Instrument = ""
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

function Get-NormalizedPath {
    param([Parameter(Mandatory = $true)][string]$Path)
    return (Resolve-Path -LiteralPath $Path).Path
}

function Get-Sha256Record {
    param(
        [Parameter(Mandatory = $true)][string]$Path,
        [Parameter(Mandatory = $true)][string]$Purpose
    )

    $resolved = Get-NormalizedPath -Path $Path
    $item = Get-Item -LiteralPath $resolved
    if ($item.PSIsContainer) {
        throw "Expected a file but found a directory: $resolved"
    }
    $hash = Get-FileHash -LiteralPath $resolved -Algorithm SHA256
    return [pscustomobject]@{
        File = $resolved
        Purpose = $Purpose
        Bytes = $item.Length
        SHA256 = $hash.Hash.ToUpperInvariant()
    }
}

if (-not (Get-Command git -ErrorAction SilentlyContinue)) {
    throw "git was not found in PATH."
}

$resultPath = Get-NormalizedPath -Path $ResultFile
$artifactResolved = Get-NormalizedPath -Path $ArtifactPath
$resultDirectory = Split-Path -Parent $resultPath
$resultBaseName = [System.IO.Path]::GetFileNameWithoutExtension($resultPath)
$sessionPath = Join-Path $resultDirectory ($resultBaseName + ".session.json")
if (-not (Test-Path -LiteralPath $sessionPath -PathType Leaf)) {
    throw "Session identity file not found: $sessionPath"
}

$session = Get-Content -LiteralPath $sessionPath -Raw | ConvertFrom-Json
$repoRoot = (& git -C $resultDirectory rev-parse --show-toplevel 2>&1)
if ($LASTEXITCODE -ne 0) {
    throw "Result file is not inside a Git repository."
}
$repoRoot = ($repoRoot -join [Environment]::NewLine).Trim()
$currentBranch = (& git -C $repoRoot rev-parse --abbrev-ref HEAD 2>&1)
if ($LASTEXITCODE -ne 0) {
    throw "Unable to resolve current Git branch."
}
$currentBranch = ($currentBranch -join [Environment]::NewLine).Trim()
if ($currentBranch -ne [string]$session.evidence_branch) {
    throw "Current branch '$currentBranch' does not match session branch '$($session.evidence_branch)'."
}

$artifactRecord = Get-Sha256Record -Path $artifactResolved -Purpose "Programmed C2000 artifact"
$records = New-Object System.Collections.Generic.List[object]
$records.Add($artifactRecord)
foreach ($evidencePath in $EvidenceFiles) {
    $records.Add((Get-Sha256Record -Path $evidencePath -Purpose "Measurement evidence"))
}

$manifestPath = Join-Path $resultDirectory ($resultBaseName + ".manifest.csv")
$records | Export-Csv -LiteralPath $manifestPath -NoTypeInformation -Encoding UTF8

$resultText = Get-Content -LiteralPath $resultPath -Raw
$artifactRelative = $artifactResolved
if ($artifactResolved.StartsWith($repoRoot, [System.StringComparison]::OrdinalIgnoreCase)) {
    $artifactRelative = $artifactResolved.Substring($repoRoot.Length + 1).Replace("\", "/")
}
$resultText = $resultText -replace '(?m)^- Programmed artifact path:.*$', ("- Programmed artifact path: `{0}`" -f $artifactRelative)
$resultText = $resultText -replace '(?m)^- Programmed artifact SHA-256:.*$', ("- Programmed artifact SHA-256: `{0}`" -f $artifactRecord.SHA256)
if (-not [string]::IsNullOrWhiteSpace($BuildConfiguration)) {
    $resultText = $resultText -replace '(?m)^- Build configuration:.*$', ("- Build configuration: {0}" -f $BuildConfiguration)
}
if (-not [string]::IsNullOrWhiteSpace($Toolchain)) {
    $resultText = $resultText -replace '(?m)^- Toolchain / CCS / C2000Ware / SysConfig:.*$', ("- Toolchain / CCS / C2000Ware / SysConfig: {0}" -f $Toolchain)
}
if (-not [string]::IsNullOrWhiteSpace($Instrument)) {
    $resultText = $resultText -replace '(?m)^- Oscilloscope / logic analyzer:.*$', ("- Oscilloscope / logic analyzer: {0}" -f $Instrument)
}

$evidenceRows = New-Object System.Collections.Generic.List[string]
foreach ($record in $records) {
    $displayPath = [string]$record.File
    if ($displayPath.StartsWith($repoRoot, [System.StringComparison]::OrdinalIgnoreCase)) {
        $displayPath = $displayPath.Substring($repoRoot.Length + 1).Replace("\", "/")
    }
    $evidenceRows.Add(("| `{0}` | {1} | `{2}` |" -f $displayPath, $record.Purpose, $record.SHA256))
}
$evidenceBlock = "| File | Purpose | SHA-256 |`r`n|---|---|---|`r`n" + ($evidenceRows -join "`r`n")
$resultText = [regex]::Replace(
    $resultText,
    '(?ms)\| File \| Purpose \| SHA-256 \|\s*\|---\|---\|---\|\s*\|\s*\|\s*\|\s*',
    $evidenceBlock + "`r`n")

$utf8NoBom = New-Object System.Text.UTF8Encoding($false)
[System.IO.File]::WriteAllText($resultPath, $resultText, $utf8NoBom)

Write-Host "Evidence identity finalized."
Write-Host "  Frozen source commit: $($session.source_commit)"
Write-Host "  Artifact SHA-256    : $($artifactRecord.SHA256)"
Write-Host "  Manifest            : $manifestPath"
Write-Host ""
Write-Host "You must still enter measured values, tolerances, PASS/FAIL disposition, and safety limits in the result file."
Write-Host "The script does not prove that the artifact was actually programmed or that a waveform passed acceptance criteria."

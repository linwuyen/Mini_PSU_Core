[CmdletBinding()]
param(
    [string]$BranchName = "",
    [string]$ResultId = "",
    [string]$Board = "YXDSP-F28388D-176pin",
    [string]$Mcu = "TMS320F28388D"
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

function Invoke-Git {
    param([Parameter(Mandatory = $true)][string[]]$GitArguments)

    $output = & git @GitArguments 2>&1
    if ($LASTEXITCODE -ne 0) {
        throw "git $($GitArguments -join ' ') failed:`n$($output -join [Environment]::NewLine)"
    }
    return ($output -join [Environment]::NewLine)
}

if (-not (Get-Command git -ErrorAction SilentlyContinue)) {
    throw "git was not found in PATH."
}

$repoRoot = (Invoke-Git -GitArguments @("rev-parse", "--show-toplevel")).Trim()
Push-Location $repoRoot
try {
    $dirty = (Invoke-Git -GitArguments @("status", "--porcelain")).Trim()
    if ($dirty.Length -ne 0) {
        throw "The working tree is not clean. Commit, stash, or discard changes before freezing a board-evidence session."
    }

    $sourceCommit = (Invoke-Git -GitArguments @("rev-parse", "HEAD")).Trim()
    $sourceBranch = (Invoke-Git -GitArguments @("rev-parse", "--abbrev-ref", "HEAD")).Trim()

    if ([string]::IsNullOrWhiteSpace($ResultId)) {
        $ResultId = "{0}-f28388d" -f (Get-Date -Format "yyyyMMdd-HHmm")
    }
    if ([string]::IsNullOrWhiteSpace($BranchName)) {
        $BranchName = "board/$ResultId"
    }

    & git show-ref --verify --quiet "refs/heads/$BranchName"
    if ($LASTEXITCODE -eq 0) {
        throw "Local branch '$BranchName' already exists. Choose another -BranchName or -ResultId."
    }

    Invoke-Git -GitArguments @("checkout", "-b", $BranchName, $sourceCommit) | Out-Null

    $evidenceRoot = Join-Path $repoRoot "portfolio_demo/evidence"
    $resultsRoot = Join-Path $evidenceRoot "results"
    $templatePath = Join-Path $evidenceRoot "bench-result-template.md"
    if (-not (Test-Path -LiteralPath $templatePath -PathType Leaf)) {
        throw "Evidence template not found: $templatePath"
    }

    New-Item -ItemType Directory -Path $resultsRoot -Force | Out-Null
    $resultPath = Join-Path $resultsRoot ($ResultId + ".md")
    $sessionPath = Join-Path $resultsRoot ($ResultId + ".session.json")
    if ((Test-Path -LiteralPath $resultPath) -or (Test-Path -LiteralPath $sessionPath)) {
        throw "Evidence session '$ResultId' already exists."
    }

    $resultText = Get-Content -LiteralPath $templatePath -Raw
    $resultText = $resultText -replace '# Bench result — `<test-id>`', ("# Bench result — `{0}`" -f $ResultId)
    $resultText = $resultText -replace '(?m)^- Exact commit:.*$', ("- Exact commit: `{0}`" -f $sourceCommit)
    $resultText = $resultText -replace '(?m)^- Branch / PR:.*$', ("- Branch / PR: `{0}`" -f $BranchName)
    $resultText = $resultText -replace '(?m)^- Date and timezone:.*$', ("- Date and timezone: {0}" -f (Get-Date -Format "yyyy-MM-dd HH:mm zzz"))
    $resultText = $resultText -replace '(?m)^- Engineer:.*$', '- Engineer: linwuyen'
    $resultText = $resultText -replace '(?m)^- Board and revision:.*$', ("- Board and revision: {0}" -f $Board)
    $resultText = $resultText -replace '(?m)^- MCU and package:.*$', ("- MCU and package: {0}" -f $Mcu)

    $utf8NoBom = New-Object System.Text.UTF8Encoding($false)
    [System.IO.File]::WriteAllText($resultPath, $resultText, $utf8NoBom)

    $session = [ordered]@{
        repository = "linwuyen/Mini_PSU_Core"
        result_id = $ResultId
        source_commit = $sourceCommit
        source_branch = $sourceBranch
        evidence_branch = $BranchName
        created_at = (Get-Date).ToString("o")
        board = $Board
        mcu = $Mcu
        result_file = (Resolve-Path -LiteralPath $resultPath).Path.Substring($repoRoot.Length + 1).Replace("\", "/")
        evidence_boundary = "This session records identity only. Build, flash, waveform, timing, and safety claims remain NOT RUN until measured."
    }
    $sessionJson = $session | ConvertTo-Json -Depth 4
    [System.IO.File]::WriteAllText($sessionPath, $sessionJson + [Environment]::NewLine, $utf8NoBom)

    Write-Host "Board-evidence session created."
    Write-Host "  Source commit : $sourceCommit"
    Write-Host "  Evidence branch: $BranchName"
    Write-Host "  Result file   : $resultPath"
    Write-Host ""
    Write-Host "Next: build and program from the frozen source commit, then run complete-board-session.ps1."
    Write-Host "Do not claim PASS until the exact artifact and measurements are retained."
}
finally {
    Pop-Location
}

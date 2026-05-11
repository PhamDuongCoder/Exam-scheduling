# Run SA solver on TC_03 with convergence logging and generate plot

param(
    [string]$TestCase = "TC_03",
    [string]$LogDir = "log",
    [string]$LogFile = $null
)

# Default log file name if not specified
if (-not $LogFile) {
    $LogFile = "$LogDir\$TestCase.convergence.csv"
}

# Resolve paths
$ProjectRoot = Split-Path -Parent $PSScriptRoot
$ExePath = "$ProjectRoot\build\exam_scheduler.exe"
$InputFile = "$ProjectRoot\test_cases\Input\$TestCase"
$PythonScript = "$PSScriptRoot\plot_convergence.py"

# Check prerequisites
if (-not (Test-Path $ExePath)) {
    Write-Error "Executable not found: $ExePath"
    Write-Host "Please build the project first: cmake --build build"
    exit 1
}

if (-not (Test-Path $InputFile)) {
    Write-Error "Test case not found: $InputFile"
    exit 1
}

if (-not (Test-Path $PythonScript)) {
    Write-Error "Python script not found: $PythonScript"
    exit 1
}

# Create log directory if needed
if (-not (Test-Path $LogDir)) {
    New-Item -ItemType Directory -Path $LogDir -Force | Out-Null
}

Write-Host "Running SA solver on $TestCase with convergence logging..."
Write-Host "  Executable: $ExePath"
Write-Host "  Input file: $InputFile"
Write-Host "  Log file: $LogFile"
Write-Host ""

# Run solver with logging
Get-Content $InputFile | & $ExePath "sa" "--log" $LogFile > $null

if ($LASTEXITCODE -ne 0) {
    Write-Error "Solver failed with exit code $LASTEXITCODE"
    exit 1
}

Write-Host "Solver completed successfully."
Write-Host ""

# Check if log file was created
if (-not (Test-Path $LogFile)) {
    Write-Error "Log file was not created: $LogFile"
    exit 1
}

Write-Host "Generating convergence plot..."
Write-Host ""

# Run Python plotting script
python $PythonScript $LogFile

if ($LASTEXITCODE -ne 0) {
    Write-Error "Python script failed with exit code $LASTEXITCODE"
    exit 1
}

Write-Host ""
Write-Host "Done! Check $LogFile.png for the plot."


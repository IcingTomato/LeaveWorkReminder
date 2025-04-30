# If no parameter is passed, the script will use the default commit message: "Change: Changed $filesOutput"
param
(
    [string]$gitcommit = $null
)

# Perform git pull to update the repository
Write-Host "Pulling latest changes from the repository..."
git pull
Write-Host " "

# Get the directory where the current script is located
$CurrentDirectory = Split-Path -Parent $MyInvocation.MyCommand.Definition

# Get all .exe files in the directory and its subdirectories
$ExeFiles = Get-ChildItem -Path $CurrentDirectory -Filter *.exe -Recurse

if ($ExeFiles.Count -eq 0) {
    Write-Host "No .exe files found."
} else {
    Write-Host "The following .exe files will be deleted:"
    $ExeFiles | ForEach-Object {
        Write-Host $_.FullName
    }

    # Delete all found .exe files
    $ExeFiles | Remove-Item -Force

    Write-Host " "
    Write-Host "Cleanup completed!"
}

# Get all LeaveWork_log.txt files in the directory and its subdirectories
$LogFile = Get-ChildItem -Path $CurrentDirectory -Filter "LeaveWork_log.txt" -Recurse

if ($LogFile.Count -eq 0) {
    Write-Host "No LeaveWork_log.txt file found."
} else {
    Write-Host "The following log file(s) will be deleted:"
    $LogFile | ForEach-Object {
        Write-Host $_.FullName
    }

    # Delete all found LeaveWork_log.txt files
    $LogFile | Remove-Item -Force

    Write-Host " "
    Write-Host "Log file cleanup completed!"
}

# Get modified files using git diff instead of git status
$modifiedFiles = git diff --name-only --cached
$modifiedFiles += git diff --name-only
$modifiedFiles += git ls-files --others --exclude-standard

# Remove duplicates
$modifiedFiles = $modifiedFiles | Select-Object -Unique

if (-not $modifiedFiles) {
    Write-Host " "
    Write-Host "No uncommitted changes."
    Write-Host " "
    $filesOutput = ""
} else {
    # Convert to array to ensure correct handling of single file cases
    $FileNames = @($modifiedFiles)
    
    # Ensure all elements are treated as array
    Write-Host "Debug - Number of changed files:" $FileNames.Count
    $FileNames | ForEach-Object { Write-Host "File: $_" }
    
    # Format files list differently to ensure proper separation
    if ($FileNames.Count -gt 2) {
        $filesOutput = "{0}, {1}, etc." -f $FileNames[0], $FileNames[1]
    } elseif ($FileNames.Count -eq 2) {
        $filesOutput = "{0}, {1}" -f $FileNames[0], $FileNames[1]
    } else {
        $filesOutput = $FileNames[0]
    }
    
    # Add debug output to see exact formatted string
    Write-Host "Debug - Formatted output: [$filesOutput]"
}

# If no -gitcommit parameter is passed, use default value
if (-not $gitcommit) {
    $gitcommit = "Change: Changed $filesOutput"
}

# Make sure commit message is correctly quoted
$gitcommitQuoted = "`"$gitcommit`""  # Using double quotes instead of single quotes

# Stage all changes, commit, and push to Git
Write-Host " "
git add .
Write-Host " "
Write-Host "Committing with message: $gitcommit"
git commit -m $gitcommitQuoted
Write-Host " "
git push
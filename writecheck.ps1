# Define the path to the file containing the list of shares (write.txt)
$shareList = "Shares_clean.txt"

# Define the name of the blank text file to create in each share
$blankFileName = "blankfile.txt"

# Define the path to save the final Excel report (on the Desktop)
$ExcelFilePath = "C:\Users\knigh\Desktop\NFS_Share_Access_Report.xlsx"

# Initialize an array to store the results
$results = @()

# Read each share from the write.txt file
$shares = Get-Content -Path $shareList

# Loop through each share
foreach ($share in $shares) {
    # Add a 1-second delay between each iteration
    Start-Sleep -Seconds 1

    # Initialize status as "read" by default
    $status = "Read"
    $message = "Access Denied or Read-Only"

    # Trim the share path to remove leading/trailing whitespace
    $share = $share.Trim()

    # Check if the share exists and is accessible (use LiteralPath to handle spaces)
    if (Test-Path -LiteralPath $share) {
        try {
            # Create the path for the new blank text file in the share
            $filePath = Join-Path -Path $share -ChildPath $blankFileName

            # Attempt to create a blank text file in the share (this will check write access)
            New-Item -Path $filePath -ItemType File -Force -ErrorAction Stop

            # Check if the file was actually created successfully (use LiteralPath)
            if (Test-Path -LiteralPath $filePath) {
                # If file creation is successful, update status to "Write"
                $status = "Write"
                $message = "Successfully created blank file"

                # Add the success result to the results array
                $results += [PSCustomObject]@{
                    Share = $share
                    Status = $status
                    Message = $message
                }

                Write-Host "Successfully created blank file in $($share)"

                # Delete the blank file after confirming creation (use LiteralPath)
                Remove-Item -LiteralPath $filePath -Force -ErrorAction SilentlyContinue
                Write-Host "Deleted the blank file from $($share)"
            }
            else {
                # If file creation fails, update status to "Read"
                $status = "Read"
                $message = "Failed to create blank file"

                # Add the failure result to the results array
                $results += [PSCustomObject]@{
                    Share = $share
                    Status = $status
                    Message = $message
                }

                Write-Host "Failed to create blank file in $($share)"
            }
        }
        catch {
            Write-Host "Error creating file in $($share): $_"
            $message = "Error creating file: $_"

            # Add the error result to the results array
            $results += [PSCustomObject]@{
                Share = $share
                Status = "Read"
                Message = $message
            }
        }
    }
    else {
        Write-Host "Share not accessible: $($share)"
        # Add the share not accessible result to the results array
        $results += [PSCustomObject]@{
            Share = $share
            Status = "NoAccess"
            Message = "Share not accessible"
        }
    }
}

# Check if results exist
if ($results.Count -gt 0) {
    try {
        # Export the results to an Excel file on the Desktop
        $results | Export-Excel -Path $ExcelFilePath -AutoSize -WorksheetName 'SharePermissions'
        Write-Host "Excel report saved to: $ExcelFilePath"
    }
    catch {
        Write-Host "Error exporting to Excel: $_"
    }
}
else {
    Write-Host "No results to export."
}
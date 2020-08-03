# Set location to scripts directory
Set-Location $PSScriptRoot
# Set build to Release
# Build to \executable in Release config
# \executable is not ignored by git.
dotnet build --configuration Release -o .\executable\
# bin/obj Debug directories. C# generates errors, dont need them.
Start-Sleep -s 5
Remove-Item .\bin\ -Recurse
Remove-Item .\obj\Debug -Recurse
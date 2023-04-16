#Requires -Version 5

# args
param (
    [Parameter(Mandatory)][ValidateSet('COPY', 'SOURCEGEN', 'DISTRIBUTE')][string]$Mode,
    [string]$Version,
    [string]$Path,
    [string]$Project
)


$ErrorActionPreference = "Stop"

$Folder = $PSScriptRoot | Split-Path -Leaf
$SourceExt = @('.c', '.cpp', '.cxx', '.h', '.hpp', '.hxx')
$ConfigExt = @('.ini', '.json', '.toml')
$env:ScriptCulture = (Get-Culture).Name -eq 'zh-CN'

function Resolve-Files {
    param (
        [Parameter(ValueFromPipeline)][string]$a_parent = $PSScriptRoot,
        [string[]]$a_directory = @('include', 'src', 'test')
    )
    
    process {
        Push-Location $PSScriptRoot
        $_generated = [System.Collections.ArrayList]::new()

        try {
            foreach ($directory in $a_directory) {
                if (!$env:RebuildInvoke) {
                    Write-Host "`t[$a_parent/$directory]"
                }

                Get-ChildItem "$a_parent/$directory" -Recurse -File -ErrorAction SilentlyContinue | Where-Object {
                    ($_.Extension -in $SourceExt) -and 
                    ($_.Name -notmatch 'Plugin.h|Version.h')
                } | Resolve-Path -Relative | ForEach-Object {
                    if (!$env:RebuildInvoke) {
                        Write-Host "`t`t<$_>"
                    }
                    $_generated.Add("`n`t`"$($_.Substring(2) -replace '\\', '/')`"") | Out-Null
                }
            }               
            
            Get-ChildItem "$a_parent" -File -ErrorAction SilentlyContinue | Where-Object {
                ($_.Extension -in $ConfigExt) -and 
                ($_.Name -notmatch 'cmake|vcpkg')
            } | Resolve-Path -Relative | ForEach-Object {
                if (!$env:RebuildInvoke) {
                    Write-Host "`t`t<$_>"
                }
                $_generated.Add("`n`t`"$($_.Substring(2) -replace '\\', '/')`"") | Out-Null
            }
        } finally {
            Pop-Location
        }

        return $_generated
    }
}


Write-Host "`n`t<$Folder> [$Mode]"


# @@COPY
if ($Mode -eq 'COPY') {
    # process newly added files
    $BuildFolder = Get-ChildItem (Get-Item $Path).Parent.Parent.FullName "$Project.sln" -Depth 2 -File -Exclude ('*CMakeFiles*', '*CLib*')
    $NewFiles = Get-ChildItem $BuildFolder.DirectoryName -File | Where-Object {$_.Extension -in $SourceExt}
    if ($NewFiles) { # trigger ZERO_CHECK
        $NewFiles | Move-Item -Destination "$PSScriptRoot/src" -Force -Confirm:$false -ErrorAction:SilentlyContinue | Out-Null
        [IO.File]::WriteAllText("$PSScriptRoot/CMakeLists.txt", [IO.File]::ReadAllText("$PSScriptRoot/CMakeLists.txt"))
    }

    # Build Target
    Write-Host "`t$Folder $Version"
    $vcpkg = [IO.File]::ReadAllText("$PSScriptRoot/vcpkg.json") | ConvertFrom-Json
    $Install = $vcpkg.'features'.'mo2-install'.'description'
    $ProjectCMake = [IO.File]::ReadAllText("$PSScriptRoot/CMakeLists.txt")
    $OldVersion = [regex]::match($ProjectCMake, '(?s)(?:(?<=\sVERSION\s)(.*?)(?=\s+))').Groups[1].Value


    function Copy-Mod {
        param (
            $Data
        )

        New-Item -Type Directory "$Data/SKSE/Plugins" -Force | Out-Null

        # binary
        Copy-Item "$Path/$Project.dll" "$Data/SKSE/Plugins/$Project.dll" -Force
        $Message.Text += "`r`n- Binary files copied"

        # configs
        Get-ChildItem $PSScriptRoot | Where-Object {
            ($_.Extension -in $ConfigExt) -and 
            ($_.Name -notmatch 'CMake|vcpkg')
        } | ForEach-Object {
            Copy-Item $_.FullName "$Data/SKSE/Plugins/$($_.Name)" -Force
            $Message.Text += "`r`n- Configuration files copied"
        }

        # shockwave
        if (Test-Path "$PSScriptRoot/Interface/*.swf" -PathType Leaf) {
            New-Item -Type Directory "$Data/Interface" -Force | Out-Null
            Copy-Item "$PSScriptRoot/Interface" "$Data" -Recurse -Force
            $Message.Text += "`r`n- Shockwave files copied"
        }

        # papyrus
        if (Test-Path "$PSScriptRoot/Scripts/*.pex" -PathType Leaf) {
            New-Item -Type Directory "$Data/Scripts" -Force | Out-Null
            xcopy.exe "$PSScriptRoot/Scripts" "$Data/Scripts" /C /I /S /E /Y
            $Message.Text += "`r`n- Papyrus scripts copied"
        }
        if (Test-Path "$PSScriptRoot/Scripts/Source/*.psc" -PathType Leaf) {
            New-Item -Type Directory "$Data/Scripts/Source" -Force | Out-Null
            xcopy.exe "$PSScriptRoot/Scripts/Source" "$Data/Scripts/Source" /C /I /S /E /Y
            $Message.Text += "`r`n- Papyrus scripts copied"
        }
    }


	Add-Type -AssemblyName Microsoft.VisualBasic
    Add-Type -AssemblyName System.Windows.Forms
    Add-Type -AssemblyName System.Drawing

    [System.Windows.Forms.Application]::EnableVisualStyles()
    $MsgBox = New-Object System.Windows.Forms.Form -Property @{
        TopLevel = $true
        ClientSize = '350, 305'
        Text = $Project
        StartPosition = 'CenterScreen'
        FormBorderStyle = 'FixedDialog'
        MaximizeBox = $false
        MinimizeBox = $false
        Font = New-Object System.Drawing.Font('Segoe UI', 10, [System.Drawing.FontStyle]::Regular)
    }
    
    $Message = New-Object System.Windows.Forms.TextBox -Property @{
        ClientSize = '225, 150'
        Location = New-Object System.Drawing.Point(20, 20)
        Multiline = $true
        ReadOnly = $true
        Text = "- [$Project - $OldVersion] has been built."
        
    }
    
    $BtnCopyMO2 = New-Object System.Windows.Forms.Button -Property @{
        ClientSize = '70, 50'
        Text = 'Copy to MO2'
        Location = New-Object System.Drawing.Point(260, 19)
        BackColor = 'Cyan'
        Add_Click = {
            foreach ($runtime in @("$($env:MO2SkyrimAEPath)/mods", "$($env:MO2SkyrimSEPath)/mods", "$($env:MO2SkyrimVRPath)/mods")) {
                if (Test-Path $runtime -PathType Container) {
                    Copy-Mod "$runtime/$Install"
                }
            }
            $Message.Text += "`r`n- Copied to MO2."
        }
    }
    
    $BtnCopyData = New-Object System.Windows.Forms.Button -Property @{
        ClientSize = '70, 50'
        Text = 'Copy to Data'
        Location = New-Object System.Drawing.Point(260, 74)
        Add_Click = {
            foreach ($runtime in @("$($env:SkyrimAEPath)/data", "$($env:SkyrimSEPath)/data", "$($env:SkyrimVRPath)/data")) {
                if (Test-Path $runtime -PathType Container) {
                    Copy-Mod "$runtime"
                }
            }
            $Message.Text += "`r`n- Copied to game data."
        }
    }
    
    $BtnRemoveData = New-Object System.Windows.Forms.Button -Property @{
        ClientSize = '70, 50'
        Text = 'Remove in Data'
        Location = New-Object System.Drawing.Point(260, 129)
        Add_Click = {
            foreach ($runtime in @("$($env:SkyrimAEPath)/data", "$($env:SkyrimSEPath)/data", "$($env:SkyrimVRPath)/data")) {
                if (Test-Path "$runtime/SKSE/Plugins/$Project.dll" -PathType Leaf) {
                    Remove-Item "$runtime/SKSE/Plugins/$Project.dll" -Force -Confirm:$false -ErrorAction:SilentlyContinue | Out-Null
                }
            }
            $Message.Text += "`r`n- Removed from game data."
        }
    }
    
    $BtnOpenFolder = New-Object System.Windows.Forms.Button -Property @{
        ClientSize = '70, 50'
        Text = 'Show in Explorer'
        Location = New-Object System.Drawing.Point(260, 185)
        BackColor = 'Yellow'
        Add_Click = {
            Invoke-Item $Path
        }
    }
    
    $BtnLaunchSKSEAE = New-Object System.Windows.Forms.Button -Property @{
        ClientSize = '70, 50'
        Text = 'SKSE (AE)'
        Location = New-Object System.Drawing.Point(20, 185)
        Add_Click = {
            Push-Location $env:SkyrimAEPath
            Start-Process ./SKSE64_loader.exe
            Pop-Location

            $Message.Text += "`r`n- SKSE (AE) Launched."
        }
    }
    if (!(Test-Path "$env:SkyrimAEPath/skse64_loader.exe" -PathType Leaf)) {
        $BtnLaunchSKSEAE.Enabled = $false
    }

    $BtnLaunchSKSESE = New-Object System.Windows.Forms.Button -Property @{
        ClientSize = '70, 50'
        Text = 'SKSE (SE)'
        Location = New-Object System.Drawing.Point(100, 185)
        Add_Click = {
            Push-Location $env:SkyrimSEPath
            Start-Process ./SKSE64_loader.exe
            Pop-Location

            $Message.Text += "`r`n- SKSE (SE) Launched."
        }
    }
    if (!(Test-Path "$env:SkyrimSEPath/skse64_loader.exe" -PathType Leaf)) {
        $BtnLaunchSKSESE.Enabled = $false
    }
 
    $BtnLaunchSKSEVR = New-Object System.Windows.Forms.Button -Property @{
        ClientSize = '70, 50'
        Text = 'SKSE (VR)'
        Location = New-Object System.Drawing.Point(180, 185)
        Add_Click = {
            Push-Location $env:SkyrimVRPath
            Start-Process ./SKSE64_loader.exe
            Pop-Location

            $Message.Text += "`r`n- SKSE (VR) Launched."
        }
    }
    if (!(Test-Path "$env:SkyrimVRPath/skse64_loader.exe" -PathType Leaf)) {
        $BtnLaunchSKSEVR.Enabled = $false
    }
    
    $BtnBuildPapyrus = New-Object System.Windows.Forms.Button -Property @{
        ClientSize = '70, 50'
        Text = 'Build Papyrus'
        Location = New-Object System.Drawing.Point(20, 240)
        Add_Click = {
            $BtnBuildPapyrus.Text = 'Compiling...'
            
            $Invocation = "`"$($env:SkyrimSEPath)/Papyrus Compiler/PapyrusCompiler.exe`" `"$PSScriptRoot/Scripts/Source`" -f=`"$env:SkyrimSEPath/Papyrus Compiler/TESV_Papyrus_Flags.flg`" -i=`"$env:SkyrimSEPath/Data/Scripts/Source;$PSScriptRoot/Scripts;$PSScriptRoot/Scripts/Source`" -o=`"$PSScriptRoot/Scripts`" -a -op -enablecache -t=`"4`""
            Start-Process cmd.exe -ArgumentList "/k $Invocation && pause && exit"
            
            $BtnBuildPapyrus.Text = 'Build Papyrus'
        }
    }
    
    $BtnChangeVersion = New-Object System.Windows.Forms.Button -Property @{
        ClientSize = '70, 50'
        Text = 'Version'
        Location = New-Object System.Drawing.Point(100, 240)
        Add_Click = {
            $NewVersion = $null
            while ($OldVersion -and !$NewVersion) {
                $NewVersion = [Microsoft.VisualBasic.Interaction]::InputBox("Input the new versioning for $Project", 'Versioning', $OldVersion)
            }
            $ProjectCMake = $ProjectCMake -replace "VERSION\s$OldVersion", "VERSION $NewVersion"
            $vcpkg.'version-string' = $NewVersion

            [IO.File]::WriteAllText("$PSScriptRoot/CMakeLists.txt", $ProjectCMake)
            $vcpkg = $vcpkg | ConvertTo-Json -Depth 9
            [IO.File]::WriteAllText("$PSScriptRoot/vcpkg.json", $vcpkg)


            $Message.Text += "`r`n- Version changed $OldVersion -> $NewVersion"
            $OldVersion = $NewVersion
        }
    }
    
    $BtnPublish = New-Object System.Windows.Forms.Button -Property @{
        ClientSize = '70, 50'
        Text = 'Publish Mod'
        Location = New-Object System.Drawing.Point(180, 240)
        Add_Click = {
            $BtnPublish.Text = 'Zipping...'

            Copy-Mod "$PSScriptRoot/Tmp/Data"
            Compress-Archive "$PSScriptRoot/Tmp/Data/*" "$Path/$($Project)-$(($OldVersion).Replace('.', '-'))" -Force
            Remove-Item "$PSScriptRoot/Tmp" -Recurse -Force -Confirm:$false -ErrorAction:SilentlyContinue | Out-Null
            Invoke-Item $Path

            $Message.Text += "`r`n- Mod files zipped & ready to go."
            $BtnPublish.Text = 'Publish Mod'
        }
    }
    
    
    $BtnExit = New-Object System.Windows.Forms.Button -Property @{
        ClientSize = '70, 50'
        Text = 'Exit'
        Location = New-Object System.Drawing.Point(260, 240)
        Add_Click = {
            $MsgBox.Close()
        }
    }
                
    $MsgBox.Controls.Add($Message)
    $MsgBox.Controls.Add($BtnCopyData)
    $MsgBox.Controls.Add($BtnCopyMO2)
    $MsgBox.Controls.Add($BtnRemoveData)
    $MsgBox.Controls.Add($BtnOpenFolder)
    $MsgBox.Controls.Add($BtnExit)
    $MsgBox.Controls.Add($BtnBuildPapyrus)
    $MsgBox.Controls.Add($BtnChangeVersion)
    $MsgBox.Controls.Add($BtnPublish)
    $MsgBox.Controls.Add($BtnLaunchSKSEAE)
    $MsgBox.Controls.Add($BtnLaunchSKSESE)
    $MsgBox.Controls.Add($BtnLaunchSKSEVR)
    
    $MsgBox.ShowDialog() | Out-Null
    Exit
}


# @@SOURCEGEN
if ($Mode -eq 'SOURCEGEN') {
    Write-Host "`tGenerating CMake sourcelist..."
    Remove-Item "$Path/sourcelist.cmake" -Force -Confirm:$false -ErrorAction Ignore

    $generated = 'set(SOURCES'
    $generated += $PSScriptRoot | Resolve-Files
    if ($Path) {
        $generated += $Path | Resolve-Files
    }
    $generated += "`n)"
    [IO.File]::WriteAllText("$Path/sourcelist.cmake", $generated)
}


# @@DISTRIBUTE
if ($Mode -eq 'DISTRIBUTE') { # update script to every project
    Get-ChildItem "$PSScriptRoot/*/*" -Directory | Where-Object {
        $_.Name -notin @('vcpkg', 'Build', '.git', '.vs') -and
        (Test-Path "$_/CMakeLists.txt" -PathType Leaf)
    } | ForEach-Object {
        Write-Host "`tUpdated <$_>"
        Robocopy.exe "$PSScriptRoot" "$_" '!Update.ps1' /MT /NJS /NFL /NDL /NJH | Out-Null
    }
}




# SIG # Begin signature block
# MIIboQYJKoZIhvcNAQcCoIIbkjCCG44CAQExCzAJBgUrDgMCGgUAMGkGCisGAQQB
# gjcCAQSgWzBZMDQGCisGAQQBgjcCAR4wJgIDAQAABBAfzDtgWUsITrck0sYpfvNR
# AgEAAgEAAgEAAgEAAgEAMCEwCQYFKw4DAhoFAAQUL10O38us2rLaFkAAv/CaJq/f
# 2nKgghYXMIIDBjCCAe6gAwIBAgIQbBejp82dcLdHI5AVyyqyxzANBgkqhkiG9w0B
# AQsFADAbMRkwFwYDVQQDDBBES1NjcmlwdFNlbGZDZXJ0MB4XDTIxMTIwMzExMTgx
# OVoXDTIyMTIwMzExMzgxOVowGzEZMBcGA1UEAwwQREtTY3JpcHRTZWxmQ2VydDCC
# ASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBAKCLTioNBJsXmC6rmQ9af4DL
# 0+zXaFKtkDFaOzLbiDB17sVAgkGjC8uSQ29qK0gr934ekXWSkk3a2QWfVUz+6uKJ
# kgc5d2yRzXItO+8Y83zXHW5xEfqA65ukCEKhoNN8y6iVq9iTYYD3Yv1hNfSSLhsj
# RICd2vkyTm0zwwh69nWMqz6AMcLr4PiNMbO/1yv6bi2lSXFfhWYjnJEKFezMv1fi
# uf85XmXYl08uqRK1NWQJASAbI3azCwR2kNSWamoz8OuBcKEvO+xdsv3UGION5jwt
# 1YyyzEauCzl3rwBU1GHeubhcz4iZqJ7Wb47bOhQBpHkLqrBNzxoVjNBx2aJ7Qz0C
# AwEAAaNGMEQwDgYDVR0PAQH/BAQDAgeAMBMGA1UdJQQMMAoGCCsGAQUFBwMDMB0G
# A1UdDgQWBBRYLE0TxN1kYtIniUU4xnRIZxovITANBgkqhkiG9w0BAQsFAAOCAQEA
# dRa495+I6eK8hxMbFkP9sRWD1ZWw4TPyGWTCBpDKkJ8mUm7SSwnZgfiZ78C6P3AQ
# D+unSQLvTwN+0PISQti0TMf3Sy+92UPyEQVKk/Wky0tZrYWje8DSayEu72SwTtUn
# GhzAMGe7roDCe8+Q4YFAKh8HH3Fz70eJQnBCNewJfiI0tVBav/bCaPfjWKdlYMoi
# LsCHVYYzLfmZWLN6fhWY4NT1F3OBCoDvqvBTUupsknzIQIkR0kl0hvyiTKuTgKmZ
# xJoYX3MvXEBZMs/WUaTDXOt4tLe7viye6T2RUeILyJiuq5PDzM6X1tUbgQEXOFhQ
# dOHtYjGwteueqI+Usmp3cDCCBY0wggR1oAMCAQICEA6bGI750C3n79tQ4ghAGFow
# DQYJKoZIhvcNAQEMBQAwZTELMAkGA1UEBhMCVVMxFTATBgNVBAoTDERpZ2lDZXJ0
# IEluYzEZMBcGA1UECxMQd3d3LmRpZ2ljZXJ0LmNvbTEkMCIGA1UEAxMbRGlnaUNl
# cnQgQXNzdXJlZCBJRCBSb290IENBMB4XDTIyMDgwMTAwMDAwMFoXDTMxMTEwOTIz
# NTk1OVowYjELMAkGA1UEBhMCVVMxFTATBgNVBAoTDERpZ2lDZXJ0IEluYzEZMBcG
# A1UECxMQd3d3LmRpZ2ljZXJ0LmNvbTEhMB8GA1UEAxMYRGlnaUNlcnQgVHJ1c3Rl
# ZCBSb290IEc0MIICIjANBgkqhkiG9w0BAQEFAAOCAg8AMIICCgKCAgEAv+aQc2je
# u+RdSjwwIjBpM+zCpyUuySE98orYWcLhKac9WKt2ms2uexuEDcQwH/MbpDgW61bG
# l20dq7J58soR0uRf1gU8Ug9SH8aeFaV+vp+pVxZZVXKvaJNwwrK6dZlqczKU0RBE
# EC7fgvMHhOZ0O21x4i0MG+4g1ckgHWMpLc7sXk7Ik/ghYZs06wXGXuxbGrzryc/N
# rDRAX7F6Zu53yEioZldXn1RYjgwrt0+nMNlW7sp7XeOtyU9e5TXnMcvak17cjo+A
# 2raRmECQecN4x7axxLVqGDgDEI3Y1DekLgV9iPWCPhCRcKtVgkEy19sEcypukQF8
# IUzUvK4bA3VdeGbZOjFEmjNAvwjXWkmkwuapoGfdpCe8oU85tRFYF/ckXEaPZPfB
# aYh2mHY9WV1CdoeJl2l6SPDgohIbZpp0yt5LHucOY67m1O+SkjqePdwA5EUlibaa
# RBkrfsCUtNJhbesz2cXfSwQAzH0clcOP9yGyshG3u3/y1YxwLEFgqrFjGESVGnZi
# fvaAsPvoZKYz0YkH4b235kOkGLimdwHhD5QMIR2yVCkliWzlDlJRR3S+Jqy2QXXe
# eqxfjT/JvNNBERJb5RBQ6zHFynIWIgnffEx1P2PsIV/EIFFrb7GrhotPwtZFX50g
# /KEexcCPorF+CiaZ9eRpL5gdLfXZqbId5RsCAwEAAaOCATowggE2MA8GA1UdEwEB
# /wQFMAMBAf8wHQYDVR0OBBYEFOzX44LScV1kTN8uZz/nupiuHA9PMB8GA1UdIwQY
# MBaAFEXroq/0ksuCMS1Ri6enIZ3zbcgPMA4GA1UdDwEB/wQEAwIBhjB5BggrBgEF
# BQcBAQRtMGswJAYIKwYBBQUHMAGGGGh0dHA6Ly9vY3NwLmRpZ2ljZXJ0LmNvbTBD
# BggrBgEFBQcwAoY3aHR0cDovL2NhY2VydHMuZGlnaWNlcnQuY29tL0RpZ2lDZXJ0
# QXNzdXJlZElEUm9vdENBLmNydDBFBgNVHR8EPjA8MDqgOKA2hjRodHRwOi8vY3Js
# My5kaWdpY2VydC5jb20vRGlnaUNlcnRBc3N1cmVkSURSb290Q0EuY3JsMBEGA1Ud
# IAQKMAgwBgYEVR0gADANBgkqhkiG9w0BAQwFAAOCAQEAcKC/Q1xV5zhfoKN0Gz22
# Ftf3v1cHvZqsoYcs7IVeqRq7IviHGmlUIu2kiHdtvRoU9BNKei8ttzjv9P+Aufih
# 9/Jy3iS8UgPITtAq3votVs/59PesMHqai7Je1M/RQ0SbQyHrlnKhSLSZy51PpwYD
# E3cnRNTnf+hZqPC/Lwum6fI0POz3A8eHqNJMQBk1RmppVLC4oVaO7KTVPeix3P0c
# 2PR3WlxUjG/voVA9/HYJaISfb8rbII01YBwCA8sgsKxYoA5AY8WYIsGyWfVVa88n
# q2x2zm8jLfR+cWojayL/ErhULSd+2DrZ8LaHlv1b0VysGMNNn3O3AamfV6peKOK5
# lDCCBq4wggSWoAMCAQICEAc2N7ckVHzYR6z9KGYqXlswDQYJKoZIhvcNAQELBQAw
# YjELMAkGA1UEBhMCVVMxFTATBgNVBAoTDERpZ2lDZXJ0IEluYzEZMBcGA1UECxMQ
# d3d3LmRpZ2ljZXJ0LmNvbTEhMB8GA1UEAxMYRGlnaUNlcnQgVHJ1c3RlZCBSb290
# IEc0MB4XDTIyMDMyMzAwMDAwMFoXDTM3MDMyMjIzNTk1OVowYzELMAkGA1UEBhMC
# VVMxFzAVBgNVBAoTDkRpZ2lDZXJ0LCBJbmMuMTswOQYDVQQDEzJEaWdpQ2VydCBU
# cnVzdGVkIEc0IFJTQTQwOTYgU0hBMjU2IFRpbWVTdGFtcGluZyBDQTCCAiIwDQYJ
# KoZIhvcNAQEBBQADggIPADCCAgoCggIBAMaGNQZJs8E9cklRVcclA8TykTepl1Gh
# 1tKD0Z5Mom2gsMyD+Vr2EaFEFUJfpIjzaPp985yJC3+dH54PMx9QEwsmc5Zt+Feo
# An39Q7SE2hHxc7Gz7iuAhIoiGN/r2j3EF3+rGSs+QtxnjupRPfDWVtTnKC3r07G1
# decfBmWNlCnT2exp39mQh0YAe9tEQYncfGpXevA3eZ9drMvohGS0UvJ2R/dhgxnd
# X7RUCyFobjchu0CsX7LeSn3O9TkSZ+8OpWNs5KbFHc02DVzV5huowWR0QKfAcsW6
# Th+xtVhNef7Xj3OTrCw54qVI1vCwMROpVymWJy71h6aPTnYVVSZwmCZ/oBpHIEPj
# Q2OAe3VuJyWQmDo4EbP29p7mO1vsgd4iFNmCKseSv6De4z6ic/rnH1pslPJSlREr
# WHRAKKtzQ87fSqEcazjFKfPKqpZzQmiftkaznTqj1QPgv/CiPMpC3BhIfxQ0z9JM
# q++bPf4OuGQq+nUoJEHtQr8FnGZJUlD0UfM2SU2LINIsVzV5K6jzRWC8I41Y99xh
# 3pP+OcD5sjClTNfpmEpYPtMDiP6zj9NeS3YSUZPJjAw7W4oiqMEmCPkUEBIDfV8j
# u2TjY+Cm4T72wnSyPx4JduyrXUZ14mCjWAkBKAAOhFTuzuldyF4wEr1GnrXTdrnS
# DmuZDNIztM2xAgMBAAGjggFdMIIBWTASBgNVHRMBAf8ECDAGAQH/AgEAMB0GA1Ud
# DgQWBBS6FtltTYUvcyl2mi91jGogj57IbzAfBgNVHSMEGDAWgBTs1+OC0nFdZEzf
# Lmc/57qYrhwPTzAOBgNVHQ8BAf8EBAMCAYYwEwYDVR0lBAwwCgYIKwYBBQUHAwgw
# dwYIKwYBBQUHAQEEazBpMCQGCCsGAQUFBzABhhhodHRwOi8vb2NzcC5kaWdpY2Vy
# dC5jb20wQQYIKwYBBQUHMAKGNWh0dHA6Ly9jYWNlcnRzLmRpZ2ljZXJ0LmNvbS9E
# aWdpQ2VydFRydXN0ZWRSb290RzQuY3J0MEMGA1UdHwQ8MDowOKA2oDSGMmh0dHA6
# Ly9jcmwzLmRpZ2ljZXJ0LmNvbS9EaWdpQ2VydFRydXN0ZWRSb290RzQuY3JsMCAG
# A1UdIAQZMBcwCAYGZ4EMAQQCMAsGCWCGSAGG/WwHATANBgkqhkiG9w0BAQsFAAOC
# AgEAfVmOwJO2b5ipRCIBfmbW2CFC4bAYLhBNE88wU86/GPvHUF3iSyn7cIoNqilp
# /GnBzx0H6T5gyNgL5Vxb122H+oQgJTQxZ822EpZvxFBMYh0MCIKoFr2pVs8Vc40B
# IiXOlWk/R3f7cnQU1/+rT4osequFzUNf7WC2qk+RZp4snuCKrOX9jLxkJodskr2d
# fNBwCnzvqLx1T7pa96kQsl3p/yhUifDVinF2ZdrM8HKjI/rAJ4JErpknG6skHibB
# t94q6/aesXmZgaNWhqsKRcnfxI2g55j7+6adcq/Ex8HBanHZxhOACcS2n82HhyS7
# T6NJuXdmkfFynOlLAlKnN36TU6w7HQhJD5TNOXrd/yVjmScsPT9rp/Fmw0HNT7ZA
# myEhQNC3EyTN3B14OuSereU0cZLXJmvkOHOrpgFPvT87eK1MrfvElXvtCl8zOYdB
# eHo46Zzh3SP9HSjTx/no8Zhf+yvYfvJGnXUsHicsJttvFXseGYs2uJPU5vIXmVnK
# cPA3v5gA3yAWTyf7YGcWoWa63VXAOimGsJigK+2VQbc61RWYMbRiCQ8KvYHZE/6/
# pNHzV9m8BPqC3jLfBInwAM1dwvnQI38AC+R2AibZ8GV2QqYphwlHK+Z/GqSFD/yY
# lvZVVCsfgPrA8g4r5db7qS9EFUrnEw4d2zc4GqEr9u3WfPwwggbGMIIErqADAgEC
# AhAKekqInsmZQpAGYzhNhpedMA0GCSqGSIb3DQEBCwUAMGMxCzAJBgNVBAYTAlVT
# MRcwFQYDVQQKEw5EaWdpQ2VydCwgSW5jLjE7MDkGA1UEAxMyRGlnaUNlcnQgVHJ1
# c3RlZCBHNCBSU0E0MDk2IFNIQTI1NiBUaW1lU3RhbXBpbmcgQ0EwHhcNMjIwMzI5
# MDAwMDAwWhcNMzMwMzE0MjM1OTU5WjBMMQswCQYDVQQGEwJVUzEXMBUGA1UEChMO
# RGlnaUNlcnQsIEluYy4xJDAiBgNVBAMTG0RpZ2lDZXJ0IFRpbWVzdGFtcCAyMDIy
# IC0gMjCCAiIwDQYJKoZIhvcNAQEBBQADggIPADCCAgoCggIBALkqliOmXLxf1knw
# FYIY9DPuzFxs4+AlLtIx5DxArvurxON4XX5cNur1JY1Do4HrOGP5PIhp3jzSMFEN
# MQe6Rm7po0tI6IlBfw2y1vmE8Zg+C78KhBJxbKFiJgHTzsNs/aw7ftwqHKm9MMYW
# 2Nq867Lxg9GfzQnFuUFqRUIjQVr4YNNlLD5+Xr2Wp/D8sfT0KM9CeR87x5MHaGjl
# RDRSXw9Q3tRZLER0wDJHGVvimC6P0Mo//8ZnzzyTlU6E6XYYmJkRFMUrDKAz200k
# heiClOEvA+5/hQLJhuHVGBS3BEXz4Di9or16cZjsFef9LuzSmwCKrB2NO4Bo/tBZ
# mCbO4O2ufyguwp7gC0vICNEyu4P6IzzZ/9KMu/dDI9/nw1oFYn5wLOUrsj1j6siu
# gSBrQ4nIfl+wGt0ZvZ90QQqvuY4J03ShL7BUdsGQT5TshmH/2xEvkgMwzjC3iw9d
# RLNDHSNQzZHXL537/M2xwafEDsTvQD4ZOgLUMalpoEn5deGb6GjkagyP6+SxIXuG
# Z1h+fx/oK+QUshbWgaHK2jCQa+5vdcCwNiayCDv/vb5/bBMY38ZtpHlJrYt/YYcF
# aPfUcONCleieu5tLsuK2QT3nr6caKMmtYbCgQRgZTu1Hm2GV7T4LYVrqPnqYklHN
# P8lE54CLKUJy93my3YTqJ+7+fXprAgMBAAGjggGLMIIBhzAOBgNVHQ8BAf8EBAMC
# B4AwDAYDVR0TAQH/BAIwADAWBgNVHSUBAf8EDDAKBggrBgEFBQcDCDAgBgNVHSAE
# GTAXMAgGBmeBDAEEAjALBglghkgBhv1sBwEwHwYDVR0jBBgwFoAUuhbZbU2FL3Mp
# dpovdYxqII+eyG8wHQYDVR0OBBYEFI1kt4kh/lZYRIRhp+pvHDaP3a8NMFoGA1Ud
# HwRTMFEwT6BNoEuGSWh0dHA6Ly9jcmwzLmRpZ2ljZXJ0LmNvbS9EaWdpQ2VydFRy
# dXN0ZWRHNFJTQTQwOTZTSEEyNTZUaW1lU3RhbXBpbmdDQS5jcmwwgZAGCCsGAQUF
# BwEBBIGDMIGAMCQGCCsGAQUFBzABhhhodHRwOi8vb2NzcC5kaWdpY2VydC5jb20w
# WAYIKwYBBQUHMAKGTGh0dHA6Ly9jYWNlcnRzLmRpZ2ljZXJ0LmNvbS9EaWdpQ2Vy
# dFRydXN0ZWRHNFJTQTQwOTZTSEEyNTZUaW1lU3RhbXBpbmdDQS5jcnQwDQYJKoZI
# hvcNAQELBQADggIBAA0tI3Sm0fX46kuZPwHk9gzkrxad2bOMl4IpnENvAS2rOLVw
# Eb+EGYs/XeWGT76TOt4qOVo5TtiEWaW8G5iq6Gzv0UhpGThbz4k5HXBw2U7fIyJs
# 1d/2WcuhwupMdsqh3KErlribVakaa33R9QIJT4LWpXOIxJiA3+5JlbezzMWn7g7h
# 7x44ip/vEckxSli23zh8y/pc9+RTv24KfH7X3pjVKWWJD6KcwGX0ASJlx+pedKZb
# NZJQfPQXpodkTz5GiRZjIGvL8nvQNeNKcEiptucdYL0EIhUlcAZyqUQ7aUcR0+7p
# x6A+TxC5MDbk86ppCaiLfmSiZZQR+24y8fW7OK3NwJMR1TJ4Sks3KkzzXNy2hcC7
# cDBVeNaY/lRtf3GpSBp43UZ3Lht6wDOK+EoojBKoc88t+dMj8p4Z4A2UKKDr2xpR
# oJWCjihrpM6ddt6pc6pIallDrl/q+A8GQp3fBmiW/iqgdFtjZt5rLLh4qk1wbfAs
# 8QcVfjW05rUMopml1xVrNQ6F1uAszOAMJLh8UgsemXzvyMjFjFhpr6s94c/MfRWu
# FL+Kcd/Kl7HYR+ocheBFThIcFClYzG/Tf8u+wQ5KbyCcrtlzMlkI5y2SoRoR/jKY
# pl0rl+CL05zMbbUNrkdjOEcXW28T2moQbh9Jt0RbtAgKh1pZBHYRoad3AhMcMYIE
# 9DCCBPACAQEwLzAbMRkwFwYDVQQDDBBES1NjcmlwdFNlbGZDZXJ0AhBsF6OnzZ1w
# t0cjkBXLKrLHMAkGBSsOAwIaBQCgeDAYBgorBgEEAYI3AgEMMQowCKACgAChAoAA
# MBkGCSqGSIb3DQEJAzEMBgorBgEEAYI3AgEEMBwGCisGAQQBgjcCAQsxDjAMBgor
# BgEEAYI3AgEVMCMGCSqGSIb3DQEJBDEWBBSKG7DwrX21ECIU6/345cP86w5apDAN
# BgkqhkiG9w0BAQEFAASCAQB4TTLHRS3zsUvD84PWvqX8GYhg35vO8HyerjN9Mv4G
# F1hF7RqHAGmYAjCyfqf+BTxbrfFaxh24EjVMgWVfKO/FhH8VEVHhstUAjbZECH42
# ecoqOWs3qbG9gxAONCjQ6gTPWQ74+sRaqvs+t2PTFGYhRN8+bsYXC91DjXcPp1pd
# g9bNzUKM/0NYWCL1ad7nMgoggBglkirfVRxZgLdacziNPGk67x18Xnv+kYiqeWO1
# U+gmr5x04w7M+Gb/1MDsjAaL6CfdEftCI8/5Rtp6IRaBCs0r18fZ/Lrcw7CEUzdA
# eyU4AhBlXHUPs4FQIhbMwtiBJqTk2Y6jPmKitJLBo5GjoYIDIDCCAxwGCSqGSIb3
# DQEJBjGCAw0wggMJAgEBMHcwYzELMAkGA1UEBhMCVVMxFzAVBgNVBAoTDkRpZ2lD
# ZXJ0LCBJbmMuMTswOQYDVQQDEzJEaWdpQ2VydCBUcnVzdGVkIEc0IFJTQTQwOTYg
# U0hBMjU2IFRpbWVTdGFtcGluZyBDQQIQCnpKiJ7JmUKQBmM4TYaXnTANBglghkgB
# ZQMEAgEFAKBpMBgGCSqGSIb3DQEJAzELBgkqhkiG9w0BBwEwHAYJKoZIhvcNAQkF
# MQ8XDTIyMDkwODE1MzEzMFowLwYJKoZIhvcNAQkEMSIEIDf6RCAPs1+8Zcun2Jio
# w+zPQI+GAnbfDdGRdwQdEwXEMA0GCSqGSIb3DQEBAQUABIICAKcOjiJc/IK2XaRM
# VF/TXmaAKULRe2lWhTwd6Hfde4DhdbOWgmRv0yqgfZsDG/QOKpRJSplEWIDOrfdP
# 72Wf3wXnr8Eqi+F7CtdnG5F2+G5/E/fWIEIqTJBF9cdbmctsltE30IPPrI0CGmEO
# dPbyCYC8J0MK1qxYdrA6Wf0fGUGJJLWbP7PEDEOlQQtGrB8OGEsOPwctiM5seguX
# zNvD0/vSTTP8rdPG1ZmhhEdsglmfdhzvjR2pN9x3Kab5MBPrCo5HZYCEan/40ZZR
# NHmdQa1JPwGqG3Cu5WQmWi0iQdX92NZD8J8ZLJr8Rhodq/1wHsHtdBSUbJ6FRnoS
# 4IbOR14AbZzomWK13Y3Xd4J8pszjP6IA8VkLf6kHnAhYZOrlwKHZL3SMBrhbZ3Jj
# oneklTseD2BWKXjalj0oYbBMaazLydgFZ0nyo8kTqcUu8sFWxkxgBOt7sIj/wOj1
# 1wVM6qCWmkRN0r/Kf1qon/Em1W63Y5Ptlz0f2oIRnbagxvjQRDHFwglBA/vmEQN+
# 6bA6MYVblGTvxBVxIyarXKshBdSXtJLVLxOnleSsUrJVu7zWFCe/v1RQhaOBMl9m
# F9CmH9sNWSqI8jzD9jJR6BnsXVqSRgbgcrbUgdItZ4TGaz4IY4xpJpJgebSTzPvZ
# lpRDenwMBLqvfNOw5CYb1LGo3w6S
# SIG # End signature block

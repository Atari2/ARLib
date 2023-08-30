$Arch = "x64"
$moduleAlreadyLoaded = Get-Module Microsoft.VisualStudio.DevShell
if ($moduleAlreadyLoaded) {
    Write-Error "Module with the same name already loaded, cannot load another one because there will be assemblies conflicts"
    return
}
$assemblyPresent = [System.AppDomain]::CurrentDomain.GetAssemblies() | Where-Object { $_.FullName -like '*Microsoft.VisualStudio.DevShell*' }
if ($assemblyPresent) {
    Write-Error "Assembly with the same name already loaded, cannot load another one because there will be assemblies conflicts"
    return
}

if ($Pre) {
    $vsPath = &"${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe" -prerelease -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationpath
} else {
    $vsPath = &"${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationpath
}
$commonLocation = "$vsPath\Common7\Tools\Microsoft.VisualStudio.DevShell.dll"
if (Test-Path $commonLocation) {
    $dllPath = $commonLocation
} else {
    $dllPath = (Get-ChildItem $vsPath -Recurse -File -Filter Microsoft.VisualStudio.DevShell.dll).FullName
}
Import-Module -Force $dllPath
Enter-VsDevShell -VsInstallPath $vsPath -SkipAutomaticLocation -DevCmdArguments "-arch=$Arch"
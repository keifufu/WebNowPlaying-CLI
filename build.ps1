function Invoke-Environment {
  param
  (
    [Parameter(Mandatory=$true)]
    [string] $Command
  )

  $Command = "`"" + $Command + "`""
  cmd /c "$Command > nul 2>&1 && set" | . { process {
    if ($_ -match '^([^=]+)=(.*)') {
      [System.Environment]::SetEnvironmentVariable($matches[1], $matches[2])
    }
  }}
}

$origEnv = Get-ChildItem Env:*;
$vsInstallationPath = & 'vswhere.exe' -property installationPath

if (-not $vsInstallationPath) {
  Write-Host "No Visual Studio installation found."
  exit 1
}

if (-Not (Test-Path "$PSScriptRoot/libwnp")) {
  git clone https://github.com/keifufu/WebNowPlaying-Library "$PSScriptRoot/libwnp"
  & "$PSScriptRoot\libwnp\build.ps1"
}
$LIBWNP_DIR = "$PSScriptRoot\libwnp\build\win64\lib\cmake\libwnp"

Remove-Item -Path "$PSScriptRoot\build" -Recurse -Force -ErrorAction SilentlyContinue
Remove-Item -Path "$PSScriptRoot\dist" -Recurse -Force -ErrorAction SilentlyContinue

New-Item -Path "$PSScriptRoot\build" -ItemType Directory
New-Item -Path "$PSScriptRoot\dist" -ItemType Directory

Invoke-Environment "$vsInstallationPath\VC\Auxiliary\Build\vcvars64.bat"
cmake -S "$PSScriptRoot" -B "$PSScriptRoot\build" -DCMAKE_INSTALL_PREFIX="$PSScriptRoot\build" -DCMAKE_BUILD_TYPE=Release -G "NMake Makefiles" -Dlibwnp_DIR="$LIBWNP_DIR"
cmake --build "$PSScriptRoot\build"
cmake --install "$PSScriptRoot\build"
cpack -B "$PSScriptRoot\dist" --config "$PSScriptRoot\build\CPackConfig.cmake"
Remove-Item -Path "$PSScriptRoot\dist\_CPack_Packages" -Recurse -Force -ErrorAction SilentlyContinue
$origEnv | ForEach-Object { [System.Environment]::SetEnvironmentVariable($_.Name, $_.Value) }

$files = @(
  "$PSScriptRoot\README.md",
  "$PSScriptRoot\LICENSE",
  "$PSScriptRoot\CHANGELOG.md",
  "$PSScriptRoot\VERSION",
  "$PSScriptRoot\build\bin"
)
  
$version = (Get-Content "$PSScriptRoot\VERSION" | Out-String).Trim()
Compress-Archive -Path $files -DestinationPath "$PSScriptRoot\dist\wnpcli-${version}_win64.zip" -CompressionLevel Optimal

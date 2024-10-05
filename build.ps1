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

Remove-Item -Path "$PSScriptRoot\build" -Recurse -Force
New-Item -Path "$PSScriptRoot\build" -ItemType Directory
New-Item -Path "$PSScriptRoot\build\x64" -ItemType Directory
New-Item -Path "$PSScriptRoot\build\x86" -ItemType Directory
Remove-Item -Path "$PSScriptRoot\dist" -Recurse -Force
New-Item -Path "$PSScriptRoot\dist" -ItemType Directory
New-Item -Path "$PSScriptRoot\dist\x64" -ItemType Directory
New-Item -Path "$PSScriptRoot\dist\x86" -ItemType Directory

Invoke-Environment "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars32.bat"
Set-Location "$PSScriptRoot\build\x86"
cmake -G "NMake Makefiles" -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX="$PSScriptRoot\dist\x86" "$PSScriptRoot"
cmake --build .
cmake --install .

$origEnv | ForEach-Object {
  [System.Environment]::SetEnvironmentVariable($_.Name, $_.Value)
}

Invoke-Environment "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"
Set-Location "$PSScriptRoot\build\x64"
cmake -G "NMake Makefiles" -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX="$PSScriptRoot\dist\x64" "$PSScriptRoot"
cmake --build .
cmake --install .

$origEnv | ForEach-Object {
  [System.Environment]::SetEnvironmentVariable($_.Name, $_.Value)
}

Set-Location "$PSScriptRoot"

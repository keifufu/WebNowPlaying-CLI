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

Invoke-Environment "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"
make win64

$origEnv | ForEach-Object {
  [System.Environment]::SetEnvironmentVariable($_.Name, $_.Value)
}
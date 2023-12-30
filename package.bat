@echo off

mkdir dist
set /p VERSION=<VERSION

set archive64=dist\wnpcli-%VERSION%_win64.zip
if exist %archive64% del %archive64%
7z a %archive64% build\wnpcli_win64.exe CHANGELOG.md README.md LICENSE VERSION
7z rn %archive64% build\wnpcli_win64.exe wnpcli.exe

@echo off
if "%1"=="" echo Copies source files to destination
if "%1"=="" echo.
if "%1"=="" echo CopyToUSB {destination}
if "%1"=="" echo.
if "%1"=="" echo e.g.: copyToUSB g:\Dev
if "%1"=="" goto :EOF

xcopy "c:\Documents and Settings\GPM\My Documents\Dev" "%1" /s/d/i/r/y /EXCLUDE:BackupExclude.txt

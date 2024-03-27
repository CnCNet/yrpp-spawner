@if not defined _echo echo off

rem Cleans build folders.

rem Ensure we're in correct directory.
cd /D "%~dp0"
cd ..

if exist Debug\ rmdir /S /Q Debug\
if exist DevBuild\ rmdir /S /Q DevBuild\
if exist Release\ rmdir /S /Q Release\
if exist Debug-HardEnd\ rmdir /S /Q Debug-HardEnd\
if exist DevBuild-HardEnd\ rmdir /S /Q DevBuild-HardEnd\
if exist Release-HardEnd\ rmdir /S /Q Release-HardEnd\

@if not defined _echo echo off

rem Cleans build folders.

rem Ensure we're in correct directory.
cd /D "%~dp0"
cd ..

if exist Debug\ rmdir /S /Q Debug\
if exist Debug-Hardened\ rmdir /S /Q Debug-Hardened\
if exist Nightly\ rmdir /S /Q Nightly\
if exist Nightly-Hardened\ rmdir /S /Q Nightly-Hardened\
if exist Release\ rmdir /S /Q Release\
if exist Release-Hardened\ rmdir /S /Q Release-Hardened\
if exist Debug-CnCNetYR\ rmdir /S /Q Debug-CnCNetYR\
if exist Debug-CnCNetYR-Hardened\ rmdir /S /Q Debug-CnCNetYR-Hardened\
if exist Nightly-CnCNetYR\ rmdir /S /Q Nightly-CnCNetYR\
if exist Nightly-CnCNetYR-Hardened\ rmdir /S /Q Nightly-CnCNetYR-Hardened\
if exist Release-CnCNetYR\ rmdir /S /Q Release-CnCNetYR\
if exist Release-CnCNetYR-Hardened\ rmdir /S /Q Release-CnCNetYR-Hardened\

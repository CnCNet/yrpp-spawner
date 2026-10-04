@if not defined _echo echo off

rem Builds YRpp-Spawner Nightly-CnCNetYR.

rem Ensure we're in correct directory.
cd /D "%~dp0"

call build Nightly-CnCNetYR

@if not defined _echo echo off

rem Builds YRpp-Spawner DevBuild-CnCNetYR-Hardened.

rem Ensure we're in correct directory.
cd /D "%~dp0"

call build DevBuild-CnCNetYR-Hardened

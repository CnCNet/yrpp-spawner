@if not defined _echo echo off

rem Builds YRpp-Spawner Debug-Hardened.

rem Ensure we're in correct directory.
cd /D "%~dp0"

call build Debug-Hardened

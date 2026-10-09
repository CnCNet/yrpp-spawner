@if not defined _echo echo off

rem Builds YRpp-Spawner Nightly-Hardened.

rem Ensure we're in correct directory.
cd /D "%~dp0"

call build Nightly-Hardened

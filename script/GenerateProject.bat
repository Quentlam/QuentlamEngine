@echo off
setlocal
pushd "%~dp0.."
powershell -ExecutionPolicy Bypass -File ".\script\BootstrapProject.ps1" -RepoRoot "%cd%"
set "EXIT_CODE=%ERRORLEVEL%"
popd
exit /b %EXIT_CODE%

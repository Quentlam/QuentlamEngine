@echo off
setlocal
pushd "%~dp0.."
powershell -ExecutionPolicy Bypass -File ".\script\BootstrapProject.ps1" -RepoRoot "%~dp0.."
set "EXIT_CODE=%ERRORLEVEL%"
popd
exit /b %EXIT_CODE%

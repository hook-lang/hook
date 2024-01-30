@echo off
setlocal

rem ------------------------------------------------------------------------------
rem Installation script for Hook
rem ------------------------------------------------------------------------------

set BASE_URL=https://github.com/hook-lang/hook/releases/download
set VERSION=0.1.0
set ARCH=x64

rem ------------------------------------------------------------------------------
rem Download and install
rem ------------------------------------------------------------------------------

set base_name=hook-%VERSION%-windows-%ARCH%
set dist_url=%BASE_URL%/%VERSION%/%base_name%.tar.gz
set temp_file=%TEMP%\hook-dist.tar.gz

echo "Downloading: %dist_url%"

curl --proto =https --tlsv1.2 -f -L -o "%temp_file%"  "%dist_url%"
if errorlevel 1 (
  echo "Unable to download: %dist_url%"
  goto end
)

echo "Unpacking.."

tar -xzf "%temp_file%"
if errorlevel 1 (
  echo "Extraction failed."
  goto end
)

set home_dir=%SystemDrive%\hook

echo "Installing to: %home_dir%"

move %base_name% %home_dir% >nul

echo "Cleaning up.."

del /q "%temp_file%"

rem ------------------------------------------------------------------------------
rem Setup the environment variable
rem ------------------------------------------------------------------------------

echo "Setting the environment variable: HOOK_HOME"

echo "%PATH%" | find "%home_dir%\bin">nul
if not errorlevel 1 goto done

where /q powershell
if not errorlevel 1 (

  set HOOK_HOME=%home_dir%
  powershell.exe -NoProfile -ExecutionPolicy Bypass -Command "[Environment]::SetEnvironmentVariable('HOOK_HOME',\""%home_dir%\"",'User');"

  set semicolon=;
  if "%PATH:~-1%"==";" (set semicolon=)
  set "PATH=%PATH%%semicolon%%home_dir%\bin"
  powershell.exe -NoProfile -ExecutionPolicy Bypass -Command "[Environment]::SetEnvironmentVariable('PATH',\""$([Environment]::GetEnvironmentVariable('PATH','User'))%semicolon%%home_dir%\bin\"",'User');"

  if not errorlevel 1 goto done
)

echo "Please add %home_dir%\bin to your PATH environment variable."
:done

rem ------------------------------------------------------------------------------
rem End
rem ------------------------------------------------------------------------------

echo "Install successful."
echo "You can check the installation by running 'hook --version'."
echo "Enjoy!"

endlocal & (
  set "HOOK_HOME=%HOOK_HOME%"
  set "PATH=%PATH%"
)

:end

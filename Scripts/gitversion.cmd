:: Get commit/branch info from git for build version info
:: Expects one argument - path for utils directory

@ECHO OFF

IF "%1"=="" ECHO Missing required path to utils project directory. & GOTO EXIT
IF NOT EXIST "%1\version_info.tmpl" ECHO Didn't find template file at '%1\version_info.tmpl' & GOTO EXIT

SET TEMP_VERSIONINFO_FILE="%TEMP%\version.%RANDOM%.txt"

CALL where git.exe > NUL: 2> NUL:
IF %ERRORLEVEL% NEQ 0 ECHO Didn't find git.exe to gather commit information. & GOTO NOGIT

:: Seed temp version info file with branch name
git rev-parse --abbrev-ref HEAD > %TEMP_VERSIONINFO_FILE%
:: Append short commit hash and date to version info file (use --pretty=format to prevent extra newline)
git show --no-patch --pretty="format:%%h%%n%%ai%%n" HEAD >> %TEMP_VERSIONINFO_FILE%
:: Append length (count) of commit chain to version info file for revision number
git rev-list --count HEAD >> %TEMP_VERSIONINFO_FILE%

GOTO TEMPLATE

:NOGIT

:: butt redirects up against text to prevent extraneous spacing
ECHO unknown branch> %TEMP_VERSIONINFO_FILE%
ECHO unknown>> %TEMP_VERSIONINFO_FILE%
ECHO date unknown>> %TEMP_VERSIONINFO_FILE%
ECHO 99999>> %TEMP_VERSIONINFO_FILE%

GOTO TEMPLATE

:TEMPLATE

:: Use Windows [Console] Script Host to execute jscript which replaces macros in the template with actual info
:: %~dp0 represents the path of this script, gitversion.js should be in the same directory
cscript //NoLogo %~dp0\gitversion.js %TEMP_VERSIONINFO_FILE% "%1\version_info.tmpl" "%1\version_info.h"

:EXIT

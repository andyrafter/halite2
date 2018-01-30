@setlocal
@setlocal enableextensions
@setlocal EnableDelayedExpansion
@echo off
set SCRIPT_DIR=%~dp0
set CPPLINT=%SCRIPT_DIR%\cpplint.py
set OUTDIR=%CD%
set INFO_FILE=%CD%\cpplint_info.txt
set ERROR_FILE=%CD%\cpplint_error.txt

:mode_one
for /f %%G in ('dir raf /s /b ^| findstr /i "\.cpp$ \.cc$ \.h$"') do cpplint.py --root=src --filter=-whitespace,-legal/copyright --counting=detailed %%G
goto :finish

:mode_two
rem for /f "tokens=*" %%F  in ('dir /AD /b') do call set directory_list=%%directory_list%% "%%F"
rem for /f "tokens=*" %%F in ('dir /b /s *.cpp') do call set expanded_list=%%expanded_list%% "%%F"
rem echo %directory_list%
rem for /f "tokens=*" %%F  in ('dir /AD /b /s') do call :runcpplint "%%F"

for /D /r %%F in (*) do call :runcpplint "%%F"
goto :finish


:mode_three


goto :finish


REM subroutines
:runcpplint
setlocal
pushd . 
rem echo %1
cd %1
rem for /f %%G in ('dir /s /b *.cpp') do cpplint.py --counting=detailed --filter=-whitespace %%G
rem for /f "tokens=*" %%F  in ('dir /b *.cpp') do call set directory_list=%%directory_list:%CD%=%% "%%F"
rem for /f %%G in ('dir /b *.cpp') do (
for %%G in (*.cpp) do (
call set file_list=%%file_list%% "%%G"
)
if not "!file_list!"=="" (
%CPPLINT% --counting=detailed --filter=-whitespace %file_list%
rem echo %file_list%
)
rem echo %file_list%
endlocal
popd

goto :finish


:finish

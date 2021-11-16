@echo off

@if exist .version goto :exit

@set topv=%1
@if "%topv%"=="" set topv=v1.1
rem set _d=%DATE:~2,2%%DATE:~5,2%%DATE:~8,2%
rem set _t=%TIME:~0,2%%TIME:~3,2%%TIME:~6,2%
rem set _ver=%DATE:~2,2%
rem set /a v=0x%_ver%
@set /a _dy=%DATE:~2,2%
@set /a _dm=%DATE:~5,2%
@set /a _dd=%DATE:~8,2%
@set /a _th=%TIME:~0,2%
@set /a _tm=%TIME:~3,2%
@set /a _ts=%TIME:~6,2%
rem @echo %_dy%%_dm%%_dd%%_th%_%_tm%%_ts%

@call :TO_HEX %_dy%
@set v=%_hex%
@call :TO_HEX %_dm%
@set v=%v%%_hex%
@call :TO_HEX %_dd%
@set v=%v%%_hex%.
@call :TO_HEX %_th%
@set v=%v%%_hex%
@call :TO_HEX %_tm%
@set v=%v%%_hex%
@echo "%topv%.%v%" > .version


:TO_HEX
@set _n=%1
@set /a _q=_n/16
@set /a _r=_n - _q*16
@set _hq=%_q%
@set _hr=%_r%
@if "%_q%" == "10" set _hq=A
@if "%_q%" == "11" set _hq=B
@if "%_q%" == "12" set _hq=C
@if "%_q%" == "13" set _hq=D
@if "%_q%" == "14" set _hq=E
@if "%_q%" == "15" set _hq=F
@if "%_r%" == "10" set _hr=A
@if "%_r%" == "11" set _hr=B
@if "%_r%" == "12" set _hr=C
@if "%_r%" == "13" set _hr=D
@if "%_r%" == "14" set _hr=E
@if "%_r%" == "15" set _hr=F
@set _hex=%_hq%%_hr%

:exit
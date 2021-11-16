@echo off

@set PLATFORMTARGET=%1
@set MTIF_PATH=..\mtif\
@echo --------------------------------------------------------------------------------
@echo  %PLATFORMTARGET%
@echo ================================================================================

@copy /Y %MTIF_PATH%\%PLATFORMTARGET%\.version                        .
@copy /Y %MTIF_PATH%\%PLATFORMTARGET%\.secret.key*                    .
@copy /Y %MTIF_PATH%\%PLATFORMTARGET%\common.h                        .


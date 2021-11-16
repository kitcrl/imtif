@set TARGET=%1
@set PLATFORMTARGET=%2
@set PACKAGENAME=%3
@set /p txt=<.version
@set _ver=%txt:~1,16%


@set MTIF_PATH=..\..\..\mtif\

if not exist %MTIF_PATH% mkdir %MTIF_PATH%

@copy /Y out\%TARGET%           %MTIF_PATH%
@copy /Y m\zio.x.h             %MTIF_PATH%

@echo ----------------------------------
@echo  %TARGET% %PLATFORMTARGET%  %_ver%
@echo ==================================

if "%PLATFORMTARGET%" == "x86" (
@..\..\xtools\zioXmake\out\zioXmake.x86.exe out\zio.x.sifr.cold.x86 -post out\%TARGET% out\%TARGET%.dll
)
if "%PLATFORMTARGET%" == "x86_64" (
@..\..\xtools\zioXmake\out\zioXmake.x86_64.exe out\zio.x.sifr.cold.x86_64 -post out\%TARGET% out\%TARGET%.dll
)

@copy /Y out\%TARGET%.dll        %MTIF_PATH%
@copy /Y out\%TARGET%.dll        %MTIF_PATH%\%TARGET%.%_ver%.dll


if NOT "%PACKAGENAME%"=="" (
@copy /Y out\%TARGET%            %MTIF_PATH%\%PACKAGENAME%.%PLATFORMTARGET%
@copy /Y out\%TARGET%            %MTIF_PATH%\%PACKAGENAME%.%PLATFORMTARGET%.%_ver%.dll
@copy /Y out\%TARGET%.dll        %MTIF_PATH%\%PACKAGENAME%.%PLATFORMTARGET%.dll
)

:exit
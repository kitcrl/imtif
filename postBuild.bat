@set PROJECTNAME=%1
@set PLATFORMTARGET=%2
@set PACKAGENAME=%3
@set /p txt=<.version
@set _ver=%txt:~1,16%
@set TARGET=%PROJECTNAME%.%PLATFORMTARGET%

@set MTIF_PATH=..\mtif\

@echo --------------------------------------------------------------------------------
@echo  %PROJECTNAME% %TARGET% %PLATFORMTARGET%  %_ver%
@echo ================================================================================

if not exist %MTIF_PATH% mkdir %MTIF_PATH%
if not exist %MTIF_PATH%\%PLATFORMTARGET% mkdir %MTIF_PATH%\%PLATFORMTARGET%

@copy /Y i\*.h                      %MTIF_PATH%\%PLATFORMTARGET%;
@copy /Y out\%TARGET%.lib           %MTIF_PATH%\%PLATFORMTARGET%;
@copy /Y out\%TARGET%.lib           %MTIF_PATH%\%PLATFORMTARGET%\%PROJECTNAME%.lib;
@copy /Y out\%TARGET%.lib           %MTIF_PATH%\%PLATFORMTARGET%\%TARGET%.%_ver%.lib;

if NOT "%PACKAGENAME%"=="" (
@copy /Y out\%TARGET%.lib           %MTIF_PATH%\%PLATFORMTARGET%\%PACKAGENAME%.lib
@copy /Y out\%TARGET%.lib           %MTIF_PATH%\%PLATFORMTARGET%\%PACKAGENAME%.%PLATFORMTARGET%.lib
@copy /Y out\%TARGET%.lib           %MTIF_PATH%\%PLATFORMTARGET%\%PACKAGENAME%.%PLATFORMTARGET%.%_ver%.lib
)

:exit

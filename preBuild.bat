@echo off

@set PARENT_PATH=..

@if not exist then mkdir h

@copy /Y %PARENT_PATH%\io\libio\u\*.h              h
@copy /Y %PARENT_PATH%\io\libio\m\*.h              h
@copy /Y %PARENT_PATH%\z\m\*.h                  h
@copy /Y %PARENT_PATH%\z\xm\*.h                 h

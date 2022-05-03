@ECHO OFF
FOR /r %%d IN (*.comp) DO @CALL compile-all.worker.bat "%%d"
REM .
FOR /r %%d IN (*.frag) DO @CALL compile-all.worker.bat "%%d"
REM .
FOR /r %%d IN (*.vert) DO @CALL compile-all.worker.bat "%%d"
REM .
DEL _compile-all.bat.txt
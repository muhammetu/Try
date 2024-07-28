@echo off
copy /y MakeFile_VC_Release.bat UiScriptLib\cyTemp\
cd UiScriptLib\cyTemp\
call MakeFile_VC_Release.bat

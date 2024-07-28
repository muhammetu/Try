@echo off
mkdir out
copy /y UiScriptLib\PythonUiScriptLibManager.cpp out\
copy /y UiScriptLib\PythonUiScriptLibManager.h out\
copy /y UiScriptLib\cyTemp\uiscriptlib.lib out\
echo copy completed
exit

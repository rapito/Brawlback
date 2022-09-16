set ORIGINAL_DIR=%CD% 


call "get_settings.bat"

:: set target for this build. all means build everything, you can also specify a specific project
set buildTarget=all

:: cmake build - got this command from the console output of running cmake build from the command palette
set cmakebuild="%CMAKE_EXE_PATH%" --build %CD%/build --config Debug --target %buildTarget% -j 14 --

%cmakebuild% && cd /d %~dp0 && call "%~dp0MakeSD\main.bat"

cd /d %ORIGINAL_DIR%

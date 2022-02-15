SET POSSIBLE_PATHS="!ROOT!\..\..\LuaJIT\" "!ROOT!\..\LuaJIT\" "!ProgramFiles!\LuaJIT\" "!ProgramFiles(x86)!\LuaJIT\" ^
"!ProgramFiles!\Lua\" "!ProgramFiles(x86)!\Lua\" "!ProgramFiles!\Lua\5.1\" "!ProgramFiles(x86)!\Lua\5.1\"
SET POSSIBLE_LIBPATHS=".\" "lib\" "src\"
SET POSSIBLE_INCPATHS=".\" "include\" "src\"
SET POSSIBLE_LIBS="lua51.lib" "lua5.1.lib"

FOR %%a IN (%POSSIBLE_PATHS%) DO (
	IF EXIST %%a (
		CALL :testpath %%a
		IF "!ERRORLEVEL!"=="0" GOTO done
	)
)
GOTO :fail

:done
EXIT /b 0

:testpath
SET __SUBINC=
SET __SUBLIB=
SET __SUBLIBPATH=
FOR %%a IN (%POSSIBLE_INCPATHS%) DO (
	IF EXIST %1\%%a\lua.h (
		SET __SUBINC=%%a
		GOTO :incfound
	)
)
GOTO subfail

:incfound
FOR %%a IN (%POSSIBLE_LIBPATHS%) DO (
	FOR %%b IN (%POSSIBLE_LIBS%) DO (
		IF EXIST %1\%%a\%%b (
			SET __SUBLIBPATH=%%a
			SET __SUBLIB=%%b
			GOTO :libfound
		)
	)
)
GOTO subfail

:libfound
SET LUAPATH=%1
SET LUAPATH=%LUAPATH:~1,-1%
ECHO Using Lua from %LUAPATH%
SET LIBS=!__SUBLIB:~1,-1! !LIBS!
SET LIB=%LUAPATH%!__SUBLIBPATH:~1,-1!;!LIB!
SET INCLUDE=%LUAPATH%!__SUBINC:~1,-1!;..;!INCLUDE!
SET __DLLNAME=!__SUBLIB:~1,-4!dll
IF NOT EXIST "!SERVER_OUTROOT!\!__DLLNAME!" (
	IF EXIST "%LUAPATH%!__SUBLIBPATH:~1,-1!\!__DLLNAME!" (
		COPY "%LUAPATH%!__SUBLIBPATH:~1,-1!\!__DLLNAME!" "!SERVER_OUTROOT!\!__DLLNAME!"
	)
)
EXIT /b 0

:subfail
EXIT /b 1

:fail
ECHO Lua not found
EXIT /b 1

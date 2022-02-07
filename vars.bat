set POSSIBLE_PATHS="!ROOT!\..\..\LuaJIT\" "!ROOT!\..\LuaJIT\" "!ProgramFiles!\LuaJIT\" "!ProgramFiles(x86)!\LuaJIT\" ^
"!ProgramFiles!\Lua\" "!ProgramFiles(x86)!\Lua\" "!ProgramFiles!\Lua\5.1\" "!ProgramFiles(x86)!\Lua\5.1\"
set POSSIBLE_LIBPATHS=".\" "lib\" "src\"
set POSSIBLE_INCPATHS=".\" "include\" "src\"
set POSSIBLE_LIBS="lua51.lib" "lua5.1.lib"

FOR %%a IN (%POSSIBLE_PATHS%) DO (
	IF EXIST %%a (
		CALL :testpath %%a
		IF "!ERRORLEVEL!"=="0" GOTO done
	)
)
GOTO :fail

:done
exit /b 0

:testpath
set __SUBINC=
set __SUBLIB=
set __SUBLIBPATH=
FOR %%a IN (%POSSIBLE_INCPATHS%) DO (
	IF EXIST %1\%%a\lua.h (
		set __SUBINC=%%a
		GOTO :incfound
	)
)
GOTO subfail

:incfound
FOR %%a IN (%POSSIBLE_LIBPATHS%) DO (
	FOR %%b IN (%POSSIBLE_LIBS%) DO (
		IF EXIST %1\%%a\%%b (
			set __SUBLIBPATH=%%a
			set __SUBLIB=%%b
			GOTO :libfound
		)
	)
)
GOTO subfail

:subfail
exit /b 1

:fail
echo Lua not found
exit /b 1

:libfound
set LUAPATH=%1
set LUAPATH=%LUAPATH:~1,-1%
echo Using Lua from %LUAPATH%
set LIBS=!__SUBLIB:~1,-1! !LIBS!
set LIB=%LUAPATH%!__SUBLIBPATH:~1,-1!;!LIB!
set INCLUDE=%LUAPATH%!__SUBINC:~1,-1!;!INCLUDE!

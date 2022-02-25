:detectlua
SET POSSIBLE_PATHS="!ROOT!\..\..\LuaJIT\" "!ROOT!\..\LuaJIT\" ^
"!ProgramFiles!\LuaJIT\" "!ProgramFiles(x86)!\LuaJIT\" ^
"!ROOT!\..\..\Lua\" "!ROOT!\..\Lua\" ^
"!ROOT!\..\..\Lua\5.1\" "!ROOT!\..\Lua\5.1\" ^
"!ROOT!\..\..\Lua\5.2\" "!ROOT!\..\Lua\5.2\" ^
"!ROOT!\..\..\Lua\5.3\" "!ROOT!\..\Lua\5.3\" ^
"!ProgramFiles!\Lua\" "!ProgramFiles(x86)!\Lua\" ^
"!ProgramFiles!\Lua\5.1\" "!ProgramFiles(x86)!\Lua\5.1\" ^
"!ProgramFiles!\Lua\5.2\" "!ProgramFiles(x86)!\Lua\5.2\" ^
"!ProgramFiles!\Lua\5.3\" "!ProgramFiles(x86)!\Lua\5.3\"

SET POSSIBLE_LIBPATHS=".\" "lib\" "src\"
SET POSSIBLE_INCPATHS=".\" "include\" "src\"
SET POSSIBLE_LIBS="lua51." "lua5.1." ^
"lua52." "lua5.2." "lua53." "lua5.3."

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
		IF EXIST "%1\%%a\%%blib" (
			SET __SUBLIBPATH=%%a
			SET __SUBLIBPATH=!__SUBLIBPATH:~1,-1!
			SET __SUBLIB=%%b
			SET __SUBLIB=!__SUBLIB:~1,-1!
			GOTO :libfound
		)
	)
)
GOTO subfail

:libfound
SET LUAPATH=%1
SET LUAPATH=%LUAPATH:~1,-1%
ECHO Using Lua from %LUAPATH%
SET LIBS=!__SUBLIB!lib !LIBS!
SET LIB=%LUAPATH%!__SUBLIBPATH!;!LIB!
SET INCLUDE=%LUAPATH%!__SUBINC:~1,-1!;..;!INCLUDE!
IF NOT EXIST "!SERVER_OUTROOT!\!__SUBLIB!dll" (
	IF EXIST "%LUAPATH%!__SUBLIBPATH!\!__SUBLIB!dll" (
		COPY "%LUAPATH%!__SUBLIBPATH!\!__SUBLIB!dll" "!SERVER_OUTROOT!\!__SUBLIB!dll"
	)
)
IF NOT EXIST "!SERVER_OUTROOT!\!__SUBLIB!pdb" IF "!DEBUG!"=="1" (
	IF EXIST "%LUAPATH%!__SUBLIBPATH!\!__SUBLIB!pdb" (
		COPY "%LUAPATH%!__SUBLIBPATH!\!__SUBLIB!pdb" "!SERVER_OUTROOT!\!__SUBLIB!pdb"
	)
)
EXIT /b 0

:subfail
EXIT /b 1

:fail
ECHO Lua not found
ECHO Would you like the script to automatically clone and build LuaJIT?
ECHO Note: The LuaJIT will be cloned from Mike Pall's GitHub, then compiled.
ECHO Warning: If "..\LuaJIT" exists it will be removed!
SET /P LQUESTION="[Y/n]>"
IF "!ZQUESTION!"=="" GOTO downlj
IF "!ZQUESTION!"=="y" GOTO downlj
IF "!ZQUESTION!"=="Y" GOTO downlj
EXIT /b 1

:downlj
IF "%GITOK%"=="0" (
	ECHO Looks like you don't have Git for Windows
	ECHO You can download it from https://git-scm.com/download/win
) ELSE (
	RMDIR /S /Q "..\LuaJIT"
	git clone https://luajit.org/git/luajit.git "..\LuaJIT"
	GOTO buildlj
)
EXIT /b 1

:buildlj
set LJBUILDFLAGS=
IF "!DEBUG!"=="1" (
	set LJBUILDFLAGS=debug
)
PUSHD ..\LuaJIT\src\
CALL msvcbuild !LJBUILDFLAGS!
POPD
GOTO detectlua

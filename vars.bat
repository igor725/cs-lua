:detectlua
SET POSSIBLE_LIBS="lualib." 
SET PREFER_LVERSION=

IF NOT "%PLUGIN_ARGS%"=="" (
	FOR %%a IN (%PLUGIN_ARGS%) DO (
		SET PREFER_LVERSION=%%a
	)
)

IF NOT "!PREFER_LVERSION!"=="" (
	IF "!PREFER_LVERSION!"=="jit2" (
		GOTO :lj2
	) ELSE IF "!PREFER_LVERSION!"=="jit" (
		GOTO :lj
	) ELSE IF "!PREFER_LVERSION!"=="51" (
		GOTO :l51
	) ELSE IF "!PREFER_LVERSION!"=="52" (
		GOTO :l52
	) ELSE IF "!PREFER_LVERSION!"=="53" (
		GOTO :l53
	) ELSE IF "!PREFER_LVERSION!"=="54" (
		GOTO :l54
	) ELSE (
		ECHO Invalid Lua version specified
		EXIT /b 1
	)
)

:lj2
@REM LuaJIT форк от OpenResty
SET POSSIBLE_LIBS=!POSSIBLE_LIBS!"lua51." "lua5.1."
SET POSSIBLE_PATHS="!ROOT!\..\..\luajit2\" "!ROOT!\..\luajit2\" 
SET POSSIBLE_PATHS=!POSSIBLE_PATHS!"!ROOT!\..\..\Lua\luajit2\" "!ROOT!\..\Lua\luajit2\" 
SET POSSIBLE_PATHS=!POSSIBLE_PATHS!"!ProgramFiles!\luajit2\" "!ProgramFiles(x86)!\luajit2\" 
IF NOT "!PREFER_LVERSION!"=="" GOTO lpathdone

:lj
@REM LuaJIT у Майка застоппился на Lua 5.1
SET POSSIBLE_LIBS=!POSSIBLE_LIBS!"lua51." "lua5.1."
SET POSSIBLE_PATHS="!ROOT!\..\..\LuaJIT\" "!ROOT!\..\LuaJIT\" 
SET POSSIBLE_PATHS=!POSSIBLE_PATHS!"!ROOT!\..\..\Lua\JIT\" "!ROOT!\..\Lua\JIT\" 
SET POSSIBLE_PATHS=!POSSIBLE_PATHS!"!ROOT!\..\..\Lua\LuaJIT\" "!ROOT!\..\Lua\LuaJIT\" 
SET POSSIBLE_PATHS=!POSSIBLE_PATHS!"!ProgramFiles!\LuaJIT\" "!ProgramFiles(x86)!\LuaJIT\" 
IF NOT "!PREFER_LVERSION!"=="" GOTO lpathdone

:l54
SET POSSIBLE_LIBS=!POSSIBLE_LIBS!"lua54." "lua5.4." 
SET POSSIBLE_PATHS=!POSSIBLE_PATHS!"!ROOT!\..\..\Lua\5.4\" "!ROOT!\..\Lua\5.4\" 
SET POSSIBLE_PATHS=!POSSIBLE_PATHS!"!ProgramFiles!\Lua\5.4\" "!ProgramFiles(x86)!\Lua\5.4\" 
IF NOT "!PREFER_LVERSION!"=="" GOTO lpathdone

:l53
SET POSSIBLE_LIBS=!POSSIBLE_LIBS!"lua53." "lua5.3." 
SET POSSIBLE_PATHS=!POSSIBLE_PATHS!"!ROOT!\..\..\Lua\5.3\" "!ROOT!\..\Lua\5.3\" 
SET POSSIBLE_PATHS=!POSSIBLE_PATHS!"!ProgramFiles!\Lua\5.3\" "!ProgramFiles(x86)!\Lua\5.3\" 
IF NOT "!PREFER_LVERSION!"=="" GOTO lpathdone

:l52
SET POSSIBLE_LIBS=!POSSIBLE_LIBS!"lua52." "lua5.2." 
SET POSSIBLE_PATHS=!POSSIBLE_PATHS!"!ROOT!\..\..\Lua\5.2\" "!ROOT!\..\Lua\5.2\" 
SET POSSIBLE_PATHS=!POSSIBLE_PATHS!"!ProgramFiles!\Lua\5.2\" "!ProgramFiles(x86)!\Lua\5.2\" 
IF NOT "!PREFER_LVERSION!"=="" GOTO lpathdone

:l51
SET POSSIBLE_LIBS=!POSSIBLE_LIBS!"lua51." "lua5.1." 
SET POSSIBLE_PATHS=!POSSIBLE_PATHS!"!ROOT!\..\..\Lua\5.1\" "!ROOT!\..\Lua\5.1\" 
SET POSSIBLE_PATHS=!POSSIBLE_PATHS!"!ProgramFiles!\Lua\5.1\" "!ProgramFiles(x86)!\Lua\5.1\" 
IF NOT "!PREFER_LVERSION!"=="" GOTO lpathdone

:lgen
SET POSSIBLE_PATHS=!POSSIBLE_PATHS!"!ProgramFiles!\Lua\" "!ProgramFiles(x86)!\Lua\" 
SET POSSIBLE_PATHS=!POSSIBLE_PATHS!"!ROOT!\..\..\Lua\" "!ROOT!\..\Lua\" 
IF NOT "!PREFER_LVERSION!"=="" GOTO lpathdone

:lpathdone
SET POSSIBLE_LIBPATHS=".\" "lib\" "src\"
SET POSSIBLE_INCPATHS=".\" "include\" "src\"

FOR %%a IN (%POSSIBLE_PATHS%) DO (
	IF EXIST %%a (
		CALL :testpath %%a
		IF "!ERRORLEVEL!"=="0" GOTO done
	)
)
GOTO :fail

:done
IF EXIST "..\cs-survival\src\survitf.h" (
	SET CFLAGS=!CFLAGS! /DCSLUA_USE_SURVIVAL
)
IF "%PLUGIN_INSTALL%"=="1" (
	IF NOT EXIST "%SERVER_OUTROOT%\scripts" (
		MD %SERVER_OUTROOT%\scripts
	)
	IF NOT EXIST "%SERVER_OUTROOT%\scripts\chatexec.lua" (
		IF NOT EXIST "%SERVER_OUTROOT%\scripts\disabled\chatexec.lua" (
			COPY %ROOT%\scripts\chatexec.lua %SERVER_OUTROOT%\scripts\chatexec.lua 2>nul
		)
	)
)
EXIT /b 0

:testpath
SET __SUBINC=
SET __SUBLIB=
SET __SUBLIBPATH=
FOR %%a IN (%POSSIBLE_INCPATHS%) DO (
	IF EXIST %1\%%a\lua.h (
		SET __SUBINC=%%a
		CALL :incfound %1
		IF "!ERRORLEVEL!"=="0" EXIT /b 0
	)
)
EXIT /b 1

:incfound
SET LUAPATH=%1
SET LUAPATH=%LUAPATH:~1,-1%
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
GOTO :trybuild

:libfound
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

:trybuild
IF EXIST "%LUAPATH%\src\lib_jit.c" GOTO buildlj
IF EXIST "%LUAPATH%\src\lapi.c" GOTO buildlv
EXIT /b 1

:fail
ECHO Lua!PREFER_LVERSION! not found
IF NOT "!PREFER_LVERSION!"=="" IF NOT "!PREFER_LVERSION!"=="jit" (
	ECHO So far this script can clone only LuaJIT
	EXIT /b 1
)

ECHO Would you like the script to automatically clone and build LuaJIT?
ECHO Note: The LuaJIT will be cloned from Mike Pall's Git, then compiled.
ECHO Warning: If "..\LuaJIT" exists it will be removed!
SET /P LQUESTION="[Y/n]>"
IF "!LQUESTION!"=="" GOTO downlj
IF "!LQUESTION!"=="y" GOTO downlj
IF "!LQUESTION!"=="Y" GOTO downlj
EXIT /b 1

:downlj
IF "%GITOK%"=="0" (
	ECHO Looks like you don't have Git for Windows
	ECHO You can download it from https://git-scm.com/download/win
) ELSE (
	RMDIR /S /Q "..\LuaJIT">nul
	git clone https://luajit.org/git/luajit.git "..\LuaJIT"
	GOTO detectlua
)
EXIT /b 1

:buildlj
set LJBUILDFLAGS=
IF "!DEBUG!"=="1" (
	set LJBUILDFLAGS=debug
)
PUSHD %LUAPATH%\src\
CALL msvcbuild !LJBUILDFLAGS!
IF NOT "!ERRORLEVEL!"=="0" (
	POPD
	EXIT /b 1
)
POPD
GOTO detectlua

:buildlv
SETLOCAL
SET _LIBNAME=lib
CALL :detectlibname %1
PUSHD %LUAPATH%\src\
CL /c /DLUA_BUILD_AS_DLL *.c
IF NOT "!ERRORLEVEL!"=="0" GOTO buildlv_fail
DEL lua.obj luac.obj>nul
LINK /DLL *.obj /OUT:lua!_LIBNAME!.dll
IF NOT "!ERRORLEVEL!"=="0" GOTO buildlv_fail
POPD
ENDLOCAL
GOTO detectlua

:detectlibname
IF NOT "!PREFER_LVERSION!"=="" (
	SET _LIBNAME=!PREFER_LVERSION!
	EXIT /b 0
)

IF NOT "%LUAPATH:5.1=%"=="%LUAPATH%" (
	SET _LIBNAME=51
) ELSE IF NOT "%LUAPATH:5.2=%"=="%LUAPATH%" (
	SET _LIBNAME=52
) ELSE IF NOT "%LUAPATH:5.3=%"=="%LUAPATH%" (
	SET _LIBNAME=53
) ELSE IF NOT "%LUAPATH:5.4=%"=="%LUAPATH%" (
	SET _LIBNAME=54
)
EXIT /b 0

:buildlv_fail
ENDLOCAL
EXIT /b 1

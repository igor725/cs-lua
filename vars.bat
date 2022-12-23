IF NOT "%PLUGIN_ARGS%"=="" (
	FOR %%a IN (%PLUGIN_ARGS%) DO (
		IF "%%a"=="lua" (
			CALL %~dp0\vars\lua.bat
			EXIT /B !ERRORLEVEL!
		)
		IF "%%a"=="python" (
			CALL %~dp0\vars\python.bat
			EXIT /B !ERRORLEVEL!
		)
		IF "%%a"=="js" (
			CALL %~dp0\vars\duktape.bat
			EXIT /B !ERRORLEVEL!
		)
	)
)

ECHO Target interpreter is not specified, using Lua
CALL %~dp0\vars\lua.bat

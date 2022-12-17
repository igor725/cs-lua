IF NOT "%PLUGIN_ARGS%"=="" (
	FOR %%a IN (%PLUGIN_ARGS%) DO (
		IF "%%a"=="lua" (
			CALL %ROOT%\vars\lua.bat
			EXIT /B !ERRORLEVEL!
		)
		IF "%%a"=="python" (
			CALL %ROOT%\vars\python.bat
			EXIT /B !ERRORLEVEL!
		)
	)
)

ECHO Target language is not specified, using Lua
CALL %ROOT%\vars\lua.bat

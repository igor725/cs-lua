FOR /F "delims=" %%F IN (!ROOT!\out\python.path) DO (
	IF NOT "%%F"=="" SET PYPATH=%%F
)

IF "!PYPATH!"=="" (
	FOR /F "tokens=* USEBACKQ" %%F IN (`python3 -c "import os, sys; print(os.path.dirname(sys.executable))" 2^> nul`) DO (
		SET PYPATH=%%F
	)

	IF "!PYPATH!"=="" GOTO enterpy
)

:detectpy
IF NOT EXIST "!PYPATH!\libs\" (
	ECHO ERROR: Python static libraries directory was not found in !PYPATH!
	GOTO enterpy
)

IF NOT EXIST "!PYPATH!\include\Python.h" (
	ECHO ERROR: Python headers directory was not found in !PYPATH!
	GOTO enterpy
)

ECHO !PYPATH!>!ROOT!\out\python.path
GOTO addpy

:enterpy
ECHO Your python installation was not found. Please enter exact python path below.
SET /P PYPATH=">"
GOTO detectpy

:addpy
ECHO Using Python from !PYPATH!
SET LIBS=!__SUBLIB!lib !LIBS!
SET LIB=!PYPATH!\libs;!LIB!
SET INCLUDE=!PYPATH!\include;..;!INCLUDE!
SET CFLAGS=/DCSSCRIPTS_BUILD_PYTHON !CFLAGS!

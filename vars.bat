set LUAPATH=!ROOT!\..\..\LuaJIT\src
IF NOT EXIST !LUAPATH! (
	echo Lua not found
	exit /b 1
)
set LIBS=lua51.lib !LIBS!
set CFLAGS=!CFLAGS! /I!LUAPATH!
set LDFLAGS=!LDFLAGS! /libpath:!LUAPATH!

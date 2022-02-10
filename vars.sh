LUA_LIB=$(pkg-config --libs luajit)
if [ $? -ne 0 ]; then
	LUA_LIB=$(pkg-config --libs lua)
	if [ $? -ne 0 ]; then
		echo "Lua not found"
		exit 1
	else
		CFLAGS="$CFLAGS $(pkg-config --cflags lua)"
	else
else
	CFLAGS="$CFLAGS $(pkg-config --cflags luajit)"
fi

LIBS="$LIBS $LUA_LIB"

LUA_LIB=$(pkg-config --libs luajit)
if [ $? -ne 0 ]; then
	echo "Lua not found"
	exit 1
fi
CFLAGS="$CFALGS $(pkg-config --cflags luajit)"
LIBS="$LIBS $LUA_LIB"

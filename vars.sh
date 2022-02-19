LJNAME="luajit >= 2.0"
LNAME="lua >= 5.1"

if pkg-config --exists $LJNAME; then
	LUA_LIB=$(pkg-config --libs $LJNAME)
	CFLAGS="$CFLAGS $(pkg-config --cflags $LJNAME)"
else
	if pkg-config --exists $LNAME; then
		LUA_LIB=$(pkg-config --libs $LNAME)
		CFLAGS="$CFLAGS $(pkg-config --cflags $LNAME)"
	else
		echo "pkg-config: failed to find luajit >= 2.0 or lua >= 5.1"
		exit 1
	fi
fi

LIBS="$LIBS $LUA_LIB"
CFLAGS="$CFLAGS -I../"

LJNAME="luajit >= 2.0"
LNAME="lua >= 5.1"
PKCFG=$MACH-pkg-config

if $PKCFG --exists $LJNAME; then
	LUA_LIB=$($PKCFG --libs $LJNAME)
	CFLAGS="$CFLAGS $($PKCFG --cflags $LJNAME)"
else
	if $PKCFG --exists $LNAME; then
		LUA_LIB=$($PKCFG --libs $LNAME)
		CFLAGS="$CFLAGS $($PKCFG --cflags $LNAME)"
	else
		echo "pkg-config: failed to find luajit >= 2.0 or lua >= 5.1"
		exit 1
	fi
fi

LIBS="$LIBS $LUA_LIB"
CFLAGS="$CFLAGS -I../"

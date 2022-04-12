LUA_FROM_PKG=0

for a in $PLUGIN_ARGS; do
	if [ "$a" == "pkg" ]; then LUA_FROM_PKG=1; fi
done

if [ $LUA_FROM_PKG -eq 1 ]; then
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
else
	if [ $GITOK -eq 0 ]; then
		echo "Git: No git binary found"
		exit 1
	fi
	if [ ! -d "../luajit"  ]; then
		git clone https://luajit.org/git/luajit.git ../luajit
	fi
	if [ ! -f "../luajit/src/libluajit.a" ]; then
		pushd ../luajit
		make CC="$CC"
		popd
	fi
	LIBS="$LIBS -L../luajit/src/ -lluajit"
	CFLAGS="$CFLAGS -I../luajit/src/"
fi

CFLAGS="$CFLAGS -I../"

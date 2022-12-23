LUA_FROM_PKG=0

for a in $PLUGIN_ARGS; do
	if [ "$a" == "pkg" ]; then LUA_FROM_PKG=1; fi
	if [ "$a" == "mem" ]; then CFLAGS="$CFLAGS -DCSSCRIPTS_PROFILE_MEMORY"; fi
done

if [ $LUA_FROM_PKG -eq 1 ]; then
	LJNAME="luajit >= 2.0"
	LNAME="lua >= 5.1"
	PKCFG=$MACH-pkg-config
	if ! command -v $PKCFG 2>&1 > /dev/null; then
		PKCFG=pkg-config
	fi

	if $PKCFG --exists $LJNAME; then
		LUA_LIB=$($PKCFG --libs $LJNAME)
		CFLAGS="$CFLAGS $($PKCFG --cflags $LJNAME)"
	else
		if $PKCFG --exists $LNAME; then
			LUA_LIB=$($PKCFG --libs $LNAME)
			CFLAGS="$CFLAGS $($PKCFG --cflags $LNAME)"
		else
			echo "pkg-config: failed to find luajit >= 2.0 or lua >= 5.1"
			return 1
		fi
	fi

	LIBS="$LIBS $LUA_LIB"
else
	if [ $GITOK -eq 0 ]; then
		echo "Git: No git binary found"
		return 1
	fi
	if [ ! -d "../luajit"  ]; then
		git clone https://luajit.org/git/luajit.git ../luajit
	fi
	if [ ! -f "../luajit/src/libluajit.a" ]; then
		pushd ../luajit
		if [ -n "$HOSTCC" ]; then
			make HOST_CC="$HOSTCC" CROSS="$MACH-"
		else
			make CC="$CC"
		fi
		if [ $? -ne 0 ]; then
			popd
			echo "Failed to build LuaJIT"
			return 1
		fi
		popd
	fi
	LIBS="$LIBS -L../luajit/src/ -lluajit"
	CFLAGS="$CFLAGS -I../luajit/src/"
fi

CFLAGS="$CFLAGS -DCSSCRIPTS_BUILD_LUA -I../"

if [ -f "../cs-survival/src/survitf.h" ]; then
	CFLAGS="$CFLAGS -DCSSCRIPTS_USE_SURVIVAL"
fi

if [ $PLUGIN_INSTALL -eq 1 ]; then
	if [ ! -d "$SERVER_OUTROOT/scripts" ]; then
		mkdir "$SERVER_OUTROOT/scripts"
	fi
	if [ ! -f "$SERVER_OUTROOT/scripts/chatexec.lua" ] &&
	   [ ! -f "$SERVER_OUTROOT/scripts/.chatexec.lua" ]; then
		cp $ROOT/scripts/chatexec.lua $SERVER_OUTROOT/scripts/chatexec.lua 2> /dev/null
	fi
fi

return 0

STATIC_LUA=0
NOPKG_LUA=0
_STATIC_ADD_FPIC=""

for a in $PLUGIN_ARGS; do
	if [ "$a" == "mem" ]; then CFLAGS="$CFLAGS -DCSSCRIPTS_PROFILE_MEMORY"; fi
	if [ "$a" == "static" ]; then STATIC_LUA=1; _STATIC_ADD_FPIC=" -fpic"; fi
	if [ "$a" == "nopkg" ]; then NOPKG_LUA=1; fi
done

LJNAME="luajit >= 2.0"
LNAME="lua >= 5.1"
PKCFG=$MACH-pkg-config
if ! command -v $PKCFG 2>&1 > /dev/null; then
	PKCFG=pkg-config
fi

PKG_LUA_OK=0

if [ $NOPKG_LUA -ne 1 ]; then
	if [ $STATIC_LUA -ne 1 ]; then
		if $PKCFG --exists $LJNAME 2>&1 > /dev/null; then
			LIBS="$LIBS $($PKCFG --libs $LJNAME)"
			CFLAGS="$CFLAGS $($PKCFG --cflags $LJNAME)"
			PKG_LUA_OK=1
		else
			if $PKCFG --exists $LNAME 2>&1 > /dev/null; then
				LIBS="$LIBS $($PKCFG --libs $LNAME)"
				CFLAGS="$CFLAGS $($PKCFG --cflags $LNAME)"
				PKG_LUA_OK=1
			fi
		fi

		if [ $PKG_LUA_OK -ne 1 ]; then
			echo "pkg-config failed to find $LJNAME or $LNAME, trying to build own binary from LuaJIT sources..."
		fi
	else
		echo "Static linking is not possible with Lua from pkg yet"
	fi
fi

if [ $PKG_LUA_OK -ne 1 ]; then
	if [ $GITOK -eq 0 ]; then
		echo "Git: No git binary found"
		return 1
	fi
	if [ ! -d "../luajit"  ]; then
		if ! git clone https://luajit.org/git/luajit.git ../luajit; then
			echo "Git: Failed to clone LuaJIT"
			return 1
		fi
	fi
	if [ ! -f "../luajit/src/libluajit.a" ]; then
		pushd ../luajit
		if [ -n "$HOSTCC" ]; then
			make HOST_CC="$HOSTCC$_STATIC_ADD_FPIC" CROSS="$MACH-"
		else
			make CC="$CC$_STATIC_ADD_FPIC"
		fi
		if [ $? -ne 0 ]; then
			popd
			echo "Failed to build LuaJIT"
			return 1
		fi
		popd
		if [ $STATIC_LUA -ne 1 ]; then
			echo "Please note that the cs-lua plugin may not load properly until you install LuaJIT system-wide!"
			echo "If you don't want to install LuaJIT system-wide, you can specify \"static\" plugin argument to the build script"
		fi
	fi

	if [ $STATIC_LUA -eq 1 ]; then
		LIBS="$LIBS ../luajit/src/libluajit.a -lm"
		CFLAGS="$CFLAGS -DCSSCRIPTS_STATIC_LIBLINK"
	else
		LIBS="$LIBS -L../luajit/src/ -lluajit"
	fi
	CFLAGS="$CFLAGS -I../luajit/src/"
fi

CFLAGS="$CFLAGS -I../"

if [ -f "../cs-survival/src/survitf.h" ]; then
	echo "Survival interface connected"
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

for a in $PLUGIN_ARGS; do
	if [ "$a" == "mem" ]; then CFLAGS="$CFLAGS -DCSSCRIPTS_PROFILE_MEMORY"; fi
done

LJNAME="luajit >= 2.0"
LNAME="lua >= 5.1"
PKCFG=$MACH-pkg-config
if ! command -v $PKCFG 2>&1 > /dev/null; then
	PKCFG=pkg-config
fi

if $PKCFG --exists $LJNAME 2>&1 > /dev/null; then
	LIBS="$LIBS $($PKCFG --libs $LJNAME)"
	CFLAGS="$CFLAGS $($PKCFG --cflags $LJNAME)"
else
	if $PKCFG --exists $LNAME 2>&1 > /dev/null; then
		LIBS="$LIBS $($PKCFG --libs $LNAME)"
		CFLAGS="$CFLAGS $($PKCFG --cflags $LNAME)"
	else
		echo "pkg-config: failed to find $LJNAME or $LNAME, trying to build own binary from sources.."
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
			echo "Please note that the cs-lua plugin may not load properly until you install LuaJIT system-wide!"
		fi

		LIBS="$LIBS -L../luajit/src/ -lluajit"
		CFLAGS="$CFLAGS -I../luajit/src/"
	fi
fi

CFLAGS="$CFLAGS -DCSSCRIPTS_BUILD_LUA -I../"

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

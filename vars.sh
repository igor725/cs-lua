for a in $PLUGIN_ARGS; do
	if [ "$a" == "lua" ]; then
		. "$ROOT/vars/lua.sh"
		exit $?
	fi
	if [ "$a" == "python" ]; then
		. "$ROOT/vars/python.sh"
		exit $?
	fi
done

echo "Target language is not specified, using Lua"
. "$ROOT/vars/lua.sh"
exit $?

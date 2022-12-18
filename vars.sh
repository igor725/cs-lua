CURRDIR="$ROOT"

if test -n "$BASH"; then
	CURRDIR=$(dirname ${BASH_SOURCE[0]})
elif test -n "$ZSH_NAME"; then
	CURRDIR=$(dirname ${(%):-%x})
fi

for a in $PLUGIN_ARGS; do
	if [ "$a" == "lua" ]; then
		. "$CURRDIR/vars/lua.sh"
		return $?
	fi
	if [ "$a" == "python" ]; then
		. "$CURRDIR/vars/python.sh"
		return $?
	fi
done

echo "Target interpreter is not specified, using Lua"
. "$CURRDIR/vars/lua.sh"
return $?

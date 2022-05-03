#include <core.h>
#include "luascript.h"

static cs_str const keys[256] = {
	0, "ESCAPE", "1", "2", "3", "4", "5", "6", "7", "8", "9", "0", "MINUS", "EQUALS",
	"BACKSPACE", "TAB", "Q", "W", "E", "R", "T", "Y", "U", "I", "O", "P", "LBRACKET",
	"RBRACKET", "ENTER", "LCTRL", "A", "S", "D", "F", "G", "H", "J", "K", "L",
	"SEMICOLON", "QUOTE", "TILDE", "LSHIFT", "BACKSLASH", "Z", "X", "C", "V", "B", "N",
	"M", "COMMA", "PERIOD", "SLASH", "RSHIFT", 0, "LALT", "SPACE", "CAPSLOCK", "F1",
	"F2", "F3", "F4", "F5", "F6", "F7", "F8", "F9", "F10", "NUMLOCK", "SCROLLLOCK",
	"KP7", "KP8", "KP9", "KP_MINUS", "KP4", "KP5", "KP6", "KP_PLUS", "KP1", "KP2",
	"KP3", "KP0", "KP_DECIMAL", 0, 0, 0, "F11", "F12", 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	"F13", "F14", "F15", "F16", "F17", "F18", 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, "KP_EQU", 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, "KP_ENTER", "RCTRL", 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, "KP_DIVIDE", 0, 0, "RALT", 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, "PAUSE", 0, "HOME", "UP", "PAGEUP", 0, "LEFT", 0, "RIGHT", 0,
	"END", "DOWN", "PAGEDOWN", "INSERT", "DELETE", 0, 0, 0, 0, 0, 0, 0, "LWIN", "RWIN",
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0
};

static cs_str const mods[5] = {
	"MNONE", "MCTRL",
	"MSHIFT", 0, "MALT"
};

int luaopen_key(lua_State *L) {
	lua_createtable(L, 0, 256 + 5);
	for(int i = 0; i < 256; i++) {
		if(keys[i]) {
			lua_pushinteger(L, i);
			lua_setfield(L, -2, keys[i]);
		}
		if(i < 5 && mods[i]) {
			lua_pushinteger(L, i);
			lua_setfield(L, -2, mods[i]);
		}
	}
	return 1;
}

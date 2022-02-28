#include <core.h>
#include <client.h>
#include "luascript.h"
#include "luaworld.h"

static Client *lua_checkbot(lua_State *L, int idx) {
	void **ud = luaL_checkudata(L, idx, "Bot");
	luaL_argcheck(L, *ud != NULL, idx, "Invalid bot");
	return (Client *)*ud;
}

static int meta_spawn(lua_State *L) {
	Client *bot = lua_checkbot(L, 1);
	(void)bot;
	// Спавн бота
	lua_pushboolean(L, 0);
	return 1;
}

static int meta_destroy(lua_State *L) {
	Client *bot = lua_checkbot(L, 1);
	(void)bot;
	// Очистка данных бота
	return 0;
}

static luaL_Reg botmeta[] = {
	// {"setposition", meta_setposition},
	{"spawn", meta_spawn},
	{"__gc", meta_destroy},
	{NULL, NULL}
};

static int bot_new(lua_State *L) {
	// TODO: Создание бота
	void **ud = lua_newuserdata(L, sizeof(Client *));
	luaL_setmetatable(L, "Bot");
	*ud = NULL;
	return 1;
}

static luaL_Reg botlib[] = {
	{"new", bot_new},
	{NULL, NULL}
};

int luaopen_bot(lua_State *L) {
	luaL_newmetatable(L, "Bot");
	lua_pushvalue(L, -1);
	lua_setfield(L, -2, "__index");
	luaL_setfuncs(L, botmeta, 0);
	lua_pop(L, 1);

	luaL_register(L, luaL_checkstring(L, 1), botlib);
	return 1;
}

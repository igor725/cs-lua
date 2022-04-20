#include <core.h>
#include <list.h>
#include <str.h>
#include "luascript.h"

static KListField *headContact = NULL;

static void pushcontact(lua_State *L, void *ptr) {
	lua_getfield(L, LUA_REGISTRYINDEX, "cscontact");
	lua_getfield(L, -1, "list");
	lua_pushvalue(L, 1); // Название контакта
	lua_gettable(L, -2);
	if(lua_isnil(L, -1)) {
		lua_pop(L, 1);
		
	}

	return 1;
}

static int cont_create(lua_State *L) {
	cs_str name = luaL_checkstring(L, 1);
	KListField *tmp;
	List_Iter(tmp, headContact) {
		if(String_Compare(KList_GetKey(tmp).str, name)) {
			pushcontact(L, KList_GetValue(tmp).ptr);
		}
		
	}
}

static int cont_remove(lua_State *L) {

}

const luaL_Reg contactlib[] = {
	{"create", cont_create},
	{"remove", cont_remove},
	{NULL, NULL}
};

int luaopen_contact(lua_State *L, int idx) {
	lua_newtable(L);
	lua_newtable(L); // очередь контактов
	lua_newtable(L); // список контактов
	lua_newtable(L); // метатаблица для списка контактов
	lua_pushstring(L, "v");
	lua_setfield(L, -2, "__mode");
	lua_setmetatable(L, -2);
	lua_setfield(L, -3, "list");
	lua_setfield(L, -2, "queue");
	lua_setfield(L, LUA_REGISTRYINDEX, "cscontact");

	luaL_register(L, luaL_checkstring(L, 1), contactlib);
	return 1;
}

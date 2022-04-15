#include <core.h>
#include <list.h>
#include <client.h>
#include <log.h>
#include "luascript.h"
#include "luaclient.h"
#include "luacuboid.h"
#include "luacolor.h"
#include "luavector.h"

typedef struct _LuaCuboid {
	cs_bool released;
	Client *client;
	CPECuboid *cub;
} LuaCuboid;

static void setcube(lua_State *L, LuaCuboid *luacub) {
	lua_pushinteger(L, Cuboid_GetID(luacub->cub));
	lua_pushvalue(L, -5);
	lua_settable(L, -3);
}

void lua_newcubref(lua_State *L, Client *client, CPECuboid *cub) {
	if(cub == NULL) {
		lua_pushnil(L);
		return;
	}

	LuaCuboid *luacub = lua_newuserdata(L, sizeof(LuaCuboid));
	luaL_setmetatable(L, CSLUA_MCUBOID);
	luacub->released = false;
	luacub->client = client;
	luacub->cub = cub;

	lua_getfield(L, LUA_REGISTRYINDEX, "cscuboids");
	lua_pushclient(L, client);
	lua_pushvalue(L, -1); // Отправляем клиента дважды в стек, может пригодиться
	lua_gettable(L, -3);
	if(lua_isnil(L, -1)) {
		lua_pop(L, 1);
		lua_newtable(L); // Таблица кубов
		lua_newtable(L); // Метатаблица для таблицы кубов (TODO: Сделать её общей для всех??)
		// Делаем, чтобы сборщик мусора не учитывал ссылки внутри таблицы
		lua_pushstring(L, "v");
		lua_setfield(L, -2, "__mode");
		lua_setmetatable(L, -2);
		// Заносим куб в новоиспечённую таблицу
		setcube(L, luacub);
		lua_settable(L, -3); // Сохраняем таблицу кубов клиента в cscuboids
		lua_pop(L, 1);
		return;
	}

	setcube(L, luacub);
	lua_pop(L, 3); // Убираем из стека лишнего клиента и таблицы
}

void lua_clearcuboids(lua_State *L, Client *client) {
	lua_pushclient(L, client); // Не хочу два раза вызвывать эту довольно жирную функцию
	lua_getfield(L, LUA_REGISTRYINDEX, "cscuboids");
	lua_pushvalue(L, -2);
	lua_gettable(L, -2); // Получаем таблицу кубоидов клиента

	if(lua_isnil(L, -1)) { // Если таблица не существует, прекращаем выполнение
		lua_pop(L, 2);
		return;
	}

	// Проходимся по таблице и удаляем найденные кубоиды
	for(lua_Integer i = 0; i < 16; i++) { // TODO: Сделать число 16 макросом???
		lua_pushinteger(L, i);
		lua_gettable(L, -2);
		if(lua_isnil(L, -1)) { // Кубоид с указанным id не существует в таблице кубоидов
			lua_pop(L, 1);
			continue;
		}
		lua_getfield(L, -1, "remove"); // Вытаскиваем у кубоида функцию remove
		if(lua_isfunction(L, -1)) { // Проверяем, функция ли она вообще
			lua_pushvalue(L, -2); // Отправляем сам кубоид в качестве аргумента функции
			lua_call(L, 1, 0);
		} else luaL_error(L, "How it possible? :/"); // Сюда по идее мы не должны дойти ни при каких условиях
		lua_pop(L, 1);
	}

	lua_pushvalue(L, -3); // Пушим клиента
	lua_pushnil(L);
	lua_settable(L, -4); // Убираем таблицу кубоидов клиента из cscuboids
	lua_pop(L, 3); // Выкидываем из стека клиента и две таблицы
}

static LuaCuboid *lua_checkcuboid(lua_State *L, int idx) {
	LuaCuboid *luacub = luaL_checkudata(L, idx, CSLUA_MCUBOID);
	luaL_argcheck(L, !luacub->released, 1, "Cuboid removed");
	return luacub;
}

static LuaCuboid *lua_tocuboid(lua_State *L, int idx) {
	LuaCuboid *luacub = luaL_testudata(L, idx, CSLUA_MCUBOID);
	if(luacub && luacub->released) luacub = NULL;
	return luacub;
}

static int meta_setpoints(lua_State *L) {
	LuaCuboid *luacub = lua_checkcuboid(L, 1);
	Cuboid_SetPositions(
		luacub->cub,
		*lua_checkshortvector(L, 2),
		*lua_checkshortvector(L, 3)
	);
	return 0;
}

static int meta_setcolor(lua_State *L) {
	LuaCuboid *luacub = lua_checkcuboid(L, 1);
	Cuboid_SetColor(luacub->cub, *lua_checkcolor4(L, 2));
	return 0;
}

static int meta_update(lua_State *L) {
	LuaCuboid *luacub = lua_checkcuboid(L, 1);
	Client_UpdateSelection(luacub->client, luacub->cub);
	return 0;
}

static int meta_remove(lua_State *L) {
	LuaCuboid *luacub = lua_tocuboid(L, 1);
	if(luacub) {
		luacub->released = true;
		Client_RemoveSelection(luacub->client, luacub->cub);
		lua_getfield(L, LUA_REGISTRYINDEX, "cscuboids");
		lua_pushclient(L, luacub->client);
		lua_gettable(L, -2);
		lua_pushinteger(L, Cuboid_GetID(luacub->cub));
		lua_pushnil(L);
		lua_settable(L, -3);
		lua_pop(L, 1);
	}

	return 0;
}

static const luaL_Reg cuboidmeta[] = {
	{"setpoints", meta_setpoints},
	{"setcolor", meta_setcolor},

	{"update", meta_update},
	{"remove", meta_remove},

	{"__gc", meta_remove},

	{NULL, NULL}
};

void luainit_cuboid(lua_State *L) {
	lua_newtable(L);
	lua_setfield(L, LUA_REGISTRYINDEX, "cscuboids");

	luaL_newmetatable(L, CSLUA_MCUBOID);
	lua_pushvalue(L, -1);
	lua_setfield(L, -2, "__index");
	luaL_setfuncs(L, cuboidmeta, 0);
	lua_pop(L, 1);
}